// The credits screen (cWinCredits) scrolls the names as drawn text: the mod reads them from
// GameData\Credits.iff in the game's language into the details buffer (Control+Up reads on).
#pragma once

#include <string>
#include <vector>

#include "../game/memory.h"

namespace nav::credits {

// Called with the screen's root when it changes: true on the credits screen (the details buffer
// then holds every line of the credits).
bool OnScreen(game::Ptr root);

}  // namespace nav::credits
