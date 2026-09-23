// Every address and struct offset the mod uses, for Sims.exe 1.10.7 (SHA-256 b7c8765e...).
// VAs are at the preferred base 0x400000; game::Va() relocates them. Evidence for each entry is
// in STRUCTS.md / PATCH_TARGETS.md at the repository root. Only core/game uses these.
#pragma once

#include <cstddef>
#include <cstdint>

namespace game::addr {

using Va = uintptr_t;

// ---- Globals ------------------------------------------------------------------------------
constexpr Va kWinMgr = 0xa7f484;          // cTSWinMgrW95*
constexpr Va kAppActive = 0xa52074;       // u8: WM_ACTIVATEAPP state; input is dropped while 0
constexpr Va kLotPopup = 0xa65324;        // cWinLotPopup* (shared neighborhood hover card)
constexpr Va kSimRoot = 0xa67044;         // app root: [[kSimRoot]] + kLotModule = ObjectModule
constexpr int kLotModule = 0x14;
// u8 at [kSimRoot] + kLanguage: the game's language number (util/language.h), chosen at startup
// by 0x457600 through the setter 0x45d580; the getter 0x455820 and the font code read it.
constexpr int kLanguage = 0x68;
constexpr Va kCurrentTool = 0xa67108;     // cTool*: cObjPickerTool in live mode
constexpr Va kSimsApp = 0xa63570;         // cSimsApp*: + 0xc4 CPState*, whose + 0x70 is the Catalog
namespace catalog {
constexpr int kState = 0xc4;
constexpr int kCatalog = 0x70;
constexpr int kProducts = 0x04;           // Catalog: std::vector<Product*> (begin, end)
constexpr int kProductName = 0x04;        // Product: std::string, in the game's language
constexpr int kProductType = 0x40;        // Product: object type record (resFile at +0)
}  // namespace catalog

// ---- Functions ----------------------------------------------------------------------------
// thiscall int (const char* text, int len, int width): wraps a text block, returns line count.
// Every popup and dialog text passes through it, so hooking it captures drawn text.
constexpr Va kFontCountLines = 0x723a70;
// thiscall void cWinLotPopup::Fill(lotRecord*, extra*, bool force)
constexpr Va kLotPopupFill = 0x492820;
// cdecl void (int neighborId, std::string* out): a Sim's name
constexpr Va kNeighborName = 0x43af20;
// cdecl std::string* (int neighborId): a Sim's short biography
constexpr Va kNeighborBio = 0x43a4e0;
// cdecl int (cXPerson*): neighbor ID of a Sim on the lot (matches its GUID), -1 if none
constexpr Va kPersonNeighborId = 0x43aac0;
// cdecl SimState* (int neighborId): a Sim's live state; float mood (-100..100) at +0xc. What the
// family panel's mood meter draws (0x4bb980). Returns a shared default record for unknown IDs.
constexpr Va kSimState = 0x43aec0;
constexpr int kSimStateMood = 0x0c;
// The record holds every motive as a float -100..100 at + index * 4, in the order of
// Behavior.iff STR# 134 (3 mood, 5 energy, 6 comfort, 7 hunger, 8 hygiene, 9 bladder, 13 room,
// 14 social, 15 fun): cWinMotive's value getter 0x49f590 reads [state + motive index * 4].
constexpr int kSimStateMotives = 0x00;
constexpr int kMotiveCount = 16;
// A Sim's action queue, as the queue icons get it (0x40f6c0): cdecl void (int neighborId,
// std::vector<Entry>* out) appends 0x28-byte entries, the current action first, the action's
// name in a std::string at +0x10. The icon builder keeps a static vector for it at kQueueScratch
// ({first, last, end}), empties it with the entry destructor (thiscall) and refills it.
constexpr Va kSimQueue = 0x43b230;
constexpr Va kQueueEntryDtor = 0x438700;
constexpr Va kQueueScratch = 0xa61ec0;
constexpr int kQueueEntrySize = 0x28;
constexpr int kQueueEntryName = 0x10;
// u8: 1 on the entry made from the Sim's running interaction (cXPerson + 0xaac, filled by
// 0x438f80 with its last argument 1), 0 on the queued ones (the ring at cXPerson + 0x824).
constexpr int kQueueEntryRunning = 0x09;
// The 0x28-byte entry's +0x0c points at the Sim's own 0x50-byte queue record (0x438f80 stores its
// argument there): +0x08 target object, +0x10 entry index, +0x1c priority (the "Action Queues"
// debug dump 0x52da90 prints them). Seen live: 50 for an interaction the player chose from a
// menu, 2 for one the Sim chose by free will.
constexpr int kQueueEntryRecord = 0x0c;
constexpr int kQueueRecordPriority = 0x1c;
constexpr int kPlayerPriority = 50;
// cdecl short* (int neighborId): a Sim's person data (personality traits at their index, 0..1000)
constexpr Va kSimPersonData = 0x43a480;
// cdecl Ptr (int neighborId): the Sim's person object on the lot, 0 when not on it
constexpr Va kPersonByNeighbor = 0x43abd0;
// cdecl int (int neighborId, int personDataIndex): a Sim's skill points, 0..1000 (one bar per 100)
constexpr Va kSimSkill = 0x43bc30;
// cdecl void (int fromNeighborId, int toNeighborId, RelOut* out): how the first Sim feels about
// the second, what a relationships card shows (0x4cd079 passes the selected Sim, then the
// card's). RelOut: +0 int raw daily, +4 u8 (relationship variable 1 set), +5 u8 (both ways
// above [0xa515ec]), +6 u8 (variable 3 set), +8 int lifetime, +0xc int daily, both -100..100.
constexpr Va kSimRelationship = 0x43b9c0;
constexpr int kRelOutSize = 0x10;
constexpr int kRelOutLifetime = 0x08;
constexpr int kRelOutDaily = 0x0c;
constexpr Va kJobPanelSkillIds = 0x98aca0;  // int[6]: person data indices of the job panel's skills
constexpr int kJobPanelSkillCount = 6;
// The pets' skills subpanels (cWinSubpanelDogSkills, cWinSubpanelCatSkills; both read this table in
// their slot 87, 0x4e23b0 / 0x4e1f50): person data 10, 12, 11 = housebroken, tricks, obedience
// (the same indices as a person's cooking, mechanical, charisma). Their buttons: a vector of
// cFlashyBtn* at +0x130 (begin) / +0x134 (end), in this order.
constexpr Va kPetPanelSkillIds = 0x98b02c;
constexpr int kPetPanelSkillCount = 3;
constexpr int kPetPanelSkillButtons = 0x130;

// StringSet (string list resources such as CTSS catalog strings), all thiscall:
constexpr Va kStringSetCtor = 0x58c2f0;       // void ()
constexpr Va kStringSetSource = 0x58ef00;     // void (resFile*, short id, u32 type, bool, char lang)
constexpr Va kStringSetLoad = 0x58d4c0;       // int (char, char): 0 on success
constexpr Va kStringSetGet = 0x58cd60;        // const char* (int index 1-based, char lang -1)
constexpr Va kStringSetDtor = 0x58c4b0;       // void ()
constexpr size_t kStringSetSize = 0x100;      // fields used up to +0x6c
constexpr uint32_t kResourceCTSS = 0x43545353;  // 'CTSS'
constexpr uint32_t kResourceSTR = 0x53545223;   // 'STR#'
constexpr uint32_t kResourceTTAs = 0x54544173;  // 'TTAs': the interactions' names
// thiscall TreeTable* (): an object type's interactions (cached at type + 0x64). TreeTable: +4
// int count, +0x10 entry pointer array; entry +0x00 u16 action tree, +0x02 u16 test tree, +0x14
// u32 flags, +0x18 int index of its name in the TTAs. The pie menu's test 0x60b110 drops an
// entry for a child Sim (person + 0x604 age 1..17) when flag 0x10 is set, for an adult when 0x40
// is set, for a cat (person + 0x612 bit 2) without 0x200, a dog (bit 1) without 0x400; then runs
// the test tree (FreeSO's TTABFlags agree: 0x80 debug). Seen: the cheap train set's Play 0x62,
// Join 0x41 (no adults), so adults get no menu.
constexpr Va kTreeTable = 0x596730;
constexpr int kTreeTableCount = 0x04;
constexpr int kTreeTableEntries = 0x10;
constexpr int kTreeEntryFlags = 0x14;
constexpr int kTreeEntryName = 0x18;
constexpr uint32_t kTreeFlagNoChild = 0x10, kTreeFlagNoAdult = 0x40, kTreeFlagDebug = 0x80;
constexpr uint32_t kTreeFlagCats = 0x200, kTreeFlagDogs = 0x400;
constexpr int kPersonAge = 0x604;       // short: years; 1..17 = a child
constexpr int kPersonPetFlags = 0x612;  // short: bit 1 dog, bit 2 cat

constexpr int kThiefStrings = 305;              // STR# of the burglar (thief.iff): entry 0 "Burglar!"
constexpr const char* kThiefName = "NPC Thief";   // the burglar's internal name (OBJD label)
constexpr int kTutorialTexts = 301;             // STR# of the tutorial object (Tutorial.iff): its step texts

// House ratings (the house subpanel's draw, 0x4dc390, does the same): a HouseStats of 11 dwords
// is initialised, filled by the sim root ([[kSimRoot]]), then asked for each rating (0..100).
namespace house {
constexpr Va kStatsInit = 0x56a990;   // thiscall void (): zeroes the struct
constexpr Va kStatsFill = 0x56b460;   // thiscall void (HouseStats*) on the sim root
constexpr size_t kStatsSize = 0x2c;
constexpr Va kRatings[5] = {0x56b7e0, 0x56b400, 0x56b940, 0x56b850, 0x56b620};  // thiscall int ()
}  // namespace house

// Rooms. The world ([kWorld], cRotatableWorld) keeps a room map per level: rows by tile y, each
// a u16 room id per tile x (the debug tile window 0x4e8f30 reads it through 0x4e8c70). The sim
// root keeps the room records in a std::map (id -> Room*).
constexpr Va kWorld = 0xa675b4;
constexpr int kWorldRotation = 0x84;  // world: int camera rotation 0..3
constexpr int kWorldTerrain = 0x14;   // world: the terrain map (4 corner heights per tile)
constexpr int kMapRows = 0x0c;        // a map: its rows (one pointer per tile y)
constexpr int kWorldSpan = 0x400;     // the grid in 1/16 tile units (64 tiles)
namespace rooms {
constexpr int kLevelMaps = 0x28;      // world: RoomMap* per level (index = level, 1 = ground)
constexpr int kMaxLevel = 7;
constexpr int kMapRows = 0x0c;        // RoomMap: u16** rows (by tile y), each a u16 per tile x
constexpr uint16_t kNone = 0xfffb;
constexpr int kManager = 0x24;        // sim root ([[kSimRoot]]): room manager
constexpr Va kFindRoom = 0x627c70;    // thiscall Room* (u16 id) on the manager
constexpr Va kIsOutside = 0x6280a0;   // thiscall bool () on a Room
constexpr int kArea = 0x38;           // Room: int area in half tiles
constexpr int kBeds = 0x58;           // Room: int bed count
constexpr int kBathObjects = 0x5c;    // Room: int bathroom object count
// Room fields the debug tile window prints (STRUCTS.md "Rooms"): light, dirt, the room score.
constexpr int kWindowLight = 0x44;
constexpr int kObjectLight = 0x48;
constexpr int kDirt = 0x4c;
constexpr int kImpact = 0x50;
constexpr int kGoodObjects = 0x54;
constexpr int kLamps = 0x280;
}  // namespace rooms

// cObjPickerTool vtable slot 30: void (int worldX, int worldY, short objectId, pickInfo*):
// hover + click on an object: builds its interactions and runs the pie menu modally.
constexpr Va kPickerOpenMenu = 0x621960;
constexpr int kPickerOpenMenuSlot = 30;
// cTSPieMenu vtable slots (thiscall)
constexpr Va kPieSetSelection = 0x6ec760;     // slot 102: (int index, bool choose): choose notifies
                                              // the owner (code 0xf), which queues the interaction
constexpr int kPieSetSelectionSlot = 102;
constexpr Va kPieEndModal = 0x6eb930;         // slot 147: (int result): ends DoModal with result
constexpr int kPieEndModalSlot = 147;
// cTSWinSlider slot 101: void SetValue(int): clamps, moves the thumb and notifies the owner
// (message {3, 12, slider}) the way dragging does.
constexpr Va kSliderSetValue = 0x707120;
constexpr int kSliderSetValueSlot = 101;

// ---- Buy mode (see STRUCTS.md "Buy mode") -------------------------------------------------
// [kSimRoot] is a table of subsystems: [0] the sim root, [1] the house viewer, [7] cSimulator.
namespace sim {
constexpr int kSimulator = 0x1c;      // cSimulator*: the family's money
constexpr int kFunds = 0x90;          // cSimulator: int funds
constexpr int kFundsMode = 0x50;      // cSimulator: short; 1 = unlimited (the funds getter
                                      // 0x556e20 answers 999999)
}  // namespace sim
namespace product {
constexpr int kName = 0x04;           // Product: std::string, the catalog name
constexpr int kDescription = 0x1c;    // std::string, the catalog description
constexpr int kPrice = 0x34;          // int (0x5368e0 returns it)
constexpr int kButton = 0x38;         // ProductButton* showing it
constexpr int kType = 0x40;           // object type record
constexpr int kTypeObjd = 0x60;       // type record: the lead object's OBJD (u16 fields)
}  // namespace product
constexpr int kProductButtonProduct = 0x1e0;  // ProductButton: Product*
// CPState ([kSimsApp] + catalog::kState): the buy panel's state. SelectProduct(-1, 0) is what the
// catalog does when nothing is chosen: the buy tool gives way to the plain hand tool.
constexpr Va kSelectProduct = 0x408750;       // thiscall void (int index, Product*)
constexpr Va kMarkPanelDirty = 0x407fa0;      // thiscall void (u32 flags); 0x200 after a selection
constexpr uint32_t kPanelSelectionChanged = 0x200;
constexpr Va kButtonSetChecked = 0x6e9b30;    // cTSWinBtn: thiscall void (int checked), redraws
constexpr int kCatalogProducts = 0x474;       // cWinCatalog: std::vector<Product*> of the page set

// The buy/move tool (cMoveTool, [kCurrentTool] in buy mode). Mouse handlers take (int worldX,
// int worldY, int objectUnderMouse, PickInfo*) with PickInfo {int 12, view, 0, short y, short x}
// (screen position in game coordinates). Every one of them calls slot 7 first.
// cNewWallTool (the wall and fence tool, [kCurrentTool] in build mode). Mouse handlers are
// thiscall (int viewX, int viewY, int objectId, PickInfo*), ret 0x10, in view coordinates (1/16
// tile). They round to the nearest corner (0x659590: +16 when the fraction is >= 0.5, then the
// corner index = v >> 4) and convert corners to the grid through 0x586ae0 ((c << 4) | 1, the
// camera rotation). Slot 8 sets the start and reads GetKeyState: Shift = a room (the rectangle's
// four walls, mode 2), Ctrl = delete (mode 1); slot 7 moves the end; slot 11 builds. +0x71 must
// be 1 (slot 22, "mouse over the lot"); slot 23 clears it. Errors go to 0x56f7f0.
namespace walltool {
constexpr int kMoveSlot = 7;
constexpr Va kMove = 0x659610;
constexpr int kDownSlot = 8;
constexpr Va kDown = 0x658eb0;
constexpr int kUpSlot = 11;
constexpr Va kUp = 0x6597b0;
// Slot 10: the mouse moved with the button down. Extends the drag session (+0x5c, 0xe068 bytes;
// segments through 0x636310, their cost summed at +0xe028, +0xe024 set once there is one); only
// while +0x70 is 0 (the down clears it). Slot 7 is a hover: it only moves the preview.
constexpr int kDragSlot = 10;
constexpr Va kDrag = 0x659240;
constexpr int kOverLot = 0x71;       // u8
constexpr int kSession = 0x5c;       // the drag session
constexpr int kSessionValid = 0xe024;  // u8: it holds segments
constexpr int kMode = 0x6c;          // int: 0 draw, 1 delete, 2 room
}  // namespace walltool
// cNewFloorTool: the same mouse handlers, on tiles (1/16 tile view coordinates, tile = v >> 4).
// Slot 8 reads GetKeyState: Ctrl = remove (1), Shift = fill the room (2), both = clear the room
// (3). The session at +0x30 lists changed tiles (0x3c-byte entries at +0x24..+0x28); slot 11
// commits it. The drag slot works while +0x3d is 0 (the down clears it).
namespace floortool {
constexpr int kDownSlot = 8;
constexpr Va kDown = 0x5896e0;
constexpr int kDragSlot = 10;
constexpr Va kDrag = 0x58a1b0;
constexpr int kUpSlot = 11;
constexpr Va kUp = 0x58a7d0;
constexpr int kSession = 0x30;
constexpr int kSessionFirst = 0x24;
constexpr int kSessionLast = 0x28;
}  // namespace floortool
// cNewWallpaperTool: mouse down papers the wall on the top or left edge (view) of the tile under
// the mouse (0x65a720 picks it; both walled: the screen position decides; none: error 6). Shift =
// the whole room behind that wall side (2), Ctrl = remove (1), both = strip the room (3). The
// session at +0x30 lists changed wall sides (0x34-byte entries at +0x24..+0x28); slot 11 commits.
namespace papertool {
constexpr int kDownSlot = 8;
constexpr Va kDown = 0x659d30;
constexpr int kUpSlot = 11;
constexpr Va kUp = 0x65aac0;
constexpr int kSession = 0x30;
constexpr int kSessionFirst = 0x24;
constexpr int kSessionLast = 0x28;
constexpr int kEntrySize = 0x34;
}  // namespace papertool
// cPoolTool (the water tool's pools and ponds; ground floor only): down (slot 8) starts the drag
// and runs the drag slot (10) itself, up (11) commits when +0x58 is set (slot 22, "over the lot",
// sets it) and the drag holds tiles (+0x48 count). +0x2c = 1 removes (slot 22 sets it from Ctrl
// through GetAsyncKeyState).
namespace pooltool {
constexpr int kDownSlot = 8;
constexpr Va kDown = 0x63b200;
constexpr int kDragSlot = 10;
constexpr Va kDrag = 0x63b560;
constexpr int kUpSlot = 11;
constexpr Va kUp = 0x63bd60;
constexpr int kRemoveMode = 0x2c;
constexpr int kTiles = 0x48;
constexpr int kOverLot = 0x58;
}  // namespace pooltool
// Terrain tools. cLevelDirtTool (level): down 0x658540 keeps the start in the globals 0xa6caf4/8,
// drag 0x658600 the end in 0xa6caec/f0 and the price at +0x30 (+0x38 = 6: not enough money), up
// 0x658940 flattens the rectangle to the height at the start; all only while +0x2c is 0.
// cSimpleAltTool (raise / lower, the chosen product decides): down 0x656580, up 0x657960.
namespace terraintool {
constexpr int kDownSlot = 8, kDragSlot = 10, kUpSlot = 11;
constexpr Va kLevelDown = 0x658540;
constexpr Va kLevelDrag = 0x658600;
constexpr Va kLevelUp = 0x658940;
constexpr int kLevelCost = 0x30;
constexpr int kLevelError = 0x38;
constexpr Va kAltDown = 0x656580;
constexpr Va kAltUp = 0x657960;
}  // namespace terraintool
// cdecl void (int code, int arg): the build tools' "Can't place here!" message; code = 1-based
// index into STR# 137 (as buy mode's placement errors), 0 hides it. Hooked: the mod speaks it.
constexpr Va kBuildError = 0x56f7f0;
namespace movetool {
constexpr int kMoveSlot = 7;          // 0x56dc90: re-places the held object at the tile under the
constexpr Va kMove = 0x56dc90;        //   mouse (ScreenToWorld 0x5e00e0), rotates while dragging
constexpr int kDownSlot = 8;          // 0x56ccc0: mouse down
constexpr Va kDown = 0x56ccc0;
constexpr int kUpSlot = 11;           // 0x56ec10: mouse up: buys and places a validly held object
constexpr Va kUp = 0x56ec10;
constexpr int kLeftSlot = 23;         // 0x56ea50: the mouse left the lot: clears kActive and takes the
constexpr Va kLeft = 0x56ea50;        //   held object out of the world (the view calls it every frame
                                      //   the real mouse is elsewhere; hooked while the mod shows a spot)
constexpr int kBeginSlot = 22;        // 0x56ea30: creates the held object for the chosen product
constexpr Va kBegin = 0x56ea30;       //   (the view calls it when the mouse enters the lot)
constexpr int kDropSlot = 6;          // 0x56d200: puts the held object away (cancel)
constexpr Va kDrop = 0x56d200;
constexpr int kKeySlot = 18;          // 0x56ea80: int (int vk): Delete sells (or throws away) the
constexpr Va kKey = 0x56ea80;         //   held object, Escape puts it back; nonzero = handled
constexpr int kBuying = 0x75;         // u8: the held object is a new purchase (else moved)
constexpr int kMode = 0x34;           // int: 2 = buying from the catalog
constexpr int kCursor = 0x3c;         // cXCursorObject*: the held object's cursor, 0 when none
constexpr int kProductType = 0x44;    // object type record of the product being bought
constexpr int kDragging = 0x7b;       // u8: mouse held down (dragging rotates)
constexpr int kActive = 0x97;         // u8: the held object follows the mouse; slot 22 (mouse
                                      // entered the lot) sets it, slot 23 (left) clears it and
                                      // takes the held object out of the world
constexpr int kBuyMode = 2;
// Build mode's objects are placed the same way in modes above it: 3 doors, 4 windows (seen live).
}  // namespace movetool
constexpr Va kScreenToWorld = 0x5e00e0;  // cdecl int* (int* out, u32* packedScreen, int flag):
                                         // view coordinates (1/16 tile) under a screen point
constexpr Va kPlacementError = 0xa67078; // int: the last placement check's reason, a 1-based index
                                         // into STR# 137 (PlacementErrs); 0 = none
constexpr Va kPlacementErrorStrings = 0xa6713c;  // StringSet* of STR# 137, 0 until first used
constexpr Va kLoadPlacementErrors = 0x56e6c0;    // cdecl void (): creates it
constexpr Va kStringSetGetAt = 0x58cd10;         // thiscall const char* (int index1, char lang)
constexpr Va kCurrentLevel = 0x590080;           // cdecl int (): the floor build/buy mode work on
// cXCursorObject (the held object's footprint outline; the object itself at +0x1b0)
namespace cursor {
constexpr int kObject = 0x1b0;        // cXObject*: the object being placed
constexpr int kPlacedOk = 0x1b4;      // short: the object sits validly where the cursor is
constexpr int kRemoveSlot = 20;       // 0x633fa0: takes cursor and object out of the world
constexpr Va kRemove = 0x633fa0;
constexpr int kRelayoutSlot = 34;     // 0x633200: copies the object's part offsets to the cursor
constexpr Va kRelayout = 0x633200;
}  // namespace cursor
// Objects (cXObject / cXMTObject), placement related
namespace place {
constexpr int kCanPlaceSlot = 24;     // bool (int pos[2], int level, cXObject* surface, int slot):
constexpr Va kCanPlaceSingle = 0x53efc0;  // the pure check the held object gets on every tile
constexpr Va kCanPlaceMulti = 0x630ee0;   //   (bounds, floor, walls, other objects); sets the error
constexpr int kTurnSlot = 19;         // cXObject 0x53e690: void (int turns): turns a single-tile object
constexpr Va kTurnSingle = 0x53e690;  //   by turns * kRotationStep (the held cursor's turn uses it)
constexpr int kRelayoutSlot = 34;     // cXMTObject 0x62fcd0: recomputes part offsets for the direction
constexpr Va kRelayoutMulti = 0x62fcd0;
constexpr Va kPartRefresh = 0x53a6f0; // thiscall void (int 0): a part's graphics after a turn
constexpr Va kPartRedraw = 0x53a760;  // thiscall void (): redraws a placed part's tile
constexpr int kDirection = 0x4c;      // short 0..7 (0, 2, 4, 6 = the four sides)
constexpr Va kSaleValue = 0x53adc0;   // fastcall int (cXObject*): what selling it pays now
constexpr int kContainer = 0x4e;      // short: id of the object it sits on or in (a lamp's table), 0
constexpr int kFlags = 0x5a;          // short: object flags (FreeSO VMEntityFlags): 4 no floor
                                      // extent (windows 0x404), 8 walks (Sims), 16 people pass
                                      // (doors 0x410, trees)
constexpr int kRotationStep = 0x78;   // short: direction step per turn (2 = quarter turn, 0 = fixed)
constexpr int kLevel = 0x110;         // int
constexpr int kNextPart = 0x190;      // cXMTObject*: next part (0 = last)
constexpr int kLead = 0x194;          // cXMTObject*: the lead part (0 on the lead itself)
constexpr int kPartY = 0x1a4;         // int: part offset in tiles, y
constexpr int kPartX = 0x1a8;         //                             x
constexpr int kPartLevel = 0x1ac;     //                             level
constexpr int kContainerSlots = 0x134;  // std::vector of 0x24-byte container slots (SLOT type 0):
                                        // entry 0 is the object itself, then places on it (a
                                        // table's top): +0x10 short occupant id, +0x14 int height
constexpr int kContainerSlotSize = 0x24;
constexpr int kSlotOccupant = 0x10;
constexpr int kSlotHeight = 0x14;
constexpr int kAllowedHeights = 0x52;  // short: bit (height - 1) set = fits a slot of that height
constexpr int kRoutingSlots = 0x140;  // std::vector of 0x3c-byte routing slots (SLOT type 3)
constexpr int kRoutingSlotSize = 0x3c;
}  // namespace place
// A routing slot (where a Sim stands to use the object), loaded by 0x62c820 from the object's
// SLOT resource: offsets relative to the object in its own frame.
namespace rslot {
constexpr int kX = 0x00;              // float, 1/16 tile
constexpr int kY = 0x04;              // float
constexpr int kFlags = 0x1c;          // u32: bits 0-7 N, NE, E, SE, S, SW, W, NW relative to the
                                      // object's front; 0x1000 snap to direction
constexpr int kSnapTarget = 0x20;     // int: container slot to snap to, -1 none
constexpr int kMinProximity = 0x24;   // int, 1/16 tile
constexpr int kMaxProximity = 0x28;
constexpr int kOptimalProximity = 0x2c;
}  // namespace rslot
// The lot grid ([kWorld], cRotatableWorld; the same object as [kWorldArch]). Per-level maps at
// +offset + level * 4, each {..., +0x0c rows[y]}, a row holding one entry per tile x.
constexpr Va kWorldArch = 0xa675b8;
namespace grid {
constexpr int kSize = 0x10;           // [kWorldArch]: int tiles per side (56 on a small lot)
constexpr int kFloors = 0x20;         // u16 per tile: floor pattern (0 = none)
constexpr int kFlags = 0x30;          // u8 per tile: 1 floorable, 2 seeded, 4 outside, 0x20 not editable
constexpr int kObjects = 0x38;        // u16 per tile: id of the object standing there (Sims too)
constexpr Va kWalls = 0x585a30;       // thiscall WallInfo* (WallInfo* out, u8 tile[3] {y, x, level})
                                      // on [kWorldArch]: u32 mask at +0 (1 = edge toward y-1,
                                      // 2 = toward x-1, 4 = toward y+1, 8 = toward x+1, 0x10/0x20
                                      // diagonals), u16 style per segment at +8
constexpr size_t kWallInfoSize = 0x2c;
}  // namespace grid

// ---- The cheat console (main window, cTSMainWindowW95) ------------------------------------
namespace cheat {
constexpr int kField = 0x140;         // cTSWinTextEdit2*: the console's text field (slot 105 sets it)
constexpr int kShowSlot = 107;        // 0x6f0c50: void (char show); what Ctrl+Shift+C calls from the
constexpr Va kShow = 0x6f0c50;        //   main window's key handler 0x6f0f70
constexpr int kManager = 0x144;       // cTSCheatCodeManager*
constexpr int kCommands = 0x04;       // manager: std::map<std::string, command> (head, size)
constexpr int kUsages = 0x0c;         // manager: std::map<std::string, std::string usage>
constexpr Va kRun = 0x6e67d0;         // thiscall bool (const char* line) on the manager: the interpreter
constexpr Va kSetText = 0x6ffd40;     // cTSWinTextEdit2: thiscall void (const char*, char, char)
}  // namespace cheat

// ---- cTSWinMgr ----------------------------------------------------------------------------
namespace winmgr {
constexpr int kInputRoot = 0x24;   // cTSWin*: the widget that owns input: a screen, a modal
                                   // dialog, a panel inside them, or the text field being typed in
constexpr int kMainWindow = 0x2c;  // cTSMainWindowW95*
}  // namespace winmgr

// ---- cTSWin (every widget) ----------------------------------------------------------------
namespace win {
constexpr int kRect = 0x0c;        // RECT, absolute 1024x768 game coordinates
constexpr int kChildren = 0x38;    // std::list<cTSWin*>: head node, count at +4
constexpr int kParent = 0x44;      // cTSWin*
constexpr int kCaption = 0x50;     // std::string
constexpr int kTooltip = 0xbc;     // std::string
constexpr int kFlags = 0xf4;       // u32
constexpr uint32_t kFlagVisible = 0x1;
}  // namespace win

// cTSWin message types, delivered through vtable slot 0 with {type, x, y, flags}
// (coordinates relative to the widget), as recorded from real mouse clicks.
namespace msg {
constexpr uint32_t kMouseEnter = 24;  // slot 64: sets the hovered flag release needs
constexpr uint32_t kLeftDown = 7;     // slots 69 + 55
constexpr uint32_t kLeftUp = 11;      // slots 71 + 59: notifies the owner (click)
constexpr uint32_t kMouseMove = 13;   // slots 70 + 61
constexpr uint32_t kMouseLeave = 25;  // slot 65
constexpr uint32_t kLeftButtonFlag = 0x10;
}  // namespace msg

// ---- cTSWinSlider --------------------------------------------------------------------------
namespace slider {
constexpr int kMin = 0x12c;        // int (slot 99 returns it)
constexpr int kMax = 0x130;        // int (slot 100)
constexpr int kValue = 0x134;      // int (slot 98)
}  // namespace slider

// ---- cTSWinTextEdit2 (a text field) -------------------------------------------------------
namespace textedit {
constexpr int kCaret = 0x128;      // int caret position (characters)
constexpr int kText = 0x138;       // std::string of game text (its size can count a trailing NUL)
}  // namespace textedit

// ---- cTSWinBtn ----------------------------------------------------------------------------
namespace btn {
constexpr int kChecked = 0x130;    // int
constexpr int kType = 0x148;       // int: 0 push, 1 toggle/mode, 2 check/radio
constexpr int kEnabled = 0x1b8;    // u8
}  // namespace btn

// ---- cWinLotBtn (a house on the neighborhood map) -----------------------------------------
// ---- cWinEditBio (a Sim's card: Ctrl+click on a family portrait, 0x4ba8d0) --------------------
namespace editbio {
constexpr int kNeighborId = 0x128;  // int: whose biography (+0x12c the field, then OK and Cancel)
}  // namespace editbio

namespace lot {
constexpr int kRecord = 0x1e0;     // lot record passed to the popup fill
constexpr int kFamily = 0x1f8;     // std::string, empty when vacant
constexpr int kNumber = 0x220;     // int house number
constexpr int kExtra = 0x274;      // second popup fill argument
constexpr int kFilterBits = 0x1f0; // what the travel map's filter bar marks the lot for (bits below)
}  // namespace lot

// ---- The travel maps (cWinNeighborhoodUL: Old Town) ------------------------------------------
// The screen keeps the filter bar's choice at +0x1ec (one bit, 0 none); its draw code
// (0x4a4530) puts a marker on every community lot whose kFilterBits share it. The lot kinds are
// a vector of {house number, kind} at [[kSimRoot] + 0x24] + 0x180 (0x54bf70 looks one up): 1 a
// home, 2 a community lot. Old Town's bar, left to right (pressed and read live): adoption
// centers 0x10, caged pets 0x20, shops 8, garden shops 4, restaurants 2, recreation 0x40,
// lodging 1 (the homes carry 1 too).
namespace travelmap {
constexpr int kFilter = 0x1ec;
constexpr int kFiltersOff = 0x185;  // byte: 1 on the home neighborhood (no filter bar), 0 on a travel map
constexpr int kNeighborhood = 0x24;
constexpr int kLotKinds = 0x180;
constexpr int kKindCommunity = 2;
constexpr int kOldTownBits[] = {0x10, 0x20, 8, 4, 2, 0x40, 1};
// cWinStudiotown (Superstar's map; not a cWinNeighborhood): nine filter buttons at +0x144, the
// chosen filter at +0x16c; its click handler 0x4d7650 sets them through 0x4d6540: caged pets,
// shops, garden, restaurants, recreation, spas 0x80, acting 0x100, fashion 0x200, music 0x400.
constexpr int kStudioFilters = 0x144;
constexpr int kStudioFilterCount = 9;
constexpr int kStudioFilter = 0x16c;
constexpr int kStudioBits[] = {0x20, 8, 4, 2, 0x40, 0x80, 0x100, 0x200, 0x400};
// cWinMagicland (Makin' Magic's map, a cWinNeighborhood): seven filter buttons at +0x15c, the
// chosen filter at +0x17c (0x49d830 -> 0x49c2c0): attractions 0x1000, caged pets, shops,
// garden, restaurants, recreation, magic places 0x800. Its lots 90-92 are homes.
constexpr int kMagicFilters = 0x15c;
constexpr int kMagicFilterCount = 7;
constexpr int kMagicFilter = 0x17c;
constexpr int kMagicBits[] = {0x1000, 0x20, 8, 4, 2, 0x40, 0x800};
// cWinVacation (Vacation Island's map): seven filter buttons at +0x140, the chosen filter at
// +0x160 (0x4f3f20 -> 0x4f2e00), Old Town's bits.
constexpr int kVacationFilters = 0x140;
constexpr int kVacationFilterCount = 7;
constexpr int kVacationFilter = 0x160;
}  // namespace travelmap

// ---- cPickFamilyItem (family picker row) / cPickFamilyItemMember (portrait) ---------------
namespace famrow {
constexpr int kMembers = 0x1e0;    // cPickFamilyItemMember*[8]; hidden portraits are empty slots
constexpr int kMemberSlots = 8;
constexpr int kName = 0x210;       // std::string family name
}  // namespace famrow
namespace fammember {
constexpr int kNeighborId = 0x1e0; // int
}  // namespace fammember

// ---- cWinMotive (need meter; caption is the need's name) ----------------------------------
namespace motive {
constexpr int kValue = 0x1e0;      // int -100..100
constexpr int kHelp = 0x20c;       // std::string, the game's explanation of the need
}  // namespace motive

// ---- cWinBudgetRow (a row of the family budget dialog, cWinBudgetDlg) ---------------------
namespace budget {
constexpr int kColumn1 = 0x148;    // std::string: value in the first period column ("$0"), or its name
constexpr int kColumn2 = 0x160;    // std::string: second period column ("Hoy" on the header row)
constexpr int kSummaryFormat = 0x17c;  // std::string: "Total contabilidad doméstica: %s"
constexpr int kSummaryValue = 0x194;   // std::string: "$9.500"
}  // namespace budget

// ---- cWinViewControl (the lot view's control panel: clock, funds, speed) ------------------
namespace viewctl {
constexpr int kLiveModeButton = 0x154; // cTSWinBtn* "Modo Vivir - F1" (then buy, build, options, camera)
constexpr int kModeButtons = 5;
constexpr int kFundsButton = 0x168;    // cTSWinBtn*: caption "$9.500", tooltip "Fondos"
constexpr int kPauseButton = 0x178;    // cTSWinBtn* toggle, checked while paused
constexpr int kSpeedButtons = 0x17c;   // cTSWinBtn*[3]: normal, high, ultra; the checked one is the speed
constexpr int kSpeedLevels = 3;
constexpr int kClock = 0x1cc;          // std::string: the time as drawn ("11:46")
constexpr int kFriendsButton = 0x25c;  // cTSWinBtn*: caption = the family's friend count
constexpr int kFloorButtons = 0x16c;   // cTSWinBtn*[2]: "Primer piso", "Segundo piso" (then "Tejado")
constexpr int kFloors = 2;
}  // namespace viewctl

// ---- cWinPeople (the live mode family panel) ----------------------------------------------
namespace people {
constexpr int kMemberButtons = 0x148;  // std::vector<cTSWinBtn*>: portraits (tooltip = first name,
                                       // checked = selected Sim); unused slots are hidden
constexpr int kNeedsBox = 0x158;       // cTSWin*: the "Necesidades" container
constexpr int kLivePopup = 0x1b0;      // cWinLivePopup*
constexpr int kSubpanelButtons = 0x1b4;  // cTSWinBtn*[7]: mood, relationships, job, house,
constexpr int kSubpanelCount = 7;        // personality, interests, inventory
}  // namespace people

// ---- cWinSubpanelJob (the family panel's job subpanel) ------------------------------------
namespace jobpanel {
constexpr int kSkillButtons = 0x130;  // cFlashyBtn** [6], same order as kJobPanelSkillIds
constexpr int kInfoButtons = 0x13c;   // cFlashyBtn** [4]: job, pay, performance, family friends
                                      // needed; their captions are set while the panel is drawn
}  // namespace jobpanel

// ---- cWinPersonality (a personality trait; caption = trait name) ---------------------------
namespace trait {
constexpr int kIndex = 0x1e0;      // int person data index of the trait
constexpr int kNeighborId = 0x1e4; // int whose trait it is (-1 = none)
// The hover popup's texts (0x4be260 picks them): std::string[3] titles and [3] descriptions for
// low (points below 3), middle and high (8 or more); a Sim's first, then a pet's.
constexpr int kTitles = 0x21c;
constexpr int kTexts = 0x264;
constexpr int kPetTitles = 0x2ac;
constexpr int kPetTexts = 0x2f4;
constexpr int kTextStride = 0x18;
}  // namespace trait

// ---- cWinGift (an item on the inventory subpanel: magic objects, ingredients, other) ------
namespace gift {
constexpr int kName = 0x1e8;         // std::string ("Tu varita mágica"; also the tooltip)
constexpr int kDescription = 0x200;  // std::string: what its hover popup says
}  // namespace gift

// ---- cWinInterest (an interest on the interests subpanel; tooltip = its name) -------------
namespace interest {
constexpr int kIndex = 0x1e0;      // int interest number
constexpr int kPoints = 0x1e4;     // int 0..10 (the draw 0x48df00 makes the bar 4 px per point)
}  // namespace interest

// ---- cWinSubpanelHouse (the family panel's house subpanel) --------------------------------
namespace housepanel {
constexpr int kButtons = 0x130;       // cTSWinBtn** [9]: 0-4 ratings (size, furnishings, yard,
constexpr int kButtonCount = 9;       // upkeep, layout), 5-8 area, bedrooms, bathrooms, lot size
}  // namespace housepanel

// ---- cWinDesignCharacter (create a Sim) and its personality bars (cWinPersonalityWidget) ---
namespace designsim {
constexpr int kAdult = 0x174;         // cTSWinBtn* age, sex and skin choices (checked = chosen)
constexpr int kChild = 0x178;
constexpr int kFemale = 0x17c;
constexpr int kMale = 0x180;
constexpr int kSkinDark = 0x184;
constexpr int kSkinMedium = 0x188;
constexpr int kSkinLight = 0x18c;
constexpr int kPreview = 0x1cc;       // cWinVitaBtn*
constexpr int kZodiac = 0x260;        // cTSWinBtn*: caption "(Cáncer)" once points are spent
constexpr int kPointsLeft = 0x16c;    // int personality points still to spend (of 25)
constexpr int kPrevBody = 0x190;      // cTSWinBtn* arrows: previous/next body, previous/next head
constexpr int kNextBody = 0x194;
constexpr int kPrevHead = 0x198;
constexpr int kNextHead = 0x19c;
constexpr int kHeadIndex = 0x1b8;     // int into the designer's head list
constexpr int kBodyIndex = 0x1bc;     // int into the designer's body list
}  // namespace designsim
// The character designer ([kDesigner]): the heads and bodies that fit the Sim's sex, age and
// skin, as vectors of 0x98-byte entries with the mesh name ("HEAD-HEAD=c036malgt_dave") in a
// StackString<64> at +0x4c (text at +0x0c of it).
constexpr Va kDesigner = 0xa6347c;
namespace designer {
constexpr int kHeads = 0x78;          // vector*: {begin, end}
constexpr int kBodies = 0x7c;
constexpr int kEntrySize = 0x98;  // also the outfit lists of cWinClothingChoiceDlg (below)
constexpr int kMeshName = 0x4c + 0x0c;
constexpr int kMeshNameSize = 64;
}  // namespace designer
namespace traitbar {
constexpr int kPoints = 0x1e0;        // int 0..10
constexpr int kName = 0x1e8;          // std::string trait name ("Pulcritud")
constexpr int kScreen = 0x23c;        // cWinDesignCharacter*
}  // namespace traitbar

// ---- cWinRelationship (a card on the relationships subpanel; tooltip "Marisa Compañeras") -
namespace relcard {
constexpr int kOtherId = 0x1e0;    // int neighbor ID of the Sim on the card
constexpr int kOwnerId = 0x1e4;    // int neighbor ID of the selected Sim
constexpr int kLifetime = 0x1e8;   // int -100..100, the lower number and bar
constexpr int kDaily = 0x1ec;      // int -100..100, the upper number and bar (moves with each chat)
}  // namespace relcard

// ---- The lot (ObjectModule) and its objects -----------------------------------------------
// The help system's waiting tip: a click on its icon (cTutorialIcon slot 55, 0x46a2e0) calls this
// on the ObjectModule, which runs the help system object's tree "show situation info" through
// the run-tree-by-name helper 0x613850.
constexpr Va kShowHelpTip = 0x532060;  // thiscall void () on the ObjectModule
constexpr int kModuleSituation = 0xa4; // ObjectModule: the object of the waiting tip, 0 when none
namespace world {
constexpr int kObjects = 0x18;     // std::vector<cXObject*>, index = object id - 1
constexpr int kPeople = 0x80;      // std::vector<cXPerson*>
}  // namespace world

// ---- SimAntics (object behavior; see STRUCTS.md "SimAntics and balloons") ------------------
// Primitives run through the object's vtable slot 0 (cXPerson's 0x5f6880 handles person-only
// opcodes and passes the rest here): thiscall int (Frame*, Instruction*), ret 8. An instruction
// is u16 opcode (low 15 bits), u8 true target, u8 false target, 8 operand bytes.
// thiscall bool (const char* eventName, int sourceObjectId), ret 8, on the sound manager
// ([[kSimRoot] + 0x10]): plays a named sound event (FWAV name) for an object; the UI passes -4
// ("UI_queued" from 0x40df10). Play Sound Event (primitive 0x17, 0x61ab20) looks the name up in
// the object's resources and calls it; so does every other sound of the game.
constexpr Va kPlaySoundEvent = 0x527250;
namespace simantics {
constexpr Va kDispatch = 0x615cb0;
constexpr int kOpcodeMask = 0x7fff;
constexpr int kOperands = 4;          // instruction: operand bytes
constexpr int kOpRemoveObject = 0x12; // Remove Object Instance: operand short 0 = 0 self, 1 the
                                      // stack object; handled inline (ObjectModule 0x52bfc0)
constexpr int kOpCreateObject = 0x2a; // Create New Object Instance (0x614700): the new object
                                      // becomes the stack object (hypothesis; logged)
// Frame (the running tree's stack frame): u16 tree id at +0, u16 instruction index at +2
constexpr int kFrameTree = 0x00;
constexpr int kFrameStackObject = 0x04;  // short: id of the stack object
constexpr int kModule = 0xdc;         // cXObject: its ObjectModule*
// Primitive 41, Set Balloon/Headline (the dispatcher's only caller of it): thiscall int (Frame*,
// const u8 operand[8]), ret 8. Operand (from it and the debugger's describer 0x65f8b0):
//   short 0: (local index << 1) | 1 = shown over the stack object instead of the running object
//   u8 2: icon index in the group   u8 3: group (Behavior.iff STR# 500: 0 old style, 1 balloon,
//   2 conversation, 3 motive, 4 relationship, 5 headline, 6 social, 7 algorithmic, 8 route
//   failure, 9 progress, 10 conversation 2)
//   short 4: duration (-1 forever, 0 = clear)   u8 6: balloon (0 thought, 1 scream, 2 speak, 3 none)
//   u8 7: 1 show when inactive, 2 crossed out, 4 animation reversed, 8 duration in loops,
//         0x10 icon index + the running object's short at kIconOffset (a temp)
constexpr Va kSetBalloon = 0x61bc40;
constexpr int kIconOffset = 0x3a;     // cXObject: short added to the icon index (flag 0x10)
constexpr int kGroupAlgorithmic = 7;  // index 0 the stack object's picture, 1 the face of the
                                      // Sim whose neighbor id is the stack object value, 2 the
                                      // picture of the object whose id is in local (short 0 >> 1)
constexpr int kFrameLocalCount = 0x06;  // Frame: u8
constexpr int kFrameArgCount = 0x07;  // Frame: u8; the locals (shorts) follow the arguments at
constexpr int kFrameArgs = 0x10;      //   kFrameArgs (the frame's own accessor 0x62db70)
// Global tree 0x18e "route failure feedback": the router runs it when a Sim cannot get where it
// was going (args: failure code, blocking object); it shows the blocker's picture or a reason.
constexpr uint16_t kRouteFailureTree = 0x18e;
// Graves bring their ghost out at night by creating it (Create Object Instance, the grave's tree
// 0x1004); the graves' internal names ("Urn/Tombstone" seen live; UrnStone.iff, UrnStonePet.iff,
// UrnStoneCelebrity.iff) contain this.
constexpr const char* kGraveNamePart = "Urn";
constexpr const char* kReaperName = "NPC Grim Reaper";  // internal name; the dying Sim creates him
// The balloon over an object: the first entry of a std::vector of 0x3c-byte records
constexpr int kHeadlines = 0x14c;
constexpr int kHeadlineSize = 0x3c;
constexpr int kHeadlineLeft = 0x10;   // int: ticks left, -1 forever, 0 = nothing shown
constexpr int kHeadlineSprite = 0x14; // int: sprite id (group base + index), -1 = a picture (below)
constexpr int kHeadlinePicture = 0x18;// the sprite set, or the object's picture when sprite is -1
constexpr int kHeadlineBalloon = 0x2c;// int: balloon sprite (100 thought, 101 scream, 102 speak), 0
constexpr int kHeadlineCrossed = 0x30;// int: 103 = crossed out, 0
constexpr int kHeadlinePriority = 0x34;  // int: 2 for the relationship group, else 1; a lower one
                                         // does not replace a balloon still shown
}  // namespace simantics
namespace obj {
constexpr int kId = 0xf0;          // short
constexpr int kX = 0xf4;           // int, 1/16 tile
constexpr int kY = 0xf8;           // int, 1/16 tile
constexpr int kPlacement = 0xfc;   // u32: byte0 tile y, byte1 tile x, byte2 level; 0xffff = not placed
constexpr int kFlags = 0x114;      // u32; people: bit 0x2 = selected Sim
constexpr uint32_t kFlagSelected = 0x2;
constexpr int kObjd = 0x11c;       // raw OBJD resource (little-endian u16 fields)
constexpr int kType = 0x120;       // object type record
}  // namespace obj
namespace objtype {
constexpr int kResFile = 0x00;     // ObjResFile* (the object's IFF)
constexpr int kInternalName = 0x18; // char* (English internal name, e.g. "Trash Outside")
}  // namespace objtype
// OBJD field indices (u16 each; the version takes fields 0-1)
namespace objd {
constexpr int kObjectType = 9;     // 2 person, 4 normal, 7 system, 12 inventory, 34 food (FreeSO OBJDType)
constexpr int kMasterId = 10;      // multi-tile group; 0 = single tile
constexpr int kSubIndex = 11;      // the part's tile offset in the group, (y << 8) | x
constexpr int kRoomFlags = 39;     // suggested rooms
constexpr int kFunctionFlags = 40; // buy mode categories
constexpr int kCatalogStringsId = 41;  // CTSS resource id (name = string 1)
constexpr int kTreeTableId = 13;   // TTAB (interactions) resource id; its names are the TTAs of the same id
constexpr int kFieldCount = 42;
}  // namespace objd

// ---- cWinClothingChoiceDlg (the wardrobe's "Cambiar de ropa") -------------------------------
// +0x13c / +0x140 previous / next outfit buttons, +0x1c4 the preview (cWinVitaBtnSolo). The preview
// keeps the outfit selector at +0x204 and the outfit kind at +0x208; the selector keeps one
// pointer to a vector of 0x98-byte entries per kind (mesh name as in the designer's lists):
// kind 0 everyday +0x7c, 2 formal +0x84, 4 swimwear +0x80, 5 pajamas +0x88, 0x11 winter +0x8c,
// 0x12 high fashion +0x90 (0x5e4a10 and its siblings). 0x4fa630 (fastcall on the preview)
// returns the shown outfit's index, -1 none.
namespace clothing {
constexpr int kPrev = 0x13c;
constexpr int kNext = 0x140;
constexpr int kPreview = 0x1c4;
constexpr int kSelector = 0x204;
constexpr int kKind = 0x208;
constexpr Va kCurrentIndex = 0x4fa630;
// cWinPetSelectDlg (a pet pen's "Adoptar"): the same preview and lists (kind 0 holds the pen's
// breeds, "b002dog_dalmation"), its buttons 4 bytes lower (0x4bfb40 handles them).
constexpr int kPetPrev = 0x138;
constexpr int kPetNext = 0x13c;
constexpr int kPetPreview = 0x1c0;
// cWinTransformMeDlg (Makin' Magic, "¿Bella o bestia?"): its click handler 0x4ecf50 cycles the
// look shown at +0x1cc through 0..7 with the buttons at +0x13c (previous) and +0x140 (next).
// Not seen live yet.
constexpr int kTransformPrev = 0x13c;
constexpr int kTransformNext = 0x140;
constexpr int kTransformChoice = 0x1cc;
constexpr int kTransformChoices = 8;
}  // namespace clothing

// ---- cTSPieMenu ---------------------------------------------------------------------------
namespace pie {
constexpr int kItems = 0x1e8;      // std::vector of 0x34-byte items: std::string label at +0
constexpr int kItemSize = 0x34;
constexpr int kItemFlags = 0x1c;   // u32 (0x1 normal; other bits unmapped)
constexpr int kHighlight = 0x128;  // int, -1 = none
// DoModal results the menu runner (0x61fe70) understands: an item index, -3 next page, -2 leave
// the runner (the picker then re-hovers and REOPENS the top-level menu), -1 back (from the top
// level: the runner returns 0 and the picker closes without doing anything).
constexpr int kResultLeave = -2;
constexpr int kResultBack = -1;
}  // namespace pie

// ---- cObjPickerTool (the live mode tool) --------------------------------------------------
namespace picker {
constexpr int kTopMenu = 0x148;    // cTSPieMenu* of the top level
constexpr int kSubMenu = 0x14c;    // cTSPieMenu* of sub-levels (entries ending in "...")
}  // namespace picker

// ---- Camera and the selected Sim ----------------------------------------------------------
// The lot view scrolls to a Sim or object through the object's own "center on me" (0x53a210),
// which the family panel calls when the selected portrait is clicked again. Following: the
// house viewer (0x567d30) calls slot 33 of [kFollowed] every frame while it is set (what Enter
// does in live mode through cWinPeople 0x4bc220).
namespace camera {
constexpr Va kCenterOnObject = 0x53a210;   // cXObject: thiscall void (char immediate): scrolls the
                                           // view (rotation applied, floor set) to the object
constexpr Va kFollowed = 0xa67548;         // cXPerson*: the Sim the camera follows, 0 = none
constexpr Va kSetFollowed = 0x57f480;      // cdecl void (cXPerson*): sets it (fires event 0xf5)
constexpr Va kPeopleStopFollow = 0x4ba880; // cWinPeople: thiscall void (): clears its PIP, then follows nobody
constexpr Va kSelectedNeighborId = 0x43bba0;  // cdecl int (): the selected Sim's neighbor id, -1
// What a click on a family portrait does (cWinPeople notify 0x4ba8d0, code 1): index into the
// panel's portrait vector (people::kMemberButtons), the family member's neighbor id, the buy panel
// state's select, mark dirty (8), then the panel's own update.
constexpr int kPeopleSelectedIndex = 0x140;   // cWinPeople: int, -1 none
constexpr Va kFamilyMemberId = 0x43b3b0;      // cdecl int (int index): neighbor id of the family's member
constexpr Va kSelectById = 0x4083e0;          // CPState: thiscall bool (int neighborId), ret 4
constexpr uint32_t kDirtySelection = 8;       // CPState::MarkDirty flag after a selection
constexpr Va kPeopleAfterSelect = 0x4b9900;   // cWinPeople: thiscall void (int neighborId)
}  // namespace camera

}  // namespace game::addr
