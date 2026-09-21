#include "hooks.h"

#include <windows.h>

#include "../util/log.h"
#include "../util/text.h"
#include "MinHook.h"
#include "addresses.h"
#include "memory.h"

namespace game {
namespace {

bool g_minhookReady = false;

// ---- cTSFont::CountLines (text capture) -----------------------------------------------------
typedef int(__thiscall* CountLinesFn)(void* font, const char* text, int len, int width);
CountLinesFn g_countLines = nullptr;
bool g_captureHooked = false;
bool g_capturing = false;
std::string g_captured;  // cp1252

int __fastcall CountLinesDetour(void* font, void* /*edx*/, const char* text, int len, int width) {
    if (g_capturing && g_captured.empty() && text && len > 0 && len < 4096) g_captured.assign(text, len);
    return g_countLines(font, text, len, width);
}

// ---- ScreenToWorld (buy mode placement) -----------------------------------------------------
typedef int*(__cdecl* ScreenToWorldFn)(int* out, uint32_t* packedScreen, int flag);
ScreenToWorldFn g_screenToWorld = nullptr;
bool g_worldPointerHooked = false;
bool g_worldOverride = false;
int g_worldX = 0, g_worldY = 0;

int* __cdecl ScreenToWorldDetour(int* out, uint32_t* packedScreen, int flag) {
    int* result = g_screenToWorld(out, packedScreen, flag);
    if (g_worldOverride && out) {
        out[0] = g_worldX;
        out[1] = g_worldY;
    }
    return result;
}

// ---- SetCursorPos (development only) ---------------------------------------------------------
typedef BOOL(WINAPI* SetCursorPosFn)(int, int);
SetCursorPosFn g_setCursorPos = nullptr;

BOOL WINAPI SetCursorPosDetour(int x, int y) {
    LogVerbose("blocked SetCursorPos(%d, %d)", x, y);
    return TRUE;
}

bool Install(Ptr preferredVa, void* detour, void** original, const char* name) {
    if (!FunctionOk(preferredVa)) {
        Log("hook %s skipped: signature mismatch", name);
        return false;
    }
    void* target = (void*)Va(preferredVa);
    MH_STATUS st = MH_CreateHook(target, detour, original);
    if (st == MH_OK) st = MH_EnableHook(target);
    Log("hook %s: %s", name, MH_StatusToString(st));
    return st == MH_OK;
}

}  // namespace

void InitHooks(bool blockCursorWarp) {
    MH_STATUS st = MH_Initialize();
    g_minhookReady = st == MH_OK || st == MH_ERROR_ALREADY_INITIALIZED;
    if (!g_minhookReady) {
        Log("MinHook init failed: %s", MH_StatusToString(st));
        return;
    }
    g_captureHooked = Install(addr::kFontCountLines, (void*)&CountLinesDetour, (void**)&g_countLines,
                              "cTSFont::CountLines");
    g_worldPointerHooked = Install(addr::kScreenToWorld, (void*)&ScreenToWorldDetour, (void**)&g_screenToWorld,
                                   "ScreenToWorld");
    if (blockCursorWarp) {
        MH_STATUS st = MH_CreateHookApi(L"user32", "SetCursorPos", (void*)&SetCursorPosDetour, (void**)&g_setCursorPos);
        if (st == MH_OK) st = MH_EnableHook(MH_ALL_HOOKS);
        Log("hook SetCursorPos (development): %s", MH_StatusToString(st));
    }
}

void ShutdownHooks() {
    if (!g_minhookReady) return;
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    g_minhookReady = false;
    g_captureHooked = false;
    g_worldPointerHooked = false;
    g_worldOverride = false;
}

bool TextCaptureAvailable() { return g_captureHooked; }

void BeginTextCapture() {
    g_captured.clear();
    g_capturing = true;
}

std::string EndTextCapture() {
    g_capturing = false;
    return util::Cp1252ToUtf8(g_captured);
}

bool WorldPointerHooked() { return g_worldPointerHooked; }

void SetWorldPointerOverride(int viewX, int viewY) {
    g_worldX = viewX;
    g_worldY = viewY;
    g_worldOverride = true;
}

void ClearWorldPointerOverride() { g_worldOverride = false; }

}  // namespace game
