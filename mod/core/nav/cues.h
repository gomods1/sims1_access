// Sound cues for the cursors (the build cursor, placing's tile cursor, the live-mode map): besides
// speech, a short sound whenever the cursor's tile changes, so the space is heard as it is crossed
// (a tester asked, 2026-11-01). SpeechHost plays them (SimsAccess\sounds\*.wav, made with genny from
// mod\sounds\batch.json), mixed beside the screen reader. On a step:
//   at once       the floor (grass, wood, tile, carpet, paving, water, no floor upstairs) and,
//                 when the room changes, a chime
//   then          each side with a wall, fence, door or window: west panned left, east right,
//                 north higher, south lower
//   last          stairs, what stands on the tile (an object, a Sim), a slope
// Ctrl+Shift+S turns them off and on (SimsAccess.ini [mod] cues=1, cue_volume=80 in percent).
#pragma once

#include <string>

#include "../game/lotgrid.h"
#include "../util/strings.h"

namespace nav::cues {

// What stands on a side of a tile.
enum class Edge { None, Wall, Fence, Railing, Door, Window };  // a railing: a fence upstairs
enum Side { kNorth, kEast, kSouth, kWest, kSides };

struct TileSense {
    game::Surface surface = game::Surface::Ground;
    Edge sides[kSides] = {};
    bool stairs = false;
    bool object = false;    // furniture or any other visible object (doors and windows are sides)
    bool person = false;    // a Sim or a pet
    bool sloped = false;
    int room = -1;
};

// One read of the tile (walls, portals on its edges, what stands there, the floor).
// `ignore`: an object left out (the one being placed stands on the tile it is tried on).
TileSense Sense(int level, int x, int y, game::Ptr ignore = 0);

bool Enabled();
void Toggle();      // Ctrl+Shift+S
void Reset();       // forgets the last room (a new cursor starts)

// The tile cursors: the floor, the four sides, what stands there, the room when it changed.
void Tile(int level, int x, int y, game::Ptr ignore = 0);
// The wall cursor's corner: the walls leaving it (the four grid lines), the floor south-east of it.
void Corner(int level, int x, int y);
void LotEdge();     // the cursor could not move further
void Blocked();     // the corner cannot be built on
void Fit(bool fits);  // placing: the held object fits here or not

// Words for speech.
std::string SurfaceWord(game::Surface s);
Str SideWord(int side);          // north, east, south, west (the build cursor's words)
std::string EdgesLine(const TileSense& t);  // "pared al norte, puerta al este"; empty when none

}  // namespace nav::cues
