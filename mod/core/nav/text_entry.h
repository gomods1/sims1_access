// Speaks what the player types into the game's text fields (family names, Sim names): each
// character as it is typed, and the character Backspace deletes. The game draws the field itself,
// so no screen reader echoes it.
#pragma once

#include <windows.h>

namespace nav::text {

// WM_CHAR the mod did not take, before the game handles it: speaks the character.
void OnChar(WPARAM ch);

// Backspace key-down, before the game handles it (the game deletes on key-down, before WM_CHAR):
// speaks the character about to be deleted.
void OnBackspace();

// Gives the game's typing to the field the mod's focus is on (the bio after the name field: the
// letters used to go on into the name).
void TakeTyping(uintptr_t field);

// Left/Right (Ctrl: by word), Home/End, Delete while the mod's focus is on the field that takes
// typing: true when the key is left to the field, which moves its caret; the mod then speaks the
// character (or word) at the caret, or the one Delete removes.
bool OnCaretKey(WPARAM vk, bool ctrl, uintptr_t focusedWidget);

// Speaks where a caret key left the caret, once the game has handled it.
void Tick();

}  // namespace nav::text
