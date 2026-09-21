#include "pie.h"

#include "../util/log.h"
#include "addresses.h"
#include "rtti.h"

namespace game {
namespace {

// Calls a cTSPieMenu virtual slot after checking it is the function this build is known to have.
template <class Fn, class... Args>
bool CallPieSlot(Ptr pie, int slot, Ptr expected, Args... args) {
    Ptr fn = Rd<Ptr>(Rd<Ptr>(pie) + slot * 4);
    if (fn != Va(expected) || !FunctionOk(expected)) {
        Log("pie menu: unexpected slot %d %p", slot, (void*)fn);
        return false;
    }
    return Guarded([&] { ((Fn)fn)((void*)pie, args...); });
}

}  // namespace

bool IsPieMenu(Ptr w) { return IsKindOf(w, "cTSPieMenu"); }

std::vector<std::string> PieItems(Ptr pie) {
    std::vector<std::string> labels;
    Ptr begin = Rd<Ptr>(pie + addr::pie::kItems);
    Ptr end = Rd<Ptr>(pie + addr::pie::kItems + 4);
    if (end < begin || (end - begin) / addr::pie::kItemSize > 64) return labels;
    for (Ptr it = begin; it < end; it += addr::pie::kItemSize) labels.push_back(ReadStdStringUtf8(it));
    return labels;
}

void PieHighlight(Ptr pie, int index) {
    typedef void(__thiscall * Fn)(void*, int, char);
    CallPieSlot<Fn>(pie, addr::kPieSetSelectionSlot, addr::kPieSetSelection, index, (char)0);
}

// Same path as a mouse release on the item: the menu notifies its owner (the family panel), which
// has the live mode picker queue the interaction (or open the sub-level) and close the menu.
void PieChoose(Ptr pie, int index) {
    typedef void(__thiscall * Fn)(void*, int, char);
    CallPieSlot<Fn>(pie, addr::kPieSetSelectionSlot, addr::kPieSetSelection, index, (char)1);
}

void PieEnd(Ptr pie, int result) {
    typedef void(__thiscall * Fn)(void*, int);
    CallPieSlot<Fn>(pie, addr::kPieEndModalSlot, addr::kPieEndModal, result);
}

bool PieIsSubMenu(Ptr pie) {
    Ptr tool = Rd<Ptr>(Va(addr::kCurrentTool));
    return IsKindOf(tool, "cObjPickerTool") && Rd<Ptr>(tool + addr::picker::kSubMenu) == pie;
}

void PieCancel(Ptr pie) {
    PieEnd(pie, PieIsSubMenu(pie) ? addr::pie::kResultLeave : addr::pie::kResultBack);
}

}  // namespace game
