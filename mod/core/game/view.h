// The lot view's camera: scrolling to an object and following a Sim, through the game's own
// code (the same paths a click on a family portrait and the Enter key take).
#pragma once

#include "memory.h"

namespace game {

// Scrolls the view to the object (any cXObject or cXPerson on the lot), switching floors if
// needed, immediately. false when the object refuses (not placed, or the function is missing).
bool CenterViewOn(Ptr object);

// The Sim the camera follows every frame (0 = none), and setting it. Following a Sim goes
// through the family panel when it is there, so the panel's own state stays consistent.
Ptr FollowedSim();
bool FollowSim(Ptr person);   // 0 stops following
bool StopFollowing(Ptr lotView);

// Live mode: selects the n-th (0-based) shown family portrait the way clicking it does, without
// the mouse. Returns the Sim's neighbor id, -1 when there is no such portrait or no panel.
// card: with Ctrl held, as the game reads it, the panel opens the Sim's card (cWinEditBio: name and
// biography) instead of selecting.
int SelectFamilyMember(Ptr lotView, int index, bool card = false);
int SelectedNeighborId();  // -1 when none
Ptr FamilyPanel(Ptr lotView);  // the cWinPeople panel of the lot view, shown or not; 0 if none

}  // namespace game
