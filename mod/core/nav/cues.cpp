#include "cues.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "../game/lot.h"
#include "../game/rooms.h"
#include "../game/rtti.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/settings.h"
#include "../util/text.h"

namespace nav::cues {
namespace {

int g_enabled = -1;   // read from the ini on first use
int g_volume = -1;    // percent
int g_lastRoom = -2;  // the room the last tile cue was in (-2: none yet)
int g_lastLevel = 0;

// Each side's sound: west to the left, east to the right, north a little higher, south lower.
struct Placement {
    float pan, pitch;
};
const Placement kPlace[kSides] = {{0.0f, 1.22f}, {0.8f, 1.0f}, {0.0f, 0.82f}, {-0.8f, 1.0f}};

const uint32_t kWallBit[kSides] = {game::kWallNorth, game::kWallEast, game::kWallSouth, game::kWallWest};
const int kStyleIndex[kSides] = {0, 3, 2, 1};  // WallStylesAt: north, west, south, east

// The side of a tile a direction points to (engine directions: 6 north, 4 east, 2 south, 0 west).
int SideOfDirection(int direction) {
    switch (direction & 7) {
    case 6: return kNorth;
    case 4: return kEast;
    case 2: return kSouth;
    default: return kWest;
    }
}

game::Tile Beyond(game::Tile t, int side) {
    switch (side) {
    case kNorth: return game::Tile{t.x, t.y - 1};
    case kEast: return game::Tile{t.x + 1, t.y};
    case kSouth: return game::Tile{t.x, t.y + 1};
    default: return game::Tile{t.x - 1, t.y};
    }
}

float Volume() {
    if (g_volume < 0) g_volume = util::GetSetting("cue_volume", 80);
    return (g_volume < 0 ? 0 : g_volume > 100 ? 100 : g_volume) / 100.0f;
}

void Play(const char* name, float pan = 0, float pitch = 1, float gain = 1, int delay = 0) {
    if (!Enabled()) return;
    speech::Cue(name, pan, pitch, gain * Volume(), delay);
}

const char* SurfaceSound(game::Surface s) {
    switch (s) {
    case game::Surface::Wood: return "floor_wood";
    case game::Surface::Tile: return "floor_tile";
    case game::Surface::Carpet: return "floor_carpet";
    case game::Surface::Paving: return "floor_stone";
    case game::Surface::PoolRim: return "floor_stone";
    case game::Surface::Water: return "floor_water";
    case game::Surface::NoFloor: return "floor_none";
    default: return "floor_grass";
    }
}

const char* EdgeSound(Edge e) {
    switch (e) {
    case Edge::Fence: case Edge::Railing: return "fence";
    case Edge::Door: return "door";
    case Edge::Window: return "window";
    default: return "wall";
    }
}

// Doors, windows and stairs near a tile (the portals of its floor within a tile of it).
std::vector<game::PlacedPart> PortalsNear(int level, game::Tile t) {
    std::vector<game::PlacedPart> out;
    for (const game::PlacedPart& p : game::PlacedParts()) {
        if (p.level != level || std::abs(p.tileX - t.x) > 1 || std::abs(p.tileY - t.y) > 1) continue;
        if (game::IsPortal(p.object) && !game::IsInvisibleHelper(p.object)) out.push_back(p);
    }
    return out;
}

// What stands on one side of a tile: a portal in that wall (standing on the tile with the wall
// at its back, or on the tile beyond with the wall at its back: nav/room_layout.cpp's rule),
// else the wall or fence.
Edge EdgeOf(int level, game::Tile t, int side, uint32_t walls, const uint16_t styles[4],
            const std::vector<game::PlacedPart>& portals) {
    if (!(walls & kWallBit[side])) return Edge::None;
    game::Tile other = Beyond(t, side);
    for (const game::PlacedPart& p : portals) {
        int back = SideOfDirection(game::ObjectDirection(p.object) + 4);
        bool here = p.tileX == t.x && p.tileY == t.y && back == side;
        bool there = p.tileX == other.x && p.tileY == other.y && back == (side + 2) % kSides;
        if (here || there) return game::PeoplePass(p.object) ? Edge::Door : Edge::Window;
    }
    // Wall styles (verified on the Lápida mansion, 2026-11-01): 1 the plain wall, 3 the wall piece
    // a door stands in, 22 a window's; the rest are fences and railings (13 its iron fence, 14 the
    // railing upstairs around the stairwell).
    uint16_t style = styles[kStyleIndex[side]];
    if (style == 1 || style == 3 || style == 22) return Edge::Wall;
    return level > 1 ? Edge::Railing : Edge::Fence;
}

}  // namespace

TileSense Sense(int level, int x, int y, game::Ptr ignore) {
    TileSense s;
    game::Tile t{x, y};
    s.surface = game::SurfaceAt(level, x, y);
    s.room = game::RoomAt(level, x, y);
    uint32_t walls = game::WallsAt(level, x, y);
    uint16_t styles[4];
    game::WallStylesAt(level, x, y, styles);
    std::vector<game::PlacedPart> portals = PortalsNear(level, t);
    for (int side = 0; side < kSides; side++) s.sides[side] = EdgeOf(level, t, side, walls, styles, portals);
    for (const game::PlacedPart& p : portals)
        if (p.tileX == x && p.tileY == y && game::PeoplePass(p.object) &&
            !(walls & kWallBit[SideOfDirection(game::ObjectDirection(p.object) + 4)]))
            s.stairs = true;  // a portal people walk through with no wall behind it
    std::vector<game::Ptr> ignored = ignore ? game::ObjectParts(ignore) : std::vector<game::Ptr>{};
    for (const game::PlacedPart& p : game::PlacedParts()) {
        if (p.level != level || p.tileX != x || p.tileY != y) continue;
        if (p.object == ignore || std::find(ignored.begin(), ignored.end(), p.object) != ignored.end()) continue;
        if (game::IsPortal(p.object) || game::IsInvisibleHelper(p.object)) continue;
        if (game::IsKindOf(p.object, "cXPerson")) s.person = true;
        else s.object = true;
    }
    uint8_t h[4];
    if (game::TileHeights(x, y, h)) s.sloped = h[0] != h[1] || h[0] != h[2] || h[0] != h[3];
    return s;
}

bool Enabled() {
    if (g_enabled < 0) g_enabled = util::GetSetting("cues", 1) ? 1 : 0;
    return g_enabled == 1;
}

void Toggle() {
    g_enabled = Enabled() ? 0 : 1;
    util::SetSetting("cues", g_enabled);
    speech::Say(S(g_enabled ? Str::CuesOn : Str::CuesOff));
    if (g_enabled) Play("fits");
}

void Reset() {
    g_lastRoom = -2;
    g_lastLevel = 0;
}

namespace {

// The sounds of a sensed tile, with the sides given (a tile's own, or a corner's lines).
void PlayTile(const TileSense& s, int level, const Edge sides[kSides]) {
    Play(SurfaceSound(s.surface), 0, 1, 1, 0);
    // The half tiles beside a diagonal wall belong to no room: they change nothing.
    if (s.room != game::kNoRoom) {
        if (g_lastRoom != -2 && (s.room != g_lastRoom || level != g_lastLevel)) Play("room", 0, 1, 0.8f, 0);
        g_lastRoom = s.room;
        g_lastLevel = level;
    }
    int delay = 70;
    for (int side = 0; side < kSides; side++) {
        if (sides[side] == Edge::None) continue;
        Play(EdgeSound(sides[side]), kPlace[side].pan, kPlace[side].pitch, 1, delay);
        delay += 45;
    }
    if (s.stairs) { Play("stairs", 0, 1, 0.9f, delay); delay += 60; }
    if (s.person) { Play("person", 0, 1, 0.8f, delay); delay += 60; }
    else if (s.object) { Play("object", 0, 1, 0.9f, delay); delay += 60; }
    if (s.sloped) Play("slope", 0, 1, 0.6f, delay);
}

}  // namespace

void Tile(int level, int x, int y, game::Ptr ignore) {
    if (!Enabled()) return;
    TileSense s = Sense(level, x, y, ignore);
    PlayTile(s, level, s.sides);
}

void Corner(int level, int x, int y) {
    if (!Enabled()) return;
    TileSense s = Sense(level, x, y);  // the tile south-east of the corner: its floor, its things
    // The four grid lines leaving the corner, as edges of the tiles that own them: north is the
    // west edge of the tile north of it, east the north edge of its own tile, south its own west
    // edge, west the north edge of the tile west of it.
    Edge lines[kSides];
    lines[kNorth] = Sense(level, x, y - 1).sides[kWest];
    lines[kEast] = s.sides[kNorth];
    lines[kSouth] = s.sides[kWest];
    lines[kWest] = Sense(level, x - 1, y).sides[kNorth];
    PlayTile(s, level, lines);
}

void LotEdge() { Play("edge"); }

void Blocked() { Play("blocked", 0, 1, 0.8f, 30); }

void Fit(bool fits) { Play(fits ? "fits" : "nofit", 0, 1, 0.7f, 0); }

std::string SurfaceWord(game::Surface s) {
    switch (s) {
    case game::Surface::Wood: return S(Str::SurfaceWood);
    case game::Surface::Tile: return S(Str::SurfaceTile);
    case game::Surface::Carpet: return S(Str::SurfaceCarpet);
    case game::Surface::Paving: return S(Str::SurfacePaving);
    case game::Surface::PoolRim: return S(Str::SurfacePoolRim);
    case game::Surface::Water: return S(Str::SurfaceWater);
    case game::Surface::NoFloor: return S(Str::SurfaceNone);
    default: return S(Str::SurfaceGround);
    }
}

Str SideWord(int side) {
    const Str words[kSides] = {Str::BuildNorth, Str::BuildEast, Str::BuildSouth, Str::BuildWest};
    return words[side & 3];
}

std::string EdgesLine(const TileSense& t) {
    std::string line;
    for (int side = 0; side < kSides; side++) {
        Str form;
        switch (t.sides[side]) {
        case Edge::Wall: form = Str::EdgeWall; break;
        case Edge::Fence: form = Str::EdgeFence; break;
        case Edge::Railing: form = Str::EdgeRailing; break;
        case Edge::Door: form = Str::EdgeDoor; break;
        case Edge::Window: form = Str::EdgeWindow; break;
        default: continue;
        }
        line += (line.empty() ? "" : ", ") + util::Format(S(form), S(SideWord(side)));
    }
    return line;
}

}  // namespace nav::cues
