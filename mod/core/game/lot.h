// The lot being played: its objects and Sims, their names and buy-mode categories, the selected
// Sim, and opening an object's interaction (pie) menu without the mouse.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "memory.h"
#include "rooms.h"

namespace game {

// OBJD function flags = the buy mode catalog categories (verified against catalog products).
enum FunctionFlag : uint16_t {
    kFnSeating = 0x01,
    kFnSurfaces = 0x02,
    kFnAppliances = 0x04,
    kFnElectronics = 0x08,
    kFnPlumbing = 0x10,
    kFnDecorative = 0x20,
    kFnMiscellaneous = 0x40,
    kFnLighting = 0x80,
};

struct LotObject {
    Ptr object = 0;       // cXObject* (cXPerson* for Sims)
    int id = 0;
    int x = 0, y = 0;     // position in 1/16 tile
    int level = 0;        // floor, 1 = ground
    int room = kNoRoom;   // the engine's room id (game/rooms.h)
    bool person = false;
    uint16_t functionFlags = 0;
    uint16_t roomFlags = 0;
};

Ptr LotModule();  // the ObjectModule of the loaded lot, 0 when none

// The game's tutorial object (house 7) is on the lot: the tutorial is running. Cheap: cached for a
// second.
bool TutorialObjectPresent();

// The tutorial's step texts in the game's language (its STR# 301, UTF-8), by their index there;
// empty when the tutorial object is not on the lot.
std::vector<std::string> TutorialTexts();

// A STR# resource of the object's own file in the game's language (UTF-8, entry 0 first); empty
// when it has none.
std::vector<std::string> ObjectStringTable(Ptr object, int strId);

// The game's own cry for the burglar ("¡Ladrón!", its STR# 305) when the Sim is the burglar (the
// object "NPC Thief"), else empty.
std::string BurglarShout(Ptr person);

// Shows the help system's waiting tip (what a click on its icon does). false when unavailable.
bool ShowHelpTip();
bool HelpTipPending();  // the help system has a tip waiting (the icon blinks)

// The object standing on the tile under view coordinates (1/16 tile, view orientation) on a
// floor, invisible helpers left out; 0 when none. The grid tile is returned too.
Ptr ObjectAtView(int viewX, int viewY, int level, int* tileX, int* tileY);

// Placed, visible objects a player can interact with: one entry per multi-tile object (its lead
// part), invisible helpers (portals, phone line, the plumb-bob) left out.
std::vector<LotObject> LotObjects();

// A Sim's name, or the object's catalog name in the game's language; empty for objects that are
// not in the catalog (default lot objects such as the mailbox). UTF-8.
std::string ObjectName(const LotObject& o);
// The object's internal English name from its resource ("MailBox - Part B").
std::string ObjectInternalName(const LotObject& o);

Ptr ObjectById(int id);           // any object of the lot (every part, Sims too), 0 if none

// Every placed object of the lot, one entry per part (a double bed is several), Sims included:
// several can share a tile (a counter, the lamp on it, the window on its wall).
struct PlacedPart {
    Ptr object = 0;
    int id = 0;
    int tileX = 0, tileY = 0;
    int level = 0;
};
std::vector<PlacedPart> PlacedParts();
bool IsInvisibleHelper(Ptr object);  // portals of the lot edge, phone line, plumb-bob, markers

Ptr SelectedSim();                // cXPerson* of the Sim the player controls, 0 if none
// Every Sim placed on the lot (the family, visitors, pets), cXPerson*. The game also keeps
// unplaced NPCs in its list ("Grandma", "Mystery Man"); they are left out.
std::vector<Ptr> LotPeople();
int SimNeighborId(Ptr person);    // the Sim's neighbor ID (for game/people.h), -1 if none
bool GetObject(Ptr object, LotObject* out);

// Opens the object's interaction menu for the selected Sim, the way clicking it does. The game
// runs the menu modally inside this call (the mod keeps receiving messages and ticks while it is
// open); choosing an entry (game::PieChoose) queues the interaction. The menu appears at
// screenX/Y.
bool OpenInteractionMenu(const LotObject& o, int screenX, int screenY);

// Sends the selected Sim to a spot on the lot (1/16 tile units), the way a click on the floor
// does: the picker places its "go here" target there and the Sim walks to it.
bool WalkTo(int worldX, int worldY, int screenX, int screenY);

}  // namespace game
