#include "rooms.h"

#include "addresses.h"
#include "rtti.h"

namespace game {

// What the debug tile window does (0x4e8f30 via 0x4e8c70), without its camera rotation step:
// object positions are already in grid coordinates.
int RoomAt(int level, int tileX, int tileY) {
    if (level < 0 || level > addr::rooms::kMaxLevel || tileX < 0 || tileX > 255 || tileY < 0 || tileY > 255)
        return kNoRoom;
    Ptr world = Rd<Ptr>(Va(addr::kWorld));
    Ptr map = world ? Rd<Ptr>(world + addr::rooms::kLevelMaps + level * 4) : 0;
    Ptr rows = map ? Rd<Ptr>(map + addr::rooms::kMapRows) : 0;
    Ptr row = rows ? Rd<Ptr>(rows + tileY * 4) : 0;
    if (!row) return kNoRoom;
    uint16_t id = addr::rooms::kNone;
    if (!Read(row + tileX * 2, &id, sizeof id) || id == addr::rooms::kNone) return kNoRoom;
    return id;
}

bool GetRoom(int id, Room* out) {
    typedef Ptr(__thiscall * FindFn)(void* manager, uint16_t id);
    typedef bool(__thiscall * OutsideFn)(void* room);
    if (id < 0 || !FunctionOk(addr::rooms::kFindRoom) || !FunctionOk(addr::rooms::kIsOutside)) return false;
    Ptr root = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)));
    Ptr manager = root ? Rd<Ptr>(root + addr::rooms::kManager) : 0;
    if (!manager) return false;
    Ptr room = 0;
    bool outside = false;
    if (!Guarded([&] {
            room = ((FindFn)Va(addr::rooms::kFindRoom))((void*)manager, (uint16_t)id);
            if (room) outside = ((OutsideFn)Va(addr::rooms::kIsOutside))((void*)room);
        }) ||
        !room)
        return false;
    out->id = id;
    out->tiles = Rd<int>(room + addr::rooms::kArea) / 2;  // GetArea (0x6278a0) halves it
    out->outside = outside;
    out->beds = Rd<int>(room + addr::rooms::kBeds);
    out->bathObjects = Rd<int>(room + addr::rooms::kBathObjects);
    return true;
}

}  // namespace game
