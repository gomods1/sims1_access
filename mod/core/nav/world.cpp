#include "world.h"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/people.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "camera.h"
#include "narrator.h"
#include "navigator.h"
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
bool IsOther(const LotObject& o) { return !o.person && o.functionFlags == 0; }

// Sims first, then the buy mode categories in catalog order, then everything without one
// (doors, windows, trees, mailbox...). An object with several functions is in each of them.
const Category kCategories[] = {
    {Str::CategorySims, IsSim},
    {Str::CategorySeating, HasFunction<game::kFnSeating>},
    {Str::CategorySurfaces, HasFunction<game::kFnSurfaces>},
    {Str::CategoryAppliances, HasFunction<game::kFnAppliances>},
    {Str::CategoryElectronics, HasFunction<game::kFnElectronics>},
    {Str::CategoryPlumbing, HasFunction<game::kFnPlumbing>},
    {Str::CategoryLighting, HasFunction<game::kFnLighting>},
    {Str::CategoryDecorative, HasFunction<game::kFnDecorative>},
    {Str::CategoryMiscellaneous, HasFunction<game::kFnMiscellaneous>},
    {Str::CategoryOther, IsOther},
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
};
const KnownObject kTreeSpecies[] = {
    {"Pine", Str::ObjPineTree},     {"Birch", Str::ObjBirchTree},   {"Sycamore", Str::ObjSycamoreTree},
    {"Oak", Str::ObjOakTree},       {"Maple", Str::ObjMapleTree},   {"Palm", Str::ObjPalmTree},
    {"Willow", Str::ObjWillowTree}, {"Apple", Str::ObjAppleTree},   {"Cherry", Str::ObjCherryTree},
    {"Elm", Str::ObjElmTree},       {"Cypress", Str::ObjCypressTree},
};

// What PageUp/PageDown walk through: a buy mode category or a room.
enum class Mode { Category, Room };

Mode g_mode = Mode::Category;
int g_category = 0;
int g_room = game::kNoRoom;  // room id in room mode
int g_focusId = -1;          // object id of the focused object
int g_boughtId = -1;         // object id of the last purchase ("recién comprado")

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
    lot.objects = game::LotObjects();
    lot.rooms = rooms::NameRooms(lot.objects);
    lot.hasRef = game::GetObject(game::SelectedSim(), &lot.ref);
    return lot;
}

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

std::vector<Entry> CurrentList(const Lot& lot) {
    return Collapse(g_mode == Mode::Room ? InRoom(lot, g_room) : InCategory(lot, g_category));
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

// "Nevera Porcina, cocina, a 21 casillas al sur"; "Silla Parisienne, 6 iguales, comedor, a 11
// casillas al suroeste" (the nearest one). The room is left out in room mode, where the player
// already knows it.
std::string Describe(const Entry& e, const Lot& lot) {
    const LotObject& o = e.nearest;
    std::string s = e.name;
    if (e.ids.size() > 1) s += ", " + util::Format(S(Str::SameObjects), (int)e.ids.size());
    if (o.id == g_boughtId) s += std::string(", ") + S(Str::JustBought);
    const LotObject* ref = lot.Ref();
    if (ref && o.object == ref->object) s += std::string(", ") + S(Str::SelectedSimSuffix);
    if (o.person) {  // what the Sim is doing: its current action
        std::vector<std::string> actions = game::SimActions(game::SimNeighborId(o.object));
        if (!actions.empty()) s += ", " + actions.front();
    }
    std::string balloon = narrator::ShownBalloon(o.object);  // "piensa: hambre", flies over the trash
    if (!balloon.empty()) s += ", " + balloon;
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

// The details buffer's lines for an entry: its name, what a Sim is doing and has queued, where it
// is, its buy mode categories and what selling it would pay.
std::vector<std::string> Details(const Entry& e, const Lot& lot) {
    const LotObject& o = e.nearest;
    std::vector<std::string> lines;
    lines.push_back(e.ids.size() > 1 ? e.name + ", " + util::Format(S(Str::SameObjects), (int)e.ids.size()) : e.name);
    if (o.person) {
        std::vector<std::string> actions = game::SimActions(game::SimNeighborId(o.object));
        for (size_t i = 0; i < actions.size(); i++)
            lines.push_back(i == 0 ? actions[i] : util::Format(S(Str::QueuedAction), actions[i].c_str()));
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

// Speaks an entry and puts its details in the buffer.
void SayEntry(const std::string& line, const Entry& e, const Lot& lot) {
    buffers::SetDetails(Details(e, lot));
    camera::LookAt(e.nearest.object);  // the player hears what the camera shows
    speech::Say(line);
}

// "cocina, 12 objetos, 37 casillas"; the outdoors without its size (the whole lot).
std::string RoomSummary(const rooms::NamedRoom& room, size_t count, bool outside) {
    std::string s = count == 1 ? util::Format(S(Str::RoomObjectsOne), room.name.c_str())
                               : util::Format(S(Str::RoomObjects), room.name.c_str(), (int)count);
    if (!outside) s += ", " + util::Format(S(Str::RoomTiles), room.tiles);
    return s;
}

// Index of the entry holding the focused object, or -1.
int FocusIndex(const std::vector<Entry>& list) {
    for (int i = 0; i < (int)list.size(); i++)
        if (std::find(list[i].ids.begin(), list[i].ids.end(), g_focusId) != list[i].ids.end()) return i;
    return -1;
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
    std::string name = game::ObjectName(o);
    if (!name.empty()) return name;
    std::string internal = game::ObjectInternalName(o);
    for (const KnownObject& k : kKnownObjects)
        if (internal.rfind(k.word, 0) == 0) return S(k.name);
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

void Reset() {
    g_boughtId = -1;
    g_mode = Mode::Category;
    g_category = 0;
    g_room = game::kNoRoom;
    g_focusId = -1;
}

void NextCategory(int delta) {
    if (!OnLot()) return;
    Lot lot = ReadLot();
    int from = g_mode == Mode::Category ? g_category : (delta > 0 ? kCategoryCount - 1 : 0);
    g_mode = Mode::Category;
    for (int step = 1; step <= kCategoryCount; step++) {
        int c = ((from + delta * step) % kCategoryCount + kCategoryCount) % kCategoryCount;
        std::vector<Entry> list = Collapse(InCategory(lot, c));
        if (list.empty()) continue;
        g_category = c;
        g_focusId = list.front().nearest.id;
        SayEntry(Summary(S(kCategories[c].name), list.size()) + ". " + Describe(list.front(), lot), list.front(), lot);
        return;
    }
    speech::Say(S(Str::NoObjects));
}

void NextRoom(int delta) {
    if (!OnLot()) return;
    Lot lot = ReadLot();
    std::vector<const rooms::NamedRoom*> order = RoomsByDistance(lot);
    if (order.empty()) {
        speech::Say(S(Str::NoObjects));
        return;
    }
    int current = -1;
    for (int i = 0; g_mode == Mode::Room && i < (int)order.size(); i++)
        if (order[i]->id == g_room) current = i;
    int n = (int)order.size();
    int next = current < 0 ? 0 : ((current + delta) % n + n) % n;  // the first press: the Sim's room
    const rooms::NamedRoom& room = *order[next];
    g_mode = Mode::Room;
    g_room = room.id;
    std::vector<LotObject> objects = InRoom(lot, room.id);
    std::vector<Entry> list = Collapse(objects);
    game::Room info;
    bool outside = game::GetRoom(room.id, &info) && info.outside;
    g_focusId = list.empty() ? -1 : list.front().nearest.id;
    std::string line = RoomSummary(room, objects.size(), outside);
    if (!list.empty()) {
        SayEntry(line + ". " + Describe(list.front(), lot), list.front(), lot);
        return;
    }
    buffers::ClearDetails();
    speech::Say(line);
}

void NextObject(int delta) {
    if (!OnLot()) return;
    Lot lot = ReadLot();
    std::vector<Entry> list = CurrentList(lot);
    if (list.empty()) {  // the category or room emptied: move on to one that has objects
        if (g_mode == Mode::Room) NextRoom(+1);
        else NextCategory(+1);
        return;
    }
    int index = FocusIndex(list);
    index = index < 0 ? 0 : std::clamp(index + delta, 0, (int)list.size() - 1);  // ends repeat
    g_focusId = list[index].nearest.id;  // Enter acts on the nearest of a merged entry
    SayEntry(Describe(list[index], lot), list[index], lot);
}

void Repeat() { NextObject(0); }

game::Ptr FocusedObject() {
    for (const LotObject& o : game::LotObjects())
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
    for (const LotObject& o : game::LotObjects()) {
        if (o.id != g_focusId) continue;
        // The menu opens at the centre of the game view; the game runs it modally in this call.
        // An object with nothing to offer right now makes no menu at all (the bed of a Sim who is
        // not tired): the call returns as if one had been closed.
        ULONGLONG before = GetTickCount64();
        if (!game::OpenInteractionMenu(o, 512, 384) || !PieMenuSeenSince(before)) speech::Say(WhyNoInteractions(o));
        return;
    }
    NextObject(0);  // the focused object is gone: say what is focused now
}

void WalkToFocus() {
    if (!OnLot()) return;
    for (const LotObject& o : game::LotObjects()) {
        if (o.id != g_focusId) continue;
        ChooseInNextMenu(0);  // the floor's menu: "Ir a este punto"
        if (game::WalkTo(o.x, o.y, 512, 384)) speech::Say(S(Str::WalkingThere));
        else speech::Say(S(Str::CannotWalk));
        return;
    }
    NextObject(0);
}

}  // namespace nav::world
