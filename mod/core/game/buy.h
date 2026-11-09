// Buy mode: the catalog's products, the family's money, and the object a player is placing (held
// by the buy tool, cMoveTool): lifting and turning it, asking the engine whether it fits on a
// tile, and buying it there the way a click does. All on the game's main thread.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lotgrid.h"
#include "memory.h"

namespace game {

constexpr int kRatingCount = 7;  // hunger, comfort, hygiene, bladder, energy, fun, room
constexpr int kSkillCount = 7;   // cooking, mechanical, logic, body, creativity, charisma, study

struct ProductInfo {
    Ptr product = 0;
    std::string name, description;  // UTF-8, the catalog's
    int price = 0;
    Ptr type = 0;                    // object type record
    uint16_t functionFlags = 0;      // buy mode category (game/lot.h FunctionFlag)
    uint16_t subsort = 0;            // subcategory bits within it
    uint16_t roomFlags = 0;          // suggested rooms (OBJD field 39)
    int ratings[kRatingCount] = {};  // what the catalog popup lists ("Comodidad: 2")
    uint16_t skillFlags = 0;         // bit i = skill i above
};

bool IsProductButton(Ptr w);
bool ReadProduct(Ptr product, ProductInfo* out);
bool ReadProductOfButton(Ptr button, ProductInfo* out);
// The floor a product takes as the catalog shows it (before any turn): its tiles across (x) and
// deep (y), from the OBJD tile offsets of its multi-tile group. false when unknown.
bool ProductSize(Ptr type, int* across, int* deep);

// The product the buy tool holds, as the catalog lists it.
bool HeldProduct(ProductInfo* out);

// The family's money; false when the lot has unlimited funds or none is known.
bool GetFunds(int* funds);
// True on a lot nobody lives in: the simulator's unlimited-funds mode (the game shows no money
// and its funds getter answers 999999).
bool UnlimitedFunds();

// ---- The object being placed ----------------------------------------------------------------

bool InBuyTool();          // the buy/move tool is the current tool (buy mode, doors in build mode)
bool IsPlacing();          // the tool holds an object: a new purchase or one being moved
bool IsBuying();           // ...a new purchase from the catalog
Ptr HeldObject();          // the object (its lead part), 0 when none
Ptr HeldProductType();     // the product's type record (to find it in the catalog)
int HeldLevel();           // the floor the tool works on
bool BeginPlacing();       // makes the tool create the held object if it has none yet
void CancelPlacing();      // puts it away without buying

// Takes the held object out of the world (the ghost the mouse drags), so checks see the lot
// without it. The tool puts it back on its next mouse move.
bool LiftHeld();
bool TurnHeld(int direction);  // only while lifted
int HeldRotationStep();        // direction step per turn: 2 quarter turns, 1 eighths, 0 fixed

// The engine's own placement check for the held object with its lead part on `lead`: 0 when it
// fits, else the reason (a 1-based index into the game's placement messages).
int CheckHeldAt(Tile lead, int level);
// The catalog's "Atrás" (back from a category's subsorts to the categories), 0 when not shown.
Ptr CatalogBackButton();

constexpr int kErrorNeedsSurface = 24;  // STR# 137 entry 23, "Must place on a table or surface"
std::string PlacementErrorText(int code);  // the game's message, UTF-8
// The same on top of another object (a lamp on a table): `surface` is the part standing on the
// tile, `slot` one of FreeSlotsFor's indices. The buy tool finds such a slot by itself when the
// mouse is over the surface, so placing there is PlaceHeldAt on its tile.
int CheckHeldOn(Tile lead, int level, Ptr surface, int slot);
// The places on top of `surface` the held object fits (free, of a height it allows).
std::vector<int> FreeSlotsFor(Ptr surface);

// Shows the held object on this tile (it must be lifted and turned first): the tool gets a mouse
// move there now, and its later mouse moves also land there instead of under the mouse.
void ShowHeldAt(Tile lead);
void StopShowingHeld();

// Buys (or, when moving, puts down) the held object on this tile: a mouse move there, mouse
// down, mouse up. Returns 0 when it was placed, else the reason the engine gave.
int PlaceHeldAt(Tile lead);

// Picks an object of the lot up to move it, as a click on it does in buy mode: 0, or the reason
// the engine refuses (in use, cannot be picked up).
int PickUp(Ptr object);
int SaleValue(Ptr object);      // what selling it pays now
// The held object that is being moved: Delete sells it (the refund goes to the family), Escape
// puts it back where it was. false when the tool did not take the key.
bool SellHeld();
// Removes a door or window the way the game's Ctrl+click with their tool does, with whatever move
// tool is up; true when it is gone. (Neither can be picked up.)
bool RemovePortal(Ptr object);
bool PutBackHeld();

}  // namespace game
