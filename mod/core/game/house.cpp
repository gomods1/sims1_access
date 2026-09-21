#include "house.h"

#include <cstring>

#include "addresses.h"
#include "rtti.h"

namespace game {

// The panel's draw (0x4dc390) builds a HouseStats on its stack, has the sim root fill it, and
// asks it for each rating (0..100, drawn as rating / 10 bars).
bool HouseRatings(int out[kHouseRatingCount]) {
    typedef void(__thiscall * InitFn)(void* stats);
    typedef void(__thiscall * FillFn)(void* root, void* stats);
    typedef int(__thiscall * RatingFn)(void* stats);
    if (!FunctionOk(addr::house::kStatsInit) || !FunctionOk(addr::house::kStatsFill)) return false;
    for (addr::Va getter : addr::house::kRatings)
        if (!FunctionOk(getter)) return false;
    Ptr root = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)));
    if (!root || !Rd<Ptr>(root + addr::kLotModule)) return false;
    alignas(8) unsigned char stats[addr::house::kStatsSize * 2] = {};  // spare room, in case
    return Guarded([&] {
        ((InitFn)Va(addr::house::kStatsInit))(stats);
        ((FillFn)Va(addr::house::kStatsFill))((void*)root, stats);
        for (int i = 0; i < kHouseRatingCount; i++) {
            int rating = ((RatingFn)Va(addr::house::kRatings[i]))(stats);
            rating = rating < 0 ? 0 : rating > 100 ? 100 : rating;
            out[i] = rating / 10;
        }
    });
}

std::vector<Ptr> HousePanelButtons(Ptr housePanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(housePanel, "cWinSubpanelHouse")) return buttons;
    Ptr array = Rd<Ptr>(housePanel + addr::housepanel::kButtons);
    for (int i = 0; array && i < addr::housepanel::kButtonCount; i++) buttons.push_back(Rd<Ptr>(array + i * 4));
    return buttons;
}

}  // namespace game
