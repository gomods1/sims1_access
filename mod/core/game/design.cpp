#include "design.h"

#include "addresses.h"
#include "rtti.h"
#include "ui.h"

namespace game {

bool CurrentLook(Ptr screen, LookPart part, Look* out) {
    if (!IsKindOf(screen, "cWinDesignCharacter")) return false;
    Ptr designer = Rd<Ptr>(Va(addr::kDesigner));
    Ptr list = designer ? Rd<Ptr>(designer + (part == LookPart::Head ? addr::designer::kHeads : addr::designer::kBodies)) : 0;
    if (!list) return false;
    Ptr begin = Rd<Ptr>(list), end = Rd<Ptr>(list + 4);
    if (!begin || end < begin || (end - begin) % addr::designer::kEntrySize) return false;
    out->count = (int)((end - begin) / addr::designer::kEntrySize);
    out->index = Rd<int>(screen + (part == LookPart::Head ? addr::designsim::kHeadIndex : addr::designsim::kBodyIndex), -1);
    if (out->index < 0 || out->index >= out->count) return false;
    char name[addr::designer::kMeshNameSize + 1] = {};
    Read(begin + out->index * addr::designer::kEntrySize + addr::designer::kMeshName, name, addr::designer::kMeshNameSize);
    out->mesh = name;
    size_t eq = out->mesh.rfind('=');
    if (eq != std::string::npos) out->mesh.erase(0, eq + 1);  // "HEAD-HEAD=", "BODY="
    return true;
}

bool IsLookArrow(Ptr screen, Ptr button, LookPart* part) {
    if (!button || !IsKindOf(screen, "cWinDesignCharacter")) return false;
    if (button == Rd<Ptr>(screen + addr::designsim::kPrevHead) || button == Rd<Ptr>(screen + addr::designsim::kNextHead)) {
        *part = LookPart::Head;
        return true;
    }
    if (button == Rd<Ptr>(screen + addr::designsim::kPrevBody) || button == Rd<Ptr>(screen + addr::designsim::kNextBody)) {
        *part = LookPart::Body;
        return true;
    }
    return false;
}

std::vector<Ptr> LookControls(Ptr screen) {
    std::vector<Ptr> controls;
    if (!IsKindOf(screen, "cWinDesignCharacter")) return controls;
    using namespace addr::designsim;
    for (int offset : {kChild, kAdult, kMale, kFemale, kSkinLight, kSkinMedium, kSkinDark, kPreview, kPrevHead,
                       kNextHead, kPrevBody, kNextBody})
        controls.push_back(Rd<Ptr>(screen + offset));
    return controls;
}

Ptr ZodiacButton(Ptr screen) {
    return IsKindOf(screen, "cWinDesignCharacter") ? Rd<Ptr>(screen + addr::designsim::kZodiac) : 0;
}

bool IsTraitBar(Ptr w) { return IsKindOf(w, "cWinPersonalityWidget"); }

std::string TraitBarName(Ptr bar) { return ReadStdStringUtf8(bar + addr::traitbar::kName); }

int TraitBarPoints(Ptr bar) { return Rd<int>(bar + addr::traitbar::kPoints); }

int PersonalityPointsLeft(Ptr bar) {
    Ptr screen = Rd<Ptr>(bar + addr::traitbar::kScreen);
    if (!IsKindOf(screen, "cWinDesignCharacter")) return -1;
    return Rd<int>(screen + addr::designsim::kPointsLeft, -1);
}

// The bar's mouse-move handler (slot 61) picks the direction from the pointer: right of the
// filled part adds, on it (or on the label to its left) removes. The press then applies it.
bool AdjustTraitBar(Ptr bar, int delta) {
    RECT r = Rect(bar);
    int x = delta > 0 ? r.right - r.left - 1 : 0;
    return PressAt(bar, x, (r.bottom - r.top) / 2);
}

}  // namespace game
