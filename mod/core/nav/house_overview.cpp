#include "house_overview.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../game/house.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/neighborhood.h"
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
#include "house_plan.h"
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

Essentials FindEssentials(const std::vector<std::string>& names) {
    Essentials e;
    for (const std::string& n : names) {
        if (Has(n, "Toilet")) e.toilet = true;
        if (Has(n, "Shower") || Has(n, "Bathtub") || (Has(n, "Tub") && !Has(n, "Hot Tub"))) e.wash = true;
        if ((Has(n, "Fridge") && !Has(n, "Barbecue")) || Has(n, "Pantry")) e.fridge = true;  // a pantry feeds as well
        if (Has(n, "Stove")) e.stove = true;
        if ((Has(n, "Phone - Table") || Has(n, "Phone - Wall") || Has(n, "Payphone") || Has(n, "Phone - Pay")))
            e.phone = true;
        if (Has(n, "Alarm - Smoke") || Has(n, "Smoke Alarm") || Has(n, "Smoke Detector")) e.smokeAlarm = true;
        if (n.rfind("Bed", 0) == 0 && !Has(n, "Bedside")) e.bedPlaces += Has(n, "Double") ? 2 : 1;
    }
    return e;
}

// The house's floors: room id -> tiles per level, outdoor tiles, the rooms' names.
struct Layout {
    std::map<int, std::map<int, int>> floors;
    int outsideTiles = 0;
    std::vector<rooms::NamedRoom> named;

    // Inside a floor's list the floor is said once, not after every room.
    std::string Label(int room, int tiles, int level) const {
        std::string name = rooms::NameOf(named, room);
        std::string floor = ", " + util::Format(S(Str::FloorNumber), level);
        if (name.size() > floor.size() && name.compare(name.size() - floor.size(), floor.size(), floor) == 0)
            name.erase(name.size() - floor.size());
        return name.empty() ? util::Format(S(Str::RoomEmpty), tiles) : util::Format(S(Str::HouseRoomItem), name.c_str(), tiles);
    }
    // The same with the floor when the house has several (lists that mix floors).
    std::string LabelWithFloor(int room, int tiles, int level) const {
        std::string name = Label(room, tiles, level);
        if (floors.size() > 1) name += ", " + util::Format(S(Str::FloorNumber), level);
        return name;
    }
};

// The summary and one line per floor.
std::vector<std::string> LayoutLines(const Layout& l) {
    std::vector<std::string> lines;
    int indoor = 0, roomCount = 0;
    for (const auto& [level, rooms] : l.floors)
        for (const auto& [room, tiles] : rooms) indoor += tiles, roomCount++;
    if (!roomCount) {
        lines.push_back(util::Format(S(Str::HouseNone), l.outsideTiles));
        return lines;
    }
    if (l.floors.size() > 1)
        lines.push_back(util::Format(S(Str::HouseSummary), (int)l.floors.size(), roomCount, indoor, l.outsideTiles));
    else if (roomCount > 1)
        lines.push_back(util::Format(S(Str::HouseSummaryOneFloor), roomCount, indoor, l.outsideTiles));
    else
        lines.push_back(util::Format(S(Str::HouseSummaryOneRoom), indoor, l.outsideTiles));
    for (const auto& [level, rooms] : l.floors) {
        std::vector<std::string> items;
        for (const auto& [room, tiles] : rooms) items.push_back(l.Label(room, tiles, level));
        lines.push_back(util::Format(S(Str::HouseFloorRooms), level, Join(items).c_str()));
    }
    return lines;
}

// What a household needs and the house lacks, by the internal names of its objects.
std::vector<std::string> MissingLines(const std::vector<std::string>& objectNames, int sims) {
    Essentials e = FindEssentials(objectNames);
    std::vector<std::string> missing, lines;
    if (!e.toilet) missing.push_back(S(Str::NeedToilet));
    if (!e.wash) missing.push_back(S(Str::NeedWash));
    if (!e.fridge) missing.push_back(S(Str::NeedFridge));
    if (!e.stove) missing.push_back(S(Str::NeedStove));
    if (!e.phone) missing.push_back(S(Str::NeedPhone));
    if (!e.smokeAlarm) missing.push_back(S(Str::NeedSmokeAlarm));
    if (!missing.empty()) lines.push_back(util::Format(S(Str::HouseMissing), Join(missing).c_str()));
    if (sims > 0 && e.bedPlaces < sims) lines.push_back(util::Format(S(Str::HouseBeds), e.bedPlaces, sims));
    return lines;
}

// An overview: its lines (the details buffer), how many of the first are spoken, and how many
// are problems.
struct Overview {
    std::vector<std::string> lines;
    int spoken = 0;
    int problems = 0;
};

void Speak(const Overview& o) {
    std::string say;
    for (int i = 0; i < o.spoken && i < (int)o.lines.size(); i++) say += (say.empty() ? "" : ". ") + o.lines[i];
    say += ". " + (o.problems == 0   ? std::string(S(Str::HouseAllGood))
                   : o.problems == 1 ? std::string(S(Str::HouseToCheckOne))
                                     : util::Format(S(Str::HouseToCheck), o.problems));
    speech::Say(say);
}

// A house of the neighborhood from its file: the overview's floors and rooms, what is missing,
// rooms with no way in.
bool FromFile(int houseNumber, Overview* out) {
    plan::Plan p;
    if (!plan::Build(houseNumber, &p)) return false;
    Layout layout;
    layout.outsideTiles = p.outsideTiles;
    std::vector<rooms::RoomContents> named;  // the rooms that hold something, as NameRooms wants them
    for (const plan::Room& r : p.rooms) {
        if (!r.contents.outside) layout.floors[r.contents.level][r.contents.id] = r.contents.tiles;
        if (!r.contents.things.empty()) named.push_back(r.contents);
    }
    layout.named = rooms::NameRooms(named);
    std::vector<std::string> lines = LayoutLines(layout);
    int spoken = (int)lines.size();
    std::vector<std::string> problems;
    // A community lot needs no beds or fridge, and an empty lot has nothing to miss yet.
    if (!layout.floors.empty() && !game::IsCommunityLot(houseNumber)) problems = MissingLines(p.objectNames, p.familySize);
    for (const auto& [level, rooms] : layout.floors) {
        std::vector<std::string> closed;
        for (const plan::Room& r : p.rooms)
            if (r.contents.level == level && !r.contents.outside && !r.reachable)
                closed.push_back(layout.Label(r.contents.id, r.contents.tiles, level));
        if (!closed.empty()) problems.push_back(util::Format(S(Str::HouseClosedRooms), Join(closed).c_str()));
    }
    lines.insert(lines.end(), problems.begin(), problems.end());
    if (problems.empty()) lines.push_back(S(Str::HouseAllGood));
    *out = {lines, spoken, (int)problems.size()};
    for (const std::string& line : lines) LogVerbose("house plan: House%02d: %s", houseNumber, line.c_str());
    return true;
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
    // The neighborhood (or a travel map): the focused house, from its file. The lot module
    // outlives the lot there, so it says nothing about where the player is.
    int number = FocusedLotNumber();
    if (number >= 0) {
        Overview o;
        if (!FromFile(number, &o)) {
            speech::Say(S(Str::NotOnLot));
            return;
        }
        buffers::SetDetails(o.lines);
        Speak(o);
        return;
    }
    if (!game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    std::vector<game::LotObject> objects = game::LotObjects();
    Layout layout;
    layout.named = rooms::NameRooms(objects);
    int n = game::GridSize();

    // Rooms by floor, with their tiles counted on the room map.
    std::map<int, int> outsideOf;  // room -> 1 outdoors, 0 indoors, -1 unknown (looked up once)
    for (int level = 1; level <= 2; level++)
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++) {
                int room = game::RoomAt(level, x, y);
                if (room == game::kNoRoom) continue;
                // Upstairs, tiles with no floor (the opening over a staircase, a room of its own
                // for the engine) are nowhere to stand.
                if (level > 1 && !game::HasFloor(level, x, y)) continue;
                auto known = outsideOf.find(room);
                if (known == outsideOf.end()) {
                    game::Room info;
                    known = outsideOf.emplace(room, game::GetRoom(room, &info) ? (info.outside ? 1 : 0) : -1).first;
                }
                if (known->second < 0) continue;
                if (known->second) {
                    if (level == 1) layout.outsideTiles++;
                    continue;
                }
                layout.floors[level][room]++;
            }
    auto& floors = layout.floors;
    auto label = [&](int room, int tiles, int level) { return layout.Label(room, tiles, level); };

    std::vector<std::string> lines = LayoutLines(layout);
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
    std::vector<std::string> names;
    for (const game::LotObject& o : objects)
        if (!o.person) names.push_back(game::ObjectInternalName(o));
    std::vector<int> family = game::FamilyNeighborIds();
    int sims = (int)family.size();
    for (game::Ptr person : game::LotPeople()) {  // pets need no bed
        game::SimKind kind = game::KindOfSim(person);
        if ((kind == game::SimKind::Dog || kind == game::SimKind::Cat) &&
            std::find(family.begin(), family.end(), game::SimNeighborId(person)) != family.end())
            sims--;
    }
    std::vector<std::string> problems = MissingLines(names, sims);

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
            std::string name = layout.LabelWithFloor(room, tiles, level);
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
    Speak({lines, spoken, (int)problems.size()});
}

std::vector<std::string> FileOverview(int houseNumber) {
    Overview o;
    return FromFile(houseNumber, &o) ? o.lines : std::vector<std::string>{};
}

}  // namespace nav::house
