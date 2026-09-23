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

}  // namespace

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
