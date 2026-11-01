// Buy mode by keyboard. The catalog's product buttons read their name, price, ratings and the
// hover popup's description; the details buffer (Ctrl+Up/Down, nav/buffers.h) has every rating,
// the rooms it is meant for and the description by sentences. Choosing one (Enter) starts placing it: the mod finds the
// spots where it fits and can be used (placement.h), best first, and the object shows there.
//   PageDown / PageUp (Tab)        next / previous spot in the area
//   Ctrl+PageDown / Ctrl+PageUp    next / previous area (rooms, the outdoors)
//   Shift+PageDown / Shift+PageUp  the other directions that work on the same tile (comma, period)
//   Enter                          buy it on the spot
//   Escape                         stop placing (nothing is bought; a moved object goes back)
//   Delete                         sell the object being moved
// Enter on a thing of the lot (PageUp/PageDown, nav/world.h) picks it up to move it.
//   R                              repeat the spot (its details, and what else was checked in the
//                                  area, are in the details buffer: Ctrl+Up/Down)
#pragma once

#include <string>
#include <vector>

#include "../game/memory.h"

namespace nav::buy {

void Reset();
// A product was pressed: the next held object gets new spots even when it is the same product
// (the lot may have changed since: a new upper floor makes room for stairs).
void Renew();
void Tick();        // follows the buy tool: placing starts and ends, the next one of a kind
bool Placing();     // an object is being placed: the spot keys work

void NextSpot(int delta);
void NextArea(int delta);
void NextTurn(int delta);
// Ctrl+Shift+PageDown/PageUp: the lot's things, nearest the cursor first; the held object goes on
// a free tile beside the chosen one, facing it when it fits that way, and the arrows go on from
// there (a tester: placing a chair at a desk took twenty spots).
void NextThing(int delta);
void Confirm();
// Arrows while placing: the held object moves one tile (the engine says whether it fits there);
// Shift+PageUp/PageDown or comma/period then turn it, Enter puts it there, PageUp/PageDown go back
// to the spots.
void MoveFree(int dx, int dy);
bool FreeTurn(int delta);
bool Cancel();      // false when nothing is being placed
void Sell();        // Delete while moving an object: sell it (a purchase is cancelled instead)
void PickUpFocused();  // buy mode, Enter on an object of the lot browser: pick it up to move it
void Repeat();

// Catalog product buttons: "Silla todoterreno Nalghaus, $80, comodidad 2. <description>"; the
// details: name and price, each rating and skill, the rooms it is meant for, each sentence.
std::string ProductText(game::Ptr button);
std::vector<std::string> ProductDetails(game::Ptr button);

}  // namespace nav::buy
