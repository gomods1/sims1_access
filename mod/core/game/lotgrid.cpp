#include "lotgrid.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "addresses.h"
#include "iff_strings.h"
#include "rtti.h"
#include "../util/text.h"
#include "../util/log.h"

namespace game {
namespace {

Ptr Grid() { return Rd<Ptr>(Va(addr::kWorldArch)); }

// Row y of a per-level map at `mapOffset` in the grid object, 0 when missing.
Ptr MapRow(int mapOffset, int level, int y) {
    if (level < 0 || level > addr::rooms::kMaxLevel || y < 0 || y >= GridSize()) return 0;
    Ptr grid = Grid();
    Ptr map = grid ? Rd<Ptr>(grid + mapOffset + level * 4) : 0;
    Ptr rows = map ? Rd<Ptr>(map + addr::rooms::kMapRows) : 0;
    return rows ? Rd<Ptr>(rows + y * 4) : 0;
}

int16_t Short(Ptr object, int offset) { return Rd<int16_t>(object + offset); }

// One tile toward each engine direction (0xa50f88 x, 0xa50f64 y).
const int kStepX[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int kStepY[8] = {0, 1, 1, 1, 0, -1, -1, -1};

// A unit vector toward an engine direction.
void DirectionVector(int direction, double* dx, double* dy) {
    double length = (direction & 1) ? 1.41421356237 : 1.0;
    *dx = kStepX[direction & 7] / length;
    *dy = kStepY[direction & 7] / length;
}

}  // namespace

int GridSize() {
    int size = Rd<int>(Grid() + addr::grid::kSize);
    return size > 0 && size <= 256 ? size : 0;
}

bool InGrid(int x, int y) {
    int size = GridSize();
    return x >= 1 && y >= 1 && x < size - 1 && y < size - 1;
}

int ObjectIdAt(int level, int x, int y) {
    Ptr row = x >= 0 && x < GridSize() ? MapRow(addr::grid::kObjects, level, y) : 0;
    return row ? Rd<uint16_t>(row + x * 2) : 0;
}

uint8_t TileFlags(int level, int x, int y) {
    Ptr row = x >= 0 && x < GridSize() ? MapRow(addr::grid::kFlags, level, y) : 0;
    return row ? Rd<uint8_t>(row + x) : 0;
}

bool TileHeights(int x, int y, uint8_t out[4]) {
    int n = GridSize();
    if (x < 0 || y < 0 || x >= n || y >= n) return false;
    Ptr map = Rd<Ptr>(Rd<Ptr>(Va(addr::kWorld)) + addr::kWorldTerrain);
    Ptr rows = map ? Rd<Ptr>(map + addr::kMapRows) : 0;
    Ptr row = rows ? Rd<Ptr>(rows + y * 4) : 0;
    return row && Read(row + x * 4, out, 4);
}

bool HasFloor(int level, int x, int y) {
    Ptr row = x >= 0 && x < GridSize() ? MapRow(addr::grid::kFloors, level, y) : 0;
    return row && Rd<uint16_t>(row + x * 2) != 0;
}

int FloorPattern(int level, int x, int y) {
    Ptr row = x >= 0 && x < GridSize() ? MapRow(addr::grid::kFloors, level, y) : 0;
    return row ? Rd<uint16_t>(row + x * 2) : 0;
}

namespace {

// A byte of the pool or pond map (ground floor), 0xff when none.
uint8_t ByteMapAt(int mapOffset, int x, int y) {
    Ptr grid = Grid();
    Ptr map = grid ? Rd<Ptr>(grid + mapOffset) : 0;
    if (!map || x < 0 || y < 0 || x >= Rd<int>(map + 0x08) || y >= Rd<int>(map + 0x0c)) return 0xff;
    Ptr rows = Rd<Ptr>(map + addr::grid::kByteMapRows);
    Ptr row = rows ? Rd<Ptr>(rows + y * 4) : 0;
    return row ? Rd<uint8_t>(row + x, 0xff) : 0xff;
}

// The footstep sound of a floor pattern, from the game's floor styles (a red-black tree: left
// +0, parent +4, right +8, is-nil byte +0xd, key +0x10, value +0x14).
Ptr FloorStyle(int pattern) {
    Ptr map = Rd<Ptr>(Va(addr::grid::kFloorStyles));
    Ptr head = map ? Rd<Ptr>(map) : 0;
    if (!head) return 0;
    Ptr node = Rd<Ptr>(head + 4);  // the root
    for (int depth = 0; node && node != head && !Rd<uint8_t>(node + 0xd) && depth < 64; depth++) {
        int key = Rd<int>(node + 0x10);
        if (key == pattern) return Rd<Ptr>(node + 0x14);
        node = Rd<Ptr>(node + (pattern < key ? 0 : 8));
    }
    return 0;
}

int FloorSound(int pattern) {
    Ptr style = FloorStyle(pattern);
    return style ? Rd<uint8_t>(style + addr::grid::kFloorStyleSound, 2) : 2;
}

}  // namespace

std::string FloorName(int level, int x, int y) {
    int pattern = FloorPattern(level, x, y);
    if (pattern == 0 || pattern == 0xff) return "";
    Ptr style = FloorStyle(pattern);
    std::string name = style ? util::CleanForSpeech(ReadStdStringUtf8(style + addr::grid::kFloorStyleName)) : "";
    // The base game's 29 floors keep no name in their style: Build.iff STR# 130 has them, three
    // entries per floor (price, name, description).
    if (name.empty() && pattern >= 1 && pattern <= addr::grid::kBaseFloors)
        name = util::CleanForSpeech(GameDataString("Build.iff", addr::grid::kBaseFloorStrings, (pattern - 1) * 3 + 1));
    return name;
}

Surface SurfaceAt(int level, int x, int y) {
    if (level == 1) {
        uint8_t pool = ByteMapAt(addr::grid::kPoolMap, x, y);
        if (pool < 0x10) return Surface::Water;
        if (pool == 0x10) return Surface::PoolRim;
        if (ByteMapAt(addr::grid::kPondMap, x, y) != 0xff) return Surface::Water;
    }
    int pattern = FloorPattern(level, x, y);
    if (pattern == 0) return level > 1 ? Surface::NoFloor : Surface::Ground;
    if (pattern == 0xff) return Surface::Tile;  // the half tile beside a diagonal wall
    switch (FloorSound(pattern)) {
    case 0: return Surface::Wood;
    case 1: return Surface::Tile;
    case 3: return Surface::Paving;
    default: return Surface::Carpet;
    }
}

void WallStylesAt(int level, int x, int y, uint16_t out[4]) {
    typedef void*(__thiscall * WallsFn)(void* grid, void* out, const uint8_t* tile);
    out[0] = out[1] = out[2] = out[3] = 0;
    if (!FunctionOk(addr::grid::kWalls) || x < 0 || y < 0 || x >= GridSize() || y >= GridSize()) return;
    Ptr grid = Grid();
    if (!grid || level < 1 || level > addr::rooms::kMaxLevel) return;
    alignas(4) unsigned char info[addr::grid::kWallInfoSize] = {};
    uint8_t tile[4] = {(uint8_t)y, (uint8_t)x, (uint8_t)level, 0};
    if (!Guarded([&] { ((WallsFn)Va(addr::grid::kWalls))((void*)grid, info, tile); })) return;
    uint32_t mask;
    memcpy(&mask, info, sizeof mask);
    const uint32_t bits[4] = {kWallNorth, kWallWest, kWallSouth, kWallEast};
    for (int i = 0; i < 4; i++)
        if (mask & bits[i]) memcpy(&out[i], info + addr::grid::kWallStyles + 2 * i, 2);
}

uint32_t WallsAt(int level, int x, int y) {
    typedef void*(__thiscall * WallsFn)(void* grid, void* out, const uint8_t* tile);
    if (!FunctionOk(addr::grid::kWalls) || x < 0 || y < 0 || x >= GridSize() || y >= GridSize()) return 0;
    Ptr grid = Grid();
    if (!grid || level < 1 || level > addr::rooms::kMaxLevel) return 0;
    alignas(4) unsigned char info[addr::grid::kWallInfoSize] = {};
    uint8_t tile[4] = {(uint8_t)y, (uint8_t)x, (uint8_t)level, 0};
    if (!Guarded([&] { ((WallsFn)Va(addr::grid::kWalls))((void*)grid, info, tile); })) return 0;
    uint32_t mask;
    memcpy(&mask, info, sizeof mask);
    return mask;
}

Tile Step(Tile t, int direction) { return Tile{t.x + kStepX[direction & 7], t.y + kStepY[direction & 7]}; }

int CompassOf(int direction) { return (6 - direction) & 7; }

std::vector<Ptr> ObjectParts(Ptr object) {
    std::vector<Ptr> parts;
    if (!IsKindOf(object, "cXObject")) return parts;
    if (!IsKindOf(object, "cXMTObject")) return {object};
    Ptr lead = Rd<Ptr>(object + addr::place::kLead);
    for (Ptr p = lead ? lead : object; p && parts.size() < 64; p = Rd<Ptr>(p + addr::place::kNextPart))
        parts.push_back(p);
    return parts;
}

int ObjectDirection(Ptr object) { return Short(object, addr::place::kDirection) & 7; }

int ObjectLevel(Ptr object) { return Rd<int>(object + addr::place::kLevel); }

bool IsPortal(Ptr object) { return IsKindOf(object, "cXPortal"); }

int ContainerOf(Ptr object) { return Rd<uint16_t>(object + addr::place::kContainer); }

bool CanWalkOver(Ptr object) { return (Short(object, addr::place::kFlags) & 0x1c) != 0; }

bool PeoplePass(Ptr object) { return (Short(object, addr::place::kFlags) & 0x14) == 0x10; }

uint32_t RoutingSides(Ptr object) {
    uint32_t sides = 0;
    for (Ptr part : ObjectParts(object)) {
        Ptr slots = Rd<Ptr>(part + addr::place::kRoutingSlots);
        Ptr end = Rd<Ptr>(part + addr::place::kRoutingSlots + 4);
        for (Ptr s = slots; s && s < end && s - slots < addr::place::kRoutingSlotSize * 32;
             s += addr::place::kRoutingSlotSize)
            sides |= Rd<uint32_t>(s + addr::rslot::kFlags) & 0xff;
    }
    return sides;
}

std::vector<Tile> FootprintAt(Ptr object, Tile lead) {
    std::vector<Tile> tiles;
    std::vector<Ptr> parts = ObjectParts(object);
    if (parts.empty()) return tiles;
    int baseX = Rd<int>(parts[0] + addr::place::kPartX), baseY = Rd<int>(parts[0] + addr::place::kPartY);
    for (Ptr p : parts) {
        if (parts.size() == 1) {
            tiles.push_back(lead);
            break;
        }
        tiles.push_back(Tile{lead.x + Rd<int>(p + addr::place::kPartX) - baseX,
                             lead.y + Rd<int>(p + addr::place::kPartY) - baseY});
    }
    return tiles;
}

// Routing slots are written for the object facing north (-y, direction 6): offsets with -y in
// front, +x to its right, and side bits 0 N (its front), 1 NE, 2 E ... clockwise. For an object
// facing `direction`, side bit b points toward direction - b, and the offset turns with it.
// Proximities are in 1/32 tile (a fridge's 32 puts the Sim on the next tile, as measured).
std::vector<Tile> UseTilesAt(Ptr object, Tile lead, int direction) {
    std::vector<Tile> out;
    std::vector<Tile> footprint = FootprintAt(object, lead);
    std::vector<Ptr> parts = ObjectParts(object);
    auto add = [&](Tile t) {
        if (std::find(out.begin(), out.end(), t) == out.end() &&
            std::find(footprint.begin(), footprint.end(), t) == footprint.end())
            out.push_back(t);
    };
    double fx, fy, rx, ry;  // the object's front and right in the lot's grid
    DirectionVector(direction, &fx, &fy);
    DirectionVector(direction - 2 + 8, &rx, &ry);
    for (size_t i = 0; i < parts.size() && i < footprint.size(); i++) {
        Ptr slots = Rd<Ptr>(parts[i] + addr::place::kRoutingSlots);
        Ptr end = Rd<Ptr>(parts[i] + addr::place::kRoutingSlots + 4);
        if (!slots || end <= slots) continue;
        size_t count = std::min<size_t>((end - slots) / addr::place::kRoutingSlotSize, 32);
        double cx = footprint[i].x * 16 + 8, cy = footprint[i].y * 16 + 8;
        for (size_t s = 0; s < count; s++) {
            Ptr slot = slots + s * addr::place::kRoutingSlotSize;
            uint32_t flags = Rd<uint32_t>(slot + addr::rslot::kFlags);
            double ox = Rd<float>(slot + addr::rslot::kX), oy = Rd<float>(slot + addr::rslot::kY);
            double px = cx + ox * rx - oy * fx, py = cy + ox * ry - oy * fy;
            if ((flags & 0xff) == 0) {  // no sides: the Sim stands on the offset itself (a sofa's
                if (ox != 0 || oy != 0)  // front); none at all means a spot on the object
                    add(Tile{(int)std::floor(px / 16), (int)std::floor(py / 16)});
                continue;
            }
            int proximity = Rd<int>(slot + addr::rslot::kMinProximity);
            if (proximity <= 0) proximity = Rd<int>(slot + addr::rslot::kOptimalProximity);
            if (proximity <= 0 || proximity > 32 * 8) proximity = 32;
            double reach = proximity / 2.0;  // 1/16 tile
            for (int side = 0; side < 8; side++) {
                if (!(flags & (1u << side))) continue;
                double dx, dy;
                DirectionVector((direction - side + 8) & 7, &dx, &dy);
                add(Tile{(int)std::floor((px + dx * reach) / 16), (int)std::floor((py + dy * reach) / 16)});
            }
        }
    }
    if (out.empty() && !footprint.empty()) add(Step(footprint[0], direction));  // no routing slots: the front
    return out;
}

}  // namespace game
