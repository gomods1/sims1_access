// The neighborhood map: houses (cWinLotBtn widgets) and their hover cards.
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

int LotNumber(Ptr lot);            // house number (street address number); -1 if unreadable
std::string LotFamily(Ptr lot);    // resident family, UTF-8; empty when vacant

// The text of the house's hover card exactly as the game draws it, e.g.
// "La Autovía Gunther Goth,76\n\nLa familia Flores\r\nValor neto: $14.931\r\nAmigos: 0".
// UTF-8; empty when it cannot be produced.
std::string LotCardText(Ptr lot);

// A lot's kind in the neighborhood's list: 1 a home, 2 a community lot, 0 unknown.
int LotKind(int number);
bool IsCommunityLot(int number);
// The travel map (Old Town's "where to?") the lot button is on, 0 on any other screen.
Ptr TravelMap(Ptr widget);
// The travel map's filter bar: its buttons left to right with the bit each one marks lots by
// (the bar draws every button twice; one of each is kept). Empty on other screens.
struct MapFilter {
    Ptr button;
    int bit;
};
std::vector<MapFilter> MapFilters(Ptr map);
int MapFilterChosen(Ptr map);
bool IsStudioTown(Ptr map);
bool MapLotVisitable(Ptr map, int number);  // a lot the trip can go to (Old Town: its community lots)  // the bit the bar marks lots by now, 0 none
int LotFilterBits(Ptr lot);

}  // namespace game
