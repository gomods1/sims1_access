// Where the held object can go. For every area (room) of its floor, the engine says which tiles
// and directions it fits in; the mod then keeps the spots where the object is usable: a Sim can
// reach a tile to use it from (its routing slots), it blocks no door, and every object already
// there stays usable. Spots are scored (back to a wall for objects used from the front, seats
// facing a table, next to related objects) and described in words. Everything comes from the
// engine's data; nothing is known per object.
#pragma once

#include <string>
#include <vector>

#include "../game/lotgrid.h"
#include "../game/rooms.h"
#include "../util/strings.h"

namespace nav::placement {

struct Spot {
    game::Tile tile;        // where the lead part goes
    int direction = 0;      // the engine's direction of the object's front; -1 on a surface
                            // (the game turns it to match)
    int surface = 0;        // object id of the table or counter it goes on, 0 on the floor
    int score = 0;
    std::vector<game::Tile> footprint;
    std::vector<game::Tile> useTiles;  // reachable tiles a Sim uses it from
    int wallSides = 0;      // bit per side (0 front, 1 left, 2 back, 3 right) with a wall along it
    int nextTo = 0;         // object id of the nearest neighbor (0 none)
    int facing = 0;         // object id of what the front faces (a table for a chair), 0 none
    int cutOff = 0;         // floor tiles of the area it would cut off from the rest
};

struct Area {
    int room = game::kNoRoom;
    std::string name;
    Str kind = Str::RoomOther;
    bool suggested = false;   // the object's room flags suggest this kind of room (when moving:
                              // the room it comes from)
    std::vector<Spot> spots;  // best first
    // What was left out, for "no spot" explanations.
    int engineRefused = 0;
    int commonError = 0;      // the engine's most frequent reason
    int unusable = 0;         // fits, but no Sim could get to it
    int blocksDoor = 0;
    int blocksOthers = 0;
    int blockedObject = 0;    // an object that would become unusable (id)
};

// Evaluates the held object on its floor. Lifts it out of the world (the buy tool puts it back
// on its next mouse move) and leaves it facing as before.
std::vector<Area> FindSpots();

// "junto a la pared norte, mirando al sur, junto a Nevera Porcina, a 3 casillas al este"; the
// parts one by one for the details buffer.
std::string Describe(const Spot& spot);
std::vector<std::string> DescribeParts(const Spot& spot);
// Why an area has no spot: the engine's reason, or what the mod's checks found.
std::string WhyNone(const Area& area);

}  // namespace nav::placement
