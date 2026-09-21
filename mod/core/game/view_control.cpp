#include "view_control.h"

#include "addresses.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

Ptr ButtonAt(Ptr vc, int offset) {
    if (!IsKindOf(vc, "cWinViewControl")) return 0;
    Ptr b = Rd<Ptr>(vc + offset);
    return IsKindOf(b, "cTSWinBtn") ? b : 0;
}

}  // namespace

Ptr FindViewControl(Ptr lotView) {
    for (Ptr c : Children(lotView))
        if (IsKindOf(c, "cWinViewControl")) return c;
    return 0;
}

Ptr LiveModeButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kLiveModeButton); }

bool ModePanelShown(Ptr lotView) {
    static const char* const kModePanels[] = {"cWinPeople", "cWinCatalog", "cWinArch", "cWinOptions", "cWinCamPanel"};
    for (Ptr panel : Children(lotView)) {
        if (!IsKindOf(panel, "cWinCPanel") || !Visible(panel)) continue;
        for (Ptr c : Children(panel))
            for (const char* cls : kModePanels)
                if (IsKindOf(c, cls) && Visible(c)) return true;
    }
    return false;
}

std::string ClockText(Ptr vc) {
    if (!IsKindOf(vc, "cWinViewControl")) return "";
    return ReadStdStringUtf8(vc + addr::viewctl::kClock);
}

Ptr FundsButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kFundsButton); }

Ptr FriendsButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kFriendsButton); }

Ptr PauseButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kPauseButton); }

Ptr SpeedButton(Ptr vc, int level) {
    if (level < 1 || level > addr::viewctl::kSpeedLevels) return 0;
    return ButtonAt(vc, addr::viewctl::kSpeedButtons + (level - 1) * 4);
}

GameSpeed ReadGameSpeed(Ptr vc) {
    GameSpeed s;
    Ptr pause = PauseButton(vc);
    s.paused = pause && ButtonChecked(pause);
    for (int level = 1; level <= addr::viewctl::kSpeedLevels; level++) {
        Ptr b = SpeedButton(vc, level);
        if (b && ButtonChecked(b)) s.level = level;
    }
    return s;
}

bool IsSpeedButton(Ptr w) {
    Ptr vc = Parent(w);
    if (!w || !IsKindOf(vc, "cWinViewControl")) return false;
    if (w == PauseButton(vc)) return true;
    for (int level = 1; level <= addr::viewctl::kSpeedLevels; level++)
        if (w == SpeedButton(vc, level)) return true;
    return false;
}

}  // namespace game
