// Speaks pause and speed changes in the lot view, whether they come from the game's keys (P, 1,
// 2, 3) or from pressing the buttons. Pausing also moves focus to the status line, so the
// arrows review the HUD (time, funds, needs, family) while the game stands still.
#pragma once

#include "model.h"

namespace nav::speed {

void Reset();
void Tick(Ptr root);
// Ctrl+T: every new hour of the game's clock spoken ("son las 14:00"), or not ([mod] hours).
void ToggleHours();  // from the navigator's tick, with the current input root

}  // namespace nav::speed
