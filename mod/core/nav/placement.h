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
    int serves = 0;         // a seat on a use tile of this object, facing it (a computer's chair)
    bool servesHasSeat = false;  // ... which another seat serves already
    int cutOff = 0;         // floor tiles of the area it would cut off from the rest
    // Doors and windows: they stand in the wall on their back side, between two rooms.
    bool portal = false;
    int roomHere = game::kNoRoom;    // the room of the tile it stands on
    int roomBehind = game::kNoRoom;  // the room on the other side of that wall
    bool firstDoor = false;          // one of the rooms has no door yet
    // Stairs: the engine direction they climb (-1 unknown) and the upper floor tile they reach.
    bool stairs = false;
    int climb = -1;
    game::Tile bottom, top;
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
// Where an object about to be moved stands now (before the pick-up puts it under the pointer):
// the room it comes from and the distances of its spots go by this.
void SetMoveOrigin(game::Tile tile, int level);
// After placing: how the object is used sitting down. For a seat, what it serves (the object it
// faces on whose use tile it stands: "Computadora"); for anything else, the seat in front of it
// on one of its use tiles. Empty when none.
std::string SeatServesNow(game::Ptr seat);
std::string ServedByNow(game::Ptr object);
// Whether Sims can get to a placed object to use it: 1 yes, 2 no (its use tiles are cut off),
// 0 unknown (it has no use tiles, or they are all taken by furniture).
int CanUseNow(game::Ptr object);

// "junto a la pared norte, mirando al sur, junto a Nevera Porcina, a 3 casillas al este"; the
// parts one by one for the details buffer.
std::string Describe(const Spot& spot);
std::vector<std::string> DescribeParts(const Spot& spot);
// Why an area has no spot: the engine's reason, or what the mod's checks found.
std::string WhyNone(const Area& area);

// What new walls would do to the floor, measured from the lot's entrance: the objects a Sim could
// use before and could not after
// (their names, each once), and the floor tiles Sims could reach before and could not after
// (an enclosed new room with no door counts here). An edge is a tile and kWallNorth or
// kWallWest of it (the neighbor's opposite edge is set too).
struct WallEdgeAdd {
    game::Tile tile;
    uint32_t edge = 0;
};
struct WallCheck {
    std::vector<std::string> blockedObjects;
    int cutOffTiles = 0;
    int doorsBlocked = 0;  // walls across the tile in front of or behind a door
    int stairsBlocked = 0; // the same across the way onto or off a staircase
    std::vector<std::string> trappedSims;  // Sims who could no longer get out to the lot's entrance
};
WallCheck CheckNewWalls(int level, const std::vector<WallEdgeAdd>& edges);
// Whether Sims can use a door standing on this tile: what is said after placing one, since nothing
// else tells a player who cannot see it. 0 usable; 1 something stands in its way (`blocker`); 2 no
// way to it from the lot's entrance.
int CheckDoor(game::Tile tile, int direction, int level, std::string* blocker);
std::string RoomNameOf(int room);  // "baño", "exterior", "habitación vacía, 20 casillas"
// "luz 80 %": the room's light as the game scores it (windows, lamps, daylight outdoors), which
// counts toward its Room rating (a tester); empty when unknown.
std::string RoomLight(int room);
// The room's name with its light: "baño, luz 80 %".
std::string RoomWithLight(int room);
Str CompassWord(int compass);      // 0 north ... 7 north-west
// What Sims cannot reach from the lot's entrance on a floor now: the objects they could not use
// (names, each once) and the rooms with floor they could not get into.
struct ReachReport {
    std::vector<std::string> unusableObjects;
    std::vector<int> unusableIds;  // their object ids
    std::vector<int> closedRooms;
};
ReachReport CheckReach(int level);

// The same for tiles that would become impassable (a pool dug there).
WallCheck CheckBlockedTiles(int level, const std::vector<game::Tile>& tiles);

}  // namespace nav::placement
