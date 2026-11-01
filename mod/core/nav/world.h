// The lot's things (every Sim and object a player can interact with) as two groups of the lot
// view (NAVIGATION.md): "things by kind" (Sims, then the buy mode categories, then the rest) and
// "things by room" (the selected Sim's room first). Their items are kinds or rooms; Enter opens
// one (its things, nearest to the selected Sim first), Backspace goes back to the kinds or rooms.
// The camera looks at the thing focused. From anywhere in the lot view (every mode):
//   PageDown / PageUp              next / previous thing of the kind or room chosen last
//   Shift+PageDown / Shift+PageUp  next / previous kind (its things, from the nearest)
//   Ctrl+PageDown / Ctrl+PageUp    next / previous room (the first press: the selected Sim's)
// On a thing:
//   Enter        its interaction menu (buy mode: pick it up to move it)
//   Ctrl+Enter   walk the selected Sim to its spot (into its room)
#pragma once

#include <string>
#include <vector>

#include "../game/lot.h"
#include "model.h"

namespace nav::world {

// The name a player hears for an object: its catalog name, the mod's name for default lot objects
// (mailbox, trees), else its internal name.
std::string DisplayName(const game::LotObject& o);

void Reset();
Group KindsGroup();   // kKindsGroup: the kinds, or the things of the open kind
Group RoomsGroup();   // kRoomsGroup: the rooms, or the things of the open room
std::string LevelName(game::Ptr group);  // the group's name: its open kind or room, else its own
// Enter on a kind or room (its item's number) shows its things; Backspace goes back and returns
// the number of the kind or room that was open (0: none was).
void Open(game::Ptr group, int number);
int Close(game::Ptr group);
// The kind or room after the open one (delta +1) or before it, skipping empty ones, wrapping; with
// none open the first kind (the last going back) or the selected Sim's room. 0: the lot has none.
int Step(game::Ptr group, int delta);
int OpenNumber(game::Ptr group);  // the kind or room open in the group, 0: none
game::Ptr LastGroup();            // the group a kind or room was opened in last (kKindsGroup at first)
int FocusedId();                  // object id of the thing focus was last on, -1: none
void Focus(int objectId);   // the thing focus is on: the camera looks at it; Enter acts on it
bool HasFocus();            // a thing was focused and is still on the lot
std::vector<std::string> DetailsOf(int objectId);  // a listed thing's details
void Interact();
// Why the object's menu is empty for the selected Sim ("sin interacciones. solo para niños: Jugar").
std::string WhyNoInteractions(const game::LotObject& o);     // opens the focused object's interaction menu (runs modally)
void WalkToFocus();  // sends the selected Sim to the focused object's spot
void WalkToSpot(int worldX, int worldY);  // to a spot of the floor (1/16 tile units): the live map
game::Ptr FocusedObject();  // the thing focus was last on, 0 if gone
// The object bought last reads "recién comprado", so it can be told from identical ones (in buy
// mode identical objects are listed one by one, since one of them is to be moved).
void MarkBought(game::Ptr object);

// What the browser would say for this object (name, room, distance from the selected Sim), with
// its details in the buffer; or for a bare tile of a floor.
std::string DescribeObject(game::Ptr object);
std::string DescribeSpot(int tileX, int tileY, int level);

}  // namespace nav::world
