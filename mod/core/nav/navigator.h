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
bool WorldFocused();  // the focus is on a thing of the lot ("things by kind / room"): the camera stays on it

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
void JumpToGroup(char letter);
void SayLook();  // Ctrl+Shift+E: what the focused or selected Sim looks like
void ToggleShowFocus();         // Ctrl+Shift+V: the focused control lit on screen, for a sighted helper  // Alt+letter: the next group whose spoken name starts with it
bool Back();                 // Backspace: up a level (a kind's or room's things to the kinds or
                             // rooms); false at a top level
// PageUp/PageDown in the lot view: the next thing of the kind or room chosen last (Thing), the
// next kind (Shift) or room (Ctrl), opened in its group with focus on its first thing.
enum class Browse { Thing, Kind, Room };
void BrowseThings(Browse how, int delta);
void Activate();             // press the focused item

// Ctrl+F (lot view): find a thing by typing part of its name. While active, typing goes to the
// search, Up/Down (Page keys, Home/End) go through what matches, Enter acts on it as on any thing
// of the lot, Ctrl+Enter walks there, Escape goes back to where the focus was. The matches are a
// list of the "things by kind" group, so everything else goes on from there once it is over.
bool FindActive();
void FindBegin();
void FindChar(WPARAM ch);    // a typed character (ANSI, a double-byte one in two calls)
void FindErase();            // Backspace
void FindMove(int delta);
void FindEdge(bool last);
void FindConfirm(bool walk);
void FindCancel();
void FindEnd();              // over, with the matches left as they are (another key took over)
// Left / Right in a paged list of the lot view (the catalogs): turns the page; false elsewhere.
bool TurnPageKey(int delta);
bool Adjust(int delta);      // change the focused value (a slider, a value picker, a personality
                             // bar); false if the focus holds none
void RepeatFocus();          // say the focused item again, with its group
uintptr_t FocusedWidget();   // the focused UI item's widget (cTSWin*), 0 when none
int FocusedLotNumber();      // the focused house of a neighborhood or travel map, -1 when none

// The description of the item nearest to a rectangle on screen (within a few pixels), and the
// name of its group; empty when nothing is near. For the tutorial arrow.
std::string NearestItemTo(const RECT& r, std::string* groupName);

}  // namespace nav
