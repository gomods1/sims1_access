#include "build.h"

#include "../util/log.h"
#include "addresses.h"
#include "lotgrid.h"
#include "hooks.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

int g_lastError = 0;  // what the build tools last showed (OnBuildError), 0 none
int g_lastCost = 0;   // the price they showed

// The pick message the lot view passes the tool's mouse handlers (see game/buy.cpp).
struct PickInfo {
    uint32_t type;
    Ptr view;
    uint32_t unknown;
    int16_t screenY;
    int16_t screenX;
    uint32_t rest[8];
};

Ptr WallTool() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cNewWallTool") ? tool : 0;
}

// screenDx: the pick message's screen position moves by this much, so a drag reads as a drag.
bool CallMouse(Ptr tool, int slot, Ptr expected, const char* what, int vx, int vy, int screenDx = 0) {
    typedef void(__thiscall * Fn)(void*, int, int, int, PickInfo*);
    Ptr fn = Rd<Ptr>(Rd<Ptr>(tool) + slot * 4);
    if (fn != Va(expected) || !FunctionOk(expected)) {
        Log("build: unexpected %s (slot %d = %p)", what, slot, (void*)fn);
        return false;
    }
    PickInfo pick = {12, InputRoot(), 0, 300, (int16_t)(512 + screenDx), {}};
    return Guarded([&] { ((Fn)fn)((void*)tool, vx, vy, 0, &pick); });
}

// The tool turns a view corner into the grid the way 0x6599f0 does: (corner << 4) | 1 through
// the camera rotation (0x586ae0 with span 0x400), then >> 4. The view corner for a grid corner is
// found by trying them all (65 x 65, cheap), so no inverse has to be right for every rotation.
Corner ToolGridCorner(int vx, int vy) {
    int x = (vx << 4) | 1, y = (vy << 4) | 1;
    int r = -Rd<int>(Rd<Ptr>(Va(addr::kWorld)) + addr::kWorldRotation) & 3;
    int n = addr::kWorldSpan - 1;
    int gx = x, gy = y;
    switch (r) {
    case 1: gy = n - x; gx = y; break;
    case 2: gy = n - y; gx = n - x; break;
    case 3: gx = n - y; gy = x; break;
    default: break;
    }
    return {gx >> 4, gy >> 4};
}

bool ViewCornerFor(Corner grid, int* vx, int* vy) {
    int corners = addr::kWorldSpan / 16;
    for (int y = 0; y <= corners; y++)
        for (int x = 0; x <= corners; x++)
            if (ToolGridCorner(x, y) == grid) {
                *vx = x * 16;  // no fraction: the tool's rounding keeps this corner
                *vy = y * 16;
                return true;
            }
    return false;
}

Ptr FloorTool() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cNewFloorTool") ? tool : 0;
}

// Tile centers go to the view the way the picker's rotation (0x586ae0) expects them back.
void TileToView(Corner t, int* vx, int* vy) {
    int gx = t.x * 16 + 8, gy = t.y * 16 + 8;
    int r = -Rd<int>(Rd<Ptr>(Va(addr::kWorld)) + addr::kWorldRotation) & 3;
    int n = addr::kWorldSpan - 1;
    switch (r) {
    case 1: *vx = n - gy; *vy = gx; break;
    case 2: *vx = n - gx; *vy = n - gy; break;
    case 3: *vx = gy; *vy = n - gx; break;
    default: *vx = gx; *vy = gy; break;
    }
}

}  // namespace

bool FloorToolActive() { return FloorTool() != 0; }

namespace {
Ptr PaperTool() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cNewWallpaperTool") ? tool : 0;
}
int PaperSessionCount(Ptr tool) {
    Ptr session = Rd<Ptr>(tool + addr::papertool::kSession);
    if (!session) return 0;
    return (int)((Rd<Ptr>(session + addr::papertool::kSessionLast) - Rd<Ptr>(session + addr::papertool::kSessionFirst)) /
                 addr::papertool::kEntrySize);
}
}  // namespace

bool PaperToolActive() { return PaperTool() != 0; }

namespace {
Ptr PoolTool() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cPoolTool") ? tool : 0;
}
}  // namespace

bool PoolToolActive() { return PoolTool() != 0; }

int TerrainToolKind() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    if (IsKindOf(tool, "cLevelDirtTool")) return 1;
    if (IsKindOf(tool, "cSimpleAltTool")) return 2;
    return 0;
}

int LevelTerrain(Corner from, Corner to, int* cost) {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    if (cost) *cost = 0;
    if (!IsKindOf(tool, "cLevelDirtTool")) return -1;
    int ax, ay, bx, by;
    TileToView(from, &ax, &ay);
    TileToView(to, &bx, &by);
    g_lastError = 0;
    bool ok = CallMouse(tool, addr::terraintool::kDownSlot, addr::terraintool::kLevelDown, "cLevelDirtTool::MouseDown", ax, ay) &&
              CallMouse(tool, addr::terraintool::kDragSlot, addr::terraintool::kLevelDrag, "cLevelDirtTool::MouseDrag", bx, by, 60);
    int price = Rd<int>(tool + addr::terraintool::kLevelCost);
    int refused = Rd<int>(tool + addr::terraintool::kLevelError);
    ok = ok && CallMouse(tool, addr::terraintool::kUpSlot, addr::terraintool::kLevelUp, "cLevelDirtTool::MouseUp", bx, by, 60);
    Log("build: level %d,%d -> %d,%d: %s, cost %d, refused %d, error %d", from.x, from.y, to.x, to.y,
        ok ? "done" : "failed", price, refused, g_lastError);
    if (cost) *cost = price;
    if (!ok) return -1;
    if (refused) return refused;
    return g_lastError;
}

int PressTerrain(Corner at) {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    if (!IsKindOf(tool, "cSimpleAltTool")) return -1;
    int vx, vy;
    TileToView(at, &vx, &vy);
    g_lastError = 0;
    g_lastCost = 0;
    bool ok = CallMouse(tool, addr::terraintool::kDownSlot, addr::terraintool::kAltDown, "cSimpleAltTool::MouseDown", vx, vy) &&
              CallMouse(tool, addr::terraintool::kUpSlot, addr::terraintool::kAltUp, "cSimpleAltTool::MouseUp", vx, vy);
    Log("build: terrain press %d,%d: %s, error %d", at.x, at.y, ok ? "done" : "failed", g_lastError);
    if (!ok) return -1;
    return g_lastError;
}

int DrawPool(Corner fromTile, Corner toTile, bool remove, int* tilesOut) {
    Ptr tool = PoolTool();
    if (tilesOut) *tilesOut = 0;
    if (!tool) return -1;
    int ax, ay, bx, by;
    TileToView(fromTile, &ax, &ay);
    TileToView(toTile, &bx, &by);
    g_lastError = 0;
    g_lastCost = 0;
    WriteU8(tool + addr::pooltool::kOverLot, 1);
    WriteU32(tool + addr::pooltool::kRemoveMode, remove ? 1 : 0);
    bool ok = CallMouse(tool, addr::pooltool::kDownSlot, addr::pooltool::kDown, "cPoolTool::MouseDown", ax, ay) &&
              CallMouse(tool, addr::pooltool::kDragSlot, addr::pooltool::kDrag, "cPoolTool::MouseDrag", bx, by, 60);
    int tiles = Rd<int>(tool + addr::pooltool::kTiles);
    ok = ok && CallMouse(tool, addr::pooltool::kUpSlot, addr::pooltool::kUp, "cPoolTool::MouseUp", bx, by, 60);
    WriteU32(tool + addr::pooltool::kRemoveMode, 0);
    Log("build: pool %d,%d -> %d,%d%s: %s, %d tiles, error %d, cost %d", fromTile.x, fromTile.y, toTile.x, toTile.y,
        remove ? " (remove)" : "", ok ? "done" : "failed", tiles, g_lastError, g_lastCost);
    if (tilesOut) *tilesOut = tiles;
    if (!ok) return -1;
    if (g_lastError) return g_lastError;
    return tiles > 0 ? 0 : kNothingBuilt;
}

int PaperRoom(const std::vector<Corner>& roomTiles, bool strip, int* wallSides) {
    Ptr tool = PaperTool();
    if (wallSides) *wallSides = 0;
    if (!tool) return -1;
    g_lastError = 0;
    g_lastCost = 0;
    ForceModifiers(true, strip);
    int sides = 0, tried = 0;
    Corner hit{};
    for (Corner t : roomTiles) {
        int vx, vy;
        TileToView(t, &vx, &vy);
        tried++;
        if (!CallMouse(tool, addr::papertool::kDownSlot, addr::papertool::kDown, "cNewWallpaperTool::MouseDown", vx, vy))
            break;
        sides = PaperSessionCount(tool);
        if (sides > 0) {
            hit = t;
            CallMouse(tool, addr::papertool::kUpSlot, addr::papertool::kUp, "cNewWallpaperTool::MouseUp", vx, vy);
            break;
        }
    }
    ClearForcedModifiers();
    Log("build: paper room (%s): %d wall sides after %d tiles (at %d,%d), cost %d", strip ? "strip" : "paper", sides,
        tried, hit.x, hit.y, g_lastCost);
    if (wallSides) *wallSides = sides;
    if (sides > 0) {
        g_lastError = 0;  // the tiles without a wall on their edge reported "no wall here"
        return 0;
    }
    return kNothingBuilt;
}

int DrawFloor(Corner fromTile, Corner toTile, FloorMode mode, int* tilesChanged) {
    Ptr tool = FloorTool();
    if (!tool) return -1;
    int ax, ay, bx, by;
    TileToView(fromTile, &ax, &ay);
    TileToView(toTile, &bx, &by);
    g_lastError = 0;
    g_lastCost = 0;
    bool shift = mode == FloorMode::FillRoom || mode == FloorMode::ClearRoom;
    bool ctrl = mode == FloorMode::Remove || mode == FloorMode::ClearRoom;
    ForceModifiers(shift, ctrl);
    bool ok = CallMouse(tool, addr::floortool::kDownSlot, addr::floortool::kDown, "cNewFloorTool::MouseDown", ax, ay) &&
              CallMouse(tool, addr::floortool::kDragSlot, addr::floortool::kDrag, "cNewFloorTool::MouseDrag", bx, by, 60);
    Ptr session = Rd<Ptr>(tool + addr::floortool::kSession);
    int tiles = session ? (Rd<Ptr>(session + addr::floortool::kSessionLast) - Rd<Ptr>(session + addr::floortool::kSessionFirst)) / 0x3c : 0;
    if (tilesChanged) *tilesChanged = tiles;
    ok = ok && CallMouse(tool, addr::floortool::kUpSlot, addr::floortool::kUp, "cNewFloorTool::MouseUp", bx, by, 60);
    ClearForcedModifiers();
    Log("build: floor %d,%d -> %d,%d mode %d: %s, %d tiles, error %d, cost %d", fromTile.x, fromTile.y, toTile.x,
        toTile.y, (int)mode, ok ? "done" : "failed", tiles, g_lastError, g_lastCost);
    if (!ok) return -1;
    if (g_lastError) return g_lastError;
    return tiles > 0 ? 0 : kNothingBuilt;
}

bool WallToolActive() { return WallTool() != 0; }

bool CornerBuildable(Corner c) { return (TileFlags(1, c.x, c.y) & kTileNotBuildable) == 0; }

void OnBuildError(int code, int arg) {
    if (code == kBuildShowCost) g_lastCost = arg;
    else if (code > 0) g_lastError = code;
}

int LastDrawCost() { return g_lastCost; }

int DrawWalls(Corner from, Corner to, WallMode mode) {
    Ptr tool = WallTool();
    int ax, ay, bx, by;
    if (!tool || !ViewCornerFor(from, &ax, &ay) || !ViewCornerFor(to, &bx, &by)) return -1;
    g_lastError = 0;
    g_lastCost = 0;
    ForceModifiers(mode == WallMode::Room, mode == WallMode::Delete);
    WriteU8(tool + addr::walltool::kOverLot, 1);
    auto state = [&](const char* when) {
        Ptr session = Rd<Ptr>(tool + 0x5c);
        LogVerbose("build: %s: session %p valid %d error %d mode %d start %02x,%02x end %02x,%02x over %d", when,
                   (void*)session, session ? Rd<uint8_t>(session + 0xe024) : -1, session ? Rd<int>(session + 0xe064) : -1,
                   Rd<int>(tool + 0x6c), Rd<uint8_t>(tool + 0x31), Rd<uint8_t>(tool + 0x30), Rd<uint8_t>(tool + 0x34),
                   Rd<uint8_t>(tool + 0x33), Rd<uint8_t>(tool + 0x71));
    };
    LogVerbose("build: view corners %d,%d -> %d,%d", ax, ay, bx, by);
    state("before");
    bool ok = CallMouse(tool, addr::walltool::kDownSlot, addr::walltool::kDown, "cNewWallTool::MouseDown", ax, ay);
    state("after down");
    ok = ok && CallMouse(tool, addr::walltool::kDragSlot, addr::walltool::kDrag, "cNewWallTool::MouseDrag", bx, by, 60);
    state("after drag");
    // The session holds segments once the drag added one; none and no message: the tool refused
    // silently (a corner outside the lot's buildable area, for one).
    Ptr session = Rd<Ptr>(tool + addr::walltool::kSession);
    bool anything = session && Rd<uint8_t>(session + addr::walltool::kSessionValid) != 0;
    ok = ok && CallMouse(tool, addr::walltool::kUpSlot, addr::walltool::kUp, "cNewWallTool::MouseUp", bx, by, 60);
    state("after up");
    ClearForcedModifiers();
    Log("build: walls %d,%d -> %d,%d mode %d: %s, error %d, cost %d", from.x, from.y, to.x, to.y, (int)mode,
        ok ? "done" : "failed", g_lastError, g_lastCost);
    if (!ok) return -1;
    if (g_lastError) return g_lastError;
    return anything ? 0 : kNothingBuilt;
}

}  // namespace game
