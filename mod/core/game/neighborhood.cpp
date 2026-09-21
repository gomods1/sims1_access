#include "neighborhood.h"

#include "../util/log.h"
#include "addresses.h"
#include "hooks.h"
#include "rtti.h"

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

}  // namespace game
