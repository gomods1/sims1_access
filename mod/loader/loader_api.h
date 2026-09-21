// Interface between the version.dll proxy (loader) and the mod core (SimsAccess.dll).
// The loader owns everything that must survive a hot reload of the core: the window subclass,
// the tick timer and the log file. The core owns speech, hooks and all game knowledge.
#pragma once
#include <windows.h>

#define SA_LOADER_API_VERSION 1

struct SaLoaderApi {
    int version;               // SA_LOADER_API_VERSION
    HWND hwnd;                 // game main window (class "Gonzo")
    HMODULE game;              // Sims.exe module
    const wchar_t* mod_dir;    // ...\The Sims Legacy Collection\SimsAccess (no trailing slash)
    int build_ok;              // 1 when Sims.exe matches the supported build hash
    const char* exe_hash;      // SHA-256 of Sims.exe, lowercase hex
    int reloaded;              // 1 when this core was loaded by a hot reload
    void (*log)(const char* line);  // append one line to SimsAccess.log (thread-safe)
};

// Core exports (all called on the game's main thread):
//   int  SA_Init(const SaLoaderApi* api)   -> 1 on success
//   void SA_Shutdown(void)                  -> must remove every hook it installed
//   int  SA_OnMessage(HWND, UINT, WPARAM, LPARAM, LRESULT* result) -> 1 = swallowed
//   void SA_OnTick(void)                    -> about every 30 ms
typedef int(__cdecl* SaInitFn)(const SaLoaderApi*);
typedef void(__cdecl* SaShutdownFn)(void);
typedef int(__cdecl* SaOnMessageFn)(HWND, UINT, WPARAM, LPARAM, LRESULT*);
typedef void(__cdecl* SaOnTickFn)(void);
