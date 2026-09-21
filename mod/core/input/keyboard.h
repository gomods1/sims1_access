// Keyboard messages from the game window: decides which keys the mod claims, runs their actions
// and keeps claimed keys (and the characters TranslateMessage makes from them) away from the game.
#pragma once

#include <windows.h>

namespace input {

// Each returns true when the message is consumed and must not reach the game.
bool OnKeyDown(WPARAM vk, LPARAM lp);
bool OnKeyUp(WPARAM vk, LPARAM lp);
bool OnChar(WPARAM ch);
void OnFocusLost();  // forget tracked modifier keys

}  // namespace input
