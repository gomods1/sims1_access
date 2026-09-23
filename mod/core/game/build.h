// Build mode's drawing tools, worked without the mouse: the wall tool (cNewWallTool) is given
// the same mouse down / move / up a drag makes, at corners the mod chose. The game does the rest
// (its checks, its cost, its undo). Corners are grid corners (camera independent): corner (x, y)
// is the north-west corner of tile (x, y).
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

struct Corner {
    int x = 0, y = 0;
    bool operator==(const Corner& o) const { return x == o.x && y == o.y; }
};

bool WallToolActive();  // the current tool is the wall and fence tool
bool FloorToolActive(); // the current tool is the floor tool

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

enum class FloorMode {
    Area,        // the rectangle of tiles between the two
    Remove,      // take the floor off that rectangle (Ctrl)
    FillRoom,    // the whole room of the first tile (Shift)
    ClearRoom,   // take the floor off the whole room (Shift+Ctrl)
};
// Floors tiles (x, y = tile coordinates) on the current floor; results as DrawWalls.
// tilesChanged: how many tiles the game's session held (0 when it made nothing).
int DrawFloor(Corner fromTile, Corner toTile, FloorMode mode, int* tilesChanged = nullptr);

bool PaperToolActive(); // the current tool is the wallpaper tool
// Papers (or strips) every wall of a room from inside: the tool is pressed with Shift on the
// room's tiles in turn until one has a wall it takes. wallSides = the wall sides changed.
int PaperRoom(const std::vector<Corner>& roomTiles, bool strip, int* wallSides);

bool PoolToolActive();  // the water tool (pools and ponds)
// Digs (or fills in) the rectangle of tiles; tiles = the drag's tile count.
int DrawPool(Corner fromTile, Corner toTile, bool remove, int* tiles);

// Terrain: 1 the level tool, 2 raise or lower (by the chosen product), 0 none of them.
int TerrainToolKind();
// Levels the tiles from `from` to `to` to the height at `from`; cost = the price the tool showed.
int LevelTerrain(Corner from, Corner to, int* cost);
// Raise or lower once at a tile, as a click does.
int PressTerrain(Corner at);

// The lot's buildable area: the engine's tile flag 0x20 marks tiles outside it (0x58a660).
bool CornerBuildable(Corner c);

// Called by the hook on the build tools' message display (0x56f7f0): an error, or with code 26
// the price of the drag so far (arg = the amount; the game's "show cost" tooltip).
void OnBuildError(int code, int arg);
constexpr int kBuildShowCost = 26;
int LastDrawCost();  // the price the last DrawWalls showed, 0 when none

}  // namespace game
