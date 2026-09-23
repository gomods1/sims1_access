#include "house_overview.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../game/house.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
#include "../game/rtti.h"
#include "../game/simantics.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "navigator.h"
#include "placement.h"
#include "room_names.h"
#include "world.h"

namespace nav::house {
namespace {

bool g_buildShown = false;  // the build panel was up at the last tick
game::Ptr g_lot = 0;

std::string Join(const std::vector<std::string>& items) {
    std::string out;
    for (const std::string& s : items) out += (out.empty() ? "" : ", ") + s;
    return out;
}

constexpr int kLikedImpact = 50;

bool Has(const std::string& name, const char* word) { return name.find(word) != std::string::npos; }

// What a household needs, by the objects' internal (English, resource) names: stable across
// languages and expansions ("Toilet - Cheap", "Fridge - Moderate", "Alarm - Smoke").
struct Essentials {
    bool toilet = false, wash = false, fridge = false, stove = false, phone = false, smokeAlarm = false;
    int bedPlaces = 0;
};

Essentials FindEssentials(const std::vector<game::LotObject>& objects) {
    Essentials e;
    for (const game::LotObject& o : objects) {
        if (o.person) continue;
        std::string n = game::ObjectInternalName(o);
        if (Has(n, "Toilet")) e.toilet = true;
        if (Has(n, "Shower") || Has(n, "Bathtub") || (Has(n, "Tub") && !Has(n, "Hot Tub"))) e.wash = true;
        if (Has(n, "Fridge") && !Has(n, "Barbecue")) e.fridge = true;
        if (Has(n, "Stove")) e.stove = true;
        if ((Has(n, "Phone - Table") || Has(n, "Phone - Wall") || Has(n, "Payphone") || Has(n, "Phone - Pay")))
            e.phone = true;
        if (Has(n, "Alarm - Smoke") || Has(n, "Smoke Alarm") || Has(n, "Smoke Detector")) e.smokeAlarm = true;
        if (n.rfind("Bed", 0) == 0 && !Has(n, "Bedside")) e.bedPlaces += Has(n, "Double") ? 2 : 1;
    }
    return e;
}

}  // namespace

void Reset() {
    g_buildShown = false;
    g_lot = 0;
}

void Tick() {
    game::Ptr root = game::InputRoot();
    game::Ptr lot = game::LotModule();
    if (lot != g_lot) {
        g_lot = lot;
        g_buildShown = false;
    }
    bool shown = lot && game::IsKindOf(root, "cDDDSimsView") && game::BuildPanelShown(root);
    if (shown && !g_buildShown) SayOverview();
    g_buildShown = shown;
}

void SayOverview() {
    if (!game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    std::vector<game::LotObject> objects = game::LotObjects();
    std::vector<rooms::NamedRoom> named = rooms::NameRooms(objects);
    int n = game::GridSize();

    // Rooms by floor, with their tiles counted on the room map.
    std::map<int, std::map<int, int>> floors;  // level -> room -> tiles
    int outsideTiles = 0;
    std::map<int, int> outsideOf;  // room -> 1 outdoors, 0 indoors, -1 unknown (looked up once)
    for (int level = 1; level <= 2; level++)
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++) {
                int room = game::RoomAt(level, x, y);
                if (room == game::kNoRoom) continue;
                auto known = outsideOf.find(room);
                if (known == outsideOf.end()) {
                    game::Room info;
                    known = outsideOf.emplace(room, game::GetRoom(room, &info) ? (info.outside ? 1 : 0) : -1).first;
                }
                if (known->second < 0) continue;
                if (known->second) {
                    if (level == 1) outsideTiles++;
                    continue;
                }
                floors[level][room]++;
            }
    // Inside a floor's list the floor is said once, not after every room.
    auto label = [&](int room, int tiles, int level) {
        std::string name = rooms::NameOf(named, room);
        std::string floor = ", " + util::Format(S(Str::FloorNumber), level);
        if (name.size() > floor.size() && name.compare(name.size() - floor.size(), floor.size(), floor) == 0)
            name.erase(name.size() - floor.size());
        return name.empty() ? util::Format(S(Str::RoomEmpty), tiles) : util::Format(S(Str::HouseRoomItem), name.c_str(), tiles);
    };

    std::vector<std::string> lines;
    int indoor = 0, roomCount = 0;
    for (const auto& [level, rooms] : floors)
        for (const auto& [room, tiles] : rooms) indoor += tiles, roomCount++;
    if (!roomCount) {
        lines.push_back(util::Format(S(Str::HouseNone), outsideTiles));
    } else {
        if (floors.size() > 1)
            lines.push_back(util::Format(S(Str::HouseSummary), (int)floors.size(), roomCount, indoor, outsideTiles));
        else if (roomCount > 1)
            lines.push_back(util::Format(S(Str::HouseSummaryOneFloor), roomCount, indoor, outsideTiles));
        else
            lines.push_back(util::Format(S(Str::HouseSummaryOneRoom), indoor, outsideTiles));
        for (const auto& [level, rooms] : floors) {
            std::vector<std::string> items;
            for (const auto& [room, tiles] : rooms) items.push_back(label(room, tiles, level));
            lines.push_back(util::Format(S(Str::HouseFloorRooms), level, Join(items).c_str()));
        }
    }
    int spoken = (int)lines.size();

    int ratings[game::kHouseRatingCount];
    if (game::HouseRatings(ratings)) {
        static const Str kNames[game::kHouseRatingCount] = {Str::HouseRateSize, Str::HouseRateFurnishings, Str::HouseRateYard,
                                                            Str::HouseRateUpkeep, Str::HouseRateLayout};
        std::vector<std::string> parts;
        for (int i = 0; i < game::kHouseRatingCount; i++) parts.push_back(util::Format(S(Str::HouseRating), S(kNames[i]), ratings[i]));
        lines.push_back(util::Format(S(Str::HouseRatings), Join(parts).c_str()));
    }

    // What is missing.
    Essentials e = FindEssentials(objects);
    std::vector<std::string> missing;
    if (!e.toilet) missing.push_back(S(Str::NeedToilet));
    if (!e.wash) missing.push_back(S(Str::NeedWash));
    if (!e.fridge) missing.push_back(S(Str::NeedFridge));
    if (!e.stove) missing.push_back(S(Str::NeedStove));
    if (!e.phone) missing.push_back(S(Str::NeedPhone));
    if (!e.smokeAlarm) missing.push_back(S(Str::NeedSmokeAlarm));
    int sims = (int)game::FamilyNeighborIds().size();
    std::vector<std::string> problems;
    if (!missing.empty()) problems.push_back(util::Format(S(Str::HouseMissing), Join(missing).c_str()));
    if (sims > 0 && e.bedPlaces < sims) problems.push_back(util::Format(S(Str::HouseBeds), e.bedPlaces, sims));

    // What is wrong: rooms nobody can get into, objects nobody can reach, dark or disliked rooms.
    for (int level = 1; level <= 2; level++) {
        if (!floors.count(level)) continue;
        placement::ReachReport r = placement::CheckReach(level);
        std::vector<std::string> closed;
        for (int room : r.closedRooms)
            if (floors[level].count(room)) closed.push_back(label(room, floors[level][room], level));
        if (!closed.empty()) problems.push_back(util::Format(S(Str::HouseClosedRooms), Join(closed).c_str()));
        if (!r.unusableObjects.empty()) problems.push_back(util::Format(S(Str::HouseUnreachable), Join(r.unusableObjects).c_str()));
    }
    std::vector<std::string> dark, disliked, liked;
    for (const auto& [level, rooms] : floors)
        for (const auto& [room, tiles] : rooms) {
            game::Room info;
            if (!game::GetRoom(room, &info)) continue;
            LogVerbose("house: room %d (%d tiles): window light %d, object light %d, lamps %d, dirt %d, impact %d, good %d",
                       room, tiles, info.windowLight, info.objectLight, info.lamps, info.dirt, info.impact, info.goodObjects);
            std::string name = label(room, tiles, level);
            if (floors.size() > 1) name += ", " + util::Format(S(Str::FloorNumber), level);
            if (info.windowLight <= 0 && info.objectLight <= 0 && info.lamps <= 0) dark.push_back(name);
            // The room record's impact (a furnished house read 96 and 123 for its kitchen and
            // living room, 1 to 40 for plainer rooms): only a clear margin counts as liked.
            if (info.impact < 0) disliked.push_back(name);
            else if (info.impact >= kLikedImpact) liked.push_back(name);
        }
    if (!dark.empty()) problems.push_back(util::Format(S(Str::HouseDark), Join(dark).c_str()));
    if (!disliked.empty()) problems.push_back(util::Format(S(Str::HouseDisliked), Join(disliked).c_str()));
    std::vector<std::string> info;
    if (!liked.empty()) info.push_back(util::Format(S(Str::HouseLiked), Join(liked).c_str()));

    lines.insert(lines.end(), problems.begin(), problems.end());
    lines.insert(lines.end(), info.begin(), info.end());
    if (problems.empty()) lines.push_back(S(Str::HouseAllGood));
    buffers::SetDetails(lines);
    std::string say;
    for (int i = 0; i < spoken; i++) say += (say.empty() ? "" : ". ") + lines[i];
    say += ". " + (problems.empty()        ? std::string(S(Str::HouseAllGood))
                   : problems.size() == 1 ? std::string(S(Str::HouseToCheckOne))
                                          : util::Format(S(Str::HouseToCheck), (int)problems.size()));
    speech::Say(say);
}

}  // namespace nav::house
