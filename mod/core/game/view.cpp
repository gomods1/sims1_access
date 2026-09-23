#include "view.h"

#include "../util/log.h"
#include "addresses.h"
#include "hooks.h"
#include "people.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

Ptr PanelState() {
    Ptr app = Rd<Ptr>(Va(addr::kSimsApp));
    return app ? Rd<Ptr>(app + addr::catalog::kState) : 0;
}

}  // namespace

Ptr FamilyPanel(Ptr lotView) {
    if (!IsKindOf(lotView, "cDDDSimsView")) return 0;
    for (Ptr panel : Children(lotView)) {
        if (!IsKindOf(panel, "cWinCPanel")) continue;
        for (Ptr c : Children(panel))
            if (IsKindOf(c, "cWinPeople")) return c;
    }
    return 0;
}

bool CenterViewOn(Ptr object) {
    typedef void(__thiscall * Fn)(void*, char immediate);
    if (!IsKindOf(object, "cXObject") || !FunctionOk(addr::camera::kCenterOnObject)) return false;
    bool ok = Guarded([&] { ((Fn)Va(addr::camera::kCenterOnObject))((void*)object, 1); });
    if (!ok) Log("camera: centering on %p failed", (void*)object);
    return ok;
}

Ptr FollowedSim() { return Rd<Ptr>(Va(addr::camera::kFollowed)); }

bool FollowSim(Ptr person) {
    typedef void(__cdecl * Fn)(void*);
    if (!FunctionOk(addr::camera::kSetFollowed)) return false;
    if (person && !IsKindOf(person, "cXPerson")) return false;
    return Guarded([&] { ((Fn)Va(addr::camera::kSetFollowed))((void*)person); });
}

bool StopFollowing(Ptr lotView) {
    typedef void(__thiscall * Fn)(void*);
    Ptr people = FamilyPanel(lotView);
    if (people && FunctionOk(addr::camera::kPeopleStopFollow))
        return Guarded([&] { ((Fn)Va(addr::camera::kPeopleStopFollow))((void*)people); });
    return FollowSim(0);
}

int SelectedNeighborId() {
    typedef int(__cdecl * Fn)();
    if (!FunctionOk(addr::camera::kSelectedNeighborId)) return -1;
    int id = -1;
    Guarded([&] { id = ((Fn)Va(addr::camera::kSelectedNeighborId))(); });
    return id;
}

int SelectFamilyMember(Ptr lotView, int index, bool card) {
    Ptr people = FamilyPanel(lotView);
    if (!people) return -1;
    // The n-th shown portrait, pressed like a click: the panel selects the Sim, centers the view
    // on it when it was selected already, and the click reaches the tutorial as a UI event.
    std::vector<Ptr> buttons = FamilyMemberButtons(people);
    int slot = -1, shown = 0;
    for (int i = 0; i < (int)buttons.size() && slot < 0; i++)
        if (Visible(buttons[i]) && shown++ == index) slot = i;
    if (slot < 0 || !FunctionOk(addr::camera::kFamilyMemberId)) return -1;
    typedef int(__cdecl * MemberIdFn)(int index);
    int id = -1;
    if (!Guarded([&] { id = ((MemberIdFn)Va(addr::camera::kFamilyMemberId))(slot); })) return -1;
    if (card) ForceCtrlFor(400);  // Ctrl held: the panel opens the Sim's card (name and biography)
    else SuppressCtrlFor(400);  // the click is handled on a later frame
    Press(buttons[slot]);
    Unhover(buttons[slot]);
    Log("select family member %d (slot %d): neighbor %d, portrait pressed", index, slot, id);
    return id;
}

}  // namespace game
