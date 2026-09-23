// Rooms as the engine computes them: walls split each floor into rooms, and each floor's outdoors
// is a room too. Every tile of the lot knows its room. The engine numbers rooms per floor
// (upper floors add level << 10); it does not name them.
#pragma once

#include "memory.h"

namespace game {

constexpr int kNoRoom = -1;

int RoomAt(int level, int tileX, int tileY);  // kNoRoom when the tile has none

struct Room {
    int id = kNoRoom;
    int tiles = 0;        // floor area
    bool outside = false;
    int beds = 0;         // the engine's own counters (the house panel counts bedrooms and
    int bathObjects = 0;  // bathrooms with them)
};
bool GetRoom(int id, Room* out);
// The room's tiles counted on the room map (its floor is id >> 10, plus 1). The record's own
// area can be far off for a room just built (34 for a new 5 by 4 room, read live).
int RoomTileCount(int id);

}  // namespace game
