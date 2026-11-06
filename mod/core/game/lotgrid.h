// The lot as a grid of tiles, read from the engine's own maps: what stands on each tile, the walls
// on its edges, and the tiles an object covers and where Sims stand to use it (its routing slots,
// from the object's SLOT resource). Nothing here is known per object: it all comes from memory.
// Tile coordinates are grid coordinates (camera independent); level 1 is the ground floor.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "memory.h"

namespace game {

int GridSize();                  // tiles per side
bool InGrid(int x, int y);       // tiles the engine lets objects use (not the outer border)

int ObjectIdAt(int level, int x, int y);   // the object standing on the tile, 0 = none
uint8_t TileFlags(int level, int x, int y);
constexpr uint8_t kTileOutside = 0x04;
constexpr uint8_t kTileNotBuildable = 0x20;  // outside the lot's buildable area (walls refused)
bool HasFloor(int level, int x, int y);    // a floor pattern (upper levels need one to stand on)
int FloorPattern(int level, int x, int y);  // 0 none, 255 the half tile of a diagonal wall
// What the floor is made of, as the game's footsteps hear it.
enum class Surface { Ground, Wood, Tile, Carpet, Paving, NoFloor, Water, PoolRim };
Surface SurfaceAt(int level, int x, int y);
std::string FloorName(int level, int x, int y);  // the floor's catalog name, empty on bare ground
// The ground height at a tile's four corners (the terrain map, world + 0x14: 4 bytes per tile,
// grid order; what 0x585940 reads). false outside the grid.
bool TileHeights(int x, int y, uint8_t out[4]);

// Wall edges of a tile: north is toward y - 1, west toward x - 1.
enum WallEdge : uint32_t {
    kWallNorth = 0x1,
    kWallWest = 0x2,
    kWallSouth = 0x4,
    kWallEast = 0x8,
    kWallDiagonals = 0x30,
};
uint32_t WallsAt(int level, int x, int y);
// The style of each wall of the tile, north, west, south, east (0 where there is none); 1 is the
// wall tool's wall, anything else a fence, a balustrade or a wall of another kind.
void WallStylesAt(int level, int x, int y, uint16_t out[4]);

struct Tile {
    int x = 0, y = 0;
    bool operator==(const Tile& o) const { return x == o.x && y == o.y; }
    bool operator<(const Tile& o) const { return y != o.y ? y < o.y : x < o.x; }
};

// Directions are the engine's: 0 toward -x, 2 toward +y, 4 toward +x, 6 toward -y, odd values
// diagonal between them (the tables its turn code uses at 0xa50f64 / 0xa50f88; a Sim using a
// fridge that faces 6 stands on the fridge's -y side and faces 2). The mod's compass calls -y
// north and +x east, so direction 6 is north, 0 west, 2 south, 4 east.
Tile Step(Tile t, int direction);
int CompassOf(int direction);    // the mod's compass: 0 north, 1 north-east ... 7 north-west

// An object's parts: the lead first. Multi-tile objects (cXMTObject) are one object per tile.
std::vector<Ptr> ObjectParts(Ptr object);
int ObjectDirection(Ptr object);
int ObjectLevel(Ptr object);
bool IsPortal(Ptr object);       // cXPortal: doors, windows, stairs
int ContainerOf(Ptr object);     // the object it sits on (a lamp's table), 0 when on the floor
// Sims walk through it: its flags (FreeSO's VMEntityFlags) say it has no floor extent (4: windows,
// paintings, the lot's helpers), walks itself (8: Sims, pets) or lets people through (16: doors,
// trees, flowers, stair landings).
bool CanWalkOver(Ptr object);
bool PeoplePass(Ptr object);     // flag 16 alone: doors, stair landings (not windows)

// The sides its routing slots let a Sim use it from, over every part (bits 0-7: front, then
// clockwise as seen from above with the object facing north: 4 is its back); 0 when none.
uint32_t RoutingSides(Ptr object);

// Where the parts of the object would stand with its lead part on `lead` (their offsets as the
// object is turned right now).
std::vector<Tile> FootprintAt(Ptr object, Tile lead);

// Tiles where a Sim can stand to use the object if its lead part stood on `lead` facing
// `direction`: every routing slot of every part, each allowed side at the slot's proximity.
// Objects with no routing slots are used from the tile in front of them.
std::vector<Tile> UseTilesAt(Ptr object, Tile lead, int direction);

}  // namespace game
