#include "placement.h"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <map>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/rtti.h"
#include "../util/log.h"
#include "../util/text.h"
#include "room_names.h"
#include "world.h"

namespace nav::placement {
namespace {

using game::Ptr;
using game::Tile;

constexpr int kOutdoorRadius = 8;   // outdoors, candidate tiles this close to the selected Sim...
constexpr int kYardDepth = 5;       // ...or to the house (its indoor tiles)
constexpr int kMaxCutOff = 4;       // spots that cut off more floor than this are left out

// Relative sides of an object, as routing slot direction bits (0 = its front).
constexpr uint32_t kBackSides = (1u << 3) | (1u << 4) | (1u << 5);  // SE, S, SW

// One floor of the lot, read once per search.
struct Floor {
    int size = 0;
    int level = 1;
    std::vector<uint32_t> walls;      // wall edges per tile
    std::vector<int> objectId;        // what stands on each tile
    std::vector<uint8_t> blocked;     // something Sims cannot walk through
    std::vector<uint8_t> portal;      // a door or stairs: Sims cross its walls
    std::vector<uint8_t> keepClear;   // in front of and behind doors
    std::vector<int> room;
    std::vector<Tile> starts;         // where Sims come from: the Sims, the lot's entrance
    std::vector<Tile> entrance;       // the lot's entrance alone (or the stairs, upstairs)

    bool Inside(Tile t) const { return t.x >= 0 && t.y >= 0 && t.x < size && t.y < size; }
    int Index(Tile t) const { return t.y * size + t.x; }
};

// The wall bit for leaving a tile toward an engine direction (6 -y, 0 -x, 2 +y, 4 +x).
uint32_t WallBit(int direction) {
    switch (direction & 7) {
    case 6: return game::kWallNorth;
    case 0: return game::kWallWest;
    case 2: return game::kWallSouth;
    case 4: return game::kWallEast;
    }
    return 0;
}

int DirectionBetween(Tile a, Tile b) {  // orthogonal neighbors only, engine direction
    if (b.y < a.y) return 6;
    if (b.x < a.x) return 0;
    if (b.y > a.y) return 2;
    return 4;
}

bool CanCross(const Floor& f, Tile a, Tile b) {
    if (!f.Inside(a) || !f.Inside(b)) return false;
    if (!(f.walls[f.Index(a)] & WallBit(DirectionBetween(a, b)))) return true;
    return f.portal[f.Index(a)] || f.portal[f.Index(b)];
}

bool Standable(const Floor& f, Tile t, const std::vector<uint8_t>& extra) {
    if (!game::InGrid(t.x, t.y)) return false;
    int i = f.Index(t);
    if (f.blocked[i] || extra[i] || (f.walls[i] & game::kWallDiagonals)) return false;
    return f.level == 1 || game::HasFloor(f.level, t.x, t.y) || f.portal[i];
}

// Tiles a Sim can walk to from the floor's starting points, with `extra` also blocked.
std::vector<uint8_t> Reach(const Floor& f, const std::vector<uint8_t>& extra) {
    std::vector<uint8_t> seen(f.size * f.size, 0);
    std::vector<Tile> queue;
    for (Tile s : f.starts)
        if (f.Inside(s) && !seen[f.Index(s)] && !extra[f.Index(s)]) {
            seen[f.Index(s)] = 1;
            queue.push_back(s);
        }
    for (size_t head = 0; head < queue.size(); head++) {
        Tile t = queue[head];
        for (int d = 0; d < 8; d += 2) {
            Tile n = game::Step(t, d);
            if (!f.Inside(n) || seen[f.Index(n)] || !Standable(f, n, extra) || !CanCross(f, t, n)) continue;
            seen[f.Index(n)] = 1;
            queue.push_back(n);
        }
    }
    return seen;
}

Tile TileOf(const game::LotObject& o) { return Tile{o.x >> 4, o.y >> 4}; }

Tile TileOf(Ptr object) {
    game::LotObject o;
    return game::GetObject(object, &o) ? TileOf(o) : Tile{-1, -1};
}

bool IsPerson(Ptr object) { return game::IsKindOf(object, "cXPerson"); }

uint16_t FunctionOf(Ptr object) {
    game::LotObject o;
    return game::GetObject(object, &o) ? o.functionFlags : 0;
}

Floor ReadFloor(int level) {
    Floor f;
    f.size = game::GridSize();
    f.level = level;
    int n = f.size * f.size;
    f.walls.assign(n, 0);
    f.objectId.assign(n, 0);
    f.blocked.assign(n, 0);
    f.portal.assign(n, 0);
    f.keepClear.assign(n, 0);
    f.room.assign(n, game::kNoRoom);
    for (int y = 0; y < f.size; y++)
        for (int x = 0; x < f.size; x++) {
            int i = y * f.size + x;
            f.walls[i] = game::WallsAt(level, x, y);
            f.room[i] = game::RoomAt(level, x, y);
        }
    // What stands on each tile, from every placed part (the engine's per-tile map keeps one id,
    // and a window on the wall hides the counter below it). The object that blocks the tile wins.
    for (const game::PlacedPart& part : game::PlacedParts()) {
        Tile t{part.tileX, part.tileY};
        if (part.level != level || !f.Inside(t)) continue;
        int i = f.Index(t);
        Ptr object = part.object;
        if (IsPerson(object) || game::IsInvisibleHelper(object) || game::ContainerOf(object) != 0) continue;
        // Doors and stair landings let Sims through (across the wall a door sits in); windows
        // stand in walls without a floor footprint; a stair's body blocks the floor.
        if (game::IsPortal(object) && game::CanWalkOver(object)) {
            if (game::PeoplePass(object)) {
                f.portal[i] = 1;
                if (!f.objectId[i]) f.objectId[i] = part.id;
            }
            continue;
        }
        if (game::CanWalkOver(object)) {
            if (!f.objectId[i]) f.objectId[i] = part.id;
            continue;
        }
        if (!f.blocked[i]) f.objectId[i] = part.id;  // the first blocking part: the floor one
        f.blocked[i] = 1;
    }
    // Doors and stairs keep the tiles in front of and behind them clear.
    for (int i = 0; i < n; i++) {
        if (!f.portal[i]) continue;
        Tile t{i % f.size, i / f.size};
        int dir = game::ObjectDirection(game::ObjectById(f.objectId[i]));
        f.keepClear[i] = 1;
        for (int d : {dir, dir + 4}) {
            Tile s = game::Step(t, d);
            if (f.Inside(s)) f.keepClear[f.Index(s)] = 1;
        }
    }
    // Where Sims come from: every Sim on this floor, the lot's entrance (its invisible portals on
    // the ground floor), the stairs on upper floors.
    for (const game::LotObject& o : game::LotObjects())
        if (o.person && o.level == level) f.starts.push_back(TileOf(o));
    // The ground floor's entrance: the lot's invisible edge portals (where visitors walk on). The
    // per-tile ids above leave helpers out, so they are taken from the parts themselves.
    if (level == 1) {
        for (const game::PlacedPart& part : game::PlacedParts()) {
            Tile t{part.tileX, part.tileY};
            if (part.level == level && f.Inside(t) && game::IsLotEntrance(part.object))
                f.entrance.push_back(t);
        }
    } else {
        for (int i = 0; i < n; i++)
            if (f.portal[i]) f.entrance.push_back({i % f.size, i / f.size});
    }
    f.starts.insert(f.starts.end(), f.entrance.begin(), f.entrance.end());
    return f;
}

// Where a Sim uses the object from, among `use`: standable, reachable, and on the object's side
// of any wall.
std::vector<Tile> UsableTiles(const Floor& f, const std::vector<Tile>& use, const std::vector<Tile>& footprint,
                              const std::vector<uint8_t>& extra, const std::vector<uint8_t>& reach) {
    std::vector<Tile> out;
    for (Tile u : use) {
        if (!f.Inside(u) || !Standable(f, u, extra) || !reach[f.Index(u)]) continue;
        bool adjacent = false, open = false;
        for (Tile p : footprint) {
            int dx = p.x - u.x, dy = p.y - u.y;
            if (std::abs(dx) > 1 || std::abs(dy) > 1) continue;
            adjacent = true;
            if (dx == 0 || dy == 0) open |= CanCross(f, u, p);
            else open |= (CanCross(f, u, {p.x, u.y}) && CanCross(f, {p.x, u.y}, p)) ||
                         (CanCross(f, u, {u.x, p.y}) && CanCross(f, {u.x, p.y}, p));
        }
        if (!adjacent || open) out.push_back(u);
    }
    return out;
}

// An object already on the floor, with the tiles it is used from.
struct Existing {
    int id = 0;
    std::vector<Tile> use;
};

struct HeldTraits {
    uint32_t sides = 0;       // routing slot sides of every part (relative)
    uint16_t function = 0;    // buy mode category flags
    uint16_t rooms = 0;       // suggested rooms
};

HeldTraits ReadTraits(Ptr held) {
    HeldTraits t;
    t.sides = game::RoutingSides(held);
    game::LotObject o;
    if (game::GetObject(held, &o)) {
        t.function = o.functionFlags;
        t.rooms = o.roomFlags;
    }
    return t;
}

// Sides (bit r = front, left, back, right: direction + 2r, the engine turning counter-clockwise)
// along which the whole footprint touches a wall.
int WallSides(const Floor& f, const std::vector<Tile>& footprint, int direction) {
    if (direction & 1) return 0;
    int sides = 0;
    for (int r = 0; r < 4; r++) {
        int d = (direction + 2 * r) & 7;
        int edges = 0, walled = 0;
        for (Tile t : footprint) {
            Tile n = game::Step(t, d);
            if (std::find(footprint.begin(), footprint.end(), n) != footprint.end()) continue;
            edges++;
            walled += (f.walls[f.Index(t)] & WallBit(d)) != 0;
        }
        if (edges > 0 && walled == edges) sides |= 1 << r;
    }
    return sides;
}

// Next to each other for a player: in the same room with no wall between them.
bool Touching(const Floor& f, Tile a, Tile b) {
    if (!f.Inside(a) || !f.Inside(b) || f.room[f.Index(a)] != f.room[f.Index(b)]) return false;
    if (a.x == b.x || a.y == b.y) return CanCross(f, a, b);
    return (CanCross(f, a, {b.x, a.y}) && CanCross(f, {b.x, a.y}, b)) ||
           (CanCross(f, a, {a.x, b.y}) && CanCross(f, {a.x, b.y}, b));
}

// The object the front of the footprint touches (a table in front of a chair), 0 if none.
int FacedObject(const Floor& f, const std::vector<Tile>& footprint, int direction) {
    for (Tile t : footprint) {
        Tile n = game::Step(t, direction);
        if (!f.Inside(n) || std::find(footprint.begin(), footprint.end(), n) != footprint.end()) continue;
        if (f.blocked[f.Index(n)] && Touching(f, t, n)) return f.objectId[f.Index(n)];
    }
    return 0;
}

// Objects around the footprint (not Sims) on its side of the walls: their ids.
std::vector<int> Neighbors(const Floor& f, const std::vector<Tile>& footprint) {
    std::vector<int> ids;
    for (Tile t : footprint)
        for (int d = 0; d < 8; d++) {
            Tile n = game::Step(t, d);
            if (!f.Inside(n) || !f.blocked[f.Index(n)] || !Touching(f, t, n)) continue;
            int id = f.objectId[f.Index(n)];
            if (std::find(ids.begin(), ids.end(), id) == ids.end()) ids.push_back(id);
        }
    return ids;
}

// Lead objects share one name for all their parts; neighbors are named by their lead.
Ptr LeadOf(Ptr object) {
    std::vector<Ptr> parts = game::ObjectParts(object);
    return parts.empty() ? object : parts[0];
}

// The tiles of an area. The outdoors is the whole lot, so only the yard is searched: tiles near
// the house and near the selected Sim.
std::vector<Tile> AreaTiles(const Floor& f, int room, bool outside, Tile center) {
    std::vector<int> nearHouse;
    if (outside) {  // distance (in tiles, 8-neighborhood) to the nearest indoor tile, up to the yard's depth
        nearHouse.assign(f.size * f.size, kYardDepth + 1);
        std::vector<Tile> queue;
        for (int i = 0; i < f.size * f.size; i++) {
            game::Room info;
            int r = f.room[i];
            if (r == game::kNoRoom || r == room) continue;
            nearHouse[i] = 0;
            queue.push_back({i % f.size, i / f.size});
        }
        for (size_t head = 0; head < queue.size(); head++) {
            Tile t = queue[head];
            int d = nearHouse[f.Index(t)];
            if (d >= kYardDepth) continue;
            for (int dir = 0; dir < 8; dir++) {
                Tile n = game::Step(t, dir);
                if (!f.Inside(n) || nearHouse[f.Index(n)] <= d + 1) continue;
                nearHouse[f.Index(n)] = d + 1;
                queue.push_back(n);
            }
        }
    }
    std::vector<Tile> tiles;
    for (int y = 1; y < f.size - 1; y++)
        for (int x = 1; x < f.size - 1; x++) {
            if (f.room[y * f.size + x] != room) continue;
            bool nearSim = std::abs(x - center.x) <= kOutdoorRadius && std::abs(y - center.y) <= kOutdoorRadius;
            if (outside && !nearSim && nearHouse[y * f.size + x] > kYardDepth) continue;
            tiles.push_back({x, y});
        }
    return tiles;
}

double Distance(Tile a, Tile b) { return std::hypot(double(a.x - b.x), double(a.y - b.y)); }

// Spots on top of the object standing on `t` (a lamp on a table): the first free place on it the
// engine accepts, as the buy tool picks one. Usable when a Sim can reach the surface.
void TrySurface(const Floor& f, Area& area, Tile t, int level, Ptr held) {
    Ptr surface = game::ObjectById(f.objectId[f.Index(t)]);
    for (int slot : game::FreeSlotsFor(surface)) {
        if (game::CheckHeldOn(t, level, surface, slot) != 0) continue;
        Spot spot;
        spot.tile = t;
        spot.direction = -1;
        spot.surface = f.objectId[f.Index(t)];
        spot.footprint = {t};
        std::vector<uint8_t> none(f.size * f.size, 0);
        std::vector<uint8_t> reach = Reach(f, none);
        for (int d = 0; d < 8; d += 2) {
            Tile n = game::Step(t, d);
            if (f.Inside(n) && reach[f.Index(n)] && Touching(f, t, n)) spot.useTiles.push_back(n);
        }
        if (spot.useTiles.empty()) {
            area.unusable++;
            return;
        }
        spot.score = 10;
        spot.nextTo = spot.surface;
        area.spots.push_back(spot);
        return;
    }
}

}  // namespace

std::vector<Area> FindSpots() {
    std::vector<Area> areas;
    Ptr held = game::HeldObject();
    if (!held) return areas;
    ULONGLONG started = GetTickCount64();
    int level = game::HeldLevel();
    bool moving = !game::IsBuying();
    Tile from = TileOf(held);  // a moved object is still where it was
    int fromRoom = moving && game::InGrid(from.x, from.y) ? game::RoomAt(level, from.x, from.y) : game::kNoRoom;
    game::LiftHeld();
    Floor f = ReadFloor(level);
    std::vector<uint8_t> none(f.size * f.size, 0);
    std::vector<uint8_t> reachBefore = Reach(f, none);
    HeldTraits traits = ReadTraits(held);
    bool blocksFloor = !game::CanWalkOver(held);

    // Objects already here and the tiles they are used from (only those usable now count).
    std::vector<Existing> existing;
    std::vector<game::LotObject> objects = game::LotObjects();
    for (const game::LotObject& o : objects) {
        if (o.person || o.level != level || game::IsPortal(o.object) || o.object == held) continue;
        Tile at = TileOf(o);
        std::vector<Tile> fp = game::FootprintAt(o.object, at);
        std::vector<Tile> use =
            UsableTiles(f, game::UseTilesAt(o.object, at, game::ObjectDirection(o.object)), fp, none, reachBefore);
        if (!use.empty()) existing.push_back({o.id, use});
    }

    // Doors and windows are placed by wall; rooms that have no door yet come first.
    bool heldPortal = game::IsPortal(held);
    bool heldDoor = heldPortal && game::PeoplePass(held);
    std::map<int, int> doorsIn;  // room -> doors on its walls
    for (int i = 0; i < f.size * f.size; i++) {
        if (!f.portal[i]) continue;
        Tile t{i % f.size, i / f.size};
        Ptr door = game::ObjectById(f.objectId[i]);
        doorsIn[f.room[i]]++;
        Tile b = game::Step(t, (game::ObjectDirection(door) + 4) & 7);
        if (f.Inside(b) && f.room[f.Index(b)] != f.room[i]) doorsIn[f.room[f.Index(b)]]++;
    }

    // The reference point for distances and the outdoors: the selected Sim.
    game::LotObject sim;
    bool hasSim = game::GetObject(game::SelectedSim(), &sim) && sim.level == level;
    Tile center = hasSim ? TileOf(sim) : Tile{f.size / 2, f.size / 2};

    std::vector<rooms::NamedRoom> named = rooms::NameRooms(objects);
    std::map<int, bool> roomSeen;
    for (int i = 0; i < f.size * f.size; i++)
        if (f.room[i] != game::kNoRoom) roomSeen[f.room[i]] = true;

    int startDirection = game::ObjectDirection(held);
    int step = game::HeldRotationStep();
    std::vector<int> directions;
    if (step <= 0) directions.push_back(startDirection);
    else
        for (int d = 0; d < 8; d += std::max(step, 2)) directions.push_back((startDirection + d) & 7);

    for (const auto& entry : roomSeen) {
        Area area;
        area.room = entry.first;
        game::Room info;
        bool outside = game::GetRoom(area.room, &info) && info.outside;
        area.name = rooms::NameOf(named, area.room);
        for (const rooms::NamedRoom& r : named)
            if (r.id == area.room) area.kind = r.kind;
        if (area.name.empty()) {
            area.kind = outside ? Str::RoomOutside : Str::RoomOther;
            area.name = outside ? S(Str::RoomOutside) : util::Format(S(Str::RoomEmpty), info.tiles);
        }
        areas.push_back(area);
    }

    std::map<int, int> errors;  // per area: reason -> count, folded below
    for (int direction : directions) {
        if (!game::TurnHeld(direction)) {
            Log("placement: cannot turn to %d", direction);
            continue;
        }
        for (Area& area : areas) {
            game::Room info;
            bool outside = game::GetRoom(area.room, &info) && info.outside;
            std::map<int, int> reasons;
            for (Tile t : AreaTiles(f, area.room, outside, center)) {
                if (direction == directions.front() && f.blocked[f.Index(t)]) TrySurface(f, area, t, level, held);
                int error = game::CheckHeldAt(t, level);
                if (error != 0) {
                    area.engineRefused++;
                    reasons[error]++;
                    continue;
                }
                Spot spot;
                spot.tile = t;
                spot.direction = direction;
                spot.footprint = game::FootprintAt(held, t);
                // Objects with no floor footprint (wall hangings) block nothing; the rest block
                // their tiles, which must not be in front of a door.
                std::vector<uint8_t> extra(f.size * f.size, 0);
                bool clear = true;
                for (Tile p : spot.footprint) {
                    if (!f.Inside(p) || !blocksFloor) continue;
                    extra[f.Index(p)] = 1;
                    clear &= !f.keepClear[f.Index(p)];
                }
                if (!clear) {
                    area.blocksDoor++;
                    continue;
                }
                std::vector<uint8_t> reach = blocksFloor ? Reach(f, extra) : reachBefore;
                spot.useTiles = UsableTiles(f, game::UseTilesAt(held, t, direction), spot.footprint, extra, reach);
                if (spot.useTiles.empty()) {
                    area.unusable++;
                    continue;
                }
                int broken = 0;
                for (const Existing& e : existing) {
                    bool stillUsable = false;
                    for (Tile u : e.use) stillUsable |= !extra[f.Index(u)] && reach[f.Index(u)];
                    if (!stillUsable) broken = e.id;
                }
                if (broken) {
                    area.blocksOthers++;
                    area.blockedObject = broken;
                    continue;
                }
                for (int i = 0; i < f.size * f.size; i++)
                    spot.cutOff += f.room[i] == area.room && reachBefore[i] && !reach[i] && !extra[i];
                if (spot.cutOff > kMaxCutOff) {
                    area.unusable++;
                    continue;
                }
                if (heldPortal) {  // a door or window: which wall, which rooms, a room's first door
                    spot.portal = true;
                    spot.roomHere = f.room[f.Index(t)];
                    Tile behind = game::Step(t, (direction + 4) & 7);
                    spot.roomBehind = f.Inside(behind) ? f.room[f.Index(behind)] : game::kNoRoom;
                    bool joins = spot.roomHere != spot.roomBehind;
                    // The same place in the wall seen from outside: said from the room's side.
                    game::Room hereInfo, behindInfo;
                    if (joins && game::GetRoom(spot.roomHere, &hereInfo) && hereInfo.outside &&
                        game::GetRoom(spot.roomBehind, &behindInfo) && !behindInfo.outside)
                        continue;
                    spot.firstDoor = heldDoor && joins && (!doorsIn[spot.roomHere] || !doorsIn[spot.roomBehind]);
                    spot.score = (spot.firstDoor ? 100 : 0) + (joins ? 20 : 0);
                    area.spots.push_back(spot);
                    continue;
                }
                // Score: what makes a spot look deliberate to a sighted player.
                spot.wallSides = WallSides(f, spot.footprint, direction);
                spot.facing = FacedObject(f, spot.footprint, direction);
                std::vector<int> around = Neighbors(f, spot.footprint);
                spot.nextTo = around.empty() ? 0 : around.front();
                bool usedFromFront = traits.sides != 0 && !(traits.sides & kBackSides);
                if (usedFromFront && (spot.wallSides & 4)) spot.score += 30;
                else if (usedFromFront && (spot.wallSides & 0xa)) spot.score += 5;
                if (!usedFromFront) spot.score += 2 * (int)spot.useTiles.size();
                Ptr faced = spot.facing ? LeadOf(game::ObjectById(spot.facing)) : 0;
                if (faced && (traits.function & game::kFnSeating) && (FunctionOf(faced) & game::kFnSurfaces)) spot.score += 40;
                int related = 0;
                for (int id : around)
                    related += (FunctionOf(LeadOf(game::ObjectById(id))) & traits.function) != 0;
                spot.score += 5 * std::min(related, 2);
                spot.score -= 15 * spot.cutOff;
                for (Tile p : spot.footprint)
                    for (int d = 0; d < 8; d += 2) {
                        Tile n = game::Step(p, d);
                        if (f.Inside(n) && f.keepClear[f.Index(n)]) spot.score -= 5;
                    }
                area.spots.push_back(spot);
            }
            for (const auto& r : reasons) errors[area.room * 64 + std::min(r.first, 63)] += r.second;
        }
    }
    game::TurnHeld(startDirection);

    for (Area& area : areas) {
        int best = 0;
        for (const auto& e : errors)
            if (e.first / 64 == area.room && e.second > best) {
                best = e.second;
                area.commonError = e.first % 64;
            }
        std::stable_sort(area.spots.begin(), area.spots.end(), [&](const Spot& a, const Spot& b) {
            if (a.score != b.score) return a.score > b.score;
            return Distance(a.tile, center) < Distance(b.tile, center);
        });
        area.suggested = moving ? area.room == fromRoom : rooms::Suits(area.kind, traits.rooms);
        if (heldDoor) area.suggested = !area.spots.empty() && area.spots.front().firstDoor;
        game::Room areaInfo;
        if (heldPortal && !heldDoor)  // windows: in a room's walls first
            area.suggested = game::GetRoom(area.room, &areaInfo) && !areaInfo.outside;
    }
    // Suggested rooms with spots first, then any with spots by nearest spot, then the rest.
    auto nearest = [&](const Area& a) {
        double d = 1e9;
        for (const Spot& s : a.spots) d = std::min(d, Distance(s.tile, center));
        return d;
    };
    std::stable_sort(areas.begin(), areas.end(), [&](const Area& a, const Area& b) {
        bool fa = !a.spots.empty(), fb = !b.spots.empty();
        if (fa != fb) return fa;
        if (a.suggested != b.suggested) return a.suggested;
        return nearest(a) < nearest(b);
    });
    int total = 0;
    for (const Area& a : areas) total += (int)a.spots.size();
    Log("placement: %d areas, %d spots on level %d in %llu ms", (int)areas.size(), total, level,
        GetTickCount64() - started);
    return areas;
}

namespace {

Str DirectionName(int direction) {  // compass: 0 north ... 7 north-west
    static const Str kNames[8] = {Str::DirNorth, Str::DirNorthEast, Str::DirEast, Str::DirSouthEast,
                                  Str::DirSouth, Str::DirSouthWest, Str::DirWest, Str::DirNorthWest};
    return kNames[direction & 7];
}

std::string NameOfId(int id) {
    game::LotObject o;
    Ptr object = LeadOf(game::ObjectById(id));
    if (!game::GetObject(object, &o)) return "";
    return world::DisplayName(o);
}

// Compass direction from a to b (north is -y), eight sectors.
int Bearing(Tile a, Tile b) {
    double angle = std::atan2(double(b.x - a.x), double(a.y - b.y)) * 180.0 / 3.14159265358979;
    return (int)std::floor((angle + 360.0 + 22.5) / 45.0) % 8;
}

}  // namespace

std::string Describe(const Spot& spot) {
    std::string s;
    for (const std::string& p : DescribeParts(spot)) s += (s.empty() ? "" : ", ") + p;
    return s;
}

// A room's name for door descriptions ("comedor", "exterior").
std::string RoomNameOf(int room) {
    std::string name = rooms::NameOf(rooms::NameRooms(game::LotObjects()), room);
    if (!name.empty()) return name;
    game::Room info;  // a room with nothing in it yet has no name: its size says which
    if (game::GetRoom(room, &info) && !info.outside) return util::Format(S(Str::RoomEmpty), info.tiles);
    return S(Str::RoomOutside);
}

std::vector<std::string> DescribeParts(const Spot& spot) {
    std::vector<std::string> parts;
    if (spot.portal) {  // "en la pared oeste de comedor, da a exterior; la primera puerta de comedor"
        std::string here = RoomNameOf(spot.roomHere), behind = RoomNameOf(spot.roomBehind);
        const char* wall = S(DirectionName(game::CompassOf((spot.direction + 4) & 7)));
        parts.push_back(util::Format(S(Str::DoorInWall), wall, here.c_str(), behind.c_str()));
        if (spot.firstDoor) parts.push_back(S(Str::DoorFirst));
        parts.push_back(util::Format(S(Str::TileAt), spot.tile.x, spot.tile.y));
        game::LotObject sim;
        if (game::GetObject(game::SelectedSim(), &sim)) {
            Tile at = TileOf(sim);
            int tiles = (int)std::lround(Distance(at, spot.tile));
            const char* dir = S(DirectionName(Bearing(at, spot.tile)));
            if (tiles < 1) parts.push_back(S(Str::HereNow));
            else if (tiles == 1) parts.push_back(util::Format(S(Str::OneTileAway), dir));
            else parts.push_back(util::Format(S(Str::TilesAway), tiles, dir));
        }
        return parts;
    }
    if (spot.surface) {  // "sobre Mesa de comedor Mission, a 3 casillas al este"
        std::string name = NameOfId(spot.surface);
        parts.push_back(util::Format(S(Str::OnTopOf), name.c_str()));
        game::LotObject sim;
        if (game::GetObject(game::SelectedSim(), &sim)) {
            Tile at = TileOf(sim);
            int tiles = (int)std::lround(Distance(at, spot.tile));
            const char* dir = S(DirectionName(Bearing(at, spot.tile)));
            if (tiles < 1) parts.push_back(S(Str::HereNow));
            else if (tiles == 1) parts.push_back(util::Format(S(Str::OneTileAway), dir));
            else parts.push_back(util::Format(S(Str::TilesAway), tiles, dir));
        }
        return parts;
    }
    // Wall sides are engine directions (direction + 2r); speech names compass directions.
    const int back = 4, left = 2, right = 8;
    auto compass = [&](int engineDirection) { return S(DirectionName(game::CompassOf(engineDirection & 7))); };
    int sideDirection = (spot.wallSides & left) ? spot.direction + 2 : spot.direction + 6;
    if ((spot.wallSides & back) && (spot.wallSides & (left | right))) {
        int a = game::CompassOf((spot.direction + 4) & 7), b = game::CompassOf(sideDirection & 7);
        int corner = ((a == 0 && b == 6) || (a == 6 && b == 0)) ? 7 : (a + b) / 2;  // between two walls
        parts.push_back(util::Format(S(Str::InCorner), S(DirectionName(corner))));
    } else if (spot.wallSides & back) {
        parts.push_back(util::Format(S(Str::AgainstWall), compass(spot.direction + 4)));
    } else if (spot.wallSides & (left | right)) {
        parts.push_back(util::Format(S(Str::BesideWall), compass(sideDirection)));
    } else if (spot.wallSides & 1) {
        parts.push_back(util::Format(S(Str::BesideWall), compass(spot.direction)));
    } else {
        parts.push_back(S(Str::AwayFromWalls));
    }
    parts.push_back(util::Format(S(Str::FacingDirection), compass(spot.direction)));
    if (spot.facing) {
        std::string name = NameOfId(spot.facing);
        if (!name.empty()) parts.push_back(util::Format(S(Str::FacingObject), name.c_str()));
    } else if (spot.nextTo) {
        std::string name = NameOfId(spot.nextTo);
        if (!name.empty()) parts.push_back(util::Format(S(Str::NextToObject), name.c_str()));
    }
    game::LotObject sim;
    if (game::GetObject(game::SelectedSim(), &sim)) {
        Tile at = TileOf(sim);
        int tiles = (int)std::lround(Distance(at, spot.tile));
        const char* dir = S(DirectionName(Bearing(at, spot.tile)));
        if (tiles < 1) parts.push_back(S(Str::HereNow));
        else if (tiles == 1) parts.push_back(util::Format(S(Str::OneTileAway), dir));
        else parts.push_back(util::Format(S(Str::TilesAway), tiles, dir));
    }
    if (spot.cutOff > 0) parts.push_back(util::Format(S(Str::CutsOff), spot.cutOff));
    return parts;
}

std::string WhyNone(const Area& area) {
    if (area.blocksOthers > 0) {
        std::string name = NameOfId(area.blockedObject);
        if (!name.empty()) return util::Format(S(Str::WouldBlockObject), name.c_str());
    }
    if (area.unusable > 0) return S(Str::NoReachableSpot);
    if (area.blocksDoor > 0) return S(Str::WouldBlockDoor);
    std::string reason = game::PlacementErrorText(area.commonError);
    return reason.empty() ? S(Str::DoesNotFit) : reason;
}

WallCheck CheckNewWalls(int level, const std::vector<WallEdgeAdd>& edges) {
    WallCheck check;
    Floor before = ReadFloor(level);
    Floor after = before;
    for (const WallEdgeAdd& e : edges) {
        if (!after.Inside(e.tile)) continue;
        int i = after.Index(e.tile);
        bool had = (before.walls[i] & e.edge) != 0;
        after.walls[i] |= e.edge;
        Tile n = e.edge == game::kWallNorth ? Tile{e.tile.x, e.tile.y - 1} : Tile{e.tile.x - 1, e.tile.y};
        if (!after.Inside(n)) continue;
        int j = after.Index(n);
        after.walls[j] |= e.edge == game::kWallNorth ? game::kWallSouth : game::kWallEast;
        // A new wall between a door and the tile in front of (or behind) it closes the door.
        if (!had && ((before.portal[i] && before.keepClear[j]) || (before.portal[j] && before.keepClear[i])))
            check.doorsBlocked++;
    }
    // Reach is measured from the lot's entrance: a Sim standing inside a new room must not make
    // the room look reachable.
    LogVerbose("placement: wall check from %d entrance tiles", (int)before.entrance.size());
    if (!before.entrance.empty()) before.starts = after.starts = before.entrance;
    int cells = before.size * before.size;
    std::vector<uint8_t> none(cells, 0);
    std::vector<uint8_t> reachBefore = Reach(before, none), reachAfter = Reach(after, none);
    for (const game::LotObject& o : game::LotObjects()) {
        Tile t = TileOf(o);
        if (!o.person || o.level != level || !before.Inside(t)) continue;
        int i = before.Index(t);
        // A Sim on a chair or in bed stands on a blocked tile: the tiles around count.
        auto reachableNear = [&](const Floor& f, const std::vector<uint8_t>& reach) {
            if (reach[i]) return true;
            for (int d = 0; d < 8; d += 2) {
                Tile n = game::Step(t, d);
                if (f.Inside(n) && reach[f.Index(n)] && CanCross(f, n, t)) return true;
            }
            return false;
        };
        if (reachableNear(before, reachBefore) && !reachableNear(after, reachAfter)) check.trappedSims.push_back(game::ObjectName(o));
    }
    for (int i = 0; i < cells; i++) check.cutOffTiles += reachBefore[i] && !reachAfter[i];
    for (const game::LotObject& o : game::LotObjects()) {
        if (o.person || o.level != level || game::IsPortal(o.object)) continue;
        Tile at = TileOf(o);
        std::vector<Tile> fp = game::FootprintAt(o.object, at);
        std::vector<Tile> use = game::UseTilesAt(o.object, at, game::ObjectDirection(o.object));
        if (UsableTiles(before, use, fp, none, reachBefore).empty()) {
            LogVerbose("placement: wall check skips %s (not usable now, %d use tiles)", world::DisplayName(o).c_str(),
                       (int)use.size());
            continue;
        }
        if (!UsableTiles(after, use, fp, none, reachAfter).empty()) continue;
        std::string name = world::DisplayName(o);
        if (std::find(check.blockedObjects.begin(), check.blockedObjects.end(), name) == check.blockedObjects.end())
            check.blockedObjects.push_back(name);
    }
    return check;
}

}  // namespace nav::placement
