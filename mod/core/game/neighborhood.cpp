#include "neighborhood.h"

#include <algorithm>

#include "../util/log.h"
#include "addresses.h"
#include "hooks.h"
#include "rtti.h"
#include "ui.h"

namespace game {

int LotNumber(Ptr lot) { return Rd<int>(lot + addr::lot::kNumber, -1); }

std::string LotFamily(Ptr lot) { return ReadStdStringUtf8(lot + addr::lot::kFamily); }

// Refills the shared card for this lot (force = true, the game skips refills of the lot it last
// showed) and captures the text block it lays out. The card is not shown by this.
std::string LotCardText(Ptr lot) {
    typedef void(__thiscall* FillFn)(void* popup, void* record, void* extra, char force);
    Ptr popup = Rd<Ptr>(Va(addr::kLotPopup));
    if (!TextCaptureAvailable() || !FunctionOk(addr::kLotPopupFill) || !IsKindOf(popup, "cWinLotPopup")) return "";
    auto fill = (FillFn)Va(addr::kLotPopupFill);
    BeginTextCapture();
    bool ok = Guarded([&] { fill((void*)popup, (void*)(lot + addr::lot::kRecord), (void*)(lot + addr::lot::kExtra), 1); });
    std::string text = EndTextCapture();
    if (!ok) Log("lot card: popup fill crashed for %p", (void*)lot);
    return ok ? text : "";
}

int LotKind(int number) {
    Ptr root = Rd<Ptr>(Va(addr::kSimRoot));
    Ptr hood = root ? Rd<Ptr>(root + addr::travelmap::kNeighborhood) : 0;
    if (!hood) return 0;
    Ptr begin = Rd<Ptr>(hood + addr::travelmap::kLotKinds), end = Rd<Ptr>(hood + addr::travelmap::kLotKinds + 4);
    if (!begin || end < begin || end - begin > 8 * 1000) return 0;
    for (Ptr e = begin; e < end; e += 8)
        if (Rd<int>(e) == number) return Rd<int>(e + 4);
    return 0;
}

Ptr TravelMap(Ptr widget) {
    for (int depth = 0; widget && depth < 12; depth++, widget = Parent(widget))
        // The home neighborhood is the same class: its filter bar is off (the filter keys 1..7
        // work only while +0x185 is 0, 0x4a9c80).
        if (IsKindOf(widget, "cWinNeighborhoodUL")) return Rd<uint8_t>(widget + addr::travelmap::kFiltersOff) ? 0 : widget;
        else if (IsKindOf(widget, "cWinStudiotown") || IsKindOf(widget, "cWinMagicland")) return widget;
    return 0;
}

bool IsStudioTown(Ptr map) { return IsKindOf(map, "cWinStudiotown"); }

// The bar is a button holding one row of tooltip buttons (seven in Old Town, each twice).
std::vector<MapFilter> MapFilters(Ptr map) {
    std::vector<MapFilter> out;
    if (!map) return out;
    if (IsStudioTown(map)) {  // its own array of nine, with the bits 0x4d6540 gives them
        for (int i = 0; i < addr::travelmap::kStudioFilterCount; i++)
            if (Ptr b = Rd<Ptr>(map + addr::travelmap::kStudioFilters + i * 4)) out.push_back({b, addr::travelmap::kStudioBits[i]});
        return out;
    }
    if (IsKindOf(map, "cWinMagicland")) {
        for (int i = 0; i < addr::travelmap::kMagicFilterCount; i++)
            if (Ptr b = Rd<Ptr>(map + addr::travelmap::kMagicFilters + i * 4)) out.push_back({b, addr::travelmap::kMagicBits[i]});
        return out;
    }
    std::vector<Ptr> stack = {map};
    Ptr bar = 0;
    for (int n = 0; !stack.empty() && n < 500 && !bar; n++) {
        Ptr w = stack.back();
        stack.pop_back();
        int tipped = 0;
        for (Ptr c : Children(w)) {
            if (IsKindOf(c, "cTSWinBtn") && !IsKindOf(c, "cWinLotBtn") && !Tooltip(c).empty()) tipped++;
            if (!IsKindOf(c, "cWinLotBtn")) stack.push_back(c);
        }
        if (tipped >= 4) bar = w;
    }
    if (!bar) return out;
    std::vector<Ptr> buttons;
    for (Ptr c : Children(bar)) {
        if (!IsKindOf(c, "cTSWinBtn") || Tooltip(c).empty()) continue;
        bool twin = false;
        for (Ptr b : buttons) twin = twin || Rect(b).left == Rect(c).left;
        if (!twin) buttons.push_back(c);
    }
    std::stable_sort(buttons.begin(), buttons.end(), [](Ptr a, Ptr b) { return Rect(a).left < Rect(b).left; });
    constexpr int kCount = sizeof(addr::travelmap::kOldTownBits) / sizeof(int);
    for (int i = 0; i < (int)buttons.size() && i < kCount; i++) out.push_back({buttons[i], addr::travelmap::kOldTownBits[i]});
    return out;
}

bool IsCommunityLot(int number) { return LotKind(number) == addr::travelmap::kKindCommunity; }

bool MapLotVisitable(Ptr map, int number) { return IsStudioTown(map) || IsCommunityLot(number); }

int MapFilterChosen(Ptr map) {
    if (!map) return 0;
    int field = IsStudioTown(map) ? addr::travelmap::kStudioFilter
              : IsKindOf(map, "cWinMagicland") ? addr::travelmap::kMagicFilter : addr::travelmap::kFilter;
    return Rd<int>(map + field);
}

int LotFilterBits(Ptr lot) { return Rd<int>(lot + addr::lot::kFilterBits); }

}  // namespace game
