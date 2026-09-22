// Where the camera points, for a player who hears the lot through it (the game's sounds come
// from what is on screen). The camera follows the selected Sim (the game's own follow mode)
// unless the player is browsing: then it points at whatever PageUp/PageDown speak (an object,
// a Sim, a placement spot), and follows the Sim again once the browser is left (arrows, Tab).
//   Ctrl+F      follow the selected Sim on/off ("siguiendo a Cris" / "cámara libre")
//   Ctrl+1..9   select the n-th Sim of the family (its portrait, as a click does) and say it
#pragma once

#include "../game/memory.h"

namespace nav::camera {

void Reset();
void Tick();                  // keeps the follow target in step with the selected Sim
void LookAt(game::Ptr object); // browsing: point the camera at this object now
void ToggleFollow();          // Ctrl+F
void SelectSim(int index);    // Ctrl+1..9: index 0-based

}  // namespace nav::camera
