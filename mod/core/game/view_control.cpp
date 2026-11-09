#include "view_control.h"

#include <cctype>

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

bool IsModeButton(Ptr w) {
    Ptr vc = Parent(w);
    if (!w || !IsKindOf(vc, "cWinViewControl")) return false;
    for (int i = 0; i < addr::viewctl::kModeButtons; i++)
        if (w == ButtonAt(vc, addr::viewctl::kLiveModeButton + i * 4)) return true;
    return false;
}

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

bool BuildPanelShown(Ptr lotView) {
    for (Ptr panel : Children(lotView)) {
        if (!IsKindOf(panel, "cWinCPanel") || !Visible(panel)) continue;
        for (Ptr c : Children(panel))
            if (IsKindOf(c, "cWinArch") && Visible(c)) return true;
    }
    return false;
}

std::string ClockText(Ptr vc) {
    if (!IsKindOf(vc, "cWinViewControl")) return "";
    std::string text = ReadStdStringUtf8(vc + addr::viewctl::kClock);
    // The English game draws "AM" above the time, stored as "AM 10:00"; spoken that way it reads
    // backwards (a tester): "10:00 AM". Only a Latin prefix moves: 午前 / 오전 come first in their
    // languages.
    size_t letters = 0;
    while (letters < text.size() && (isalpha((unsigned char)text[letters]) || text[letters] == '.')) letters++;
    size_t digit = letters;
    while (digit < text.size() && (text[digit] == ' ' || text[digit] == '\n' || text[digit] == '\r')) digit++;
    if (letters > 0 && digit > letters && digit < text.size() && isdigit((unsigned char)text[digit]))
        text = text.substr(digit) + " " + text.substr(0, letters);
    return text;
}

Ptr FundsButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kFundsButton); }

Ptr FriendsButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kFriendsButton); }

Ptr PauseButton(Ptr vc) { return ButtonAt(vc, addr::viewctl::kPauseButton); }

Ptr FloorButton(Ptr vc, int level) {
    if (level < 1 || level > addr::viewctl::kFloors) return 0;
    return ButtonAt(vc, addr::viewctl::kFloorButtons + (level - 1) * 4);
}

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
