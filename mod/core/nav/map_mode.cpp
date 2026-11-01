#include "map_mode.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
#include "../game/rtti.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "build_mode.h"
#include "buy_mode.h"
#include "cues.h"
#include "navigator.h"
#include "placement.h"
#include "room_names.h"
#include "world.h"

namespace nav::map {
namespace {

constexpr int kMaxLevel = 2;  // The Sims 1 builds two floors (the maps above hold garbage)

struct State {
    bool active = false;
    int x = 0, y = 0, level = 1;
    int lastRoom = -2;
    int lastLevel = 0;
    std::string lastRoomName;
    game::Surface lastSurface = game::Surface::Ground;
    bool haveSurface = false;
    std::vector<rooms::NamedRoom> rooms;
};

State g;

std::string RoomName(int level, int x, int y) {
    int room = game::RoomAt(level, x, y);
    std::string name = rooms::NameOf(g.rooms, room);
    game::Room info;
    if (!game::GetRoom(room, &info)) return name;
    if (name.empty()) return info.outside ? S(Str::RoomOutside) : util::Format(S(Str::RoomEmpty), game::RoomTileCount(room));
    return util::Format(S(Str::BuildRoomSize), name.c_str(), info.tiles);
}

// The lot's thing (its lead entry, what the browser and the menus use) a placed part belongs to.
bool ThingOf(game::Ptr part, const std::vector<game::LotObject>& objects, game::LotObject* out) {
    for (const game::LotObject& o : objects) {
        if (o.object == part) { *out = o; return true; }
        std::vector<game::Ptr> parts = game::ObjectParts(o.object);
        if (std::find(parts.begin(), parts.end(), part) != parts.end()) { *out = o; return true; }
    }
    return false;
}

// What stands on the cursor's tile, Sims first: one entry per thing.
std::vector<game::LotObject> ThingsHere() {
    std::vector<game::LotObject> objects = game::LotObjects();
    std::vector<game::LotObject> out;
    for (const game::PlacedPart& p : game::PlacedParts()) {
        if (p.level != g.level || p.tileX != g.x || p.tileY != g.y) continue;
        if (game::IsPortal(p.object) || game::IsInvisibleHelper(p.object)) continue;
        game::LotObject o;
        if (!ThingOf(p.object, objects, &o)) continue;
        bool seen = std::any_of(out.begin(), out.end(), [&](const game::LotObject& e) { return e.id == o.id; });
        if (!seen) out.push_back(o);
    }
    std::stable_partition(out.begin(), out.end(), [](const game::LotObject& o) { return o.person; });
    return out;
}

// "Cama doble, mirando al sur"; Sims by name.
std::string ThingWords(const game::LotObject& o) {
    std::string name = world::DisplayName(o);
    if (o.person) return name;
    int compass = game::CompassOf(game::ObjectDirection(o.object));
    return name + ", " + util::Format(S(Str::FacingDirection), S(placement::CompassWord(compass)));
}

// The line for the cursor's tile: what stands there, the room and floor when they change, the
// walls and openings, stairs, a slope; the tile last (as the build cursor's line).
std::string Line(bool full) {
    std::vector<std::string> parts;
    for (const game::LotObject& o : ThingsHere()) parts.push_back(ThingWords(o));
    cues::TileSense t = cues::Sense(g.level, g.x, g.y);
    if (t.stairs) parts.push_back(S(Str::MapStairs));
    int room = game::RoomAt(g.level, g.x, g.y);
    std::string roomName = RoomName(g.level, g.x, g.y);
    // The half tiles beside a diagonal wall belong to no room: the room goes on being the last one.
    if (!roomName.empty()) {
        bool roomChanged = room != g.lastRoom || g.level != g.lastLevel;
        if (full || roomChanged) {
            if (!full && !g.lastRoomName.empty() && g.lastLevel == g.level && g.lastRoomName != roomName)
                parts.push_back(util::Format(S(Str::RoomChange), g.lastRoomName.c_str(), roomName.c_str()));
            else
                parts.push_back(roomName);
        }
        g.lastRoom = room;
        g.lastLevel = g.level;
        g.lastRoomName = roomName;
    }
    if (full || !g.haveSurface || t.surface != g.lastSurface) parts.push_back(cues::SurfaceWord(t.surface));
    g.lastSurface = t.surface;
    g.haveSurface = true;
    std::string edges = cues::EdgesLine(t);
    if (!edges.empty()) parts.push_back(edges);
    if (t.sloped) parts.push_back(S(Str::TerrainSloped));
    std::string line;
    for (const std::string& p : parts) line += (line.empty() ? "" : ", ") + p;
    return line + (line.empty() ? "" : ". ") + util::Format(S(Str::TileAt), g.x, g.y);
}

void SayHere(bool full = false) {
    std::string line = Line(full);
    std::vector<std::string> details = {line};
    for (const game::LotObject& o : ThingsHere()) {
        std::vector<std::string> more = world::DetailsOf(o.id);
        details.insert(details.end(), more.begin(), more.end());
    }
    details.push_back(util::Format(S(Str::FloorNumber), g.level));
    buffers::SetDetails(details);
    speech::Say(line);
    cues::Tile(g.level, g.x, g.y);
}

bool AtSim() {
    game::LotObject sim;
    if (!game::GetObject(game::SelectedSim(), &sim)) return false;
    g.x = sim.x >> 4;
    g.y = sim.y >> 4;
    g.level = std::clamp(sim.level, 1, kMaxLevel);
    return true;
}

// Whether a floor has anything built on it (upstairs: a floor tile or a room).
bool FloorExists(int level) {
    if (level == 1) return true;
    int n = game::GridSize();
    for (int y = 1; y < n - 1; y++)
        for (int x = 1; x < n - 1; x++)
            if (game::HasFloor(level, x, y)) return true;
    return false;
}

}  // namespace

void Reset() { g = State{}; }

bool Active() { return g.active; }

bool CursorTile(int* x, int* y, int* level) {
    if (!g.active) return false;
    *x = g.x;
    *y = g.y;
    *level = g.level;
    return true;
}

void Tick() {
    if (!g.active) return;
    // An interaction menu or a dialog over the lot only takes the keys for a while (MapKey works
    // in the lot view alone); leaving the lot or a building tool ends the map.
    if (!game::LotModule() || build::Active() || buy::Placing()) {
        Log("map: ended (view or tool changed)");
        g.active = false;
    }
}

void Toggle() {
    if (g.active) {
        End();
        return;
    }
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    if (build::Active() || buy::Placing()) return;  // those cursors own the space already
    if (!AtSim()) {
        int n = game::GridSize();
        g.x = g.y = n / 2;
        g.level = 1;
    }
    g.active = true;
    g.lastRoom = -2;
    g.lastRoomName.clear();
    g.haveSurface = false;
    g.rooms = rooms::NameRooms(game::LotObjects());
    cues::Reset();
    std::string line = Line(true);
    buffers::SetDetails({line});
    speech::Say(std::string(S(Str::MapOn)) + " " + util::Format(S(Str::FloorNumber), g.level) + ". " + line);
    cues::Tile(g.level, g.x, g.y);
}

void End() {
    if (!g.active) return;
    g.active = false;
    speech::Say(S(Str::MapOff));
}

void Move(int dx, int dy) {
    int n = game::GridSize();
    int x = std::clamp(g.x + dx, 1, n - 2), y = std::clamp(g.y + dy, 1, n - 2);
    if (x == g.x && y == g.y) {
        speech::Say(S(Str::BuildLotEdge));
        cues::LotEdge();
        return;
    }
    g.x = x;
    g.y = y;
    SayHere();
}

void Home() {
    if (!AtSim()) {
        speech::Say(S(Str::NoSimSelected));
        return;
    }
    SayHere();
}

void ChangeFloor(int delta) {
    int want = g.level + delta;
    if (want < 1 || want > kMaxLevel || !FloorExists(want)) {
        speech::Say(util::Format(S(Str::MapNoFloor), want < 1 ? 1 : want));
        return;
    }
    g.level = want;
    g.rooms = rooms::NameRooms(game::LotObjects());
    std::string line = Line(true);
    buffers::SetDetails({line});
    speech::Say(util::Format(S(Str::FloorNumber), g.level) + ". " + line);
    cues::Tile(g.level, g.x, g.y);
}

void Repeat() { SayHere(true); }

void Activate() {
    std::vector<game::LotObject> here = ThingsHere();
    if (here.empty()) {
        speech::Say(S(Str::MapNothingHere));
        return;
    }
    // A Sim standing there first, else the object (a lamp on a table: the table's last in the list).
    world::Focus(here.front().id);
    if (game::InBuyTool()) buy::PickUpFocused();
    else world::Interact();
}

void WalkHere() {
    if (g.level != 1 && !game::HasFloor(g.level, g.x, g.y)) {
        speech::Say(S(Str::CannotWalk));
        return;
    }
    world::WalkToSpot(g.x * 16 + 8, g.y * 16 + 8);
}

void FollowFocus() {
    if (!g.active) return;
    game::LotObject o;
    game::Ptr focused = world::FocusedObject();
    if (!focused || !game::GetObject(focused, &o)) return;
    g.x = o.x >> 4;
    g.y = o.y >> 4;
    g.level = std::clamp(o.level, 1, kMaxLevel);
    g.lastRoom = game::RoomAt(g.level, g.x, g.y);  // the browser named the room already
    g.lastLevel = g.level;
    g.lastRoomName = RoomName(g.level, g.x, g.y);
    cues::Tile(g.level, g.x, g.y);
}

}  // namespace nav::map
