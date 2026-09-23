#include "lot.h"

#include <windows.h>

#include <algorithm>
#include <map>
#include <unordered_map>

#include "../util/log.h"
#include "../util/text.h"
#include "addresses.h"
#include "buy.h"
#include "people.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

uint16_t ObjdField(Ptr object, int field) {
    Ptr objd = Rd<Ptr>(object + addr::obj::kObjd);
    return Rd<uint16_t>(objd + 2 * field);
}

bool IsPlaced(Ptr object) {
    return (Rd<uint32_t>(object + addr::obj::kPlacement) & 0xffff) != 0xffff;
}

// A string list resource (CTSS, STR#) of an object type through the game's StringSet, in the
// game's language: raw game text, empty when it cannot be loaded.
std::vector<std::string> ObjectStrings(Ptr type, int id, uint32_t resourceType) {
    typedef void(__thiscall * CtorFn)(void*);
    typedef void(__thiscall * SourceFn)(void*, void* resFile, short id, uint32_t type, char flag, char lang);
    typedef int(__thiscall * LoadFn)(void*, char, char);
    typedef const char*(__thiscall * GetFn)(void*, int index, char lang);
    typedef void(__thiscall * DtorFn)(void*);
    std::vector<std::string> strings;
    if (!FunctionOk(addr::kStringSetCtor) || !FunctionOk(addr::kStringSetSource) ||
        !FunctionOk(addr::kStringSetLoad) || !FunctionOk(addr::kStringSetGet) || !FunctionOk(addr::kStringSetDtor))
        return strings;
    void* resFile = (void*)Rd<Ptr>(type + addr::objtype::kResFile);
    if (!resFile) return strings;
    alignas(8) unsigned char set[addr::kStringSetSize] = {};
    bool constructed = Guarded([&] { ((CtorFn)Va(addr::kStringSetCtor))(set); });
    if (!constructed) return strings;
    // The set owns the strings: pointers first (no C++ objects inside Guarded), copies after.
    const char* raw[1000] = {};
    Guarded([&] {
        ((SourceFn)Va(addr::kStringSetSource))(set, resFile, (short)id, resourceType, 0, -1);
        if (((LoadFn)Va(addr::kStringSetLoad))(set, 1, 0) != 0) return;
        GetFn get = (GetFn)Va(addr::kStringSetGet);
        for (int i = 0; i < 1000; i++) {
            raw[i] = get(set, i + 1, -1);  // 1-based; null past the last string
            if (!raw[i]) break;
        }
    });
    for (const char* s : raw) {
        if (!s) break;
        strings.push_back(s);
    }
    Guarded([&] { ((DtorFn)Va(addr::kStringSetDtor))(set); });
    return strings;
}

// String 1 of an object type's catalog strings (CTSS) is its name.
std::string CatalogName(Ptr type, int ctssId) {
    std::vector<std::string> strings = ObjectStrings(type, ctssId, addr::kResourceCTSS);
    return strings.empty() ? "" : util::GameTextToUtf8(strings[0]);
}

// The buy catalog's product for an object: parts of multi-tile objects (fireplace halves, table
// pieces) have no catalog strings or categories of their own; the product is the one whose
// object file and multi-tile group (OBJD MasterID) match. One file can hold several products
// (the dining tables), so the file alone is only the last resort. Built once per catalog; test
// entries ("*AnimTester") are skipped.
struct CatalogEntry {
    std::string name;
    Ptr objd = 0;  // the product's OBJD (the group's master for multi-tile objects)
};

const CatalogEntry* CatalogEntryFor(Ptr resFile, uint16_t masterId) {
    static Ptr cachedCatalog = 0;
    static std::map<std::pair<Ptr, uint16_t>, CatalogEntry> byGroup;
    static std::unordered_map<Ptr, CatalogEntry> byFile;
    Ptr catalog = Rd<Ptr>(Rd<Ptr>(Rd<Ptr>(Va(addr::kSimsApp)) + addr::catalog::kState) + addr::catalog::kCatalog);
    if (!IsKindOf(catalog, "Catalog") || !resFile) return nullptr;
    if (catalog != cachedCatalog) {
        byGroup.clear();
        byFile.clear();
        cachedCatalog = catalog;
        Ptr begin = Rd<Ptr>(catalog + addr::catalog::kProducts);
        Ptr end = Rd<Ptr>(catalog + addr::catalog::kProducts + 4);
        for (Ptr p = begin; p < end && p - begin < 4 * 20000; p += 4) {
            Ptr product = Rd<Ptr>(p);
            std::string name = ReadStdStringUtf8(product + addr::catalog::kProductName);
            Ptr type = Rd<Ptr>(product + addr::catalog::kProductType);
            Ptr file = Rd<Ptr>(type + addr::objtype::kResFile);
            Ptr objd = Rd<Ptr>(type + addr::product::kTypeObjd);
            if (name.empty() || name[0] == '*' || !file) continue;
            uint16_t master = objd ? Rd<uint16_t>(objd + 2 * addr::objd::kMasterId) : 0;
            byGroup.emplace(std::make_pair(file, master), CatalogEntry{name, objd});
            byFile.emplace(file, CatalogEntry{name, objd});
        }
    }
    if (masterId != 0) {
        auto hit = byGroup.find({resFile, masterId});
        if (hit != byGroup.end()) return &hit->second;
    }
    auto hit = byFile.find(resFile);
    return hit == byFile.end() ? nullptr : &hit->second;
}

const CatalogEntry* CatalogEntryOf(Ptr object) {
    Ptr type = Rd<Ptr>(object + addr::obj::kType);
    return CatalogEntryFor(Rd<Ptr>(type + addr::objtype::kResFile), ObjdField(object, addr::objd::kMasterId));
}

std::string InternalName(Ptr type) {
    char raw[64] = {};
    Read(Rd<Ptr>(type + addr::objtype::kInternalName), raw, sizeof raw - 1);
    return raw;
}

int NeighborIdOf(Ptr person) {
    typedef int(__cdecl * Fn)(void*);
    if (!FunctionOk(addr::kPersonNeighborId)) return -1;
    int id = -1;
    Guarded([&] { id = ((Fn)Va(addr::kPersonNeighborId))((void*)person); });
    return id;
}

// Invisible helper objects, by internal name prefix (constant English resource names, the same
// in every language). No OBJD field tells them apart from visible objects.
const char* const kInvisibleHelpers[] = {"Portal - ", "Phone Line", "PlumbBob", "Location - ", "Ped Marker"};

// Names by object type, valid for one lot.
Ptr g_nameCacheLot = 0;
std::unordered_map<Ptr, std::string> g_names;

}  // namespace

bool IsInvisibleHelper(Ptr object) {
    std::string name = InternalName(Rd<Ptr>(object + addr::obj::kType));
    for (const char* prefix : kInvisibleHelpers)
        if (name.rfind(prefix, 0) == 0) return true;
    return false;
}

// Where Sims walk onto the lot: its invisible "Portal - Pedestrian" and "Portal - Car" objects
// (plain cXObjects at the lot's edge, read live on the Compañeras lot).
bool IsLotEntrance(Ptr object) {
    std::string name = InternalName(Rd<Ptr>(object + addr::obj::kType));
    return name.rfind("Portal - Pedestrian", 0) == 0 || name.rfind("Portal - Car", 0) == 0;
}

Ptr ObjectById(int id) {
    Ptr module = LotModule();
    if (!module || id <= 0) return 0;
    Ptr begin = Rd<Ptr>(module + addr::world::kObjects);
    Ptr end = Rd<Ptr>(module + addr::world::kObjects + 4);
    if (end <= begin || (Ptr)id > (end - begin) / 4) return 0;
    return Rd<Ptr>(begin + (id - 1) * 4);
}

std::vector<PlacedPart> PlacedParts() {
    std::vector<PlacedPart> out;
    Ptr module = LotModule();
    if (!module) return out;
    Ptr begin = Rd<Ptr>(module + addr::world::kObjects);
    Ptr end = Rd<Ptr>(module + addr::world::kObjects + 4);
    size_t count = (end - begin) / 4;
    if (end < begin || count > 20000) return out;
    for (size_t i = 0; i < count; i++) {
        Ptr object = Rd<Ptr>(begin + i * 4);
        if (!object || !IsPlaced(object)) continue;
        uint32_t placement = Rd<uint32_t>(object + addr::obj::kPlacement);
        out.push_back({object, Rd<uint16_t>(object + addr::obj::kId), (int)((placement >> 8) & 0xff),
                       (int)(placement & 0xff), (int)((placement >> 16) & 0xff)});
    }
    return out;
}

// The tutorial object of the lot, 0 when none.
Ptr FindTutorialObject() {
    Ptr module = LotModule();
    if (!module) return 0;
    Ptr begin = Rd<Ptr>(module + addr::world::kObjects);
    Ptr end = Rd<Ptr>(module + addr::world::kObjects + 4);
    size_t count = (end - begin) / 4;
    if (end < begin || count > 20000) return 0;
    for (size_t i = 0; i < count; i++) {
        Ptr object = Rd<Ptr>(begin + i * 4);
        if (object && InternalName(Rd<Ptr>(object + addr::obj::kType)) == "Tutorial") return object;
    }
    return 0;
}

bool TutorialObjectPresent() {
    static Ptr cachedModule = 0;
    static ULONGLONG checkedAt = 0;
    static bool present = false;
    Ptr module = LotModule();
    ULONGLONG now = GetTickCount64();
    if (module == cachedModule && now - checkedAt < 1000) return present;
    cachedModule = module;
    checkedAt = now;
    present = FindTutorialObject() != 0;
    return present;
}

std::vector<std::string> TutorialTexts() {
    static Ptr cachedType = 0;
    static std::vector<std::string> texts;
    Ptr object = FindTutorialObject();
    Ptr type = object ? Rd<Ptr>(object + addr::obj::kType) : 0;
    if (type != cachedType) {
        cachedType = type;
        texts.clear();
        if (type)
            for (const std::string& raw : ObjectStrings(type, addr::kTutorialTexts, addr::kResourceSTR))
                texts.push_back(util::GameTextToUtf8(raw));
    }
    return texts;
}

std::vector<std::string> ObjectStringTable(Ptr object, int strId) {
    std::vector<std::string> texts;
    Ptr type = IsKindOf(object, "cXObject") ? Rd<Ptr>(object + addr::obj::kType) : 0;
    if (type)
        for (const std::string& raw : ObjectStrings(type, strId, addr::kResourceSTR))
            texts.push_back(util::GameTextToUtf8(raw));
    return texts;
}

bool ObjectInteractions(const LotObject& o, std::vector<InteractionEntry>* out) {
    typedef Ptr(__thiscall * TableFn)(void* type);
    out->clear();
    if (!o.object || !FunctionOk(addr::kTreeTable)) return false;
    Ptr type = Rd<Ptr>(o.object + addr::obj::kType);
    Ptr objd = Rd<Ptr>(o.object + addr::obj::kObjd);
    if (!type || !objd) return false;
    Ptr table = 0;
    if (!Guarded([&] { table = ((TableFn)Va(addr::kTreeTable))((void*)type); })) return false;
    if (!table) return true;  // no interactions at all
    int count = Rd<int>(table + addr::kTreeTableCount);
    Ptr entries = Rd<Ptr>(table + addr::kTreeTableEntries);
    if (count < 0 || count > 200 || (count && !entries)) return false;
    int ttas = ObjdField(o.object, addr::objd::kTreeTableId);
    std::vector<std::string> names = ObjectStrings(type, ttas, addr::kResourceTTAs);
    LogVerbose("interactions of #%d: table %p, %d entries, TTAs %d with %d names", o.id, (void*)table, count, ttas,
               (int)names.size());
    for (int i = 0; i < count; i++) {
        Ptr e = Rd<Ptr>(entries + i * 4);
        if (!e) continue;
        uint32_t flags = Rd<uint32_t>(e + addr::kTreeEntryFlags);
        if (flags & addr::kTreeFlagDebug) continue;
        int index = Rd<int>(e + addr::kTreeEntryName);
        InteractionEntry ie;
        if (index >= 0 && index < (int)names.size()) ie.name = util::GameTextToUtf8(names[index]);
        if (ie.name.empty()) continue;  // unnamed entries never show in a menu
        ie.notForChildren = (flags & addr::kTreeFlagNoChild) != 0;
        ie.notForAdults = (flags & addr::kTreeFlagNoAdult) != 0;
        ie.forCats = (flags & addr::kTreeFlagCats) != 0;
        ie.forDogs = (flags & addr::kTreeFlagDogs) != 0;
        out->push_back(ie);
    }
    return true;
}

SimKind KindOfSim(Ptr person) {
    short pets = Rd<short>(person + addr::kPersonPetFlags);
    if (pets & 0x2) return SimKind::Dog;
    if (pets & 0x4) return SimKind::Cat;
    short age = Rd<short>(person + addr::kPersonAge);
    return age > 0 && age <= 0x11 ? SimKind::Child : SimKind::Adult;
}

bool KindAllows(SimKind kind, const InteractionEntry& e) {
    switch (kind) {
    case SimKind::Adult: return !e.notForAdults;
    case SimKind::Child: return !e.notForChildren;
    case SimKind::Cat: return e.forCats;
    case SimKind::Dog: return e.forDogs;
    }
    return true;
}

std::string BurglarShout(Ptr person) {
    if (!IsKindOf(person, "cXPerson") || InternalName(Rd<Ptr>(person + addr::obj::kType)) != addr::kThiefName)
        return "";
    std::vector<std::string> texts = ObjectStringTable(person, addr::kThiefStrings);
    return texts.empty() ? "" : texts[0];
}

bool ShowHelpTip() {
    typedef void(__thiscall * Fn)(void*);
    Ptr module = LotModule();
    if (!module || !FunctionOk(addr::kShowHelpTip)) return false;
    return Guarded([&] { ((Fn)Va(addr::kShowHelpTip))((void*)module); });
}

bool HelpTipPending() {
    Ptr module = LotModule();
    return module && Rd<Ptr>(module + addr::kModuleSituation) != 0;
}

// The inverse of the picker's view-to-grid rotation (0x586ae0; GridToView below is its inverse).
Ptr ObjectAtView(int viewX, int viewY, int level, int* tileX, int* tileY) {
    int r = -Rd<int>(Rd<Ptr>(Va(addr::kWorld)) + addr::kWorldRotation) & 3;
    int n = addr::kWorldSpan - 1;
    int gridX, gridY;
    switch (r) {
    case 1: gridX = viewY; gridY = n - viewX; break;
    case 2: gridX = n - viewX; gridY = n - viewY; break;
    case 3: gridX = n - viewY; gridY = viewX; break;
    default: gridX = viewX; gridY = viewY; break;
    }
    *tileX = gridX >> 4;
    *tileY = gridY >> 4;
    for (const PlacedPart& p : PlacedParts())
        if (p.tileX == *tileX && p.tileY == *tileY && p.level == level && !IsInvisibleHelper(p.object) &&
            !IsKindOf(p.object, "cXPerson"))
            return p.object;
    return 0;
}

Ptr LotModule() {
    Ptr root = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)));
    Ptr module = Rd<Ptr>(root + addr::kLotModule);
    return IsKindOf(module, "ObjectModule") ? module : 0;
}

// Where the lot's first pieces of furniture stand: another lot has other furniture at other
// places. Only objects with a sale value count: the trolley, cars, plates of food and the like
// come and go and take free low ids (the Mercado's fingerprint flapped when the trolley came).
// Cached for half a second.
uintptr_t LotIdentity() {
    static ULONGLONG at = 0;
    static uintptr_t cached = 0;
    if (!LotModule()) return cached = 0;
    ULONGLONG now = GetTickCount64();
    if (cached && now - at < 500) return cached;
    at = now;
    std::vector<PlacedPart> parts = PlacedParts();
    std::sort(parts.begin(), parts.end(), [](const PlacedPart& a, const PlacedPart& b) { return a.id < b.id; });
    uint32_t hash = 2166136261u;
    int used = 0;
    for (const PlacedPart& p : parts) {
        if (IsKindOf(p.object, "cXPerson") || SaleValue(p.object) <= 0) continue;
        for (int v : {p.id, p.tileX, p.tileY, p.level}) hash = (hash ^ (uint32_t)v) * 16777619u;
        if (++used == 12) break;
    }
    return cached = used ? (uintptr_t)(hash | 1) : 0;
}

bool GetObject(Ptr object, LotObject* out) {
    if (!IsKindOf(object, "cXObject")) return false;
    LotObject o;
    o.object = object;
    o.id = Rd<uint16_t>(object + addr::obj::kId);
    o.x = Rd<int>(object + addr::obj::kX);
    o.y = Rd<int>(object + addr::obj::kY);
    uint32_t placement = Rd<uint32_t>(object + addr::obj::kPlacement);
    o.level = (placement >> 16) & 0xff;
    o.room = RoomAt(o.level, (placement >> 8) & 0xff, placement & 0xff);
    o.person = IsKindOf(object, "cXPerson");
    o.functionFlags = ObjdField(object, addr::objd::kFunctionFlags);
    o.roomFlags = ObjdField(object, addr::objd::kRoomFlags);
    if (o.functionFlags == 0 && ObjdField(object, addr::objd::kMasterId) != 0) {
        // A part of a multi-tile object: its categories are on the catalog's master object.
        if (const CatalogEntry* entry = CatalogEntryOf(object)) {
            o.functionFlags = Rd<uint16_t>(entry->objd + 2 * addr::objd::kFunctionFlags);
            o.roomFlags = Rd<uint16_t>(entry->objd + 2 * addr::objd::kRoomFlags);
        }
    }
    *out = o;
    return true;
}

// A multi-tile object is one cXObject per tile, all sharing their file and OBJD MasterID;
// SubIndex is the tile's offset ((y << 8) | x), not a lead flag. The part that carries the
// catalog strings stands for the object (the wet bar's "Main Piece"); when no part does, the
// part at offset 0 does.
using Family = std::pair<Ptr, uint16_t>;  // (object file, MasterID)

Family FamilyOf(Ptr object) {
    return {Rd<Ptr>(Rd<Ptr>(object + addr::obj::kType) + addr::objtype::kResFile),
            ObjdField(object, addr::objd::kMasterId)};
}

// The part that stands for a multi-tile object: the one with the catalog strings, else the one
// with the lowest tile offset (a cheap single bed has parts at offsets 1, 0x101 and 0x201 and no
// strings in any: none is at 0).
bool Represents(Ptr object, const std::map<Family, bool>& familyHasStrings, const std::map<Family, uint16_t>& familyFirstSub) {
    if (ObjdField(object, addr::objd::kMasterId) == 0) return true;  // single tile
    bool hasStrings = ObjdField(object, addr::objd::kCatalogStringsId) != 0;
    auto family = familyHasStrings.find(FamilyOf(object));
    if (family != familyHasStrings.end() && family->second) return hasStrings;
    auto first = familyFirstSub.find(FamilyOf(object));
    return first != familyFirstSub.end() && ObjdField(object, addr::objd::kSubIndex) == first->second;
}

std::vector<LotObject> LotObjects() {
    std::vector<LotObject> out;
    Ptr module = LotModule();
    if (!module) return out;
    Ptr begin = Rd<Ptr>(module + addr::world::kObjects);
    Ptr end = Rd<Ptr>(module + addr::world::kObjects + 4);
    size_t count = (end - begin) / 4;
    if (count > 20000) return out;
    std::vector<Ptr> candidates;
    std::map<Family, bool> familyHasStrings;
    std::map<Family, uint16_t> familyFirstSub;
    for (size_t i = 0; i < count; i++) {
        Ptr object = Rd<Ptr>(begin + i * 4);
        if (!object || !IsPlaced(object)) continue;
        uint16_t kind = ObjdField(object, addr::objd::kObjectType);
        // People, normal objects, food (a pizza box on a counter); and type 7 with catalog strings
        // (the cheap stove is one: the other type 7 objects are invisible controllers).
        bool catalogued7 = kind == 7 && ObjdField(object, addr::objd::kCatalogStringsId) != 0;
        if (kind != 2 && kind != 4 && kind != 34 && !catalogued7) continue;
        if (IsInvisibleHelper(object)) continue;
        candidates.push_back(object);
        if (ObjdField(object, addr::objd::kMasterId) != 0) {
            Family family = FamilyOf(object);
            familyHasStrings[family] |= ObjdField(object, addr::objd::kCatalogStringsId) != 0;
            uint16_t sub = ObjdField(object, addr::objd::kSubIndex);
            auto first = familyFirstSub.find(family);
            if (first == familyFirstSub.end() || sub < first->second) familyFirstSub[family] = sub;
        }
    }
    for (Ptr object : candidates) {
        if (!Represents(object, familyHasStrings, familyFirstSub)) continue;
        LotObject o;
        if (GetObject(object, &o)) out.push_back(o);
    }
    return out;
}

std::string ObjectName(const LotObject& o) {
    if (o.person) {
        std::string name = NeighborName(NeighborIdOf(o.object));
        if (!name.empty()) return name;
    }
    Ptr type = Rd<Ptr>(o.object + addr::obj::kType);
    Ptr module = LotModule();
    if (module != g_nameCacheLot) {
        g_names.clear();
        g_nameCacheLot = module;
    }
    auto hit = g_names.find(type);
    if (hit != g_names.end()) return hit->second;
    int ctss = ObjdField(o.object, addr::objd::kCatalogStringsId);
    std::string name = ctss ? CatalogName(type, ctss) : "";
    if (name.empty()) {
        const CatalogEntry* entry = CatalogEntryOf(o.object);
        if (entry) name = entry->name;
    }
    g_names[type] = name;
    return name;
}

std::string ObjectInternalName(const LotObject& o) { return InternalName(Rd<Ptr>(o.object + addr::obj::kType)); }

int SimNeighborId(Ptr person) { return person ? NeighborIdOf(person) : -1; }

Ptr SelectedSim() {
    Ptr module = LotModule();
    if (!module) return 0;
    Ptr begin = Rd<Ptr>(module + addr::world::kPeople);
    Ptr end = Rd<Ptr>(module + addr::world::kPeople + 4);
    for (Ptr p = begin; p < end && p - begin < 4 * 64; p += 4) {
        Ptr person = Rd<Ptr>(p);
        if (Rd<uint32_t>(person + addr::obj::kFlags) & addr::obj::kFlagSelected) return person;
    }
    return 0;
}

std::vector<Ptr> LotPeople() {
    std::vector<Ptr> out;
    Ptr module = LotModule();
    if (!module) return out;
    Ptr begin = Rd<Ptr>(module + addr::world::kPeople);
    Ptr end = Rd<Ptr>(module + addr::world::kPeople + 4);
    for (Ptr p = begin; p < end && p - begin < 4 * 64; p += 4) {
        Ptr person = Rd<Ptr>(p);
        if (person && IsPlaced(person)) out.push_back(person);  // unplaced ones are hidden NPCs
    }
    return out;
}

namespace {

// The picker takes clicks in view coordinates and turns them into grid coordinates for the
// camera rotation (0x586ae0 with r = -rotation & 3, n = span - 1). This is its inverse.
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

// A click in live mode: the picker's slot 30 with the clicked spot (grid coordinates here) and
// what is under it (object id 0 = the floor).
bool LiveClick(int worldX, int worldY, int objectId, int screenX, int screenY) {
    typedef void(__thiscall * OpenFn)(void* tool, int worldX, int worldY, short objectId, void* pickInfo);
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    if (!IsKindOf(tool, "cObjPickerTool")) {
        Log("live click: current tool is %s, not the live mode picker", ClassName(tool).c_str());
        return false;
    }
    Ptr fn = Rd<Ptr>(Rd<Ptr>(tool) + addr::kPickerOpenMenuSlot * 4);
    if (fn != Va(addr::kPickerOpenMenu) || !FunctionOk(addr::kPickerOpenMenu)) {
        Log("live click: unexpected picker slot 30 %p", (void*)fn);
        return false;
    }
    // The pick message the view passes on a click: {type 12, view, ?, short y, short x}.
    struct PickInfo {
        uint32_t type;
        Ptr view;
        Ptr unknown;
        int16_t screenY;
        int16_t screenX;
        uint32_t rest[8];
    } info = {12, InputRoot(), 0, (int16_t)screenY, (int16_t)screenX, {}};
    int viewX, viewY;
    GridToView(worldX, worldY, &viewX, &viewY);
    return Guarded([&] { ((OpenFn)fn)((void*)tool, viewX, viewY, (short)objectId, &info); });
}

}  // namespace

bool OpenInteractionMenu(const LotObject& o, int screenX, int screenY) {
    Log("interaction menu: opening for object %d", o.id);
    return LiveClick(o.x, o.y, o.id, screenX, screenY);
}

bool WalkTo(int worldX, int worldY, int screenX, int screenY) {
    Log("walk to %d,%d (tile %d,%d)", worldX, worldY, worldX >> 4, worldY >> 4);
    return LiveClick(worldX, worldY, 0, screenX, screenY);
}

}  // namespace game
