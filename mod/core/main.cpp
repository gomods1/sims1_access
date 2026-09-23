// SimsAccess.dll: the mod core, loaded by the version.dll proxy (mod/loader). Every entry point
// runs on the game's main thread (the window procedure and a 30 ms timer). This file only wires
// the modules together:
//   util/     log, text conversion, the game's language, the mod's own strings
//   lang/     the mod's strings in every language the game ships
//   speech/   speech through SpeechHost.exe
//   game/     the only code that knows addresses and offsets (game/addresses.h)
//   nav/      keyboard navigation over the game's widgets
//   input/    which keys the mod claims
#include <windows.h>

#include <string>

#include "../loader/loader_api.h"
#include "game/hooks.h"
#include "game/app.h"
#include "game/memory.h"
#include "game/rtti.h"
#include "game/ui.h"
#include "input/keyboard.h"
#include "nav/navigator.h"
#include "speech/speech.h"
#include "util/language.h"
#include "util/log.h"
#include "util/settings.h"
#include "util/strings.h"

namespace {

bool g_active = false;       // false on an unsupported build: speech only, no hooks, no keys
bool g_forceActive = false;  // development: keep the game's input alive in the background

bool ModFileExists(const SaLoaderApi* api, const wchar_t* name) {
    std::wstring path = std::wstring(api->mod_dir) + L"\\" + name;
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

// Development: tools/launch.py writes the user's foreground window to return_focus.txt before
// starting the game. When the game's window takes the foreground at startup, the core hands it
// back (the foreground process may pass it on; an outside process is refused).
HWND g_gameWindow = nullptr;
HWND g_returnFocus = nullptr;
ULONGLONG g_returnFocusUntil = 0;

void ReadReturnFocus(const SaLoaderApi* api) {
    std::wstring path = std::wstring(api->mod_dir) + L"\\return_focus.txt";
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return;
    char text[32] = {};
    DWORD got = 0;
    ReadFile(f, text, sizeof text - 1, &got, nullptr);
    CloseHandle(f);
    DeleteFileW(path.c_str());
    g_returnFocus = (HWND)(uintptr_t)strtoul(text, nullptr, 0);
    g_returnFocusUntil = GetTickCount64() + 60000;
    Log("development: will hand the foreground back to window %p", (void*)g_returnFocus);
}

void ReturnFocus() {
    if (!g_returnFocus) return;
    if (GetTickCount64() > g_returnFocusUntil || !IsWindow(g_returnFocus)) {
        g_returnFocus = nullptr;
        return;
    }
    if (GetForegroundWindow() != g_gameWindow) return;
    BOOL ok = SetForegroundWindow(g_returnFocus);
    Log("development: handed the foreground back to %p (%d)", (void*)g_returnFocus, ok);
    g_returnFocus = nullptr;
}

}  // namespace

extern "C" __declspec(dllexport) int __cdecl SA_Init(const SaLoaderApi* api) {
    if (!api || api->version != SA_LOADER_API_VERSION) return 0;
    bool dev = ModFileExists(api, L"dev.txt");
    util::InitLog(api, dev);
    Log("SimsAccess core init (built %s %s)", __DATE__, __TIME__);
    util::InitSettings(api->mod_dir);
    g_gameWindow = api->hwnd;
    if (dev) ReadReturnFocus(api);
    util::InitLanguage();  // from the registry: even an unsupported build speaks the player's language
    speech::Start(api->mod_dir);
    if (!api->build_ok) {
        Log("unsupported Sims.exe build %s: installing nothing", api->exe_hash);
        speech::Say(S(Str::UnsupportedBuild));
        return 1;
    }
    game::InitMemory(api->game);
    util::SetGameLanguage(game::LanguageNumber());
    g_forceActive = ModFileExists(api, L"force_active.txt");
    game::InitHooks(g_forceActive);
    nav::Reset();
    if (g_forceActive) Log("development: forcing the game's app-active flag on");
    g_active = true;
    if (!api->reloaded) speech::Say(S(Str::ModLoaded));
    return 1;
}

extern "C" __declspec(dllexport) void __cdecl SA_Shutdown() {
    Log("SimsAccess core shutdown");
    if (g_active) {
        game::ShutdownHooks();
        game::ClearRttiCache();
    }
    g_active = false;
    speech::Stop();
}

extern "C" __declspec(dllexport) int __cdecl SA_OnMessage(HWND, UINT msg, WPARAM wp, LPARAM lp, LRESULT* result) {
    if (!g_active) return 0;
    bool consumed = false;
    switch (msg) {
    case WM_KEYDOWN: consumed = input::OnKeyDown(wp, lp); break;
    case WM_KEYUP: consumed = input::OnKeyUp(wp, lp); break;
    case WM_CHAR: consumed = input::OnChar(wp); break;
    case WM_KILLFOCUS:
    case WM_ACTIVATEAPP: input::OnFocusLost(); break;
    }
    if (consumed) *result = 0;
    return consumed ? 1 : 0;
}

// The loader asks before a hot reload: nonzero while one of the core's hooks is on the stack.
extern "C" __declspec(dllexport) int __cdecl SA_Busy() { return g_active && game::HooksBusy() ? 1 : 0; }

extern "C" __declspec(dllexport) void __cdecl SA_OnTick() {
    ReturnFocus();
    if (!g_active) return;
    if (g_forceActive) game::ForceAppActive();
    nav::Tick();
}
