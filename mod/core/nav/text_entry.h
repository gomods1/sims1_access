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

}  // namespace nav::text
