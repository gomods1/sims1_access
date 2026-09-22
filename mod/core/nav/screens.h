// Per-screen policy: screen and group names, which group gets focus first, which panels take
// focus when they open. Add a screen's special cases here, not in the navigator.
#pragma once

#include <string>

#include "model.h"

namespace nav {

// Name spoken when entering a screen; empty for dialogs (their text is read instead).
std::string ScreenName(Ptr root);

// Screens persist while the game runs; dialogs are created and destroyed (and a new dialog
// often reuses the address of the previous one), so focus is only remembered for screens.
bool IsScreen(Ptr root);

// What makes two visits of a screen the same visit: the lot view object is reused for every lot,
// so its focus is only restored while the same lot is loaded (its ObjectModule). 0 elsewhere.
uintptr_t ScreenInstance(Ptr root);

// Brings up what a screen hides at first and a player needs: the lot view's family panel (after
// a lot loads no mode panel shows until Live mode is chosen). true when it pressed something.
bool PrepareScreen(Ptr root);

std::string GroupName(const Group& g);

// Sort key for groups: lower comes first; equal keys keep screen position order. The lot view's
// HUD reads status, needs, family and the action queue before the other panels.
int GroupOrder(const Group& g);

// Item focused when moving into a group: the selected Sim in the family group, else the first
// control.
int EntryItem(const Group& g);

// The list that is the point of its screen (the families of the family picker): default focus.
bool IsPrimaryGroup(const Group& g);

// The selected Sim's action queue: changes on its own, never gets default focus.
bool IsActionQueue(const Group& g);

// A group that appears while staying on the same screen (options, catalog...) takes focus.
bool TakesFocusWhenOpened(const Group& g);

// A page of settings (an options page, or any group with a slider or value picker): in the lot
// view Up/Down walk its rows, since Left/Right change its values.
bool IsSettingsPage(const Group& g);

}  // namespace nav
