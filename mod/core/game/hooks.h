// Function hooks (MinHook). Every hook is installed in InitHooks and removed in ShutdownHooks,
// which a hot reload of the core relies on.
#pragma once

#include <string>

namespace game {

// blockCursorWarp (development): the game moves the real mouse cursor in some menus; while the
// developer shares the machine that must not happen, so SetCursorPos from the game is ignored.
void InitHooks(bool blockCursorWarp);
void ShutdownHooks();

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

}  // namespace game
