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

bool NameHas(const Thing& t, std::initializer_list<const char*> words) {
    for (const char* w : words)
        if (t.internalName.find(w) != std::string::npos) return true;
    return false;
}

// Cooking gear, by internal name (constant English resource names): what makes a kitchen. The
// kitchen room flag does not (wet bars, counters and coffee makers carry it too).
bool IsCookingFixture(const Thing& t) { return NameHas(t, {"Fridge", "Refrigerator", "Stove", "Oven", "Microwave"}); }

// Community lots: the engine counts no bath objects in their toilet rooms, and a shop's pet beds
// and display cases carry bedroom and living room flags.
bool IsBathFixture(const Thing& t) { return NameHas(t, {"Toilet", "Shower", "Bathtub"}); }
bool IsShopFixture(const Thing& t) { return NameHas(t, {"Cash Register", "Display"}); }
bool IsPetThing(const Thing& t) { return NameHas(t, {"Pet - "}); }

// The engine's counters decide bathrooms and bedrooms (as the house panel counts them), cooking
// gear makes a kitchen, and the
// rest goes by what its objects are meant for. Objects meant for more than two kinds of room
// (plants, paintings, lamps) do not vote.
Str Kind(const RoomContents& room) {
    if (room.outside) return Str::RoomOutside;
    if (room.bathObjects > 0) return Str::RoomBathroom;
    if (room.beds > 0) return Str::RoomBedroom;
    for (const Thing& t : room.things)
        if (IsBathFixture(t)) return Str::RoomBathroom;
    for (const Thing& t : room.things)
        if (IsCookingFixture(t)) return Str::RoomKitchen;
    for (const Thing& t : room.things)
        if (IsShopFixture(t)) return Str::RoomShop;
    struct Vote {
        uint16_t flag;
        Str kind;
        double score;
    } votes[] = {{kRoomLiving, Str::RoomLiving, 0},
                 {kRoomDining, Str::RoomDining, 0},
                 {kRoomStudy, Str::RoomStudy, 0},
                 {kRoomBedroom, Str::RoomBedroom, 0}};
    for (const Thing& t : room.things) {
        if (IsPetThing(t)) continue;
        int n = 0;
        for (const Vote& v : votes) n += (t.roomFlags & v.flag) != 0;
        if (n == 0 || n > 2) continue;
        for (Vote& v : votes)
            if (t.roomFlags & v.flag) v.score += 1.0 / n;
    }
    const Vote* best = nullptr;
    for (const Vote& v : votes)
        if (v.score > 0 && (!best || v.score > best->score)) best = &v;
    return best ? best->kind : Str::RoomOther;
}

}  // namespace

std::vector<NamedRoom> NameRooms(const std::vector<LotObject>& objects) {
    std::map<int, RoomContents> contents;  // by room id, ascending
    for (const LotObject& o : objects) {
        if (o.room == game::kNoRoom) continue;
        auto known = contents.find(o.room);
        if (known == contents.end()) {
            game::Room room;
            if (!game::GetRoom(o.room, &room)) continue;
            RoomContents c;
            c.id = o.room;
            c.level = o.level;
            c.tiles = room.tiles;
            c.outside = room.outside;
            c.beds = room.beds;
            c.bathObjects = room.bathObjects;
            known = contents.emplace(o.room, c).first;
        }
        if (!o.person) known->second.things.push_back({game::ObjectInternalName(o), o.roomFlags});
    }
    std::vector<RoomContents> rooms;
    for (auto& entry : contents) rooms.push_back(std::move(entry.second));
    return NameRooms(rooms);
}

std::vector<NamedRoom> NameRooms(const std::vector<RoomContents>& rooms) {
    std::set<int> levels;
    for (const RoomContents& r : rooms) levels.insert(r.level);
    std::map<std::pair<Str, int>, int> kindCount;  // by kind and floor
    std::vector<Str> kinds;
    for (const RoomContents& r : rooms) {
        kinds.push_back(Kind(r));
        kindCount[{kinds.back(), r.level}]++;
    }
    // "dormitorio 1", "dormitorio 2" when a kind repeats on a floor; the floor when the house
    // has several.
    std::map<std::pair<Str, int>, int> seen;
    std::vector<NamedRoom> out;
    for (size_t i = 0; i < rooms.size(); i++) {
        const RoomContents& r = rooms[i];
        std::string name = S(kinds[i]);
        if (kindCount[{kinds[i], r.level}] > 1) name += util::Format(" %d", ++seen[{kinds[i], r.level}]);
        if (levels.size() > 1) name += ", " + util::Format(S(Str::FloorNumber), r.level);
        out.push_back({r.id, name, r.tiles, kinds[i]});
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
