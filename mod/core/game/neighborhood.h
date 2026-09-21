// The neighborhood map: houses (cWinLotBtn widgets) and their hover cards.
#pragma once

#include <string>

#include "memory.h"

namespace game {

int LotNumber(Ptr lot);            // house number (street address number); -1 if unreadable
std::string LotFamily(Ptr lot);    // resident family, UTF-8; empty when vacant

// The text of the house's hover card exactly as the game draws it, e.g.
// "La Autovía Gunther Goth,76\n\nLa familia Flores\r\nValor neto: $14.931\r\nAmigos: 0".
// UTF-8; empty when it cannot be produced.
std::string LotCardText(Ptr lot);

}  // namespace game
