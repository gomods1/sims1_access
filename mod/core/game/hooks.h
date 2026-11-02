// Function hooks (MinHook). Every hook is installed in InitHooks and removed in ShutdownHooks,
// which a hot reload of the core relies on.
#pragma once

#include <string>

namespace game {

// blockCursorWarp (development): the game moves the real mouse cursor in some menus; while the
// developer shares the machine that must not happen, so SetCursorPos from the game is ignored.
void InitHooks(bool blockCursorWarp);
void ShutdownHooks();
bool HooksBusy();  // a detour is on the stack (a modal loop runs inside it)

// Text capture: while active, the first text block the game lays out (cTSFont::CountLines) is
// recorded. Used to read cards and popups that the game renders straight into a bitmap.
bool TextCaptureAvailable();
void BeginTextCapture();
std::string EndTextCapture();  // UTF-8, empty if nothing was laid out

// Buy mode: while an override is set, the game's screen-to-world conversion (0x5e00e0) answers
// with these view coordinates (1/16 tile), so the buy tool's mouse handlers work on a tile the
// mod chose instead of the one under the mouse.
bool WorldPointerHooked();
void SetWorldPointerOverride(int viewX, int viewY);
void ClearWorldPointerOverride();

// The game's own screen-to-world: the view coordinates (1/16 tile, view orientation) under a
// point of the 1024x768 game screen; false when nothing is there or the hook is missing.
bool ScreenPointToView(int gameX, int gameY, int* viewX, int* viewY);

// While set, the buy tool ignores the view telling it the mouse left the lot, so the object the
// mod shows on a spot stays in the world (visible, and findable by the game).
void KeepHeldShown(bool keep);

// For a moment the game reads Ctrl as not held (a portrait click with Ctrl opens the Sim's card).
void SuppressCtrlFor(unsigned ms);
void ForceCtrlFor(unsigned ms);     // ...or as held (to open the card on purpose)
short RealGetAsyncKeyState(int vk);  // the key state without that suppression

// While set, the game's GetKeyState reports Shift and Ctrl as given (held or not), whatever the
// keyboard says: the wall tool reads them for "room" and "delete" drags the mod makes.
void ForceModifiers(bool shift, bool ctrl);
void ClearForcedModifiers();

// While set (id >= 0), the game's "door or window under the pointer" answers this object: the mod
// removes one it has by id, wherever the camera is.
void ForcePortalPick(int id);

}  // namespace game
