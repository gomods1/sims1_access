// The object browser of the lot view: every Sim and object a player can interact with, by
// category (Sims, then the buy mode categories) or by room, nearest to the selected Sim first.
//   PageUp / PageDown              previous / next object in the category or room
//   Shift+PageUp / Shift+PageDown  previous / next category
//   Ctrl+PageUp / Ctrl+PageDown    previous / next room (the first press: the Sim's own room)
//   Enter (after PageUp/PageDown)  open the object's interaction menu
//   Ctrl+Enter                     walk the selected Sim to the object's spot (into its room)
#pragma once

#include <string>

#include "../game/lot.h"

namespace nav::world {

// The name a player hears for an object: its catalog name, the mod's name for default lot objects
// (mailbox, trees), else its internal name.
std::string DisplayName(const game::LotObject& o);

void Reset();
void NextObject(int delta);
void NextCategory(int delta);
void NextRoom(int delta);
void Interact();     // opens the focused object's interaction menu (runs modally)
void WalkToFocus();  // sends the selected Sim to the focused object's spot
void Repeat();       // says the focused object again
game::Ptr FocusedObject();  // the object PageUp/PageDown last spoke, 0 if gone

}  // namespace nav::world
