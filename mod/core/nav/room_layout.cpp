#include "room_layout.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "build_mode.h"
#include "navigator.h"
#include "room_names.h"
#include "world.h"

namespace nav::layout {
namespace {

using game::Tile;

// Compass sides of a room, in the order they are said: north, east, south, west.
enum Side { kNorth, kEast, kSouth, kWest, kSides };
const Str kSideWord[kSides] = {Str::BuildNorth, Str::BuildEast, Str::BuildSouth, Str::BuildWest};

Side SideOfDirection(int direction) {
    switch (direction & 7) {
    case 6: return kNorth;
    case 4: return kEast;
    case 2: return kSouth;
    default: return kWest;
    }
}

uint32_t WallBit(Side s) {
    return s == kNorth ? game::kWallNorth : s == kEast ? game::kWallEast : s == kSouth ? game::kWallSouth : game::kWallWest;
}

std::string Join(const std::vector<std::string>& items) {
    std::string out;
    for (const std::string& s : items) out += (out.empty() ? "" : ", ") + s;
    return out;
}

std::string RoomLabel(const std::vector<rooms::NamedRoom>& named, int room) {
    std::string name = rooms::NameOf(named, room);
    if (!name.empty()) return name;
    game::Room info;
    if (game::GetRoom(room, &info) && !info.outside) return util::Format(S(Str::RoomEmpty), game::RoomTileCount(room));
    return S(Str::RoomOutside);
}

}  // namespace

void SayRoomLayout() {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    int x = 0, y = 0, level = 1;
    if (!build::CursorTile(&x, &y, &level)) {
        game::LotObject sim;
        if (!game::GetObject(game::SelectedSim(), &sim)) {
            speech::Say(S(Str::NoSimSelected));
            return;
        }
        x = sim.x >> 4;
        y = sim.y >> 4;
        level = sim.level;
    }
    int room = game::RoomAt(level, x, y);
    std::vector<game::LotObject> objects = game::LotObjects();
    std::vector<rooms::NamedRoom> named = rooms::NameRooms(objects);
    game::Room info;
    if (room == game::kNoRoom || !game::GetRoom(room, &info)) {
        speech::Say(S(Str::LayoutUnknown));
        return;
    }

    // Size and bounds.
    int n = game::GridSize(), x1 = n, y1 = n, x2 = -1, y2 = -1, tiles = 0;
    for (int ty = 0; ty < n; ty++)
        for (int tx = 0; tx < n; tx++)
            if (game::RoomAt(level, tx, ty) == room) {
                tiles++;
                x1 = std::min(x1, tx), y1 = std::min(y1, ty), x2 = std::max(x2, tx), y2 = std::max(y2, ty);
            }
    std::vector<std::string> lines;
    std::string label = RoomLabel(named, room);
    if (info.outside)
        lines.push_back(util::Format(S(Str::LayoutOutside), label.c_str(), tiles));
    else
        lines.push_back(util::Format(S(Str::LayoutSize), label.c_str(), x2 - x1 + 1, y2 - y1 + 1, tiles, x1, y1, x2, y2));

    // Doors and windows: portals standing in a wall of this room (their tile or the one behind).
    std::vector<std::string> doors, windows;
    for (const game::PlacedPart& part : game::PlacedParts()) {
        if (part.level != level || !game::IsPortal(part.object) || game::IsInvisibleHelper(part.object)) continue;
        int d = game::ObjectDirection(part.object);
        Tile t{part.tileX, part.tileY};
        Tile behind = game::Step(t, (d + 4) & 7);
        int here = game::RoomAt(level, t.x, t.y), there = game::RoomAt(level, behind.x, behind.y);
        if (!(game::WallsAt(level, t.x, t.y) & WallBit(SideOfDirection(d + 4)))) continue;  // no wall: stairs
        Side side;
        int other;
        if (here == room && there != room) {
            side = SideOfDirection(d + 4);
            other = there;
        } else if (there == room && here != room) {
            side = SideOfDirection(d);
            other = here;
        } else {
            continue;
        }
        std::string item = util::Format(S(Str::LayoutOpening), S(kSideWord[side]), RoomLabel(named, other).c_str());
        (game::PeoplePass(part.object) ? doors : windows).push_back(item);
    }
    lines.push_back(doors.empty() ? std::string(S(Str::LayoutNoDoors)) : util::Format(S(Str::LayoutDoors), Join(doors).c_str()));
    if (!info.outside)
        lines.push_back(windows.empty() ? std::string(S(Str::LayoutNoWindows))
                                        : util::Format(S(Str::LayoutWindows), Join(windows).c_str()));

    // Furniture: along the wall at its back (or any wall it touches), else in the middle.
    std::map<int, std::vector<std::string>> along;  // by side, kSides = the middle
    for (const game::LotObject& o : objects) {
        if (o.person || o.level != level || o.room != room || game::IsPortal(o.object)) continue;
        Tile at{o.x >> 4, o.y >> 4};
        std::vector<Tile> fp = game::FootprintAt(o.object, at);
        int back = SideOfDirection(game::ObjectDirection(o.object) + 4);
        int found = kSides;
        for (int pass = 0; pass < 2 && found == kSides; pass++)
            for (int s = 0; s < kSides && found == kSides; s++) {
                if (pass == 0 && s != back) continue;
                for (Tile p : fp)
                    if (game::WallsAt(level, p.x, p.y) & WallBit((Side)s)) {
                        found = s;
                        break;
                    }
            }
        std::string name = world::DisplayName(o);
        std::vector<std::string>& list = along[found];
        if (std::find(list.begin(), list.end(), name) == list.end()) list.push_back(name);
    }
    int spoken = (int)lines.size();
    for (int s = 0; s < kSides; s++)
        if (!along[s].empty())
            lines.push_back(util::Format(S(Str::LayoutAlongWall), S(kSideWord[s]), Join(along[s]).c_str()));
    // The middle can be long (the outdoors): five to a buffer line.
    const std::vector<std::string>& middle = along[kSides];
    for (size_t i = 0; i < middle.size(); i += 5) {
        std::vector<std::string> chunk(middle.begin() + i, middle.begin() + std::min(middle.size(), i + 5));
        lines.push_back(util::Format(S(Str::LayoutMiddle), Join(chunk).c_str()));
    }
    if (along.empty() || lines.size() == (size_t)spoken) lines.push_back(S(Str::LayoutNothing));

    buffers::SetDetails(lines);
    std::string say;
    for (int i = 0; i < spoken; i++) say += (say.empty() ? "" : ". ") + lines[i];
    speech::Say(say + ". " + S(Str::LayoutMore));
}

}  // namespace nav::layout
