// The lot view's control panel (cWinViewControl, bottom left): clock, family funds, friend count
// and the game speed buttons.
#pragma once

#include <string>

#include "memory.h"

namespace game {

Ptr FindViewControl(Ptr lotView);  // 0 when the lot view has none
Ptr LiveModeButton(Ptr vc);
bool IsModeButton(Ptr w);  // live, buy, build, options or camera mode

// The control panel shows one of its mode panels: the live mode family panel (cWinPeople), or
// buy, build, options or camera mode. None is shown after a lot loads until a mode is chosen.
bool ModePanelShown(Ptr lotView);

std::string ClockText(Ptr vc);     // the time exactly as drawn ("11:46"), UTF-8
Ptr FundsButton(Ptr vc);           // caption: the funds ("$9.500"); pressing opens the budget
Ptr FriendsButton(Ptr vc);         // caption: the family's friend count

// Pause is a toggle of its own; one of the three speed buttons stays checked while paused.
struct GameSpeed {
    bool paused = false;
    int level = 0;  // 1 normal, 2 high, 3 ultra; 0 unknown
    bool operator==(const GameSpeed& o) const { return paused == o.paused && level == o.level; }
    bool operator!=(const GameSpeed& o) const { return !(*this == o); }
};
GameSpeed ReadGameSpeed(Ptr vc);
Ptr PauseButton(Ptr vc);
Ptr SpeedButton(Ptr vc, int level);
bool IsSpeedButton(Ptr w);  // pause or a speed level

}  // namespace game
