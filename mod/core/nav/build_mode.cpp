#include "build_mode.h"

#include <windows.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "../game/build.h"
#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
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
    bool floors = false;     // the floor tool: the cursor is on tiles, not corners
    Corner cursor;
    bool hasStart = false;
    Corner start;
    int level = 1;
    int price = 0;           // of the chosen style, per wall segment
    std::string style;       // its name
    int preset = 2;
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

std::string RoomName(Tile t) {
    int room = game::RoomAt(g.level, t.x, t.y);
    std::string name = rooms::NameOf(g.rooms, room);
    game::Room info;
    if (!game::GetRoom(room, &info)) return name;
    if (name.empty()) return info.outside ? S(Str::RoomOutside) : util::Format(S(Str::RoomEmpty), info.tiles);
    return util::Format(S(Str::BuildRoomSize), name.c_str(), info.tiles);
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

std::string PlanLine() {
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

std::string ShortLine() {
    std::string line = util::Format(S(Str::BuildCorner), g.cursor.x, g.cursor.y);
    if (!game::CornerBuildable(g.cursor)) line += ", " + std::string(S(Str::BuildNotBuildableHere));
    if (g.floors) {
        line += ", " + std::string(S(game::HasFloor(g.level, g.cursor.x, g.cursor.y) ? Str::FloorHere : Str::FloorNone));
    } else {
        std::vector<Str> walls = WallsAtCorner(g.cursor);
        if (!walls.empty()) line += ", " + util::Format(S(Str::BuildWallsHere), JoinDirections(walls).c_str());
    }
    std::string object = ObjectOn(TileSE(g.cursor));
    if (!object.empty()) line += ", " + object;
    std::string plan = PlanLine();
    if (!plan.empty()) line += ". " + plan;
    return line;
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
    speech::Say(ShortLine());
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
    if (c.cutOffTiles && c.blockedObjects.empty() && !c.doorsBlocked && c.trappedSims.empty())
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
        onlyNoDoor = c.blockedObjects.empty() && !c.doorsBlocked && c.trappedSims.empty();
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

bool FloorCursor() { return g.active && g.floors; }

bool Begin(uintptr_t productButton) {
    if (!LotViewActive()) return false;
    g.pressed = productButton;
    g.pressedAt = GetTickCount64();
    return true;
}

namespace {

constexpr ULONGLONG kSettleMs = 250, kGiveUpMs = 1500;

bool ToolActive() { return g.floors ? game::FloorToolActive() : game::WallToolActive(); }

bool Start(uintptr_t productButton) {
    if (!LotViewActive() || (!game::WallToolActive() && !game::FloorToolActive())) return false;
    bool floors = game::FloorToolActive();
    if (g.active && floors != g.floors) g.active = false;  // another tool: start afresh
    g.floors = floors;
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
    speech::Say(util::Format(S(g.floors ? Str::FloorCursorStart : Str::BuildCursorStart), g.style.c_str()) + " " + ShortLine());
    return true;
}

}  // namespace

void Tick() {
    if (g.pressed) {
        ULONGLONG waited = GetTickCount64() - g.pressedAt;
        if (waited >= kSettleMs && (Start(g.pressed) || waited > kGiveUpMs)) g.pressed = 0;
    }
    if (!g.active) return;
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

void Build(bool room, bool remove) {
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
    if (g.floors) {
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
