// Build mode by keyboard: a cursor on the lot's grid corners for the wall and fence tool. It
// speaks what a sighted builder sees: the corner, the walls that meet there, the free space
// around (tiles to the nearest wall or the lot's edge each way), the room and what stands there;
// from a marked start, the wall's length, direction and cost. Before building it checks what the
// new walls would do (objects nobody could reach any more, doors closed, floor cut off) and asks
// for a second Enter when something would be blocked.
//   Enter on a wall or fence style     start the cursor (at the selected Sim)
//   Arrows                             move one corner (Shift: five)
//   Space                              mark the start here (again: unmark)
//   Enter                              build a wall from the start to here
//   Shift+Enter                        build a room: the four walls of the start-to-here rectangle
//   Ctrl+Enter or Delete               remove the walls from the start to here
//   PageDown / PageUp                  choose a ready-made room size (said with its cost)
//   Insert                             build that room with its north-west corner here
//   Home                               back to the selected Sim
//   R                                  where the cursor is, in full (details buffer: every part)
//   Escape                             leave the cursor (back to the build panel)
// With the floor tool the cursor is on tiles: Enter floors the rectangle from the start (or the
// tile alone), Shift+Enter the whole room under the cursor, Ctrl removes (Ctrl+Shift: the room).
// With the wallpaper tool Enter papers every wall of the room under the cursor, Ctrl+Enter strips
// them (a single wall's side cannot be told apart by ear, so whole rooms only).
#pragma once

#include <cstdint>

namespace nav::build {

void Reset();
void Tick();            // ends the cursor when the tool changes or the lot view goes away
bool Active();          // the cursor owns its keys
bool CursorTile(int* x, int* y, int* level);  // where the cursor is (the tile south-east of a corner)
bool FloorCursor();     // it works on tiles for the floor tool
bool PaperCursor();     // the wallpaper tool: Enter papers the room under the cursor
bool PoolCursor();      // the water tool: Enter digs a pool on the rectangle, Ctrl+Enter fills it in
bool TerrainCursor();   // terrain: Enter levels from the start (level tool) or raises/lowers once
bool Begin(uintptr_t productButton);  // Enter on a style: start when the wall tool is active

void Move(int dx, int dy);
void MarkStart();
void Build(bool room, bool remove);
void NextPreset(int delta);
void BuildPreset();
void Home();
void Repeat();
void End();             // Escape

}  // namespace nav::build
