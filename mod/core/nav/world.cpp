#include "world.h"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/people.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "appearance.h"
#include "buffers.h"
#include "camera.h"
#include "narrator.h"
#include "navigator.h"
#include "placement.h"
#include "room_names.h"

namespace nav::world {
namespace {

using game::LotObject;

struct Category {
    Str name;
    bool (*accepts)(const LotObject&);
};

bool IsSim(const LotObject& o) { return o.person; }
template <uint16_t Flag>
bool HasFunction(const LotObject& o) { return !o.person && (o.functionFlags & Flag) != 0; }
bool IsOther(const LotObject& o) { return !o.person && o.functionFlags == 0 && !game::IsPortal(o.object); }
bool IsPortalThing(const LotObject& o) { return game::IsPortal(o.object); }

// Exact names (a word followed by " - " or nothing): "Fire" must not catch "Fireplace".
bool NamedLike(const std::string& internal, const char* word) {
    size_t n = strlen(word);
    return internal.rfind(word, 0) == 0 && (internal.size() == n || internal.compare(n, 3, " - ") == 0);
}

// Messes with no state of their own to read; the rest of the problems are broken or dirty.
bool IsMess(const std::string& internal) {
    static const char* const kMesses[] = {"Trash Pile", "Flood", "Ash", "Fire", "Flies", "Roaches", "Poop", "Dog - Dig - Hole", "Dog - Dig - Pile"};
    if (internal.find("Controller") != std::string::npos || internal.find("master") != std::string::npos) return false;
    for (const char* word : kMesses)
        if (NamedLike(internal, word)) return true;
    return false;
}

// What wants someone's attention: what the game's repairman and maid would go for, messes, and
// whatever has flies over it.
bool IsProblem(const LotObject& o) {
    if (o.person) return false;
    if (game::ObjectBroken(o.object) || game::ObjectDirty(o.object)) return true;
    if (IsMess(game::ObjectInternalName(o))) return true;
    return narrator::ShownBalloon(o.object) == S(Str::HeadlineFlies);
}

// Sims first, then the buy mode categories in catalog order, then everything without one
// (doors, windows, trees, mailbox...). An object with several functions is in each of them.
const Category kCategories[] = {
    {Str::CategorySims, IsSim},
    {Str::CategoryProblems, IsProblem},
    {Str::CategorySeating, HasFunction<game::kFnSeating>},
    {Str::CategorySurfaces, HasFunction<game::kFnSurfaces>},
    {Str::CategoryAppliances, HasFunction<game::kFnAppliances>},
    {Str::CategoryElectronics, HasFunction<game::kFnElectronics>},
    {Str::CategoryPlumbing, HasFunction<game::kFnPlumbing>},
    {Str::CategoryLighting, HasFunction<game::kFnLighting>},
    {Str::CategoryDecorative, HasFunction<game::kFnDecorative>},
    {Str::CategoryMiscellaneous, HasFunction<game::kFnMiscellaneous>},
    {Str::CategoryOther, IsOther},
    {Str::CategoryPortals, IsPortalThing},
};
constexpr int kCategoryCount = sizeof(kCategories) / sizeof(kCategories[0]);
constexpr int kUnitsPerTile = 16;

// Default lot objects are not in the catalog; these are named by the mod: by internal name
// prefix, and trees by the species word in theirs ("Trees - Unleashed - Sycamore - 2").
struct KnownObject {
    const char* word;
    Str name;
};
const KnownObject kKnownObjects[] = {
    {"MailBox", Str::ObjMailbox},
    {"Trash Outside", Str::ObjTrashOutside},
    {"Trash Pile", Str::ObjTrashPile},
    {"Dog - Dig - Hole", Str::ObjDigHole},  // a dog's digging (Unleashed)
    {"Dog - Dig - Pile", Str::ObjDigPile},
    {"Flies", Str::HeadlineFlies},  // over dirty dishes and rubbish, with their own balloon
    {"Roaches", Str::ObjRoaches},
    {"Flood - ", Str::ObjPuddle},   // water on the floor, from a broken sink or toilet
    {"Poop - ", Str::ObjPetMess},
};
const KnownObject kKnownExact[] = {{"Ash", Str::ObjAshes}, {"Fire", Str::ObjFire}};
const KnownObject kTreeSpecies[] = {
    {"Pine", Str::ObjPineTree},     {"Birch", Str::ObjBirchTree},   {"Sycamore", Str::ObjSycamoreTree},
    {"Oak", Str::ObjOakTree},       {"Maple", Str::ObjMapleTree},   {"Palm", Str::ObjPalmTree},
    {"Willow", Str::ObjWillowTree}, {"Apple", Str::ObjAppleTree},   {"Cherry", Str::ObjCherryTree},
    {"Elm", Str::ObjElmTree},       {"Cypress", Str::ObjCypressTree},
};

// Which list a thing's line is made for: a kind's (the room is said) or a room's (it is not).
enum class Mode { Category, Room };

Mode g_mode = Mode::Category;
int g_openKind = 0;          // the kind open in "things by kind" (category + 1), 0: the kinds
int g_openRoom = 0;          // the room open in "things by room" (room id + 1), 0: the rooms
Ptr g_lastGroup = kKindsGroup;  // the list a kind or room was opened in last: PageUp/PageDown's
int g_focusId = -1;          // object id of the thing focus was last on
int g_boughtId = -1;         // object id of the last purchase ("recién comprado")
std::map<int, std::vector<std::string>> g_details;  // object id -> its details, from the last list
// Identical things merged into one entry ("Silla, 2 iguales"): the members of each listed entry,
// by the entry's object id; Enter on one lists them one by one (a tester could not tell two
// identical things apart). The group key -> the entry open that way.
std::map<int, std::vector<int>> g_mergedIds;
struct SameOpen {
    std::vector<int> ids;
    std::string name;
    int entryId = 0;
};
std::map<Ptr, SameOpen> g_same;
constexpr ULONGLONG kLotCacheMs = 1000;

// One reading of the lot, taken for each key press.
struct Lot {
    std::vector<LotObject> objects;
    std::vector<rooms::NamedRoom> rooms;
    LotObject ref;  // where distances are measured from: the selected Sim
    bool hasRef = false;
    const LotObject* Ref() const { return hasRef ? &ref : nullptr; }
};

Lot ReadLot() {
    Lot lot;
    lot.rooms = rooms::NameRooms(game::LotObjects());
    lot.objects = game::LotThings();  // doors and windows too: a door must be findable to be removed
    lot.hasRef = game::GetObject(game::SelectedSim(), &lot.ref);
    return lot;
}

Lot g_lot;               // the lot as last read (CachedLot)
ULONGLONG g_lotAt = 0;
std::string g_findFolded;  // Ctrl+F: what the list holds, folded (util::FoldForSearch)
std::string g_findLabel;   // ... as it was typed

double DistanceTiles(const LotObject& a, const LotObject& b) {
    double dx = double(a.x - b.x) / kUnitsPerTile, dy = double(a.y - b.y) / kUnitsPerTile;
    return std::sqrt(dx * dx + dy * dy);
}

void SortByDistance(std::vector<LotObject>& list, const LotObject* ref) {
    if (!ref) return;
    std::stable_sort(list.begin(), list.end(), [&](const LotObject& a, const LotObject& b) {
        return DistanceTiles(a, *ref) < DistanceTiles(b, *ref);
    });
}

// A line of the browser: one object, or all objects of the same name (fence pieces, trees,
// matching chairs), represented by the nearest one.
struct Entry {
    LotObject nearest;
    std::vector<int> ids;  // every object the entry stands for
    std::string name;
};

// Objects of the same name collapse into one entry, in the order of their nearest member. Sims
// are never merged, nor anything in buy mode (one particular object is to be moved or sold).
std::vector<Entry> Collapse(const std::vector<LotObject>& sorted) {
    std::vector<Entry> out;
    std::map<std::string, size_t> byName;
    bool merge = !game::InBuyTool();
    for (const LotObject& o : sorted) {
        std::string name = DisplayName(o);
        auto hit = o.person || !merge ? byName.end() : byName.find(name);
        if (hit != byName.end()) {
            out[hit->second].ids.push_back(o.id);
            continue;
        }
        if (!o.person) byName[name] = out.size();
        out.push_back(Entry{o, {o.id}, name});
    }
    return out;
}

std::vector<LotObject> InCategory(const Lot& lot, int category) {
    std::vector<LotObject> out;
    for (const LotObject& o : lot.objects)
        if (kCategories[category].accepts(o)) out.push_back(o);
    SortByDistance(out, lot.Ref());
    return out;
}

std::vector<LotObject> InRoom(const Lot& lot, int room) {
    std::vector<LotObject> out;
    for (const LotObject& o : lot.objects)
        if (o.room == room) out.push_back(o);
    SortByDistance(out, lot.Ref());
    return out;
}

// Rooms nearest first (by their nearest object): the selected Sim's own room leads.
std::vector<const rooms::NamedRoom*> RoomsByDistance(const Lot& lot) {
    std::vector<std::pair<double, const rooms::NamedRoom*>> order;
    for (const rooms::NamedRoom& r : lot.rooms) {
        double nearest = 1e9;
        for (const LotObject& o : lot.objects)
            if (o.room == r.id) nearest = std::min(nearest, lot.hasRef ? DistanceTiles(o, lot.ref) : 0.0);
        order.push_back({nearest, &r});
    }
    std::stable_sort(order.begin(), order.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<const rooms::NamedRoom*> out;
    for (const auto& entry : order) out.push_back(entry.second);
    return out;
}

// Compass direction on the lot grid: north is -y, east is +x.
Str Direction(const LotObject& from, const LotObject& to) {
    double angle = std::atan2(double(from.y - to.y), double(to.x - from.x)) * 180.0 / 3.14159265358979;
    int sector = (int)std::floor((angle + 360.0 + 22.5) / 45.0) % 8;  // 0 = east, counter-clockwise
    static const Str kDirections[8] = {Str::DirEast,  Str::DirNorthEast, Str::DirNorth, Str::DirNorthWest,
                                       Str::DirWest,  Str::DirSouthWest, Str::DirSouth, Str::DirSouthEast};
    return kDirections[sector];
}

// Objects no Sim can get to from the lot's entrance (a tester's Sim never reached the chair he
// had placed), per floor, read at most every few seconds.
bool Unreachable(int id) {
    static std::vector<int> ids;
    static ULONGLONG at = 0;
    static Ptr module = 0;
    ULONGLONG now = GetTickCount64();
    if (!at || now - at > 3000 || module != game::LotModule()) {
        at = now;
        module = game::LotModule();
        ids.clear();
        for (int level = 1; level <= 2; level++) {
            placement::ReachReport r = placement::CheckReach(level);
            ids.insert(ids.end(), r.unusableIds.begin(), r.unusableIds.end());
        }
    }
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

// "Nevera Porcina, cocina, a 21 casillas al sur"; "Silla Parisienne, 6 iguales, comedor, a 11
// casillas al suroeste" (the nearest one). The room is left out in room mode, where the player
// already knows it.
std::string Describe(const Entry& e, const Lot& lot) {
    const LotObject& o = e.nearest;
    std::string s = e.name;
    if (e.ids.size() > 1) s += ", " + util::Format(S(Str::SameObjects), (int)e.ids.size());
    if (o.id == g_boughtId) s += std::string(", ") + S(Str::JustBought);
    if (!o.person && Unreachable(o.id)) s += std::string(", ") + S(Str::NobodyReaches);
    if (!o.person && game::ObjectBroken(o.object)) s += std::string(", ") + S(Str::ConditionBroken);
    else if (!o.person && game::ObjectDirty(o.object)) s += std::string(", ") + S(Str::ConditionDirty);
    const LotObject* ref = lot.Ref();
    if (ref && o.object == ref->object) s += std::string(", ") + S(Str::SelectedSimSuffix);
    if (o.person) {  // what the Sim is doing: its current action
        std::vector<std::string> actions = game::SimActions(game::SimNeighborId(o.object));
        if (!actions.empty()) s += ", " + actions.front();
    }
    std::string balloon = narrator::ShownBalloon(o.object);  // "piensa: hambre", flies over the trash
    if (!balloon.empty() && balloon != e.name) s += ", " + balloon;  // the flies' own "moscas" once
    if (g_mode == Mode::Category) {
        std::string room = rooms::NameOf(lot.rooms, o.room);
        if (!room.empty()) s += ", " + room;
    }
    if (!ref || o.object == ref->object) return s;
    int tiles = (int)std::lround(DistanceTiles(o, *ref));
    const char* dir = S(Direction(*ref, o));
    if (tiles < 1) s += std::string(", ") + S(Str::HereNow);
    else if (tiles == 1) s += ", " + util::Format(S(Str::OneTileAway), dir);
    else s += ", " + util::Format(S(Str::TilesAway), tiles, dir);
    if (o.level != ref->level) s += std::string(", ") + S(Str::OtherFloor);
    return s;
}

// What a Sim looks like (a tester: "the second game will need it even more"): the head and body
// in the words create-a-Sim uses.
std::vector<std::string> LookLines(Ptr person) {
    std::string head, body;
    std::vector<std::string> lines;
    if (!game::SimLook(person, &head, &body)) return lines;
    std::string h = appearance::Describe(head), b = appearance::Describe(body);
    if (!h.empty()) lines.push_back(util::Format(S(Str::SimLookHead), h.c_str()));
    if (!b.empty()) lines.push_back(util::Format(S(Str::SimLookBody), b.c_str()));
    return lines;
}

// The details buffer's lines for an entry: its name, what a Sim is doing and has queued, where it
// is, its buy mode categories and what selling it would pay.
std::vector<std::string> Details(const Entry& e, const Lot& lot) {
    const LotObject& o = e.nearest;
    std::vector<std::string> lines;
    lines.push_back(e.ids.size() > 1 ? e.name + ", " + util::Format(S(Str::SameObjects), (int)e.ids.size()) : e.name);
    if (e.ids.size() > 1) lines.push_back(S(Str::SameObjectsHint));
    if (o.person) {
        std::vector<std::string> actions = game::SimActions(game::SimNeighborId(o.object));
        for (size_t i = 0; i < actions.size(); i++)
            lines.push_back(i == 0 ? actions[i] : util::Format(S(Str::QueuedAction), actions[i].c_str()));
        for (const std::string& look : LookLines(o.object)) lines.push_back(look);
    }
    std::string balloon = narrator::ShownBalloon(o.object);
    if (!balloon.empty()) lines.push_back(balloon);
    std::string room = rooms::NameOf(lot.rooms, o.room);
    if (!room.empty()) lines.push_back(room);
    const LotObject* ref = lot.Ref();
    if (ref && o.object != ref->object) {
        int tiles = (int)std::lround(DistanceTiles(o, *ref));
        const char* dir = S(Direction(*ref, o));
        if (tiles < 1) lines.push_back(S(Str::HereNow));
        else if (tiles == 1) lines.push_back(util::Format(S(Str::OneTileAway), dir));
        else lines.push_back(util::Format(S(Str::TilesAway), tiles, dir));
        if (o.level != ref->level) lines.push_back(S(Str::OtherFloor));
    }
    if (!o.person) {
        for (int c = 1; c < kCategoryCount; c++)
            if (kCategories[c].accepts(o)) lines.push_back(S(kCategories[c].name));
        int value = game::SaleValue(o.object);
        if (value > 0) lines.push_back(util::Format(S(Str::SellsFor), util::Money(value).c_str()));
    }
    return lines;
}

// "cocina, 12 objetos, 37 casillas"; the outdoors without its size (the whole lot).
std::string RoomSummary(const rooms::NamedRoom& room, size_t count, bool outside) {
    std::string s = count == 1 ? util::Format(S(Str::RoomObjectsOne), room.name.c_str())
                               : util::Format(S(Str::RoomObjects), room.name.c_str(), (int)count);
    if (!outside) s += ", " + util::Format(S(Str::RoomTiles), room.tiles);
    return s;
}

std::string Summary(const std::string& name, size_t count) {
    return count == 1 ? util::Format(S(Str::GroupSummaryOne), name.c_str())
                      : util::Format(S(Str::GroupSummary), name.c_str(), (int)count);
}

bool OnLot() {
    if (game::LotModule()) return true;
    speech::Say(S(Str::NotOnLot));
    return false;
}

}  // namespace

std::string DisplayName(const game::LotObject& o) {
    // Known objects first: some have catalog strings that are not theirs (the flies' file also
    // holds the VR headset, and their OBJD points at its name).
    std::string internal = game::ObjectInternalName(o);
    for (const KnownObject& k : kKnownObjects)
        if (internal.rfind(k.word, 0) == 0) return S(k.name);
    for (const KnownObject& k : kKnownExact)
        if (internal == k.word) return S(k.name);
    std::string name = game::ObjectName(o);
    if (!name.empty()) return name;
    if (internal.rfind("Trees", 0) == 0) {
        for (const KnownObject& k : kTreeSpecies)
            if (internal.find(k.word) != std::string::npos) return S(k.name);
        return S(Str::ObjTree);
    }
    size_t part = internal.find(" - Part ");
    if (part != std::string::npos) internal.resize(part);
    return internal;
}

std::string DescribeObject(game::Ptr object) {
    Lot lot = ReadLot();
    LotObject o;
    if (!game::GetObject(object, &o)) return "";
    Entry e{o, {o.id}, DisplayName(o)};
    buffers::SetDetails(Details(e, lot));
    return Describe(e, lot);
}

std::string SimLookLine(Ptr person) {
    game::LotObject o;
    if (!game::GetObject(person, &o) || !o.person) return "";
    std::vector<std::string> lines = LookLines(person);
    std::string line = game::ObjectName(o);
    if (lines.empty()) return util::Format(S(Str::SimLookUnknown), line.c_str());
    for (const std::string& l : lines) line += ". " + l;
    return line;
}

std::string DescribeSpot(int tileX, int tileY, int level) {
    Lot lot = ReadLot();
    LotObject o;
    o.x = tileX * kUnitsPerTile + kUnitsPerTile / 2;
    o.y = tileY * kUnitsPerTile + kUnitsPerTile / 2;
    o.level = level;
    o.room = game::RoomAt(level, tileX, tileY);
    Entry e{o, {}, S(Str::SpotOnLot)};
    Mode mode = g_mode;
    g_mode = Mode::Category;  // the room is part of the description
    std::string s = Describe(e, lot);
    g_mode = mode;
    return s;
}

void MarkBought(game::Ptr object) {
    LotObject o;
    g_boughtId = game::GetObject(object, &o) ? o.id : -1;
}

void Refocus() { g_lotAt = 0; }

void Reset() {
    g_findFolded.clear();
    g_findLabel.clear();
    g_boughtId = -1;
    g_mode = Mode::Category;
    g_openKind = 0;
    g_openRoom = 0;
    g_lastGroup = kKindsGroup;
    g_focusId = -1;
    g_details.clear();
    g_mergedIds.clear();
    g_same.clear();
    g_lotAt = 0;
}

namespace {

// The lot read at most once a second: the groups are made again at every refresh (4 a second).
const Lot& CachedLot() {
    ULONGLONG now = GetTickCount64();
    if (!g_lotAt || now - g_lotAt > kLotCacheMs) {
        g_lot = ReadLot();
        g_lotAt = now;
    }
    return g_lot;
}

// Ctrl+F's list: names starting with the text first, then with a word starting with it, then
// containing it; within each the nearest first.
std::vector<Entry> FindMatches(const Lot& lot, const std::string& folded) {
    std::vector<LotObject> all = lot.objects;
    SortByDistance(all, lot.Ref());
    std::vector<Entry> ranked[3];
    for (Entry& e : Collapse(all)) {
        std::string name = util::FoldForSearch(e.name);
        size_t at = name.find(folded);
        if (at == std::string::npos) continue;
        int rank = at == 0 ? 0 : 2;
        if (rank == 2) {
            size_t word = name.find(" " + folded);
            if (word != std::string::npos) rank = 1;
        }
        ranked[rank].push_back(std::move(e));
    }
    std::vector<Entry> out;
    for (auto& r : ranked)
        for (Entry& e : r) out.push_back(std::move(e));
    return out;
}

// A kind's or room's things as items (number = the object id of the entry's nearest member);
// their details are kept for the details buffer.
void AddThings(Group& g, const std::vector<Entry>& list, const Lot& lot, Mode mode) {
    g_mode = mode;  // the room is part of the line in a kind's list, not in a room's
    for (const Entry& e : list) {
        Item it;
        it.widget = g.key;
        it.role = Role::LotObject;
        it.number = e.nearest.id;
        it.label = Describe(e, lot);
        g_details[e.nearest.id] = Details(e, lot);
        if (e.ids.size() > 1) g_mergedIds[e.nearest.id] = e.ids;
        else g_mergedIds.erase(e.nearest.id);
        g.items.push_back(std::move(it));
    }
}

// The members of an open merged entry, each on its own, nearest first.
bool AddSame(Group& g, const Lot& lot, Mode mode) {
    auto open = g_same.find(g.key);
    if (open == g_same.end()) return false;
    std::vector<LotObject> members;
    for (const LotObject& o : lot.objects)
        if (std::find(open->second.ids.begin(), open->second.ids.end(), o.id) != open->second.ids.end())
            members.push_back(o);
    SortByDistance(members, lot.Ref());
    std::vector<Entry> list;
    for (const LotObject& o : members) list.push_back(Entry{o, {o.id}, DisplayName(o)});
    AddThings(g, list, lot, mode);
    return true;
}

}  // namespace

Group KindsGroup() {
    Group g;
    g.key = kKindsGroup;
    if (!game::LotModule()) return g;
    const Lot& lot = CachedLot();
    if (AddSame(g, lot, Mode::Category)) return g;
    if (g_openKind == kFindKind) {
        AddThings(g, FindMatches(lot, g_findFolded), lot, Mode::Category);
        return g;
    }
    if (g_openKind > 0) {
        AddThings(g, Collapse(InCategory(lot, g_openKind - 1)), lot, Mode::Category);
        return g;
    }
    for (int c = 0; c < kCategoryCount; c++) {
        std::vector<Entry> list = Collapse(InCategory(lot, c));
        if (list.empty()) continue;
        Item it;
        it.widget = g.key;
        it.role = Role::Level;
        it.number = c + 1;
        it.label = Summary(S(kCategories[c].name), list.size());
        g.items.push_back(std::move(it));
    }
    return g;
}

Group RoomsGroup() {
    Group g;
    g.key = kRoomsGroup;
    if (!game::LotModule()) return g;
    const Lot& lot = CachedLot();
    if (AddSame(g, lot, Mode::Room)) return g;
    if (g_openRoom > 0) {
        AddThings(g, Collapse(InRoom(lot, g_openRoom - 1)), lot, Mode::Room);
        return g;
    }
    for (const rooms::NamedRoom* room : RoomsByDistance(lot)) {  // the selected Sim's room first
        std::vector<LotObject> objects = InRoom(lot, room->id);
        if (objects.empty()) continue;
        game::Room info;
        bool outside = game::GetRoom(room->id, &info) && info.outside;
        Item it;
        it.widget = g.key;
        it.role = Role::Level;
        it.number = room->id + 1;
        it.label = RoomSummary(*room, objects.size(), outside);
        g.items.push_back(std::move(it));
    }
    return g;
}

std::string LevelName(Ptr group) {
    const Lot& lot = CachedLot();
    auto same = g_same.find(group);
    if (same != g_same.end())
        return same->second.name + ", " + util::Format(S(Str::SameObjects), (int)same->second.ids.size());
    if (group == kKindsGroup && g_openKind == kFindKind) return util::Format(S(Str::FindResults), g_findLabel.c_str());
    if (group == kKindsGroup) return g_openKind > 0 ? S(kCategories[g_openKind - 1].name) : S(Str::ThingsByKind);
    if (g_openRoom > 0) return rooms::NameOf(lot.rooms, g_openRoom - 1);
    return S(Str::ThingsByRoom);
}

void Open(Ptr group, int number) {
    g_lotAt = 0;  // read the lot again: what is in the kind or room now
    g_same.erase(group);
    g_lastGroup = group;
    if (group == kKindsGroup) g_openKind = number;
    if (group == kRoomsGroup) g_openRoom = number;
}

int Step(Ptr group, int delta) {
    const Lot& lot = CachedLot();
    std::vector<int> numbers;  // the kinds or rooms that have things, in their list's order
    if (group == kKindsGroup) {
        for (int c = 0; c < kCategoryCount; c++)
            if (!InCategory(lot, c).empty()) numbers.push_back(c + 1);
    } else {
        for (const rooms::NamedRoom* room : RoomsByDistance(lot))
            if (!InRoom(lot, room->id).empty()) numbers.push_back(room->id + 1);
    }
    if (numbers.empty()) return 0;
    int open = group == kKindsGroup ? g_openKind : g_openRoom;
    int n = (int)numbers.size(), at = -1;
    for (int i = 0; i < n; i++)
        if (numbers[i] == open) at = i;
    // From the list itself: the first kind, or the selected Sim's room (rooms come nearest first).
    if (at < 0) return group == kRoomsGroup || delta > 0 ? numbers.front() : numbers.back();
    return numbers[((at + delta) % n + n) % n];
}

int SetFindQuery(const std::string& utf8) {
    g_lotAt = 0;
    std::string folded = util::FoldForSearch(utf8);
    int count = (int)FindMatches(CachedLot(), folded).size();
    if (count > 0) {
        g_findFolded = folded;
        g_findLabel = utf8;
    }
    return count;
}

void ClearFindQuery() {
    g_findFolded.clear();
    g_findLabel.clear();
}

int OpenNumber(Ptr group) { return group == kKindsGroup ? g_openKind : group == kRoomsGroup ? g_openRoom : 0; }

Ptr LastGroup() { return g_lastGroup; }

int FocusedId() { return g_focusId; }

bool OpenSame(Ptr group, int objectId) {
    auto merged = g_mergedIds.find(objectId);
    if (merged == g_mergedIds.end() || merged->second.size() < 2) return false;
    SameOpen open;
    open.ids = merged->second;
    open.entryId = objectId;
    for (const LotObject& o : CachedLot().objects)
        if (o.id == objectId) open.name = DisplayName(o);
    g_same[group] = open;
    g_lotAt = 0;
    return true;
}

int Close(Ptr group) {
    auto same = g_same.find(group);
    if (same != g_same.end()) {  // back to the kind or room, on the merged entry
        int entry = same->second.entryId;
        g_same.erase(same);
        g_lotAt = 0;
        return entry;
    }
    int* open = group == kKindsGroup ? &g_openKind : group == kRoomsGroup ? &g_openRoom : nullptr;
    if (!open || !*open) return 0;
    int was = *open;
    *open = 0;
    return was;
}

void Focus(int objectId) {
    g_focusId = objectId;
    for (const LotObject& o : CachedLot().objects)
        if (o.id == objectId) camera::LookAt(o.object);  // the player hears what the camera shows
}

bool HasFocus() { return FocusedObject() != 0; }

std::vector<std::string> DetailsOf(int objectId) {
    auto it = g_details.find(objectId);
    return it == g_details.end() ? std::vector<std::string>{} : it->second;
}

game::Ptr FocusedObject() {
    for (const LotObject& o : game::LotThings())
        if (o.id == g_focusId) return o.object;
    return 0;
}

namespace {

std::string JoinWithCommas(const std::vector<std::string>& items) {
    std::string out;
    for (const std::string& s : items) {
        if (!out.empty()) out += ", ";
        out += s;
    }
    return out;
}

}  // namespace

// The menu came up empty: the game gives no reason, so the mod reads the object's interaction
// table and says who its interactions are for, or that the game's own test refused them now.
std::string WhyNoInteractions(const LotObject& o) {
    std::vector<game::InteractionEntry> entries;
    game::Ptr sim = game::SelectedSim();
    if (!sim || !game::ObjectInteractions(o, &entries)) return S(Str::NoInteractions);
    if (entries.empty()) return S(Str::NoInteractionsAtAll);
    game::SimKind kind = game::KindOfSim(sim);
    std::vector<std::string> excluded, refused;
    for (const game::InteractionEntry& e : entries) {
        std::vector<std::string>& list = game::KindAllows(kind, e) ? refused : excluded;
        if (std::find(list.begin(), list.end(), e.name) == list.end()) list.push_back(e.name);
    }
    std::string line = S(Str::NoInteractions);
    if (!excluded.empty())
        line += ". " + util::Format(S(kind == game::SimKind::Child ? Str::OnlyForAdults : Str::OnlyForChildren),
                                    JoinWithCommas(excluded).c_str());
    if (!refused.empty())
        line += ". " + util::Format(S(Str::NotAvailableNow), JoinWithCommas(refused).c_str());
    Log("no interactions for #%d: %s", o.id, line.c_str());
    return line;
}

void Interact() {
    if (!OnLot()) return;
    for (const LotObject& o : game::LotThings()) {
        if (o.id != g_focusId) continue;
        // The menu opens at the centre of the game view; the game runs it modally in this call.
        // An object with nothing to offer right now makes no menu at all (the bed of a Sim who is
        // not tired): the call returns as if one had been closed.
        ULONGLONG before = GetTickCount64();
        if (!game::OpenInteractionMenu(o, 512, 384) || !PieMenuSeenSince(before)) speech::Say(WhyNoInteractions(o));
        return;
    }
    speech::Say(S(Str::NoObjects));  // the thing is gone
}

void WalkToFocus() {
    if (!OnLot()) return;
    for (const LotObject& o : game::LotThings()) {
        if (o.id != g_focusId) continue;
        ChooseInNextMenu(0);  // the floor's menu: "Ir a este punto"
        if (game::WalkTo(o.x, o.y, 512, 384)) speech::Say(S(Str::WalkingThere));
        else speech::Say(S(Str::CannotWalk));
        return;
    }
    speech::Say(S(Str::NoObjects));
}

void WalkToSpot(int worldX, int worldY) {
    if (!OnLot()) return;
    ChooseInNextMenu(0);  // the floor's menu: "Ir a este punto"
    if (game::WalkTo(worldX, worldY, 512, 384)) speech::Say(S(Str::WalkingThere));
    else speech::Say(S(Str::CannotWalk));
}

}  // namespace nav::world
