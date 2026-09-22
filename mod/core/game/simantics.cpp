#include "simantics.h"

#include <set>

#include "../util/log.h"
#include "addresses.h"
#include "buy.h"
#include "lot.h"
#include "rtti.h"

namespace game {
namespace {

namespace sa = addr::simantics;

std::vector<Balloon> g_balloons;
std::vector<TakenObject> g_taken;
// Sims a grave brought out (ghosts), by object id, on the lot g_ghostLot.
std::set<int> g_ghosts;
Ptr g_ghostLot = 0;
constexpr size_t kMaxQueued = 64;  // nobody took them (not in the lot view): keep the newest

template <class T>
void Queue(std::vector<T>& queue, T item) {
    if (queue.size() >= kMaxQueued) queue.erase(queue.begin());
    queue.push_back(std::move(item));
}

// The object with this id on the running object's lot, 0 when none (what the primitives do).
Ptr ObjectOnLot(Ptr runner, int id) {
    Ptr module = Rd<Ptr>(runner + sa::kModule);
    if (!module || id <= 0) return 0;
    Ptr begin = Rd<Ptr>(module + addr::world::kObjects);
    Ptr end = Rd<Ptr>(module + addr::world::kObjects + 4);
    if (end <= begin || (Ptr)id > (end - begin) / 4) return 0;
    return Rd<Ptr>(begin + (id - 1) * 4);
}

uint16_t ObjdField(Ptr object, int field) {
    Ptr objd = Rd<Ptr>(object + addr::obj::kObjd);
    return objd ? Rd<uint16_t>(objd + 2 * field) : 0;
}

bool IsFamily(int neighborId) {
    if (neighborId < 0) return false;
    for (int id : FamilyNeighborIds())
        if (id == neighborId) return true;
    return false;
}

std::string NameOf(Ptr object) {
    LotObject o;
    if (!GetObject(object, &o)) return "";
    std::string name = ObjectName(o);
    return name.empty() ? ObjectInternalName(o) : name;
}

}  // namespace

std::vector<Balloon> TakeBalloons() {
    std::vector<Balloon> out;
    out.swap(g_balloons);
    return out;
}

std::vector<TakenObject> TakeTakenObjects() {
    std::vector<TakenObject> out;
    out.swap(g_taken);
    return out;
}

bool BalloonShownOver(Ptr object, ShownBalloon* out) {
    if (!object) return false;
    Ptr begin = Rd<Ptr>(object + sa::kHeadlines);
    Ptr end = Rd<Ptr>(object + sa::kHeadlines + 4);
    if (!begin || end < begin + sa::kHeadlineSize || end - begin > 64 * sa::kHeadlineSize) return false;
    if (Rd<int>(begin + sa::kHeadlineLeft) == 0) return false;
    out->sprite = Rd<int>(begin + sa::kHeadlineSprite);
    out->picture = Rd<Ptr>(begin + sa::kHeadlinePicture);
    out->balloon = Rd<int>(begin + sa::kHeadlineBalloon);
    out->crossed = Rd<int>(begin + sa::kHeadlineCrossed);
    return true;
}

std::vector<int> FamilyNeighborIds() {
    typedef int(__cdecl * MemberIdFn)(int index);
    std::vector<int> ids;
    if (!FunctionOk(addr::camera::kFamilyMemberId)) return ids;
    MemberIdFn member = (MemberIdFn)Va(addr::camera::kFamilyMemberId);
    for (int i = 0; i < 16; i++) {
        int id = -1;
        if (!Guarded([&] { id = member(i); }) || id < 0) break;
        ids.push_back(id);
    }
    return ids;
}

void BeforeSetBalloon(Ptr runner, Ptr frame, const uint8_t* operand, BalloonCall* call) {
    *call = BalloonCall{};
    uint8_t op[8];
    if (!Read((Ptr)operand, op, sizeof op)) return;
    int16_t duration = (int16_t)(op[4] | op[5] << 8);
    if (duration == 0) return;  // clears the balloon
    int stackId = Rd<int16_t>(frame + sa::kFrameStackObject);
    Ptr owner = (op[0] & 1) ? ObjectOnLot(runner, stackId) : runner;
    if (!owner) return;
    int group = op[3], index = op[2];
    if (op[7] & 0x10) index += Rd<int16_t>(runner + sa::kIconOffset);
    if (group == 0) {  // old style: the exe's table 0x998d88 maps 0-13 to motive icons, 14-18 to topics
        if (index < 0 || index > 18) return;
        group = index < 14 ? balloon::kMotive : balloon::kConversation;
        if (index >= 14) index -= 14;
    }
    call->pictureNeighborId = -1;
    if (group == balloon::kPicture) {
        if (index == 0) {
            call->pictureObjectId = stackId;
        } else if (index == 1) {
            call->pictureNeighborId = stackId;
        } else if (index == 2) {
            int local = (int16_t)(op[0] | op[1] << 8) >> 1;
            if (local < 0 || local >= Rd<uint8_t>(frame + sa::kFrameLocalCount)) return;
            Ptr locals = frame + sa::kFrameArgs + 2 * Rd<uint8_t>(frame + sa::kFrameArgCount);
            call->pictureObjectId = Rd<int16_t>(locals + 2 * local);
        }
    }
    call->owner = owner;
    call->shownBefore = BalloonShownOver(owner, &call->before);
    call->group = group;
    call->index = index;
    call->kind = op[6];
    call->crossed = (op[7] & 0x02) != 0;
    call->tree = Rd<uint16_t>(frame + sa::kFrameTree);
    call->otherId = stackId;
    call->watch = true;
}

void AfterSetBalloon(const BalloonCall& call) {
    ShownBalloon now;
    if (!BalloonShownOver(call.owner, &now)) return;  // a balloon of higher priority is still up
    if (call.shownBefore && now == call.before) return;  // the same one again (its time restarts)
    Balloon b;
    b.ownerId = Rd<int16_t>(call.owner + addr::obj::kId);
    b.group = call.group;
    b.index = call.index;
    b.kind = call.kind <= 3 ? (BalloonKind)call.kind : BalloonKind::None;
    b.crossed = call.crossed;
    b.routeFailure = call.tree == sa::kRouteFailureTree;
    b.pictureObjectId = call.pictureObjectId;
    b.pictureNeighborId = call.pictureNeighborId;
    b.otherId = call.otherId;
    b.tree = call.tree;
    Queue(g_balloons, b);
}

void BeforePrimitive(Ptr runner, Ptr frame, const uint8_t* instruction) {
    // Remove Object Instance: an object taken off the lot by someone outside the family (the
    // burglar, the repo man). Only furniture from the catalog counts: food, trash and markers
    // come and go all the time. Named now, while the object still exists.
    int16_t which = Rd<int16_t>((Ptr)instruction + sa::kOperands);
    int id = which == 0 ? Rd<int16_t>(runner + addr::obj::kId)
                        : which == 1 ? Rd<int16_t>(frame + sa::kFrameStackObject) : 0;
    Ptr target = ObjectOnLot(runner, id);
    if (!target) return;
    if (IsKindOf(target, "cXPerson")) {  // a visitor taken off the lot (diagnostics: the burglar)
        if (Rd<Ptr>(runner + sa::kModule) == g_ghostLot) g_ghosts.erase(id);  // the id may be reused
        if (!IsFamily(SimNeighborId(target)))
            Log("Sim removed: %s (id %d) by %s (tree 0x%x)", NameOf(target).c_str(), id, NameOf(runner).c_str(),
                Rd<uint16_t>(frame + sa::kFrameTree));
        return;
    }
    if (ObjdField(target, addr::objd::kFunctionFlags) == 0) return;
    // Furniture a player can see: placed on the lot and named in the catalog. Hidden helpers with
    // category flags ("Social - Dimishing Results - Talk Gossip 1.4") remove themselves all the time.
    if ((Rd<uint32_t>(target + addr::obj::kPlacement) & 0xffff) == 0xffff) return;
    LotObject o;
    if (!GetObject(target, &o)) return;
    std::string name = ObjectName(o);
    if (name.empty()) return;
    bool byPerson = IsKindOf(runner, "cXPerson");
    if (byPerson && IsFamily(SimNeighborId(runner))) return;
    TakenObject t;
    t.object = name;
    t.value = SaleValue(target);
    if (byPerson) t.taker = NameOf(runner);
    Log("object taken: %s (id %d, $%d) by %s (id %d, tree 0x%x)", t.object.c_str(), id, t.value,
        byPerson ? t.taker.c_str() : NameOf(runner).c_str(), Rd<int16_t>(runner + addr::obj::kId),
        Rd<uint16_t>(frame + sa::kFrameTree));
    Queue(g_taken, std::move(t));
}

void AfterCreateObject(Ptr runner, Ptr frame) {
    Ptr created = ObjectOnLot(runner, Rd<int16_t>(frame + sa::kFrameStackObject));
    if (!IsKindOf(created, "cXPerson")) return;
    LotObject o;
    if (!GetObject(created, &o)) return;
    LotObject creator;
    if (GetObject(runner, &creator) && ObjectInternalName(creator).find(sa::kGraveNamePart) != std::string::npos) {
        Ptr lot = Rd<Ptr>(runner + sa::kModule);
        if (lot != g_ghostLot) {
            g_ghosts.clear();
            g_ghostLot = lot;
        }
        g_ghosts.insert(o.id);
    }
    Log("Sim created: %s [%s] (id %d, placement %08x) by %s (tree 0x%x)", NameOf(created).c_str(),
        ObjectInternalName(o).c_str(), o.id, Rd<uint32_t>(created + addr::obj::kPlacement), NameOf(runner).c_str(),
        Rd<uint16_t>(frame + sa::kFrameTree));
}

bool IsGhost(Ptr person) {
    LotObject o;
    if (!GetObject(person, &o) || Rd<Ptr>(person + sa::kModule) != g_ghostLot) return false;
    return g_ghosts.count(o.id) != 0;
}

}  // namespace game
