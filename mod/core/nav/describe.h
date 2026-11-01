// What is spoken for an item or a group: label plus the state a sighted player sees.
#pragma once

#include <string>
#include <vector>

#include "../game/design.h"
#include "model.h"

namespace nav {

std::string Describe(const Item& it);
// The details buffer's lines for a focused item: what was spoken, then what else there is (a
// product's ratings and description, the tooltip, its place in the group).
std::vector<std::string> DetailLines(const Item& it, const Group& g, int index);
// What the game's hover popup says about a live panel item (a need, trait, interest, skill, the
// job, a house rating...), empty when it has none.
std::vector<std::string> PanelHelp(const Item& it);
std::string GroupSummary(const Group& g);
// A lot's own name from its card ("Hotel Palmeras Palazzo"); the address when it has none.
std::string LotPlaceName(const Item& it);  // "casas, 41 elementos"

// Create a Sim: "cabeza 2 de 137: dave" / "cuerpo 5 de 143, atlético: beachguy2".
std::string DescribeLook(Ptr screen, game::LookPart part);

// Said after pressing a widget whose effect is not visible in its own state (create a Sim: the
// head and body arrows, and the age, sex and skin choices, which change the head and body
// lists); empty for the rest.
std::string AfterPress(Ptr root, Ptr pressed);

// The game speed in the game's own words: "Pausa", "Velocidad normal"...
std::string GameSpeedName(Ptr viewControl);

// Some descriptions are expensive (house cards) and cached per screen.
void ClearDescriptionCache();

}  // namespace nav
