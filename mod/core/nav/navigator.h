// The keyboard focus the game does not have. Follows the window that owns input (screen or modal
// dialog), keeps a focused item, announces screens, dialogs, panels and focus changes, and presses
// the focused widget. All functions run on the game's main thread.
#pragma once

#include <windows.h>

#include <string>

namespace nav {

void Reset();
void Tick();  // about every 30 ms

// A text field is on screen: letters and Space belong to it, not to the mod.
bool TextEntryActive();

// The lot view owns input, so the object browser (nav/world.h) is available.
bool LotViewActive();
// Ctrl+R in the catalog: keep products to those meant for a kind of room (all, kitchen, ...).
void CycleRoomFilter();

// Which focus Enter and R act on: the object browser after PageUp/PageDown, the UI item after
// arrows or Tab. Only meaningful while the lot view is active.
void FocusWorld(bool on);
bool WorldFocused();

// Lot view: moves focus to the first item of the status line (time) without speaking. Used
// when the game is paused, so the arrows review the HUD.
void FocusStatus();

// The next interaction menu that opens (within a few seconds) is answered with this entry
// instead of being read: walking by keyboard, the floor's menu only offers "Ir a este punto".
void ChooseInNextMenu(int index);

// True when an interaction menu owned input at some point since `time` (GetTickCount64): tells
// whether a menu call that has returned showed a menu at all.
bool PieMenuSeenSince(ULONGLONG time);

// Escape: closes a menu the mod drives (the interaction menu). false when there is none, so the
// key goes to the game.
bool Cancel();

// Actions bound to keys by the input layer.
void Move(int delta);        // previous/next item in the current group (no wrap)
void MoveToEdge(bool last);  // first/last item in the current group
void NextGroup(int delta);   // previous/next group (wraps)
void NextRowOrGroup(int delta);  // lot view Up/Down: the next group, or the next row of a settings page
void Activate();             // press the focused item
bool Adjust(int delta);      // change the focused value (a slider, a value picker, a personality
                             // bar); false if the focus holds none
void RepeatFocus();          // say the focused item again, with its group
uintptr_t FocusedWidget();   // the focused UI item's widget (cTSWin*), 0 when none

// The description of the item nearest to a rectangle on screen (within a few pixels), and the
// name of its group; empty when nothing is near. For the tutorial arrow.
std::string NearestItemTo(const RECT& r, std::string* groupName);

}  // namespace nav
