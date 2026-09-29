#include "house_plan.h"

#include <algorithm>
#include <map>
#include <set>
#include <tuple>

#include "../game/addresses.h"
#include "../game/house_file.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../util/log.h"

namespace nav::plan {
namespace {

using game::housefile::House;
namespace addr = game::addr;

// Each tile is four quarters, by the edge they touch: 0 toward y-1 (north), 1 toward x+1, 2 toward
// y+1, 3 toward x-1. A diagonal wall keeps two of them apart from the other two.
constexpr int kQuarterEdge[4] = {game::kWallNorth, game::kWallEast, game::kWallSouth, game::kWallWest};
constexpr int kStepX[4] = {0, 1, 0, -1}, kStepY[4] = {-1, 0, 1, 0};
constexpr uint8_t kDiagonalA = 0x10;  // (x+1, y) to (x, y+1): north and west quarters on one side
constexpr uint8_t kDiagonalB = 0x20;  // the other one: north and east on one side
constexpr int kKindPortal = 8;        // objt kind of door, window and stair parts
constexpr int kKindPerson = 2;

bool SameSide(uint8_t walls, int a, int b) {
    if (walls & kDiagonalA) return (a == 0 || a == 3) == (b == 0 || b == 3);
    if (walls & kDiagonalB) return (a == 0 || a == 1) == (b == 0 || b == 1);
    return true;
}

// Connected areas of one floor, over quarters.
struct Areas {
    int n = 0;
    std::vector<int> area;  // per quarter: (y * n + x) * 4 + q
    int count = 0;
    int At(int x, int y, int q) const { return area[((size_t)y * n + x) * 4 + q]; }
};

Areas FindAreas(const House& h, int level) {
    Areas a;
    a.n = h.size;
    a.area.assign((size_t)a.n * a.n * 4, -1);
    std::vector<int> stack;
    for (int start = 0; start < (int)a.area.size(); start++) {
        if (a.area[start] >= 0) continue;
        int id = a.count++;
        a.area[start] = id;
        stack.push_back(start);
        while (!stack.empty()) {
            int cur = stack.back();
            stack.pop_back();
            int q = cur % 4, tile = cur / 4, x = tile % a.n, y = tile / a.n;
            uint8_t walls = h.Walls(level, x, y);
            auto visit = [&](int next) {
                if (a.area[next] < 0) a.area[next] = id, stack.push_back(next);
            };
            for (int r = 0; r < 4; r++)
                if (r != q && SameSide(walls, q, r)) visit(tile * 4 + r);
            if (walls & kQuarterEdge[q]) continue;
            int nx = x + kStepX[q], ny = y + kStepY[q];
            if (nx < 0 || ny < 0 || nx >= a.n || ny >= a.n) continue;
            // Upper floors: where the floor starts or ends is a room's edge too (the opening over
            // a staircase is a room of its own in the engine's map).
            if (level > 1 && (h.FloorAt(level, x, y) != 0) != (h.FloorAt(level, nx, ny) != 0)) continue;
            visit((ny * a.n + nx) * 4 + (q + 2) % 4);
        }
    }
    return a;
}

// The area a tile belongs to when all its quarters share one (none for a tile a diagonal splits).
int TileArea(const Areas& a, int x, int y) {
    int first = a.At(x, y, 0);
    for (int q = 1; q < 4; q++)
        if (a.At(x, y, q) != first) return -1;
    return first;
}

bool StartsWith(const std::string& s, const char* prefix) { return s.rfind(prefix, 0) == 0; }

// One multi-tile object is one object per tile, sharing their file and OBJD MasterID; the part
// with the catalog strings stands for it, else the one at the lowest tile offset (as
// game::LotObjects picks them).
struct Kinds {
    std::map<std::pair<game::Ptr, uint16_t>, bool> hasStrings;
    std::map<std::pair<game::Ptr, uint16_t>, uint16_t> firstSub;
    std::map<std::pair<game::Ptr, uint16_t>, uint16_t> roomFlags;  // the group's, from any part
};

std::pair<game::Ptr, uint16_t> GroupOf(game::Ptr type) {
    return {game::Rd<game::Ptr>(type + addr::objtype::kResFile), game::TypeObjdField(type, addr::objd::kMasterId)};
}

// Things the overview counts: people, normal objects, food, and type 7 with catalog strings (the
// cheap stove), invisible helpers left out.
bool Counts(game::Ptr type) {
    if (!type) return false;
    uint16_t kind = game::TypeObjdField(type, addr::objd::kObjectType);
    bool catalogued7 = kind == 7 && game::TypeObjdField(type, addr::objd::kCatalogStringsId) != 0;
    if (kind != 2 && kind != 4 && kind != 34 && !catalogued7) return false;
    return !game::IsInvisibleHelperType(type);
}

bool Represents(game::Ptr type, const Kinds& k) {
    if (game::TypeObjdField(type, addr::objd::kMasterId) == 0) return true;
    auto group = GroupOf(type);
    auto strings = k.hasStrings.find(group);
    if (strings != k.hasStrings.end() && strings->second)
        return game::TypeObjdField(type, addr::objd::kCatalogStringsId) != 0;
    auto first = k.firstSub.find(group);
    return first != k.firstSub.end() && game::TypeObjdField(type, addr::objd::kSubIndex) == first->second;
}

}  // namespace

bool Build(int houseNumber, Plan* out) {
    House h;
    if (!game::housefile::Read(houseNumber, &h) || h.size <= 0) return false;
    Plan p;
    p.familySize = game::housefile::FamilySize(houseNumber);

    Kinds kinds;
    for (const auto& o : h.objects) {
        if (!Counts(o.type) || game::TypeObjdField(o.type, addr::objd::kMasterId) == 0) continue;
        auto group = GroupOf(o.type);
        kinds.hasStrings[group] |= game::TypeObjdField(o.type, addr::objd::kCatalogStringsId) != 0;
        uint16_t sub = game::TypeObjdField(o.type, addr::objd::kSubIndex);
        auto first = kinds.firstSub.find(group);
        if (first == kinds.firstSub.end() || sub < first->second) kinds.firstSub[group] = sub;
        if (uint16_t flags = game::TypeObjdField(o.type, addr::objd::kRoomFlags)) kinds.roomFlags[group] = flags;
    }
    std::set<int> represented;  // object ids that stand for their object
    for (const auto& o : h.objects) {
        if (!Counts(o.type) || !Represents(o.type, kinds)) continue;
        represented.insert(o.id);
        if (o.kind != kKindPerson) p.objectNames.push_back(game::TypeInternalName(o.type));
    }

    // Rooms per floor. Graph nodes are (level, area); rooms are the areas with whole tiles.
    struct Node {
        int level, area;
        bool operator<(const Node& o) const { return level != o.level ? level < o.level : area < o.area; }
    };
    std::map<Node, std::set<Node>> links;
    std::map<Node, int> roomIndex;  // -> p.rooms
    std::vector<Node> outdoorsStart;
    std::vector<Node> stairs[game::housefile::kLevels + 1];
    Areas areas[game::housefile::kLevels + 1];
    int lot = h.lotSize > 0 ? h.lotSize : h.size;
    for (int level = 1; level <= game::housefile::kLevels; level++) {
        if (h.floors[level - 1].walls.empty()) continue;
        Areas& a = areas[level] = FindAreas(h, level);
        struct Tally {
            int tiles = 0, floored = 0, outdoors = 0;
            bool edge = false;
        };
        std::vector<Tally> tally(a.count);
        for (int y = 0; y < a.n; y++)
            for (int x = 0; x < a.n; x++) {
                for (int q = 0; q < 4; q++)
                    if (x == 0 || y == 0 || x == a.n - 1 || y == a.n - 1) tally[a.At(x, y, q)].edge = true;
                int area = TileArea(a, x, y);
                if (area < 0 || x >= lot || y >= lot) continue;
                Tally& t = tally[area];
                t.tiles++;
                t.floored += h.FloorAt(level, x, y) != 0;
                t.outdoors += (h.FlagsAt(level, x, y) & game::kTileOutside) != 0;
            }
        int number = 0, outdoors = 0;
        for (int area = 0; area < a.count; area++) {
            const Tally& t = tally[area];
            // Outdoors: the area around the house (it reaches the map's edge), or tiles the game
            // flags as outdoors (a fenced yard).
            bool outside = t.edge || (t.tiles > 0 && t.outdoors * 2 > t.tiles);
            if (outside && level == 1) {
                outdoorsStart.push_back({level, area});
                p.outsideTiles += t.outdoors;
            }
            if (!outside && (t.tiles == 0 || (level > 1 && t.floored == 0))) continue;  // nothing to stand on
            Room r;
            // The engine's numbering: (level - 1) << 10 plus the room's number; outdoor areas
            // after the rooms.
            r.contents.id = (level - 1) << 10 | (outside ? 0x300 + outdoors++ : ++number);
            r.contents.level = level;
            r.contents.tiles = outside ? 0 : t.tiles;
            r.contents.outside = outside;
            roomIndex[{level, area}] = (int)p.rooms.size();
            p.rooms.push_back(r);
        }
    }

    // What stands in each room, and the doors and stairs between them.
    auto isDoor = [](const game::housefile::Object& o) {
        return o.kind == kKindPortal && o.type && StartsWith(game::TypeInternalName(o.type), "Door");
    };
    std::set<std::tuple<int, int, int>> doorTiles;  // (level, x, y) with a door part
    for (const auto& o : h.objects)
        if (o.x >= 0 && isDoor(o)) doorTiles.insert({o.level, o.x, o.y});
    for (const auto& o : h.objects) {
        if (o.x < 0 || !o.type || o.level < 1 || o.level > game::housefile::kLevels) continue;
        const Areas& a = areas[o.level];
        if (a.count == 0 || o.x >= a.n || o.y >= a.n) continue;
        int level = o.level, x = o.x, y = o.y;
        std::string name = game::TypeInternalName(o.type);
        if (o.kind == kKindPortal && (StartsWith(name, "Stair") || StartsWith(name, "Teleporter"))) {
            for (int q = 0; q < 4; q++) stairs[level].push_back({level, a.At(x, y, q)});
            continue;
        }
        if (isDoor(o)) {
            // A door's two parts stand on both sides of its wall: the wall between them opens. A
            // part whose other half is not across a wall opens its tile's only wall.
            uint8_t walls = h.Walls(level, x, y);
            int wallCount = 0;
            for (int q = 0; q < 4; q++) wallCount += (walls & kQuarterEdge[q]) != 0;
            for (int q = 0; q < 4; q++) {
                if (!(walls & kQuarterEdge[q])) continue;
                int nx = x + kStepX[q], ny = y + kStepY[q];
                if (nx < 0 || ny < 0 || nx >= a.n || ny >= a.n) continue;
                if (!doorTiles.count({level, nx, ny}) && wallCount != 1) continue;
                Node from{level, a.At(x, y, q)}, to{level, a.At(nx, ny, (q + 2) % 4)};
                links[from].insert(to);
                links[to].insert(from);
            }
            continue;
        }
        if (!represented.count(o.id) || o.kind == kKindPerson) continue;
        int area = TileArea(a, x, y);
        auto room = roomIndex.find({level, area < 0 ? a.At(x, y, 0) : area});
        if (room == roomIndex.end()) continue;
        rooms::RoomContents& c = p.rooms[room->second].contents;
        uint16_t flags = game::TypeObjdField(o.type, addr::objd::kRoomFlags);
        if (!flags) {
            auto group = kinds.roomFlags.find(GroupOf(o.type));
            if (group != kinds.roomFlags.end()) flags = group->second;
        }
        c.things.push_back({name, flags});
        if (StartsWith(name, "Bed") && name.find("Bedside") == std::string::npos) c.beds++;
        if (name.find("Toilet") != std::string::npos || name.find("Shower") != std::string::npos ||
            name.find("Bathtub") != std::string::npos)
            c.bathObjects++;
    }
    // Every staircase joins the areas its parts stand in, on both floors (the upper landing is
    // in the room above).
    std::vector<Node> allStairs;
    for (const auto& s : stairs) allStairs.insert(allStairs.end(), s.begin(), s.end());
    for (const Node& a : allStairs)
        for (const Node& b : allStairs)
            if (a.level != b.level) links[a].insert(b);

    // Who gets where from the lot's outdoors.
    std::set<Node> reached(outdoorsStart.begin(), outdoorsStart.end());
    std::vector<Node> queue(outdoorsStart.begin(), outdoorsStart.end());
    while (!queue.empty()) {
        Node cur = queue.back();
        queue.pop_back();
        for (const Node& next : links[cur])
            if (reached.insert(next).second) queue.push_back(next);
    }
    for (const auto& [node, index] : roomIndex) p.rooms[index].reachable = reached.count(node) != 0;
    std::stable_sort(p.rooms.begin(), p.rooms.end(),
                     [](const Room& a, const Room& b) { return a.contents.id < b.contents.id; });
    LogVerbose("house plan: House%02d, %d rooms and outdoors, %d outdoor tiles, %d objects, family %d", houseNumber,
               (int)p.rooms.size(), p.outsideTiles, (int)p.objectNames.size(), p.familySize);
    *out = std::move(p);
    return true;
}

}  // namespace nav::plan
