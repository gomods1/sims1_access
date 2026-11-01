// The live-mode map (a tester asked, 2026-11-01): a read-only cursor over the house during play,
// with the build cursor's descriptions and the same sound cues (nav/cues.h), without leaving live
// mode. "I didn't notice the bed was turned that way" is something a player should find out
// while playing. A spatial tool (NAVIGATION.md): while it is on, the arrows move it.
//   Ctrl+M                         the map on / off (it starts at the selected Sim)
//   Arrows                         one tile (Shift: five): what stands there and which way it
//                                  faces, the room when it changes, the floor when it changes,
//                                  walls, fences, doors and windows by side, stairs, slopes
//   Home                           back to the selected Sim
//   Ctrl+Shift+PageUp / PageDown   the floor above / below
//   PageDown / PageUp (Shift, Ctrl) the lot's things as everywhere; the cursor goes to the thing
//   Enter                          the interaction menu of what stands there (buy mode: pick it up)
//   Ctrl+Enter                     the selected Sim walks to the tile
//   R                              where the cursor is, in full (details buffer)
//   Escape                         leave the map
#pragma once

namespace nav::map {

void Reset();
void Tick();          // ends the map when the lot goes away or a building tool takes the keys
bool Active();
bool CursorTile(int* x, int* y, int* level);

void Toggle();
void End();
void Move(int dx, int dy);
void Home();
void ChangeFloor(int delta);
void Repeat();
void Activate();
void WalkHere();
void FollowFocus();   // after the Page keys: the cursor goes to the thing they focused

}  // namespace nav::map
