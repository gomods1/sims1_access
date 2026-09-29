// Names for the lot's rooms. The engine only numbers them; the mod names them from what is in
// them ("cocina", "baño", "dormitorio 2") and adds the floor when the house has more than one.
#pragma once

#include <string>
#include <vector>

#include "../game/lot.h"
#include "../util/strings.h"

namespace nav::rooms {

struct NamedRoom {
    int id = game::kNoRoom;
    std::string name;  // "dormitorio 2", "exterior, piso 2"
    int tiles = 0;
    Str kind = Str::RoomOther;  // what the name says it is (RoomKitchen, RoomOutside...)
};

// What names a room, from the loaded lot or from a house file (nav/house_plan.h).
struct Thing {
    std::string internalName;  // the object's English resource name ("Toilet - Cheap")
    uint16_t roomFlags = 0;    // OBJD room flags
};
struct RoomContents {
    int id = game::kNoRoom;
    int level = 1;
    int tiles = 0;
    bool outside = false;
    int beds = 0, bathObjects = 0;  // the engine's counters (a house file: beds and bath fixtures)
    std::vector<Thing> things;      // Sims left out
};

// Whether an object's OBJD room flags suggest this kind of room.
bool Suits(Str kind, uint16_t roomFlags);

// The rooms these objects are in, named, in room id order.
std::vector<NamedRoom> NameRooms(const std::vector<game::LotObject>& objects);
// The same for rooms described by their contents (the rooms that hold something), in id order.
std::vector<NamedRoom> NameRooms(const std::vector<RoomContents>& rooms);

// The name of one room in such a list; empty when the id is not in it.
std::string NameOf(const std::vector<NamedRoom>& rooms, int id);

}  // namespace nav::rooms
