// version.dll proxy and loader for the Sims accessibility mod (x86).
//
// Sims.exe imports version.dll and it is not a KnownDLL, so a version.dll next to the exe is
// loaded instead of the system one. Every export forwards to the real system DLL. The loader:
//   1. starts a thread from DllMain (nothing else happens under the loader lock);
//   2. checks the Sims.exe build (SHA-256) and waits for the game window (class "Gonzo"),
//      which also means SteamStub has decrypted the code;
//   3. subclasses the window and, on the main thread, loads SimsAccess\SimsAccess.dll (a copy,
//      so the file can be rebuilt while the game runs) and starts a 30 ms tick timer;
//   4. forwards window messages and ticks to the core; any crash inside the core is caught,
//      logged and the core is disabled instead of taking the game down.
// Dev hot reload: when SimsAccess\dev.txt exists, a changed SimsAccess.dll is reloaded
// automatically (checked once a second), and Ctrl+Alt+F12 reloads on demand.

#include <windows.h>
#include <bcrypt.h>

#include <cstdio>
#include <share.h>
#include <cstring>
#include <string>

#include "loader_api.h"

#pragma comment(lib, "bcrypt.lib")

// ---------------------------------------------------------------------------------------------
// Export forwarding

namespace {
const char* const kExportNames[] = {
    "GetFileVersionInfoA",     "GetFileVersionInfoByHandle", "GetFileVersionInfoExA",
    "GetFileVersionInfoExW",   "GetFileVersionInfoSizeA",    "GetFileVersionInfoSizeExA",
    "GetFileVersionInfoSizeExW", "GetFileVersionInfoSizeW",  "GetFileVersionInfoW",
    "VerFindFileA",            "VerFindFileW",               "VerInstallFileA",
    "VerInstallFileW",         "VerQueryValueA",             "VerQueryValueW",
};
constexpr int kExportCount = sizeof(kExportNames) / sizeof(kExportNames[0]);
}  // namespace

extern "C" FARPROC g_real[kExportCount] = {};

extern "C" void __stdcall ResolveRealExports() {
    static HMODULE real = nullptr;
    if (real) return;
    wchar_t path[MAX_PATH];
    UINT n = GetSystemDirectoryW(path, MAX_PATH);  // SysWOW64 for this 32-bit process
    wcscpy_s(path + n, MAX_PATH - n, L"\\version.dll");
    real = LoadLibraryW(path);
    for (int i = 0; i < kExportCount; i++) g_real[i] = real ? GetProcAddress(real, kExportNames[i]) : nullptr;
}

#define PROXY(index, name)                                    \
    extern "C" __declspec(naked) void Proxy_##name() {        \
        __asm { mov eax, dword ptr [g_real + index * 4] }     \
        __asm { test eax, eax }                               \
        __asm { jnz go }                                      \
        __asm { call ResolveRealExports }                     \
        __asm { mov eax, dword ptr [g_real + index * 4] }     \
        __asm { go: jmp eax }                                 \
    }

PROXY(0, GetFileVersionInfoA)
PROXY(1, GetFileVersionInfoByHandle)
PROXY(2, GetFileVersionInfoExA)
PROXY(3, GetFileVersionInfoExW)
PROXY(4, GetFileVersionInfoSizeA)
PROXY(5, GetFileVersionInfoSizeExA)
PROXY(6, GetFileVersionInfoSizeExW)
PROXY(7, GetFileVersionInfoSizeW)
PROXY(8, GetFileVersionInfoW)
PROXY(9, VerFindFileA)
PROXY(10, VerFindFileW)
PROXY(11, VerInstallFileA)
PROXY(12, VerInstallFileW)
PROXY(13, VerQueryValueA)
PROXY(14, VerQueryValueW)

// ---------------------------------------------------------------------------------------------
// Loader

namespace {

// Supported build: Sims.exe 1.10.7 (The Sims Legacy Collection, Steam), see CLAUDE.md.
const char kSupportedHash[] = "b7c8765ed2383cbda8c047694d54a21d97895c8dae57dcc87716ca6e78b342e3";
const UINT kInitMessage = WM_APP + 0x5A1;
const UINT_PTR kTimerId = 0x5A11;

HMODULE g_self = nullptr;
std::wstring g_modDir;      // ...\SimsAccess
CRITICAL_SECTION g_logLock;
FILE* g_log = nullptr;
char g_hash[65] = {};
bool g_buildOk = false;

HWND g_hwnd = nullptr;
WNDPROC g_gameProc = nullptr;

HMODULE g_core = nullptr;
std::wstring g_coreCopy;
FILETIME g_coreStamp = {};
SaInitFn g_init = nullptr;
SaShutdownFn g_shutdown = nullptr;
SaOnMessageFn g_onMessage = nullptr;
SaOnTickFn g_onTick = nullptr;
typedef int(__cdecl* SaBusyFn)(void);
SaBusyFn g_busy = nullptr;  // optional export: the core's hooks are on the stack
int g_inCore = 0;           // calls into the core under way (a modal loop inside one re-enters)
SaLoaderApi g_api = {};
bool g_coreBroken = false;
int g_loadCount = 0;
ULONGLONG g_lastStampCheck = 0;

void LogLine(const char* line) {
    EnterCriticalSection(&g_logLock);
    if (g_log) {
        SYSTEMTIME t;
        GetLocalTime(&t);
        fprintf(g_log, "%02d:%02d:%02d.%03d %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, line);
        fflush(g_log);
    }
    LeaveCriticalSection(&g_logLock);
}

void Logf(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    LogLine(buf);
}

bool HashFile(const wchar_t* path, char out[65]) {
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE h = nullptr;
    bool ok = BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) == 0 &&
              BCryptCreateHash(alg, &h, nullptr, 0, nullptr, 0, 0) == 0;
    static unsigned char buf[1 << 16];
    DWORD got = 0;
    while (ok && ReadFile(f, buf, sizeof buf, &got, nullptr) && got) ok = BCryptHashData(h, buf, got, 0) == 0;
    unsigned char digest[32];
    if (ok) ok = BCryptFinishHash(h, digest, 32, 0) == 0;
    if (h) BCryptDestroyHash(h);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    CloseHandle(f);
    if (ok) for (int i = 0; i < 32; i++) sprintf_s(out + i * 2, 3, "%02x", digest[i]);
    return ok;
}

bool FileStamp(const std::wstring& path, FILETIME* out) {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &d)) return false;
    *out = d.ftLastWriteTime;
    return true;
}

bool DevMode() { return GetFileAttributesW((g_modDir + L"\\dev.txt").c_str()) != INVALID_FILE_ATTRIBUTES; }

// SEH wrappers: a crash inside the core disables it rather than crashing the game.
int SafeInit() {
    __try { return g_init(&g_api); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return -(int)GetExceptionCode(); }
}
bool SafeShutdown() {
    __try { g_shutdown(); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
int SafeOnMessageInner(HWND h, UINT m, WPARAM w, LPARAM l, LRESULT* r, DWORD* code) {
    __try { return g_onMessage(h, m, w, l, r); }
    __except (*code = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) { return -1; }
}
int SafeOnMessage(HWND h, UINT m, WPARAM w, LPARAM l, LRESULT* r, DWORD* code) {
    g_inCore++;
    int result = SafeOnMessageInner(h, m, w, l, r, code);
    g_inCore--;
    return result;
}
bool SafeTickInner(DWORD* code) {
    __try { g_onTick(); return true; }
    __except (*code = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool SafeTick(DWORD* code) {
    g_inCore++;
    bool ok = SafeTickInner(code);
    g_inCore--;
    return ok;
}

// A reload must not free code that is still on the stack: a key handler that runs a modal loop
// (the cheat interpreter's message box) or a game dialog opened inside one of the core's hooks
// (SimAntics dialogs run inside the primitive hook) pumps messages, and so timer ticks, while
// the core's frames wait below. Returning into a freed (or reloaded) image crashed the game.
bool CoreBusy() {
    if (g_inCore > 0) return true;
    if (!g_busy) return false;
    __try { return g_busy() != 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void UnloadCore() {
    if (!g_core) return;
    if (g_shutdown && !g_coreBroken && !SafeShutdown()) LogLine("core shutdown crashed");
    FreeLibrary(g_core);
    g_core = nullptr;
    g_init = nullptr; g_shutdown = nullptr; g_onMessage = nullptr; g_onTick = nullptr; g_busy = nullptr;
    DeleteFileW(g_coreCopy.c_str());
}

void LoadCore() {
    std::wstring src = g_modDir + L"\\SimsAccess.dll";
    FileStamp(src, &g_coreStamp);
    CreateDirectoryW((g_modDir + L"\\run").c_str(), nullptr);
    g_coreCopy = g_modDir + L"\\run\\SimsAccess_" + std::to_wstring(GetCurrentProcessId()) + L"_" +
                 std::to_wstring(++g_loadCount) + L".dll";
    if (!CopyFileW(src.c_str(), g_coreCopy.c_str(), FALSE)) {
        Logf("cannot copy %ls (error %lu)", src.c_str(), GetLastError());
        return;
    }
    g_core = LoadLibraryW(g_coreCopy.c_str());
    if (!g_core) { Logf("cannot load core (error %lu)", GetLastError()); return; }
    g_init = (SaInitFn)GetProcAddress(g_core, "SA_Init");
    g_shutdown = (SaShutdownFn)GetProcAddress(g_core, "SA_Shutdown");
    g_onMessage = (SaOnMessageFn)GetProcAddress(g_core, "SA_OnMessage");
    g_onTick = (SaOnTickFn)GetProcAddress(g_core, "SA_OnTick");
    g_busy = (SaBusyFn)GetProcAddress(g_core, "SA_Busy");
    if (!g_init || !g_shutdown || !g_onMessage || !g_onTick) {
        LogLine("core is missing exports");
        FreeLibrary(g_core);
        g_core = nullptr;
        return;
    }
    g_api.reloaded = g_loadCount > 1;
    g_coreBroken = false;
    int rc = SafeInit();
    Logf("core loaded (load %d), SA_Init -> %d", g_loadCount, rc);
    if (rc != 1) g_coreBroken = true;
}

void ReloadCore() {
    LogLine("reloading core");
    UnloadCore();
    LoadCore();
}

LRESULT CALLBACK SubclassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == kInitMessage) {
        LoadCore();
        SetTimer(hwnd, kTimerId, 30, nullptr);
        return 0;
    }
    if (msg == WM_TIMER && wp == kTimerId) {
        ULONGLONG now = GetTickCount64();
        if (now - g_lastStampCheck > 1000) {
            g_lastStampCheck = now;
            FILETIME st;
            if (DevMode() && FileStamp(g_modDir + L"\\SimsAccess.dll", &st) &&
                CompareFileTime(&st, &g_coreStamp) != 0) {
                if (CoreBusy()) {
                    LogLine("core changed; the reload waits until the core is off the stack");
                } else {
                    Sleep(300);  // let the linker finish writing
                    ReloadCore();
                }
            }
        }
        if (g_core && !g_coreBroken) {
            DWORD code = 0;
            if (!SafeTick(&code)) {
                Logf("core crashed in tick (exception 0x%08lx); core disabled", code);
                g_coreBroken = true;
            }
        }
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F12 && (GetKeyState(VK_CONTROL) & 0x8000) &&
        (GetKeyState(VK_MENU) & 0x8000)) {
        ReloadCore();
        return 0;
    }
    if (msg == WM_SYSKEYDOWN && wp == VK_F12 && (GetKeyState(VK_CONTROL) & 0x8000)) {
        ReloadCore();
        return 0;
    }
    if (g_core && !g_coreBroken) {
        LRESULT result = 0;
        DWORD code = 0;
        int handled = SafeOnMessage(hwnd, msg, wp, lp, &result, &code);
        if (handled < 0) {
            Logf("core crashed in message 0x%x (exception 0x%08lx); core disabled", msg, code);
            g_coreBroken = true;
        } else if (handled) {
            return result;
        }
    }
    if (msg == WM_NCDESTROY) {
        KillTimer(hwnd, kTimerId);
        UnloadCore();
        SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)g_gameProc);
    }
    return CallWindowProcA(g_gameProc, hwnd, msg, wp, lp);
}

HWND FindGameWindow() {
    struct Ctx { DWORD pid; HWND found; } ctx = {GetCurrentProcessId(), nullptr};
    EnumWindows([](HWND h, LPARAM p) -> BOOL {
        auto* c = (Ctx*)p;
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        char cls[32];
        if (pid == c->pid && GetClassNameA(h, cls, sizeof cls) && !strcmp(cls, "Gonzo")) {
            c->found = h;
            return FALSE;
        }
        return TRUE;
    }, (LPARAM)&ctx);
    return ctx.found;
}

DWORD WINAPI InitThread(void*) {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    if (HashFile(exe, g_hash)) {
        g_buildOk = strcmp(g_hash, kSupportedHash) == 0;
        Logf("Sims.exe sha256 %s (%s)", g_hash, g_buildOk ? "supported" : "UNSUPPORTED build");
    } else {
        LogLine("cannot hash Sims.exe");
    }
    for (int i = 0; i < 20 * 60 * 5 && !g_hwnd; i++) {  // up to 5 minutes
        g_hwnd = FindGameWindow();
        if (!g_hwnd) Sleep(50);
    }
    if (!g_hwnd) { LogLine("game window not found; mod inactive"); return 0; }
    Sleep(500);  // let the game finish creating the window
    g_api.version = SA_LOADER_API_VERSION;
    g_api.hwnd = g_hwnd;
    g_api.game = GetModuleHandleW(nullptr);
    g_api.mod_dir = g_modDir.c_str();
    g_api.build_ok = g_buildOk ? 1 : 0;
    g_api.exe_hash = g_hash;
    g_api.log = LogLine;
    // The game window is ANSI: subclass with the A functions so messages stay ANSI.
    g_gameProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)SubclassProc);
    Logf("subclassed window %p (game proc %p)", g_hwnd, g_gameProc);
    PostMessageA(g_hwnd, kInitMessage, 0, 0);
    return 0;
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = inst;
        DisableThreadLibraryCalls(inst);
        InitializeCriticalSection(&g_logLock);
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(inst, path, MAX_PATH);
        std::wstring dir(path);
        dir = dir.substr(0, dir.find_last_of(L"\\/"));
        g_modDir = dir + L"\\SimsAccess";
        CreateDirectoryW(g_modDir.c_str(), nullptr);
        g_log = _wfsopen((g_modDir + L"\\SimsAccess.log").c_str(), L"w", _SH_DENYWR);  // readable while running
        LogLine("SimsAccess loader starting");
        HANDLE t = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    return TRUE;
}
