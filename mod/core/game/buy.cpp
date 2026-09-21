#include "buy.h"

#include <algorithm>
#include <cstring>

#include "../util/log.h"
#include "../util/text.h"
#include "addresses.h"
#include "hooks.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

// OBJD fields (u16) the catalog uses; FreeSO's numbering, checked on catalog objects.
constexpr int kObjdRoomFlags = 39;
constexpr int kObjdFunctionFlags = 40;
constexpr int kObjdRatings = 82;     // 82..88 hunger .. room (a chair's comfort is at 83)
constexpr int kObjdSkillFlags = 89;
constexpr int kObjdSubsort = 94;

Ptr Tool() {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cMoveTool") ? tool : 0;
}

Ptr Cursor() {
    Ptr tool = Tool();
    Ptr cursor = tool ? Rd<Ptr>(tool + addr::movetool::kCursor) : 0;
    return IsKindOf(cursor, "cXCursorObject") ? cursor : 0;
}

// Calls a virtual slot of obj after checking it is the function this build is known to have.
template <class Fn, class... Args>
bool CallSlot(Ptr obj, int slot, Ptr expected, const char* what, Args... args) {
    Ptr fn = Rd<Ptr>(Rd<Ptr>(obj) + slot * 4);
    if (fn != Va(expected) || !FunctionOk(expected)) {
        Log("buy: unexpected %s (slot %d = %p)", what, slot, (void*)fn);
        return false;
    }
    return Guarded([&] { ((Fn)fn)((void*)obj, args...); });
}

// The pick message the lot view passes the tool's mouse handlers: {12, view, 0, short y, short x}
// (screen position in game coordinates). The position only matters for dragging; the tile comes
// from the screen-to-world conversion, which the mod overrides.
struct PickInfo {
    uint32_t type;
    Ptr view;
    uint32_t unknown;
    int16_t screenY;
    int16_t screenX;
    uint32_t rest[8];
};

void GridToView(int gridX, int gridY, int* viewX, int* viewY) {
    int r = -Rd<int>(Rd<Ptr>(Va(addr::kWorld)) + addr::kWorldRotation) & 3;
    int n = addr::kWorldSpan - 1;
    switch (r) {
    case 1: *viewX = n - gridY; *viewY = gridX; break;
    case 2: *viewX = n - gridX; *viewY = n - gridY; break;
    case 3: *viewX = gridY; *viewY = n - gridX; break;
    default: *viewX = gridX; *viewY = gridY; break;
    }
}

void OverrideTile(Tile t) {
    int vx, vy;
    GridToView(t.x * 16 + 8, t.y * 16 + 8, &vx, &vy);
    SetWorldPointerOverride(vx, vy);
}

bool MouseEvent(int slot, Ptr expected, const char* what, Tile t) {
    typedef void(__thiscall * Fn)(void*, int, int, int, PickInfo*);
    Ptr tool = Tool();
    if (!tool) return false;
    PickInfo pick = {12, InputRoot(), 0, 300, 512, {}};
    int vx, vy;
    GridToView(t.x * 16 + 8, t.y * 16 + 8, &vx, &vy);
    return CallSlot<Fn>(tool, slot, expected, what, vx, vy, 0, &pick);
}

int LastError() { return Rd<int>(Va(addr::kPlacementError)); }

// The state the tool is in while the mouse is over the lot (what its slot 22 leaves): the mod
// works the tool whether or not the real mouse is there.
void Activate(Ptr tool) {
    WriteU8(tool + addr::movetool::kDragging, 0);  // a press left half done would turn it instead
    WriteU8(tool + addr::movetool::kActive, 1);
}

}  // namespace

bool IsProductButton(Ptr w) { return IsKindOf(w, "ProductButton"); }

bool ReadProduct(Ptr product, ProductInfo* out) {
    if (!IsKindOf(product, "Product")) return false;
    ProductInfo p;
    p.product = product;
    p.name = util::CleanForSpeech(ReadStdStringUtf8(product + addr::product::kName));
    p.description = util::CleanForSpeech(ReadStdStringUtf8(product + addr::product::kDescription));
    p.price = Rd<int>(product + addr::product::kPrice);
    p.type = Rd<Ptr>(product + addr::product::kType);
    Ptr objd = p.type ? Rd<Ptr>(p.type + addr::product::kTypeObjd) : 0;
    if (objd) {
        auto field = [&](int i) { return Rd<uint16_t>(objd + 2 * i); };
        p.roomFlags = field(kObjdRoomFlags);
        p.functionFlags = field(kObjdFunctionFlags);
        p.subsort = field(kObjdSubsort);
        for (int i = 0; i < kRatingCount; i++) p.ratings[i] = (int16_t)field(kObjdRatings + i);
        p.skillFlags = field(kObjdSkillFlags);
    }
    *out = p;
    return true;
}

bool ReadProductOfButton(Ptr button, ProductInfo* out) {
    return IsProductButton(button) && ReadProduct(Rd<Ptr>(button + addr::kProductButtonProduct), out);
}

namespace {

Ptr FindCatalog(Ptr w, int depth) {
    if (!w || depth > 5) return 0;
    if (IsKindOf(w, "cWinCatalog")) return w;
    for (Ptr c : Children(w))
        if (Ptr found = FindCatalog(c, depth + 1)) return found;
    return 0;
}

}  // namespace

bool HeldProduct(ProductInfo* out) {
    Ptr type = HeldProductType();
    Ptr catalog = type ? FindCatalog(MainWindow(), 0) : 0;
    if (!catalog) return false;
    Ptr begin = Rd<Ptr>(catalog + addr::kCatalogProducts), end = Rd<Ptr>(catalog + addr::kCatalogProducts + 4);
    for (Ptr p = begin; p && p < end && p - begin < 4 * 2000; p += 4) {
        Ptr product = Rd<Ptr>(p);
        if (Rd<Ptr>(product + addr::product::kType) == type) return ReadProduct(product, out);
    }
    return false;
}

bool GetFunds(int* funds) {
    Ptr simulator = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)) + addr::sim::kSimulator);
    if (!IsKindOf(simulator, "cSimulator") || Rd<int16_t>(simulator + addr::sim::kFundsMode) == 1) return false;
    *funds = Rd<int>(simulator + addr::sim::kFunds);
    return true;
}

bool InBuyTool() { return Tool() != 0; }

bool IsBuying() {
    Ptr tool = Tool();
    return tool && Rd<int>(tool + addr::movetool::kMode) == addr::movetool::kBuyMode &&
           Rd<Ptr>(tool + addr::movetool::kProductType) != 0;
}

bool IsPlacing() { return IsBuying() || HeldObject() != 0; }

Ptr HeldObject() {
    Ptr cursor = Cursor();
    Ptr object = cursor ? Rd<Ptr>(cursor + addr::cursor::kObject) : 0;
    if (!IsKindOf(object, "cXObject")) return 0;
    std::vector<Ptr> parts = ObjectParts(object);
    return parts.empty() ? object : parts[0];
}

Ptr HeldProductType() {
    Ptr tool = Tool();
    return tool && IsBuying() ? Rd<Ptr>(tool + addr::movetool::kProductType) : 0;
}

int HeldLevel() {
    typedef int(__cdecl * Fn)();
    int level = 1;
    if (FunctionOk(addr::kCurrentLevel)) Guarded([&] { level = ((Fn)Va(addr::kCurrentLevel))(); });
    return level < 1 || level > addr::rooms::kMaxLevel ? 1 : level;
}

bool BeginPlacing() {
    typedef void(__thiscall * Fn)(void*);
    Ptr tool = Tool();
    if (!tool || !IsPlacing()) return false;
    if (Cursor() && Rd<uint8_t>(tool + addr::movetool::kActive)) return true;
    CallSlot<Fn>(tool, addr::movetool::kBeginSlot, addr::movetool::kBegin, "cMoveTool::BeginPlacing");
    return Cursor() != 0;
}

// What the catalog does when no product is chosen: the buy tool (and the object it holds) gives way
// to the plain hand tool.
void CancelPlacing() {
    typedef void(__thiscall * SelectFn)(void*, int, void*);
    typedef void(__thiscall * DirtyFn)(void*, uint32_t);
    typedef void(__thiscall * CheckFn)(void*, int);
    StopShowingHeld();
    ProductInfo held;
    Ptr button = HeldProduct(&held) ? Rd<Ptr>(held.product + addr::product::kButton) : 0;
    Ptr state = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimsApp)) + addr::catalog::kState);
    if (!IsKindOf(state, "CPState") || !FunctionOk(addr::kSelectProduct) || !FunctionOk(addr::kMarkPanelDirty)) {
        Log("buy: cannot cancel, panel state %s", ClassName(state).c_str());
        return;
    }
    Guarded([&] {
        ((SelectFn)Va(addr::kSelectProduct))((void*)state, -1, nullptr);
        ((DirtyFn)Va(addr::kMarkPanelDirty))((void*)state, addr::kPanelSelectionChanged);
    });
    // The catalog leaves the product's button pressed; a click elsewhere would release it.
    if (IsProductButton(button) && FunctionOk(addr::kButtonSetChecked))
        Guarded([&] { ((CheckFn)Va(addr::kButtonSetChecked))((void*)button, 0); });
}

bool LiftHeld() {
    typedef void(__thiscall * Fn)(void*, int);
    Ptr cursor = Cursor();
    if (!cursor) return false;
    return CallSlot<Fn>(cursor, addr::cursor::kRemoveSlot, addr::cursor::kRemove, "cXCursorObject::Remove", 1);
}

int HeldRotationStep() {
    Ptr object = HeldObject();
    return object ? Rd<int16_t>(object + addr::place::kRotationStep) : 0;
}

// Turning the lifted object. A single-tile object turns with its own turn (what the held cursor's
// turn calls); a multi-tile object gets what the engine's turn (0x630a30) does to every part,
// without its fit test at the old spot, which would refuse turns the new spot allows: the new
// direction, the part's graphics, then the parts' offsets.
bool TurnHeld(int direction) {
    typedef void(__thiscall * TurnFn)(void*, int);
    typedef void(__thiscall * RefreshFn)(void*, int);
    typedef void(__thiscall * RedrawFn)(void*);
    typedef void(__thiscall * RelayoutFn)(void*);
    Ptr object = HeldObject();
    Ptr cursor = Cursor();
    if (!object || !cursor) return false;
    int delta = (direction - ObjectDirection(object)) & 7;
    if (delta == 0) return true;
    int step = HeldRotationStep();
    if (step <= 0 || delta % step != 0) return false;
    bool ok = true;
    if (!IsKindOf(object, "cXMTObject")) {
        ok = CallSlot<TurnFn>(object, addr::place::kTurnSlot, addr::place::kTurnSingle, "cXObject::Turn", delta / step);
    } else {
        if (!FunctionOk(addr::place::kPartRefresh) || !FunctionOk(addr::place::kPartRedraw)) return false;
        for (Ptr part : ObjectParts(object)) {
            int16_t dir = (int16_t)((ObjectDirection(part) + delta) & 7);
            ok &= Guarded([&] {
                *(volatile int16_t*)(part + addr::place::kDirection) = dir;
                ((RefreshFn)Va(addr::place::kPartRefresh))((void*)part, 0);
                ((RedrawFn)Va(addr::place::kPartRedraw))((void*)part);
            });
        }
        ok &= CallSlot<RelayoutFn>(object, addr::place::kRelayoutSlot, addr::place::kRelayoutMulti,
                                   "cXMTObject::Relayout");
    }
    ok &= CallSlot<RelayoutFn>(cursor, addr::cursor::kRelayoutSlot, addr::cursor::kRelayout, "cXCursorObject::Relayout");
    return ok && ObjectDirection(object) == (direction & 7);
}

int CheckHeldAt(Tile lead, int level) {
    typedef bool(__thiscall * Fn)(void*, int* pos, int level, void* surface, int slot);
    Ptr object = HeldObject();
    if (!object) return -1;
    Ptr expected = IsKindOf(object, "cXMTObject") ? addr::place::kCanPlaceMulti : addr::place::kCanPlaceSingle;
    int pos[2] = {lead.x * 16 + 8, lead.y * 16 + 8};
    bool fits = false;
    Ptr fn = Rd<Ptr>(Rd<Ptr>(object) + addr::place::kCanPlaceSlot * 4);
    if (fn != Va(expected) || !FunctionOk(expected)) {
        Log("buy: unexpected CanPlaceAt %p", (void*)fn);
        return -1;
    }
    if (!Guarded([&] { fits = ((Fn)fn)((void*)object, pos, level, nullptr, 0); })) return -1;
    if (fits) return 0;
    int error = LastError();
    return error > 0 ? error : -1;
}

int CheckHeldOn(Tile lead, int level, Ptr surface, int slot) {
    typedef bool(__thiscall * Fn)(void*, int* pos, int level, void* surface, int slot);
    Ptr object = HeldObject();
    if (!object || !IsKindOf(surface, "cXObject") || IsKindOf(object, "cXMTObject")) return -1;
    Ptr fn = Rd<Ptr>(Rd<Ptr>(object) + addr::place::kCanPlaceSlot * 4);
    if (fn != Va(addr::place::kCanPlaceSingle) || !FunctionOk(addr::place::kCanPlaceSingle)) return -1;
    int pos[2] = {lead.x * 16 + 8, lead.y * 16 + 8};
    bool fits = false;
    if (!Guarded([&] { fits = ((Fn)fn)((void*)object, pos, level, (void*)surface, slot); })) return -1;
    if (fits) return 0;
    int error = LastError();
    return error > 0 ? error : -1;
}

// What the held cursor's check (0x633290) walks when the object is not allowed on the floor
// there: the surface's container slots after the first, free ones whose height the object allows.
std::vector<int> FreeSlotsFor(Ptr surface) {
    std::vector<int> out;
    Ptr object = HeldObject();
    if (!object || !IsKindOf(surface, "cXObject")) return out;
    uint16_t heights = Rd<uint16_t>(object + addr::place::kAllowedHeights);
    Ptr begin = Rd<Ptr>(surface + addr::place::kContainerSlots);
    Ptr end = Rd<Ptr>(surface + addr::place::kContainerSlots + 4);
    if (!begin || end <= begin) return out;
    int count = std::min<int>((int)((end - begin) / addr::place::kContainerSlotSize), 32);
    for (int i = 0; i + 1 < count; i++) {
        Ptr slot = begin + (i + 1) * addr::place::kContainerSlotSize;
        int height = Rd<int>(slot + addr::place::kSlotHeight);
        if (Rd<int16_t>(slot + addr::place::kSlotOccupant) != 0 || height < 1 || height > 16) continue;
        if (heights & (1u << (height - 1))) out.push_back(i);
    }
    return out;
}

std::string PlacementErrorText(int code) {
    typedef void(__cdecl * LoadFn)();
    typedef const char*(__thiscall * GetFn)(void*, int, char);
    if (code <= 0 || !FunctionOk(addr::kStringSetGetAt) || !FunctionOk(addr::kLoadPlacementErrors)) return "";
    Ptr set = Rd<Ptr>(Va(addr::kPlacementErrorStrings));
    if (!set) {
        Guarded([&] { ((LoadFn)Va(addr::kLoadPlacementErrors))(); });
        set = Rd<Ptr>(Va(addr::kPlacementErrorStrings));
    }
    const char* text = nullptr;
    if (!set || !Guarded([&] { text = ((GetFn)Va(addr::kStringSetGetAt))((void*)set, code, (char)-1); }) || !text)
        return "";
    char copy[256] = {};
    if (!Read((Ptr)text, copy, sizeof copy - 1)) return "";
    return util::CleanForSpeech(util::Cp1252ToUtf8(copy));
}

// The tool puts the held object there now (one mouse move) and on its later moves.
void ShowHeldAt(Tile lead) {
    Ptr tool = Tool();
    if (!tool || !Cursor() || !WorldPointerHooked()) return;
    Activate(tool);
    OverrideTile(lead);
    MouseEvent(addr::movetool::kMoveSlot, addr::movetool::kMove, "cMoveTool::MouseMove", lead);
}

void StopShowingHeld() { ClearWorldPointerOverride(); }

int PlaceHeldAt(Tile lead) {
    Ptr tool = Tool();
    Ptr cursor = Cursor();
    if (!tool || !cursor || !WorldPointerHooked()) return -1;
    Activate(tool);
    OverrideTile(lead);
    MouseEvent(addr::movetool::kMoveSlot, addr::movetool::kMove, "cMoveTool::MouseMove", lead);
    int16_t placed = Rd<int16_t>(cursor + addr::cursor::kPlacedOk);
    Ptr object = HeldObject();
    int error = LastError();
    if (!placed) {
        StopShowingHeld();
        return error > 0 ? error : -1;
    }
    MouseEvent(addr::movetool::kDownSlot, addr::movetool::kDown, "cMoveTool::MouseDown", lead);
    MouseEvent(addr::movetool::kUpSlot, addr::movetool::kUp, "cMoveTool::MouseUp", lead);
    StopShowingHeld();
    // Placed: the tool let go of it (a purchase of a kind the game keeps buying hands over the next).
    bool done = HeldObject() != object;
    Log("buy: placed at %d,%d: %s (object %p)", lead.x, lead.y, done ? "done" : "still held", (void*)object);
    if (done) return 0;
    error = LastError();
    return error > 0 ? error : -1;
}

int PickUp(Ptr object) {
    typedef void(__thiscall * Fn)(void*, int, int, int, PickInfo*);
    Ptr tool = Tool();
    if (!tool || Cursor() || !IsKindOf(object, "cXObject")) return -1;
    int id = Rd<int16_t>(object + addr::obj::kId);
    Tile at{Rd<int>(object + addr::obj::kX) >> 4, Rd<int>(object + addr::obj::kY) >> 4};
    int vx, vy;
    GridToView(at.x * 16 + 8, at.y * 16 + 8, &vx, &vy);
    WriteU32(Va(addr::kPlacementError), 0);
    Activate(tool);
    PickInfo pick = {12, InputRoot(), 0, 300, 512, {}};
    // The mouse down on it picks it up; the object under the mouse is the handler's third argument.
    CallSlot<Fn>(tool, addr::movetool::kDownSlot, addr::movetool::kDown, "cMoveTool::MouseDown", vx, vy, id, &pick);
    WriteU8(tool + addr::movetool::kDragging, 0);  // held, not dragged
    Ptr held = HeldObject();
    std::vector<Ptr> parts = ObjectParts(object);
    Log("buy: pick up %d: %s", id, held && !parts.empty() && held == parts[0] ? "held" : "refused");
    if (held && !parts.empty() && held == parts[0]) return 0;
    int error = LastError();
    return error > 0 ? error : -1;
}

int SaleValue(Ptr object) {
    typedef int(__fastcall * Fn)(void*);
    if (!IsKindOf(object, "cXObject") || !FunctionOk(addr::place::kSaleValue)) return 0;
    int value = 0;
    Guarded([&] { value = ((Fn)Va(addr::place::kSaleValue))((void*)object); });
    return value;
}

namespace {

bool ToolKey(int vk) {
    typedef int(__thiscall * Fn)(void*, int);
    Ptr tool = Tool();
    if (!tool || !Cursor()) return false;
    StopShowingHeld();
    return CallSlot<Fn>(tool, addr::movetool::kKeySlot, addr::movetool::kKey, "cMoveTool::KeyDown", vk);
}

}  // namespace

bool SellHeld() { return !IsBuying() && ToolKey(VK_DELETE); }

bool PutBackHeld() { return !IsBuying() && ToolKey(VK_ESCAPE); }

}  // namespace game
