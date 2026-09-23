// Build mode's drawing tools, worked without the mouse: the wall tool (cNewWallTool) is given
// the same mouse down / move / up a drag makes, at corners the mod chose. The game does the rest
// (its checks, its cost, its undo). Corners are grid corners (camera independent): corner (x, y)
// is the north-west corner of tile (x, y).
#pragma once

#include <string>

#include "memory.h"

namespace game {

struct Corner {
    int x = 0, y = 0;
    bool operator==(const Corner& o) const { return x == o.x && y == o.y; }
};

bool WallToolActive();  // the current tool is the wall and fence tool

enum class WallMode {
    Line,    // a wall from one corner to the other (straight or diagonal), as a plain drag
    Room,    // four walls around the rectangle between the corners (the game's Shift+drag)
    Delete,  // remove the walls along the line (the game's Ctrl+drag)
};

// Draws on the current floor. Returns 0 when the game took it, else the game's error code (a
// 1-based index into STR# 137, game::PlacementErrorText), or -1 when the tool is not there.
// kNothingBuilt: the tool took the drag but made nothing of it, without a message.
constexpr int kNothingBuilt = -2;
int DrawWalls(Corner from, Corner to, WallMode mode);

// The lot's buildable area: the engine's tile flag 0x20 marks tiles outside it (0x58a660).
bool CornerBuildable(Corner c);

// Called by the hook on the build tools' message display (0x56f7f0): an error, or with code 26
// the price of the drag so far (arg = the amount; the game's "show cost" tooltip).
void OnBuildError(int code, int arg);
constexpr int kBuildShowCost = 26;
int LastDrawCost();  // the price the last DrawWalls showed, 0 when none

}  // namespace game
