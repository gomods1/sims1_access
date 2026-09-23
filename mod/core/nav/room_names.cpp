#include "room_names.h"

#include <map>
#include <set>

#include "../util/strings.h"
#include "../util/text.h"

namespace nav::rooms {
namespace {

using game::LotObject;

// OBJD room flags (the rooms buy mode suggests an object for).
constexpr uint16_t kRoomBedroom = 0x02;
constexpr uint16_t kRoomLiving = 0x08;
constexpr uint16_t kRoomDining = 0x20;
constexpr uint16_t kRoomStudy = 0x80;

// Cooking gear, by internal name (constant English resource names): what makes a kitchen. The
// kitchen room flag does not (wet bars, counters and coffee makers carry it too).
bool IsCookingFixture(const LotObject& o) {
    static const char* const kWords[] = {"Fridge", "Refrigerator", "Stove", "Oven", "Microwave"};
    std::string name = game::ObjectInternalName(o);
    for (const char* w : kWords)
        if (name.find(w) != std::string::npos) return true;
    return false;
}

bool NameHas(const LotObject& o, std::initializer_list<const char*> words) {
    std::string name = game::ObjectInternalName(o);
    for (const char* w : words)
        if (name.find(w) != std::string::npos) return true;
    return false;
}

// Community lots: the engine counts no bath objects in their toilet rooms, and a shop's pet beds
// and display cases carry bedroom and living room flags.
bool IsBathFixture(const LotObject& o) { return NameHas(o, {"Toilet", "Shower", "Bathtub"}); }
bool IsShopFixture(const LotObject& o) { return NameHas(o, {"Cash Register", "Display"}); }
bool IsPetThing(const LotObject& o) { return NameHas(o, {"Pet - "}); }

// The engine's counters decide bathrooms and bedrooms (as the house panel counts them), cooking
// gear makes a kitchen, and the
// rest goes by what its objects are meant for. Objects meant for more than two kinds of room
// (plants, paintings, lamps) do not vote.
Str Kind(const game::Room& room, const std::vector<const LotObject*>& contents) {
    if (room.outside) return Str::RoomOutside;
    if (room.bathObjects > 0) return Str::RoomBathroom;
    if (room.beds > 0) return Str::RoomBedroom;
    for (const LotObject* o : contents)
        if (!o->person && IsBathFixture(*o)) return Str::RoomBathroom;
    for (const LotObject* o : contents)
        if (!o->person && IsCookingFixture(*o)) return Str::RoomKitchen;
    for (const LotObject* o : contents)
        if (!o->person && IsShopFixture(*o)) return Str::RoomShop;
    struct Vote {
        uint16_t flag;
        Str kind;
        double score;
    } votes[] = {{kRoomLiving, Str::RoomLiving, 0},
                 {kRoomDining, Str::RoomDining, 0},
                 {kRoomStudy, Str::RoomStudy, 0},
                 {kRoomBedroom, Str::RoomBedroom, 0}};
    for (const LotObject* o : contents) {
        if (o->person || IsPetThing(*o)) continue;
        int n = 0;
        for (const Vote& v : votes) n += (o->roomFlags & v.flag) != 0;
        if (n == 0 || n > 2) continue;
        for (Vote& v : votes)
            if (o->roomFlags & v.flag) v.score += 1.0 / n;
    }
    const Vote* best = nullptr;
    for (const Vote& v : votes)
        if (v.score > 0 && (!best || v.score > best->score)) best = &v;
    return best ? best->kind : Str::RoomOther;
}

}  // namespace

std::vector<NamedRoom> NameRooms(const std::vector<LotObject>& objects) {
    std::map<int, std::vector<const LotObject*>> contents;  // by room id, ascending
    std::set<int> levels;
    for (const LotObject& o : objects) {
        if (o.room == game::kNoRoom) continue;
        contents[o.room].push_back(&o);
        levels.insert(o.level);
    }
    struct Named {
        NamedRoom room;
        Str kind;
        int level;
    };
    std::vector<Named> named;
    std::map<std::pair<Str, int>, int> kindCount;  // by kind and floor
    for (const auto& entry : contents) {
        game::Room room;
        if (!game::GetRoom(entry.first, &room)) continue;
        Str kind = Kind(room, entry.second);
        int level = entry.second.front()->level;
        kindCount[{kind, level}]++;
        named.push_back({{entry.first, "", room.tiles}, kind, level});
    }
    // "dormitorio 1", "dormitorio 2" when a kind repeats on a floor; the floor when the house
    // has several.
    std::map<std::pair<Str, int>, int> seen;
    std::vector<NamedRoom> out;
    for (Named& n : named) {
        std::string name = S(n.kind);
        if (kindCount[{n.kind, n.level}] > 1) name += util::Format(" %d", ++seen[{n.kind, n.level}]);
        if (levels.size() > 1) name += ", " + util::Format(S(Str::FloorNumber), n.level);
        n.room.name = name;
        n.room.kind = n.kind;
        out.push_back(n.room);
    }
    return out;
}

// OBJD room flags: 0x1 kitchen, 0x2 bedroom, 0x4 bathroom, 0x8 living, 0x10 outside, 0x20 dining,
// 0x80 study.
bool Suits(Str kind, uint16_t roomFlags) {
    switch (kind) {
    case Str::RoomKitchen: return roomFlags & 0x01;
    case Str::RoomBedroom: return roomFlags & kRoomBedroom;
    case Str::RoomBathroom: return roomFlags & 0x04;
    case Str::RoomLiving: return roomFlags & kRoomLiving;
    case Str::RoomOutside: return roomFlags & 0x10;
    case Str::RoomDining: return roomFlags & kRoomDining;
    case Str::RoomStudy: return roomFlags & kRoomStudy;
    default: return false;
    }
}

std::string NameOf(const std::vector<NamedRoom>& rooms, int id) {
    for (const NamedRoom& r : rooms)
        if (r.id == id) return r.name;
    return "";
}

}  // namespace nav::rooms
