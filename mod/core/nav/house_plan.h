// A house's plan from its file (game/house_file.h), for houses the game has not loaded (the
// neighborhood): its rooms found the way the engine finds them, what stands in them, and which
// ones nobody can get into. Walls split each floor into rooms; on an upper floor so does the
// edge of the floor; a diagonal wall splits its tile in halves that count for no room (the
// engine's room map leaves them out too). Doors and stairs join rooms; a room that cannot be
// reached from the ground floor's outdoors through them has no way in.
#pragma once

#include <string>
#include <vector>

#include "room_names.h"

namespace nav::plan {

struct Room {
    rooms::RoomContents contents;  // id ((level - 1) << 10 | n, as the engine numbers rooms), tiles...
    bool reachable = false;
};

struct Plan {
    std::vector<Room> rooms;                // rooms with floor, then each floor's outdoors
    int outsideTiles = 0;                   // the ground floor's outdoor tiles on the lot
    std::vector<std::string> objectNames;   // internal names of every object (one per multi-tile
                                            // object, Sims and invisible helpers left out)
    int familySize = -1;                    // Sims of the family living there (no pets), -1 unknown
};

// false when the house has no readable file.
bool Build(int houseNumber, Plan* out);

}  // namespace nav::plan
