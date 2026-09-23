#include "hooks.h"

#include <windows.h>

#include "../util/log.h"
#include "../util/text.h"
#include "MinHook.h"
#include "addresses.h"
#include "memory.h"
#include "simantics.h"
#include "sounds.h"
#include "build.h"

namespace game {
namespace {

bool g_minhookReady = false;

// Detours under way: while one is on the stack (a SimAntics dialog runs a modal loop inside the
// primitive hook) the core must not be unloaded (SA_Busy).
int g_hookDepth = 0;
struct InHook {
    InHook() { g_hookDepth++; }
    ~InHook() { g_hookDepth--; }
};

// ---- cTSFont::CountLines (text capture) -----------------------------------------------------
typedef int(__thiscall* CountLinesFn)(void* font, const char* text, int len, int width);
CountLinesFn g_countLines = nullptr;
bool g_captureHooked = false;
bool g_capturing = false;
std::string g_captured;  // game text (util/text.h)

int __fastcall CountLinesDetour(void* font, void* /*edx*/, const char* text, int len, int width) {
    InHook inHook;
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
    InHook inHook;
    int* result = g_screenToWorld(out, packedScreen, flag);
    if (g_worldOverride && out) {
        out[0] = g_worldX;
        out[1] = g_worldY;
    }
    return result;
}

// ---- cMoveTool::MouseLeft (keep the held object in the world) --------------------------------
// While the mod shows the held object on a spot, the view keeps telling the buy tool that the
// mouse left the lot (the real mouse is elsewhere), which takes the object out of the world: it
// vanished from the screen and from anything looking for it (the tutorial cancelled itself).
typedef void(__thiscall* MouseLeftFn)(void* tool);
MouseLeftFn g_mouseLeft = nullptr;
bool g_mouseLeftHooked = false;
bool g_keepHeldShown = false;

void __fastcall MouseLeftDetour(void* tool, void* /*edx*/) {
    InHook inHook;
    if (g_keepHeldShown) return;
    g_mouseLeft(tool);
}

// ---- GetAsyncKeyState (Ctrl suppression) -----------------------------------------------------
// The family panel opens a Sim's card instead of selecting when a portrait is clicked with Ctrl
// held (it polls GetAsyncKeyState when the click notification arrives). Ctrl+1..9 press the
// portrait, so for a moment the game is told Ctrl is up. The mod reads keys through the original.
typedef short(WINAPI* GetAsyncKeyStateFn)(int);
GetAsyncKeyStateFn g_getAsyncKeyState = nullptr;
ULONGLONG g_suppressCtrlUntil = 0;
ULONGLONG g_forceCtrlUntil = 0;  // the other way round: Ctrl+Shift+1..9 open the card on purpose

short WINAPI GetAsyncKeyStateDetour(int vk) {
    bool ctrlKey = vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL;
    if (ctrlKey && GetTickCount64() < g_forceCtrlUntil) return (short)0x8000;
    if (ctrlKey && GetTickCount64() < g_suppressCtrlUntil) return 0;
    return g_getAsyncKeyState(vk);
}

// ---- SimAntics: balloons and removed objects (simantics.h) ----------------------------------
// The game's behavior code throws C++ exceptions through these detours: they keep no C++
// objects alive across the call to the original.
typedef int(__thiscall* SetBalloonFn)(void* object, void* frame, const uint8_t* operand);
SetBalloonFn g_setBalloon = nullptr;

int __fastcall SetBalloonDetour(void* object, void* /*edx*/, void* frame, const uint8_t* operand) {
    InHook inHook;
    BalloonCall call;
    BeforeSetBalloon((Ptr)object, (Ptr)frame, operand, &call);
    int result = g_setBalloon(object, frame, operand);
    if (call.watch) AfterSetBalloon(call);
    return result;
}

typedef int(__thiscall* DispatchFn)(void* object, void* frame, const uint16_t* instruction);
DispatchFn g_dispatch = nullptr;

int __fastcall DispatchDetour(void* object, void* /*edx*/, void* frame, const uint16_t* instruction) {
    InHook inHook;
    int opcode = *instruction & addr::simantics::kOpcodeMask;
    if (opcode == addr::simantics::kOpRemoveObject) BeforePrimitive((Ptr)object, (Ptr)frame, (const uint8_t*)instruction);
    int result = g_dispatch(object, frame, instruction);
    if (opcode == addr::simantics::kOpCreateObject) AfterCreateObject((Ptr)object, (Ptr)frame);
    return result;
}

// ---- GetKeyState (modifiers for the mod's build drags) ---------------------------------------
typedef short(WINAPI* GetKeyStateFn)(int);
GetKeyStateFn g_getKeyState = nullptr;
bool g_forceModifiers = false, g_forceShift = false, g_forceCtrl = false;

short WINAPI GetKeyStateDetour(int vk) {
    if (g_forceModifiers) {
        if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) return g_forceShift ? (short)0x8000 : 0;
        if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) return g_forceCtrl ? (short)0x8000 : 0;
    }
    return g_getKeyState(vk);
}

// ---- Build tools' error message ------------------------------------------------------------
typedef void(__cdecl* BuildErrorFn)(int code, int arg);
BuildErrorFn g_buildError = nullptr;

void __cdecl BuildErrorDetour(int code, int arg) {
    InHook inHook;
    OnBuildError(code, arg);
    g_buildError(code, arg);
}

// ---- Sound events (sounds.h) -----------------------------------------------------------------
typedef int(__thiscall* PlaySoundEventFn)(void* manager, const char* name, int source);
PlaySoundEventFn g_playSoundEvent = nullptr;

int __fastcall PlaySoundEventDetour(void* manager, void* /*edx*/, const char* name, int source) {
    InHook inHook;
    OnSoundEvent(name, (short)source);
    return g_playSoundEvent(manager, name, source);
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
    g_mouseLeftHooked = Install(addr::movetool::kLeft, (void*)&MouseLeftDetour, (void**)&g_mouseLeft, "cMoveTool::MouseLeft");
    Install(addr::simantics::kSetBalloon, (void*)&SetBalloonDetour, (void**)&g_setBalloon, "SetBalloonPrimitive");
    Install(addr::simantics::kDispatch, (void*)&DispatchDetour, (void**)&g_dispatch, "cXObject::DoPrimitive");
    Install(addr::kBuildError, (void*)&BuildErrorDetour, (void**)&g_buildError, "ShowBuildError");
    Install(addr::kPlaySoundEvent, (void*)&PlaySoundEventDetour, (void**)&g_playSoundEvent, "SoundManager::PlayEvent");
    MH_STATUS gs = MH_CreateHookApi(L"user32", "GetKeyState", (void*)&GetKeyStateDetour, (void**)&g_getKeyState);
    Log("hook GetKeyState: %s", MH_StatusToString(gs));
    MH_STATUS ks = MH_CreateHookApi(L"user32", "GetAsyncKeyState", (void*)&GetAsyncKeyStateDetour, (void**)&g_getAsyncKeyState);
    if (ks == MH_OK) ks = MH_EnableHook(MH_ALL_HOOKS);
    Log("hook GetAsyncKeyState: %s", MH_StatusToString(ks));
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
    g_mouseLeftHooked = false;
    g_keepHeldShown = false;
    g_forceModifiers = false;
}

void ForceModifiers(bool shift, bool ctrl) {
    g_forceShift = shift;
    g_forceCtrl = ctrl;
    g_forceModifiers = g_getKeyState != nullptr;
}

void ClearForcedModifiers() { g_forceModifiers = false; }

bool HooksBusy() { return g_hookDepth > 0; }

bool TextCaptureAvailable() { return g_captureHooked; }

void BeginTextCapture() {
    g_captured.clear();
    g_capturing = true;
}

std::string EndTextCapture() {
    g_capturing = false;
    return util::GameTextToUtf8(g_captured);
}

bool WorldPointerHooked() { return g_worldPointerHooked; }

void SetWorldPointerOverride(int viewX, int viewY) {
    g_worldX = viewX;
    g_worldY = viewY;
    g_worldOverride = true;
}

void ClearWorldPointerOverride() { g_worldOverride = false; }

bool ScreenPointToView(int gameX, int gameY, int* viewX, int* viewY) {
    if (!g_worldPointerHooked || !g_screenToWorld) return false;
    uint32_t packed = (uint32_t)(uint16_t)gameY | ((uint32_t)(uint16_t)gameX << 16);  // as the pick message
    int out[4] = {0, 0, 0, 0};
    bool override = g_worldOverride;
    g_worldOverride = false;
    int* result = nullptr;
    bool ok = Guarded([&] { result = g_screenToWorld(out, &packed, 0); });
    g_worldOverride = override;
    if (!ok || !result) return false;
    *viewX = out[0];
    *viewY = out[1];
    return true;
}

void KeepHeldShown(bool keep) { g_keepHeldShown = keep && g_mouseLeftHooked; }

void SuppressCtrlFor(unsigned ms) {
    g_suppressCtrlUntil = GetTickCount64() + ms;
    g_forceCtrlUntil = 0;
}

void ForceCtrlFor(unsigned ms) {
    g_forceCtrlUntil = GetTickCount64() + ms;
    g_suppressCtrlUntil = 0;
}

short RealGetAsyncKeyState(int vk) { return g_getAsyncKeyState ? g_getAsyncKeyState(vk) : GetAsyncKeyState(vk); }

}  // namespace game
