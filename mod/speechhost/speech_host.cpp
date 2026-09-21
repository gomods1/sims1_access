// SpeechHost: the x64 process that owns Prism for the 32-bit Sims accessibility mod.
// Sims.exe is x86 and Prism ships x64 only, so speech runs out of process (template 8.3).
//
// Usage: SpeechHost.exe --pid <game pid> [--pipe <full pipe name>] [--mute]
// --mute logs lines without speaking them (for development runs while the user is busy).
// Listens on \\.\pipe\SimsAccessSpeech-<pid> and reads newline-delimited UTF-8 commands:
//   say <0|1> <text>   speak (1 = interrupt; the mod only uses 0)
//   braille <text>     braille only
//   stop               silence speech
//   quit               exit
// Exits on quit or when the game process exits; clients may disconnect and reconnect.
// Logs every spoken line to SpeechHost.log next to the exe (overwritten each launch).

#include <windows.h>
#include <shellapi.h>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <share.h>
#include <string>
#include <type_traits>

#pragma warning(push)
#pragma warning(disable : 4309)  // constant truncation inside prism.h
#include "prism.h"
#pragma warning(pop)

namespace {

FILE* g_log = nullptr;

void Log(const char* fmt, ...) {
    if (!g_log) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(g_log, "%02d:%02d:%02d.%03d ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fputc('\n', g_log);
    fflush(g_log);
}

std::wstring ExeDir() {
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring s(path, n);
    size_t slash = s.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : s.substr(0, slash);
}

// Prism, resolved at runtime so a missing or wrong-bitness DLL produces a log line.
struct Prism {
    HMODULE dll = nullptr;
    decltype(&prism_init) init = nullptr;
    decltype(&prism_shutdown) shutdown = nullptr;
    decltype(&prism_registry_create_best) create_best = nullptr;
    decltype(&prism_backend_free) backend_free = nullptr;
    decltype(&prism_backend_name) backend_name = nullptr;
    decltype(&prism_backend_get_features) get_features = nullptr;
    decltype(&prism_backend_speak) speak = nullptr;
    decltype(&prism_backend_output) output = nullptr;
    decltype(&prism_backend_braille) braille = nullptr;
    decltype(&prism_backend_stop) stop = nullptr;
    decltype(&prism_error_string) error_string = nullptr;

    PrismContext* ctx = nullptr;
    PrismBackend* backend = nullptr;
    uint64_t features = 0;
    ULONGLONG last_recreate = 0;

    bool Load(const std::wstring& path) {
        dll = LoadLibraryW(path.c_str());
        if (!dll) {
            Log("cannot load %ls (error %lu; missing file or not x64)", path.c_str(), GetLastError());
            return false;
        }
        bool ok = true;
        auto get = [&](auto& fn, const char* name) {
            fn = reinterpret_cast<std::remove_reference_t<decltype(fn)>>(GetProcAddress(dll, name));
            if (!fn) { Log("prism.dll has no export %s", name); ok = false; }
        };
        get(init, "prism_init");
        get(shutdown, "prism_shutdown");
        get(create_best, "prism_registry_create_best");
        get(backend_free, "prism_backend_free");
        get(backend_name, "prism_backend_name");
        get(get_features, "prism_backend_get_features");
        get(speak, "prism_backend_speak");
        get(output, "prism_backend_output");
        get(braille, "prism_backend_braille");
        get(stop, "prism_backend_stop");
        get(error_string, "prism_error_string");
        return ok;
    }

    bool CreateBackend() {
        if (backend) { backend_free(backend); backend = nullptr; }
        backend = create_best(ctx);  // returned already initialized; do not initialize again
        if (!backend) { Log("prism_registry_create_best found no backend"); return false; }
        features = get_features(backend);
        Log("backend: %s (features 0x%llx)", backend_name(backend), (unsigned long long)features);
        return true;
    }

    bool Start() {
        ctx = init(nullptr);
        if (!ctx) { Log("prism_init failed"); return false; }
        return CreateBackend();
    }

    // Recreate the backend at most every 2 s: that is how speech recovers when the player
    // starts or restarts their screen reader after the game.
    bool Recover() {
        if (!ctx) return false;  // Prism never loaded
        ULONGLONG now = GetTickCount64();
        if (now - last_recreate < 2000) return false;
        last_recreate = now;
        Log("recreating backend");
        return CreateBackend();
    }

    PrismError SayOnce(const char* text, bool interrupt) {
        if (features & PRISM_BACKEND_SUPPORTS_OUTPUT) return output(backend, text, interrupt);
        return speak(backend, text, interrupt);
    }

    void Say(const char* text, bool interrupt) {
        if (!backend && !Recover()) return;
        PrismError e = SayOnce(text, interrupt);
        if (e != PRISM_OK) {
            Log("speak failed: %s", error_string(e));
            if (Recover()) {
                e = SayOnce(text, interrupt);
                if (e != PRISM_OK) Log("speak failed again: %s", error_string(e));
            }
        }
    }

    void Braille(const char* text) {
        if (ctx && backend && (features & PRISM_BACKEND_SUPPORTS_BRAILLE)) braille(backend, text);
    }

    void Stop() {
        if (ctx && backend && (features & PRISM_BACKEND_SUPPORTS_STOP)) stop(backend);
    }

    void Close() {
        if (!ctx) return;
        if (backend) { backend_free(backend); backend = nullptr; }
        if (ctx) { shutdown(ctx); ctx = nullptr; }
    }
};

Prism g_prism;
bool g_mute = false;

// Returns false when the host should exit.
bool HandleLine(const std::string& line) {
    if (line.rfind("say ", 0) == 0 && line.size() >= 6) {
        bool interrupt = line[4] == '1';
        const char* text = line.c_str() + 6;
        if (*text) {
            Log("%s%s: %s", g_mute ? "Muted" : "Spoke", interrupt ? " (interrupt)" : "", text);
            if (!g_mute) g_prism.Say(text, interrupt);
        }
    } else if (line.rfind("braille ", 0) == 0) {
        g_prism.Braille(line.c_str() + 8);
    } else if (line == "stop") {
        g_prism.Stop();
    } else if (line == "quit") {
        return false;
    } else if (!line.empty()) {
        Log("unknown command: %s", line.c_str());
    }
    return true;
}

// Waits for an overlapped operation or the game's exit. Returns false if the game exited.
bool WaitIo(HANDLE event, HANDLE game) {
    HANDLE handles[2] = {event, game};
    DWORD count = game ? 2 : 1;
    return WaitForMultipleObjects(count, handles, FALSE, INFINITE) == WAIT_OBJECT_0;
}

// Reads commands from one connected client. Returns false when the host should exit.
bool ServeClient(HANDLE pipe, OVERLAPPED& ov, HANDLE game) {
    std::string pending;
    char buf[4096];
    for (;;) {
        ResetEvent(ov.hEvent);
        DWORD got = 0;
        if (!ReadFile(pipe, buf, sizeof buf, &got, &ov)) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) { Log("client disconnected (%lu)", err); return true; }
            if (!WaitIo(ov.hEvent, game)) { Log("game exited"); return false; }
            if (!GetOverlappedResult(pipe, &ov, &got, FALSE)) {
                Log("client disconnected (%lu)", GetLastError());
                return true;
            }
        }
        pending.append(buf, got);
        size_t nl;
        while ((nl = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, nl);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            pending.erase(0, nl + 1);
            if (!HandleLine(line)) { Log("quit"); return false; }
        }
    }
}

int Run(DWORD pid, std::wstring pipe_name) {
    HANDLE game = pid ? OpenProcess(SYNCHRONIZE, FALSE, pid) : nullptr;
    if (pid && !game) { Log("game pid %lu not found", pid); return 1; }

    HANDLE pipe = CreateNamedPipeW(pipe_name.c_str(), PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 64 * 1024,
                                   0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE) {
        Log("CreateNamedPipe %ls failed: %lu", pipe_name.c_str(), GetLastError());
        return 1;
    }
    Log("listening on %ls", pipe_name.c_str());

    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    // Serve one client at a time until the game exits or a client sends quit; a client that
    // disconnects (the mod reconnecting, a probe) just puts the pipe back into listening.
    for (;;) {
        ResetEvent(ov.hEvent);
        if (!ConnectNamedPipe(pipe, &ov)) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                if (!WaitIo(ov.hEvent, game)) { Log("game exited"); break; }
            } else if (err != ERROR_PIPE_CONNECTED) {
                Log("ConnectNamedPipe failed: %lu", err);
                break;
            }
        }
        Log("client connected");
        if (!ServeClient(pipe, ov, game)) break;
        DisconnectNamedPipe(pipe);
    }
    CloseHandle(pipe);
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    std::wstring dir = ExeDir();
    g_log = _wfsopen((dir + L"\\SpeechHost.log").c_str(), L"w", _SH_DENYWR);  // readable while running

    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    DWORD pid = 0;
    std::wstring pipe_name;
    for (int i = 1; i < argc; i++) {
        if (!wcscmp(argv[i], L"--mute")) g_mute = true;
        else if (i + 1 < argc && !wcscmp(argv[i], L"--pid")) pid = wcstoul(argv[++i], nullptr, 10);
        else if (i + 1 < argc && !wcscmp(argv[i], L"--pipe")) pipe_name = argv[++i];
    }
    if (pipe_name.empty()) pipe_name = L"\\\\.\\pipe\\SimsAccessSpeech-" + std::to_wstring(pid);
    Log("SpeechHost starting, game pid %lu", pid);

    if (g_mute) {
        Log("muted: lines are logged, not spoken");
    } else if (!g_prism.Load(dir + L"\\prism.dll") || !g_prism.Start()) {
        Log("speech unavailable");
        // Keep draining the pipe anyway so the game side never blocks on a full pipe.
    }
    int rc = Run(pid, pipe_name);
    g_prism.Close();
    Log("SpeechHost exiting");
    if (g_log) fclose(g_log);
    return rc;
}
