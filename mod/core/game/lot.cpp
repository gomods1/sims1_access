#include "lot.h"

#include <map>
#include <unordered_map>

#include "../util/log.h"
#include "../util/text.h"
#include "addresses.h"
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

// Catalog strings (CTSS) of an object type through the game's StringSet: string 1 is the name
// in the game's language.
std::string CatalogName(Ptr type, int ctssId) {
    typedef void(__thiscall * CtorFn)(void*);
    typedef void(__thiscall * SourceFn)(void*, void* resFile, short id, uint32_t type, char flag, char lang);
    typedef int(__thiscall * LoadFn)(void*, char, char);
    typedef const char*(__thiscall * GetFn)(void*, int index, char lang);
    typedef void(__thiscall * DtorFn)(void*);
    if (!FunctionOk(addr::kStringSetCtor) || !FunctionOk(addr::kStringSetSource) ||
        !FunctionOk(addr::kStringSetLoad) || !FunctionOk(addr::kStringSetGet) || !FunctionOk(addr::kStringSetDtor))
        return "";
    void* resFile = (void*)Rd<Ptr>(type + addr::objtype::kResFile);
    if (!resFile) return "";
    alignas(8) unsigned char set[addr::kStringSetSize] = {};
    std::string name;
    bool constructed = Guarded([&] { ((CtorFn)Va(addr::kStringSetCtor))(set); });
    if (!constructed) return "";
    Guarded([&] {
        ((SourceFn)Va(addr::kStringSetSource))(set, resFile, (short)ctssId, addr::kResourceCTSS, 0, -1);
        if (((LoadFn)Va(addr::kStringSetLoad))(set, 1, 0) == 0) {
            const char* s = ((GetFn)Va(addr::kStringSetGet))(set, 1, -1);
            if (s) name = s;
        }
    });
    Guarded([&] { ((DtorFn)Va(addr::kStringSetDtor))(set); });
    return util::Cp1252ToUtf8(name);
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
const char* const kInvisibleHelpers[] = {"Portal - ", "Phone Line", "PlumbBob", "Location - "};

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

Ptr LotModule() {
    Ptr root = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)));
    Ptr module = Rd<Ptr>(root + addr::kLotModule);
    return IsKindOf(module, "ObjectModule") ? module : 0;
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

bool Represents(Ptr object, const std::map<Family, bool>& familyHasStrings) {
    if (ObjdField(object, addr::objd::kMasterId) == 0) return true;  // single tile
    bool hasStrings = ObjdField(object, addr::objd::kCatalogStringsId) != 0;
    auto family = familyHasStrings.find(FamilyOf(object));
    if (family != familyHasStrings.end() && family->second) return hasStrings;
    return ObjdField(object, addr::objd::kSubIndex) == 0;
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
    for (size_t i = 0; i < count; i++) {
        Ptr object = Rd<Ptr>(begin + i * 4);
        if (!object || !IsPlaced(object)) continue;
        uint16_t kind = ObjdField(object, addr::objd::kObjectType);
        if (kind != 2 && kind != 4) continue;  // people and normal objects
        if (IsInvisibleHelper(object)) continue;
        candidates.push_back(object);
        if (ObjdField(object, addr::objd::kMasterId) != 0)
            familyHasStrings[FamilyOf(object)] |= ObjdField(object, addr::objd::kCatalogStringsId) != 0;
    }
    for (Ptr object : candidates) {
        if (!Represents(object, familyHasStrings)) continue;
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
