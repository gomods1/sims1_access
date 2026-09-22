// The game's tutorial (house 7, the Tutorial SimAntics object) and its help tips, by ear. The
// tutorial shows each step in a non-modal panel at the top right of the lot view (a
// cTSObjectDialog with a text and a tiny "hide" button) and points a red arrow
// (cTSWinTutorialHighlight) at the control to use. Neither is modal, so the navigator never
// announced them. This watcher speaks every new step, as the mod's keyboard version of it when
// it has one (in the game's language, matched by the step's index in the tutorial's texts),
// names the control the arrow points at, and announces the help system's tip icon when it
// appears. Nothing here changes game state.
#pragma once

#include "model.h"

namespace nav::tutorial {

void Reset();
void Tick();  // lot view only

// The tutorial is running (its object is on the lot).
bool Running();
// Escape while the tutorial runs: the game's Escape cancels it silently, so the first press only
// says so and the second within a few seconds goes through. true = swallowed.
bool GuardEscape();

// Ctrl+H: opens the help system's waiting tip (presses its icon), or says there is none.
void OpenTip();

// The step panel's group is named "tutorial" and never takes focus by itself.
bool IsStepPanel(Ptr widget);   // the non-modal cTSObjectDialog inside the lot view
bool IsStepGroup(const Group& g);
// The panel's unnamed button hides the step text (the game's "-" button).
bool IsStepHideButton(const Item& it);

}  // namespace nav::tutorial
