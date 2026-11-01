#include "build_mode.h"

#include <windows.h>

#include <algorithm>
#include <map>
#include <cstdlib>
#include <string>
#include <vector>

#include "../game/build.h"
#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
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

namespace nav::build {
namespace {

using game::Corner;
using game::Tile;

// Ready-made rooms (width east, depth south), in tiles.
struct Preset {
    int w, h;
};
const Preset kPresets[] = {{3, 3}, {4, 3}, {4, 4}, {5, 4}, {5, 5}, {6, 4}, {6, 5}, {6, 6}, {8, 6}, {10, 8}};
constexpr int kPresetCount = (int)(sizeof kPresets / sizeof kPresets[0]);
constexpr ULONGLONG kConfirmMs = 10000;  // a warned build waits this long for the second Enter
constexpr int kFreeLookTiles = 64;       // free space is counted to the wall or the lot's edge

struct State {
    bool active = false;
    bool floors = false;     // the floor or wallpaper tool: the cursor is on tiles, not corners
    bool paper = false;      // the wallpaper tool: whole rooms
    bool pool = false;       // the water tool: rectangles of pool
    int terrain = 0;         // 1 the level tool, 2 raise or lower (game::TerrainToolKind)
    Corner cursor;
    bool hasStart = false;
    Corner start;
    int level = 1;
    int price = 0;           // of the chosen style, per wall segment
    std::string style;       // its name
    int preset = 2;
    int roomIndex = -1;      // Ctrl+PageDown / Ctrl+PageUp: the room the cursor last jumped to
    ULONGLONG levelSetAt = 0;  // Shift+PageUp/Down pressed a floor button: the game follows later
    // A build that was warned about: the same one again within kConfirmMs goes ahead.
    bool pending = false;
    Corner pendingFrom, pendingTo;
    int pendingKind = 0;     // 0 line, 1 room
    ULONGLONG pendingAt = 0;
    std::vector<rooms::NamedRoom> rooms;
    // A style was pressed: the game takes the press a moment later, so the cursor starts from
    // the tick once the tool is there (a style that is really an object brings the buy tool).
    uintptr_t pressed = 0;
    ULONGLONG pressedAt = 0;
    int lastRoom = -1;       // the room the cursor's line last named (said again when it changes)
    int lastRoomLevel = 0;
};

State g;

int Size() { return game::GridSize(); }

Corner Clamp(Corner c) {
    int n = Size();
    // Walls stay off the lot's outer border (the tool refuses the first and last tiles).
    int last = g.floors ? n - 2 : n - 1;  // tiles stop one before the last corner
    c.x = std::clamp(c.x, 1, last);
    c.y = std::clamp(c.y, 1, last);
    return c;
}

std::string Money(int amount) { return util::Money(amount); }

// ---- What is around a corner ---------------------------------------------------------------

// Walls along the four grid lines that leave the corner (north: the west edge of the tile
// north-west... see the header of game/lotgrid.h: tile (x, y) owns its north and west edges).
std::vector<Str> WallsAtCorner(Corner c) {
    std::vector<Str> out;
    int lv = g.level;
    if (game::WallsAt(lv, c.x, c.y - 1) & game::kWallWest) out.push_back(Str::BuildNorth);
    if (game::WallsAt(lv, c.x, c.y) & game::kWallNorth) out.push_back(Str::BuildEast);
    if (game::WallsAt(lv, c.x, c.y) & game::kWallWest) out.push_back(Str::BuildSouth);
    if (game::WallsAt(lv, c.x - 1, c.y) & game::kWallNorth) out.push_back(Str::BuildWest);
    return out;
}

std::string JoinDirections(const std::vector<Str>& dirs) {
    std::string s;
    for (Str d : dirs) {
        if (!s.empty()) s += ", ";
        s += S(d);
    }
    return s;
}

// Tiles free from the tile south-east of the corner to the nearest wall or the lot's edge.
int FreeTiles(Tile t, int direction) {
    int n = 0;
    for (; n < kFreeLookTiles; n++) {
        uint32_t bit = direction == 6 ? game::kWallNorth : direction == 4 ? game::kWallEast
                     : direction == 2 ? game::kWallSouth : game::kWallWest;
        if (game::WallsAt(g.level, t.x, t.y) & bit) break;
        Tile next = game::Step(t, direction);
        if (!game::InGrid(next.x, next.y)) break;
        t = next;
    }
    return n + 1;  // the tile itself counts
}

std::string RoomNameAt(int level, Tile t) {
    int room = game::RoomAt(level, t.x, t.y);
    std::string name = rooms::NameOf(g.rooms, room);
    game::Room info;
    if (!game::GetRoom(room, &info)) return name;
    if (name.empty()) return info.outside ? S(Str::RoomOutside) : util::Format(S(Str::RoomEmpty), game::RoomTileCount(room));
    return util::Format(S(Str::BuildRoomSize), name.c_str(), info.tiles);
}

std::string RoomName(Tile t) { return RoomNameAt(g.level, t); }

// On an upper floor: what the tile stands over ("encima de cocina, 20 casillas"), since walls and
// floors up there need walls below to hold them.
std::string BelowLine(Tile t) {
    if (g.level <= 1) return "";
    int room = game::RoomAt(g.level - 1, t.x, t.y);
    game::Room info;
    if (room == game::kNoRoom || (game::GetRoom(room, &info) && info.outside)) return S(Str::BuildAboveNothing);
    return util::Format(S(Str::BuildAbove), RoomNameAt(g.level - 1, t).c_str());
}

std::string ObjectOn(Tile t) {
    int id = game::ObjectIdAt(g.level, t.x, t.y);
    game::LotObject o;
    if (!id || !game::GetObject(game::ObjectById(id), &o) || game::IsInvisibleHelper(o.object)) return "";
    return world::DisplayName(o);
}

// ---- The planned wall ----------------------------------------------------------------------

enum Shape { kNone, kStraight, kDiagonal, kRoomOnly };

Shape ShapeOf(Corner a, Corner b) {
    int dx = std::abs(b.x - a.x), dy = std::abs(b.y - a.y);
    if (!dx && !dy) return kNone;
    if (!dx || !dy) return kStraight;
    if (dx == dy) return kDiagonal;
    return kRoomOnly;
}

int Segments(Corner a, Corner b) { return std::max(std::abs(b.x - a.x), std::abs(b.y - a.y)); }

// East / west / north / south words for a step.
std::string Offset(Corner from, Corner to) {
    std::vector<std::string> parts;
    int dx = to.x - from.x, dy = to.y - from.y;
    if (dy < 0) parts.push_back(util::Format(S(Str::BuildSteps), -dy, S(Str::BuildNorth)));
    if (dy > 0) parts.push_back(util::Format(S(Str::BuildSteps), dy, S(Str::BuildSouth)));
    if (dx > 0) parts.push_back(util::Format(S(Str::BuildSteps), dx, S(Str::BuildEast)));
    if (dx < 0) parts.push_back(util::Format(S(Str::BuildSteps), -dx, S(Str::BuildWest)));
    std::string s;
    for (const std::string& p : parts) s += (s.empty() ? "" : ", ") + p;
    return s;
}

// What Enter would build from the start, in words, with its cost.
int Width(Corner a, Corner b) { return std::abs(b.x - a.x) + 1; }
int Depth(Corner a, Corner b) { return std::abs(b.y - a.y) + 1; }

std::string FloorPlanLine() {
    if (!g.hasStart) return "";
    int w = Width(g.start, g.cursor), h = Depth(g.start, g.cursor);
    std::string from = g.start == g.cursor ? std::string(S(Str::BuildAtStart))
                                           : util::Format(S(Str::BuildFromStart), Offset(g.start, g.cursor).c_str());
    return from + ". " + util::Format(S(Str::FloorPlan), w, h, w * h, Money(w * h * g.price).c_str());
}

std::string RoomName(Tile t);

std::string PoolPlanLine() {
    Corner a = g.hasStart ? g.start : g.cursor;
    int w = Width(a, g.cursor), h = Depth(a, g.cursor);
    std::string from = !g.hasStart || g.start == g.cursor ? "" : util::Format(S(Str::BuildFromStart), Offset(g.start, g.cursor).c_str()) + ". ";
    return from + util::Format(S(Str::PoolPlan), w, h, w * h, Money(w * h * g.price).c_str());
}

std::string PlanLine() {
    if (g.terrain == 1 && g.hasStart)
        return util::Format(S(Str::LevelPlan), Width(g.start, g.cursor), Depth(g.start, g.cursor));
    if (g.terrain) return "";
    if (g.pool) return g.hasStart ? PoolPlanLine() : "";
    if (g.paper) {
        std::string room = RoomName(Tile{g.cursor.x, g.cursor.y});
        return room.empty() ? "" : util::Format(S(Str::PaperPlan), room.c_str());
    }
    if (g.floors) return FloorPlanLine();
    if (!g.hasStart) return "";
    Shape shape = ShapeOf(g.start, g.cursor);
    int w = std::abs(g.cursor.x - g.start.x), h = std::abs(g.cursor.y - g.start.y);
    std::string from = util::Format(S(Str::BuildFromStart), Offset(g.start, g.cursor).c_str());
    switch (shape) {
    case kNone: return S(Str::BuildAtStart);
    case kStraight:
        return from + ". " + util::Format(S(Str::BuildWallPlan), Segments(g.start, g.cursor), Money(Segments(g.start, g.cursor) * g.price).c_str());
    case kDiagonal:
        return from + ". " + util::Format(S(Str::BuildDiagonalPlan), Segments(g.start, g.cursor), Money(Segments(g.start, g.cursor) * g.price).c_str());
    case kRoomOnly: break;
    }
    return from + ". " + util::Format(S(Str::BuildRoomPlan), w, h, Money(2 * (w + h) * g.price).c_str());
}

// ---- Descriptions --------------------------------------------------------------------------

Tile TileSE(Corner c) { return Tile{c.x, c.y}; }

// What is at the cursor, in the order a builder needs it (the tester, 2026-10-31: coordinates
// first made the line hard to filter): what stands there, the room when the cursor has just
// entered it, walls or floor, whether it can be built on, the plan from the start; the
// coordinates last (R and the details buffer have them too).
std::string ShortLine(bool withRoom = false) {
    std::vector<std::string> parts;
    std::string object = ObjectOn(TileSE(g.cursor));
    if (!object.empty()) parts.push_back(object);
    if (withRoom) {
        std::string room = RoomName(TileSE(g.cursor));
        if (!room.empty()) parts.push_back(room);
    }
    if (g.floors && !g.paper && !g.pool && !g.terrain) {
        parts.push_back(S(game::HasFloor(g.level, g.cursor.x, g.cursor.y) ? Str::FloorHere : Str::FloorNone));
    } else {
        std::vector<Str> walls = WallsAtCorner(g.cursor);
        if (!walls.empty()) parts.push_back(util::Format(S(Str::BuildWallsHere), JoinDirections(walls).c_str()));
    }
    if (!game::CornerBuildable(g.cursor)) parts.push_back(S(Str::BuildNotBuildableHere));
    uint8_t h[4];
    if (game::TileHeights(g.cursor.x, g.cursor.y, h)) {
        bool sloped = h[0] != h[1] || h[0] != h[2] || h[0] != h[3];
        if (g.terrain) parts.push_back(util::Format(S(Str::TerrainHeight), (int)std::min({h[0], h[1], h[2], h[3]})));
        if (sloped) parts.push_back(S(Str::TerrainSloped));
    }
    std::string below = BelowLine(TileSE(g.cursor));
    if (!below.empty()) parts.push_back(below);
    std::string line;
    for (const std::string& p : parts) line += (line.empty() ? "" : ", ") + p;
    std::string plan = PlanLine();
    if (!plan.empty()) line += (line.empty() ? "" : ". ") + plan;
    return line + (line.empty() ? "" : ". ") + util::Format(S(Str::BuildCorner), g.cursor.x, g.cursor.y);
}

// The room under the cursor changed since it was last said.
bool RoomChanged() {
    int room = game::RoomAt(g.level, g.cursor.x, g.cursor.y);
    bool changed = room != g.lastRoom || g.level != g.lastRoomLevel;
    g.lastRoom = room;
    g.lastRoomLevel = g.level;
    return changed;
}

// The line with the room always named (the cursor starts or changes floor).
std::string LineWithRoom() {
    RoomChanged();
    return ShortLine(true);
}

std::vector<std::string> FullLines() {
    std::vector<std::string> lines;
    lines.push_back(ShortLine());
    Tile t = TileSE(g.cursor);
    std::string room = RoomName(t);
    if (!room.empty()) lines.push_back(room);
    lines.push_back(util::Format(S(Str::BuildFreeSpace), FreeTiles(t, 6), FreeTiles(t, 4), FreeTiles(t, 2), FreeTiles(t, 0)));
    if (g.hasStart) lines.push_back(util::Format(S(Str::BuildStartAt), g.start.x, g.start.y));
    if (!g.style.empty())
        lines.push_back(util::Format(S(g.floors ? Str::FloorStyle : Str::BuildStyle), g.style.c_str(), Money(g.price).c_str()));
    return lines;
}

void SayShort() {
    std::vector<std::string> lines = FullLines();
    buffers::SetDetails(lines);
    speech::Say(ShortLine(RoomChanged()));
}

// ---- Building ------------------------------------------------------------------------------

std::vector<placement::WallEdgeAdd> EdgesOf(Corner a, Corner b) {
    std::vector<placement::WallEdgeAdd> edges;
    if (a.y == b.y)
        for (int x = std::min(a.x, b.x); x < std::max(a.x, b.x); x++) edges.push_back({{x, a.y}, game::kWallNorth});
    else if (a.x == b.x)
        for (int y = std::min(a.y, b.y); y < std::max(a.y, b.y); y++) edges.push_back({{a.x, y}, game::kWallWest});
    return edges;  // diagonal walls are not checked
}

std::vector<placement::WallEdgeAdd> RoomEdges(Corner a, Corner b) {
    Corner nw{std::min(a.x, b.x), std::min(a.y, b.y)}, se{std::max(a.x, b.x), std::max(a.y, b.y)};
    std::vector<placement::WallEdgeAdd> edges;
    for (auto part : {EdgesOf(nw, {se.x, nw.y}), EdgesOf({nw.x, se.y}, se), EdgesOf(nw, {nw.x, se.y}), EdgesOf({se.x, nw.y}, se)})
        edges.insert(edges.end(), part.begin(), part.end());
    return edges;
}

// The warning for new walls, empty when they block nothing. A new room with no door yet is
// said as such, since that is how every room starts.
std::string Warning(const std::vector<placement::WallEdgeAdd>& edges, bool room) {
    if (edges.empty()) return "";
    placement::WallCheck c = placement::CheckNewWalls(g.level, edges);
    std::vector<std::string> parts;
    if (!c.blockedObjects.empty()) {
        std::string names;
        for (const std::string& n : c.blockedObjects) names += (names.empty() ? "" : ", ") + n;
        parts.push_back(util::Format(S(Str::BuildBlocksObjects), names.c_str()));
    }
    if (!c.trappedSims.empty()) {
        std::string names;
        for (const std::string& n : c.trappedSims) names += (names.empty() ? "" : ", ") + n;
        parts.push_back(util::Format(S(Str::BuildTrapsSims), names.c_str()));
    }
    if (c.doorsBlocked) parts.push_back(S(Str::BuildBlocksDoor));
    if (c.stairsBlocked) parts.push_back(S(Str::BuildBlocksStairs));
    if (c.cutOffTiles && c.blockedObjects.empty() && !c.doorsBlocked && !c.stairsBlocked && c.trappedSims.empty())
        parts.push_back(util::Format(S(room ? Str::BuildRoomNeedsDoor : Str::BuildCutsOff), c.cutOffTiles));
    else if (c.cutOffTiles)
        parts.push_back(util::Format(S(Str::BuildCutsOff), c.cutOffTiles));
    std::string s;
    for (const std::string& p : parts) s += (s.empty() ? "" : ". ") + p;
    Log("build: check %d edges: %d objects, %d doors, %d tiles cut off", (int)edges.size(),
        (int)c.blockedObjects.size(), c.doorsBlocked, c.cutOffTiles);
    return s;
}

bool Confirmed(Corner from, Corner to, int kind) {
    return g.pending && g.pendingFrom == from && g.pendingTo == to && g.pendingKind == kind &&
           GetTickCount64() - g.pendingAt < kConfirmMs;
}

void AskConfirm(const std::string& warning, Corner from, Corner to, int kind) {
    g.pending = true;
    g.pendingFrom = from;
    g.pendingTo = to;
    g.pendingKind = kind;
    g.pendingAt = GetTickCount64();
    speech::Say(util::Format(S(Str::BuildWarning), warning.c_str()));
}

// Draws, and says what happened; the money line comes from the narrator.
void Draw(Corner from, Corner to, game::WallMode mode, const std::string& what) {
    g.pending = false;
    int error = game::DrawWalls(from, to, mode);
    if (error == game::kNothingBuilt) {
        bool outside = !game::CornerBuildable(from) || !game::CornerBuildable(to);
        speech::Say(S(outside ? Str::BuildOutsideArea : Str::BuildFailed));
        return;
    }
    if (error < 0) {
        speech::Say(S(Str::BuildFailed));
        return;
    }
    if (error > 0) {
        std::string reason = game::PlacementErrorText(error);
        speech::Say(reason.empty() ? std::string(S(Str::BuildFailed)) : reason);
        return;
    }
    g.rooms = rooms::NameRooms(game::LotObjects());
    // The game charges it later (on a following action), so its price is said now, as its own
    // tooltip showed it.
    int cost = game::LastDrawCost();
    buffers::Event(cost > 0 ? what + ". " + util::Format(S(Str::BuildCostWas), Money(cost).c_str()) : what);
}

void BuildRoom(Corner a, Corner b) {
    int w = std::abs(b.x - a.x), h = std::abs(b.y - a.y);
    if (!w || !h) {
        speech::Say(S(Str::BuildNeedsArea));
        return;
    }
    std::string warning = Warning(RoomEdges(a, b), true);
    bool onlyNoDoor = false;
    {
        placement::WallCheck c = placement::CheckNewWalls(g.level, RoomEdges(a, b));
        onlyNoDoor = c.blockedObjects.empty() && !c.doorsBlocked && !c.stairsBlocked && c.trappedSims.empty();
    }
    // An empty new room without a door is expected: said after building, not asked about.
    if (!warning.empty() && !onlyNoDoor && !Confirmed(a, b, 1)) {
        AskConfirm(warning, a, b, 1);
        return;
    }
    std::string what = util::Format(S(Str::BuiltRoom), w, h);
    if (!warning.empty()) what += ". " + warning;
    Draw(a, b, game::WallMode::Room, what);
}

}  // namespace

void Reset() { g = State{}; }

bool Active() { return g.active; }

bool CursorTile(int* x, int* y, int* level) {
    if (!g.active) return false;
    *x = g.cursor.x;
    *y = g.cursor.y;
    *level = g.level;
    return true;
}

bool FloorCursor() { return g.active && g.floors && !g.paper && !g.pool && !g.terrain; }

bool PaperCursor() { return g.active && g.paper; }

bool PoolCursor() { return g.active && g.pool; }

bool TerrainCursor() { return g.active && g.terrain; }

bool Begin(uintptr_t productButton) {
    if (!LotViewActive()) return false;
    g.pressed = productButton;
    g.pressedAt = GetTickCount64();
    return true;
}

namespace {

constexpr ULONGLONG kSettleMs = 250, kGiveUpMs = 1500;

bool ToolActive() {
    if (g.terrain) return game::TerrainToolKind() == g.terrain;
    if (g.pool) return game::PoolToolActive();
    return g.paper ? game::PaperToolActive() : g.floors ? game::FloorToolActive() : game::WallToolActive();
}

bool Start(uintptr_t productButton) {
    bool paper = game::PaperToolActive(), pool = game::PoolToolActive();
    int terrain = game::TerrainToolKind();
    if (!LotViewActive() || (!game::WallToolActive() && !game::FloorToolActive() && !paper && !pool && !terrain))
        return false;
    bool floors = game::FloorToolActive() || paper || pool || terrain;
    if (g.active && (floors != g.floors || paper != g.paper || pool != g.pool || terrain != g.terrain))
        g.active = false;  // another tool: start afresh
    g.floors = floors;
    g.paper = paper;
    g.pool = pool;
    g.terrain = terrain;
    game::ProductInfo p;
    if (productButton && game::ReadProductOfButton(productButton, &p)) {
        g.price = p.price;
        g.style = p.name;
    }
    bool resume = g.active;
    g.active = true;
    g.level = game::HeldLevel();
    g.rooms = rooms::NameRooms(game::LotObjects());
    if (!resume) {
        g.hasStart = false;
        game::LotObject sim;
        if (game::GetObject(game::SelectedSim(), &sim)) g.cursor = Clamp({sim.x >> 4, sim.y >> 4});
        else g.cursor = Clamp({Size() / 2, Size() / 2});
    }
    std::vector<std::string> lines = FullLines();
    buffers::SetDetails(lines);
    Str start = g.terrain ? Str::TerrainCursorStart : g.pool ? Str::PoolCursorStart : g.paper ? Str::PaperCursorStart
              : g.floors ? Str::FloorCursorStart : Str::BuildCursorStart;
    // The floor it works on: choosing a tool puts the view back on the first floor.
    speech::Say(util::Format(S(start), g.style.c_str()) + " " + util::Format(S(Str::FloorNumber), g.level) + ". " +
                LineWithRoom());
    return true;
}

}  // namespace

void Tick() {
    if (g.pressed) {
        ULONGLONG waited = GetTickCount64() - g.pressedAt;
        if (waited >= kSettleMs && (Start(g.pressed) || waited > kGiveUpMs)) g.pressed = 0;
    }
    if (!g.active) return;
    // The floor buttons (the player's, or Shift+PageUp) change the floor the tool works on.
    if (GetTickCount64() - g.levelSetAt > 1000) g.level = game::HeldLevel();
    if (!LotViewActive() || !ToolActive()) {
        Log("build: cursor ended (tool or view changed)");
        g.active = false;
        g.hasStart = false;
    }
}

void Move(int dx, int dy) {
    Corner next = Clamp({g.cursor.x + dx, g.cursor.y + dy});
    if (next == g.cursor) {
        speech::Say(S(Str::BuildLotEdge));
        return;
    }
    g.cursor = next;
    g.pending = false;
    SayShort();
}

void MarkStart() {
    g.pending = false;
    if (g.paper) {
        speech::Say(S(Str::PaperNoStart));
        return;
    }
    if (g.hasStart && g.start == g.cursor) {
        g.hasStart = false;
        speech::Say(S(Str::BuildStartCleared));
        return;
    }
    g.hasStart = true;
    g.start = g.cursor;
    speech::Say(util::Format(S(Str::BuildStartMarked), g.start.x, g.start.y));
}

namespace {

void DrawFloorAndSay(Corner a, Corner b, game::FloorMode mode, const std::string& what) {
    g.pending = false;
    int tiles = 0;
    int error = game::DrawFloor(a, b, mode, &tiles);
    if (error == game::kNothingBuilt) {
        bool outside = !game::CornerBuildable(a) || !game::CornerBuildable(b);
        speech::Say(S(outside ? Str::BuildOutsideArea : Str::FloorNothing));
        return;
    }
    if (error < 0) {
        speech::Say(S(Str::BuildFailed));
        return;
    }
    if (error > 0) {
        std::string reason = game::PlacementErrorText(error);
        speech::Say(reason.empty() ? std::string(S(Str::BuildFailed)) : reason);
        return;
    }
    // Floors show no price tooltip: their cost is the tiles laid at the style's price.
    int cost = game::LastDrawCost();
    bool laying = mode == game::FloorMode::Area || mode == game::FloorMode::FillRoom;
    if (cost <= 0 && laying) cost = tiles * g.price;
    std::string line = what;
    if (mode == game::FloorMode::FillRoom || mode == game::FloorMode::ClearRoom)
        line += ", " + util::Format(S(Str::FloorTiles), tiles);
    buffers::Event(cost > 0 ? line + ". " + util::Format(S(Str::BuildCostWas), Money(cost).c_str()) : line);
}

void BuildFloor(bool room, bool remove) {
    if (room) {  // the room under the cursor: no start needed
        DrawFloorAndSay(g.cursor, g.cursor, remove ? game::FloorMode::ClearRoom : game::FloorMode::FillRoom,
                        S(remove ? Str::FloorRoomCleared : Str::FloorRoomLaid));
        return;
    }
    Corner a = g.hasStart ? g.start : g.cursor, b = g.cursor;
    int tiles = Width(a, b) * Depth(a, b);
    DrawFloorAndSay(a, b, remove ? game::FloorMode::Remove : game::FloorMode::Area,
                    util::Format(S(remove ? Str::FloorRemoved : Str::FloorLaid), tiles));
}

}  // namespace

namespace {

void PaperRoom(bool strip) {
    g.pending = false;
    int room = game::RoomAt(g.level, g.cursor.x, g.cursor.y);
    // The room's tiles, those along a wall first: the tool takes the wall at a tile's edge.
    std::vector<Corner> walled, rest;
    int n = Size();
    for (int y = 1; y < n - 1; y++)
        for (int x = 1; x < n - 1; x++) {
            if (game::RoomAt(g.level, x, y) != room) continue;
            uint32_t w = game::WallsAt(g.level, x, y) & (game::kWallNorth | game::kWallWest | game::kWallSouth | game::kWallEast);
            (w ? walled : rest).push_back(Corner{x, y});
        }
    if (walled.empty()) {
        speech::Say(S(Str::PaperNoWalls));
        return;
    }
    // Unrotated, the tool takes a tile's north or west wall: those first (each miss beeps).
    std::stable_partition(walled.begin(), walled.end(), [&](Corner c) {
        return (game::WallsAt(g.level, c.x, c.y) & (game::kWallNorth | game::kWallWest)) != 0;
    });
    int sides = 0;
    int error = game::PaperRoom(walled, strip, &sides);
    if (error != 0) {
        speech::Say(S(Str::PaperNoWalls));
        return;
    }
    int cost = game::LastDrawCost();
    if (cost <= 0 && !strip) cost = sides * g.price;
    std::string line = util::Format(S(strip ? Str::PaperStripped : Str::PaperDone), sides);
    buffers::Event(cost > 0 ? line + ". " + util::Format(S(Str::BuildCostWas), Money(cost).c_str()) : line);
}

}  // namespace

namespace {

void BuildPool(bool remove) {
    g.pending = false;
    Corner a = g.hasStart ? g.start : g.cursor, b = g.cursor;
    Corner nw{std::min(a.x, b.x), std::min(a.y, b.y)}, se{std::max(a.x, b.x), std::max(a.y, b.y)};
    if (!remove) {
        std::vector<Tile> tiles;
        for (int y = nw.y; y <= se.y; y++)
            for (int x = nw.x; x <= se.x; x++) tiles.push_back(Tile{x, y});
        placement::WallCheck c = placement::CheckBlockedTiles(g.level, tiles);
        std::vector<std::string> parts;
        auto names = [](const std::vector<std::string>& v) {
            std::string s;
            for (const std::string& n : v) s += (s.empty() ? "" : ", ") + n;
            return s;
        };
        if (!c.blockedObjects.empty()) parts.push_back(util::Format(S(Str::BuildBlocksObjects), names(c.blockedObjects).c_str()));
        if (!c.trappedSims.empty()) parts.push_back(util::Format(S(Str::BuildTrapsSims), names(c.trappedSims).c_str()));
        if (c.doorsBlocked) parts.push_back(S(Str::BuildBlocksDoor));
        if (c.stairsBlocked) parts.push_back(S(Str::BuildBlocksStairs));
        if (c.cutOffTiles) parts.push_back(util::Format(S(Str::BuildCutsOff), c.cutOffTiles));
        std::string warning;
        for (const std::string& p : parts) warning += (warning.empty() ? "" : ". ") + p;
        if (!warning.empty() && !Confirmed(nw, se, 2)) {
            AskConfirm(warning, nw, se, 2);
            return;
        }
    }
    g.pending = false;
    int tiles = 0;
    int error = game::DrawPool(nw, se, remove, &tiles);
    if (error == game::kNothingBuilt) {
        speech::Say(S(Str::FloorNothing));
        return;
    }
    if (error != 0) {
        std::string reason = error > 0 ? game::PlacementErrorText(error) : "";
        speech::Say(reason.empty() ? std::string(S(Str::BuildFailed)) : reason);
        return;
    }
    int cost = game::LastDrawCost();
    if (cost <= 0 && !remove) cost = tiles * g.price;
    std::string line = util::Format(S(remove ? Str::PoolRemoved : Str::PoolBuilt), tiles);
    if (!remove) line += ". " + std::string(S(Str::PoolNeedsLadder));
    buffers::Event(cost > 0 ? line + ". " + util::Format(S(Str::BuildCostWas), Money(cost).c_str()) : line);
}

}  // namespace

namespace {

void BuildTerrain() {
    g.pending = false;
    if (g.terrain == 2) {
        int error = game::PressTerrain(g.cursor);
        if (error != 0) {
            std::string reason = error > 0 ? game::PlacementErrorText(error) : "";
            speech::Say(reason.empty() ? std::string(S(Str::BuildFailed)) : reason);
            return;
        }
        buffers::Event(S(Str::TerrainChanged));
        return;
    }
    Corner a = g.hasStart ? g.start : g.cursor;
    int cost = 0;
    int error = game::LevelTerrain(a, g.cursor, &cost);
    if (error != 0) {
        std::string reason = error > 0 ? game::PlacementErrorText(error) : "";
        speech::Say(reason.empty() ? std::string(S(Str::BuildFailed)) : reason);
        return;
    }
    std::string line = util::Format(S(Str::TerrainLeveled), Width(a, g.cursor), Depth(a, g.cursor));
    buffers::Event(cost > 0 ? line + ". " + util::Format(S(Str::BuildCostWas), Money(cost).c_str()) : line);
}

}  // namespace

void Build(bool room, bool remove) {
    if (g.terrain) {
        BuildTerrain();
        return;
    }
    if (g.pool) {
        BuildPool(remove);
        return;
    }
    if (g.paper) {
        (void)room;
        PaperRoom(remove);
        return;
    }
    if (g.floors) {
        BuildFloor(room, remove);
        return;
    }
    if (!g.hasStart) {
        speech::Say(S(Str::BuildNoStart));
        return;
    }
    Corner a = g.start, b = g.cursor;
    if (remove) {
        if (a == b) {
            speech::Say(S(Str::BuildAtStart));
            return;
        }
        Draw(a, b, game::WallMode::Delete, util::Format(S(Str::BuildRemoved), Segments(a, b)));
        return;
    }
    if (room) {
        BuildRoom(a, b);
        return;
    }
    Shape shape = ShapeOf(a, b);
    if (shape == kNone) {
        speech::Say(S(Str::BuildAtStart));
        return;
    }
    if (shape == kRoomOnly) {
        speech::Say(S(Str::BuildNotStraight));
        return;
    }
    std::string warning = Warning(EdgesOf(a, b), false);
    if (!warning.empty() && !Confirmed(a, b, 0)) {
        AskConfirm(warning, a, b, 0);
        return;
    }
    Draw(a, b, game::WallMode::Line, util::Format(S(shape == kDiagonal ? Str::BuiltDiagonal : Str::BuiltWall), Segments(a, b)));
    g.start = b;  // the next wall goes on from here, as a sighted builder draws room outlines
}

void NextPreset(int delta) {
    if (g.floors || g.paper || g.pool || g.terrain) {
        speech::Say(S(Str::FloorNoPresets));
        return;
    }
    g.preset = std::clamp(g.preset + delta, 0, kPresetCount - 1);
    const Preset& p = kPresets[g.preset];
    speech::Say(util::Format(S(Str::BuildPreset), p.w, p.h, p.w * p.h, Money(2 * (p.w + p.h) * g.price).c_str()));
}

void BuildPreset() {
    if (g.floors) {
        speech::Say(S(Str::FloorNoPresets));
        return;
    }
    const Preset& p = kPresets[g.preset];
    Corner b = Clamp({g.cursor.x + p.w, g.cursor.y + p.h});
    if (b.x - g.cursor.x != p.w || b.y - g.cursor.y != p.h) {
        speech::Say(S(Str::BuildPresetTooBig));
        return;
    }
    BuildRoom(g.cursor, b);
}

// Shift+PageUp / Shift+PageDown: the floor above or below, through the game's own floor buttons
// (choosing a tool puts the view back on the first floor).
void ChangeFloor(int delta) {
    int want = g.level + delta;
    game::Ptr vc = game::FindViewControl(game::InputRoot());
    game::Ptr button = vc ? game::FloorButton(vc, want) : 0;
    if (!button) {
        speech::Say(util::Format(S(Str::FloorNumber), g.level));
        return;
    }
    game::Press(button);
    game::Unhover(button);
    g.level = want;  // the game applies the press a moment later (Tick keeps g.level in step)
    g.levelSetAt = GetTickCount64();
    g.hasStart = false;
    g.rooms = rooms::NameRooms(game::LotObjects());
    std::vector<std::string> lines = FullLines();
    buffers::SetDetails(lines);
    speech::Say(util::Format(S(Str::FloorNumber), g.level) + ". " + LineWithRoom());
}

// Ctrl+PageDown / Ctrl+PageUp: the rooms of this floor (on an upper floor, of the floor below,
// where its walls and floors go), by the engine's room order; the cursor goes to the room's
// north-west corner.
void NextRoom(int delta) {
    int level = g.level;
    int n = Size();
    struct Box {
        int room, x0, y0, x1, y1;
    };
    std::vector<Box> boxes;
    for (int pass = 0; pass < 2 && boxes.empty(); pass++) {
        level = pass == 0 && g.level > 1 ? g.level - 1 : g.level;
        if (pass == 1 && level == (g.level > 1 ? g.level - 1 : g.level)) break;
        std::map<int, Box> byRoom;
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++) {
                int room = game::RoomAt(level, x, y);
                game::Room info;
                if (room == game::kNoRoom || !game::GetRoom(room, &info) || info.outside) continue;
                auto it = byRoom.find(room);
                if (it == byRoom.end()) byRoom[room] = Box{room, x, y, x, y};
                else {
                    Box& b = it->second;
                    b.x0 = std::min(b.x0, x);
                    b.y0 = std::min(b.y0, y);
                    b.x1 = std::max(b.x1, x);
                    b.y1 = std::max(b.y1, y);
                }
            }
        for (auto& e : byRoom) boxes.push_back(e.second);
    }
    if (boxes.empty()) {
        speech::Say(S(Str::BuildNoRooms));
        return;
    }
    int count = (int)boxes.size();
    g.roomIndex = g.roomIndex < 0 ? (delta > 0 ? 0 : count - 1) : ((g.roomIndex + delta) % count + count) % count;
    const Box& b = boxes[g.roomIndex];
    g.cursor = Clamp({b.x0, b.y0});
    std::string line = util::Format(S(Str::BuildRoomJump), RoomNameAt(level, Tile{b.x0, b.y0}).c_str(),
                                    b.x1 - b.x0 + 1, b.y1 - b.y0 + 1);
    std::vector<std::string> lines = FullLines();
    buffers::SetDetails(lines);
    RoomChanged();  // the jump names the room already
    speech::Say(line + ". " + ShortLine());
}

void Home() {
    game::LotObject sim;
    if (!game::GetObject(game::SelectedSim(), &sim)) return;
    g.cursor = Clamp({sim.x >> 4, sim.y >> 4});
    SayShort();
}

void Repeat() {
    std::vector<std::string> lines = FullLines();
    buffers::SetDetails(lines);
    std::string all;
    for (const std::string& l : lines) all += (all.empty() ? "" : ". ") + l;
    speech::Say(all);
}

void End() {
    g.active = false;
    g.hasStart = false;
    g.pending = false;
    speech::Say(S(Str::BuildCursorEnd));
}

}  // namespace nav::build
