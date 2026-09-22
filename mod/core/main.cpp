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
#include "util/strings.h"

namespace {

bool g_active = false;       // false on an unsupported build: speech only, no hooks, no keys
bool g_forceActive = false;  // development: keep the game's input alive in the background

bool ModFileExists(const SaLoaderApi* api, const wchar_t* name) {
    std::wstring path = std::wstring(api->mod_dir) + L"\\" + name;
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

}  // namespace

extern "C" __declspec(dllexport) int __cdecl SA_Init(const SaLoaderApi* api) {
    if (!api || api->version != SA_LOADER_API_VERSION) return 0;
    bool dev = ModFileExists(api, L"dev.txt");
    util::InitLog(api, dev);
    Log("SimsAccess core init (built %s %s)", __DATE__, __TIME__);
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

extern "C" __declspec(dllexport) void __cdecl SA_OnTick() {
    if (!g_active) return;
    if (g_forceActive) game::ForceAppActive();
    nav::Tick();
}
