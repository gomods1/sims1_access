// Houses the game has not loaded, read from their files (<neighborhood>\Houses\HouseNN.iff) as
// the game last saved them: the plan of each floor (walls, floors, what stands on each tile) and
// every object with its type. The format is in STRUCTS.md "House files". The neighborhood's own
// file gives the size of the family living in each house.
#pragma once

#include <cstdint>
#include <vector>

#include "memory.h"

namespace game::housefile {

constexpr int kLevels = 2;  // the game builds on two floors

struct Floor {
    // 8 bytes per tile; byte 0 is the edge mask WallsAt returns (1 toward y-1, 2 toward x-1,
    // 4 toward y+1, 8 toward x+1, 0x10 the diagonal from the tile's (x+1, y) corner to its
    // (x, y+1) corner, 0x20 the other one). Both tiles along an edge have its bit.
    std::vector<uint8_t> walls;
    std::vector<uint8_t> floors;    // floor pattern per tile, 0 = none (grass on the ground floor)
    std::vector<uint8_t> flags;     // the lot grid's tile flags (kTileOutside...; game/lotgrid.h)
};

struct Object {
    int id = 0;
    uint32_t guid = 0;
    int kind = 0;   // OBJD object type as the file lists it: 2 person, 4 normal, 7 system, 8 door,
                    // window or stair part, 12 inventory item
    Ptr type = 0;   // the game's object type record (game/lot.h Type*), 0 when the game has none
    int x = -1, y = -1, level = 0;  // its tile; -1 when not placed (inventory, controllers)
    int container = 0;              // the object it stands on or in (a lamp's table), 0 none
};

struct House {
    int size = 0;     // tiles per side of the maps (64)
    int lotSize = 0;  // tiles per side of the lot (as GridSize: 52 for the Goths' lot)
    Floor floors[kLevels];
    std::vector<Object> objects;  // every object of the lot, Sims and invisible controllers too
    uint8_t Walls(int level, int x, int y) const;   // edge mask; 0 outside the map
    uint8_t FloorAt(int level, int x, int y) const;
    uint8_t FlagsAt(int level, int x, int y) const;
    const Object* Find(int id) const;
};

// The house with this number in the current neighborhood; false when its file is missing or
// not in the format described (the old IFF 2.0 templates).
bool Read(int number, House* out);

// The Sims of the family living in the house (pets left out: they need no bed) from the
// neighborhood's file; 0 when nobody lives there, -1 when unknown.
int FamilySize(int houseNumber);

// The game's object type for a GUID: every object the game can load is in its ObjectFolder from
// startup, so this works on any screen. 0 when unknown.
Ptr ObjectType(uint32_t guid);

}  // namespace game::housefile
