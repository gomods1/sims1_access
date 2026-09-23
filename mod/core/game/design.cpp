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

namespace {
// The dialogs that browse a list of bodies on a preview: offsets of their preview and arrows.
struct PreviewDialog {
    const char* cls;
    int preview, prev, next;
};
const PreviewDialog kPreviewDialogs[] = {
    {"cWinClothingChoiceDlg", addr::clothing::kPreview, addr::clothing::kPrev, addr::clothing::kNext},
    {"cWinPetSelectDlg", addr::clothing::kPetPreview, addr::clothing::kPetPrev, addr::clothing::kPetNext},
};
const PreviewDialog* PreviewDialogOf(Ptr dialog) {
    for (const PreviewDialog& d : kPreviewDialogs)
        if (IsKindOf(dialog, d.cls)) return &d;
    return nullptr;
}
}  // namespace

Ptr OutfitPreview(Ptr dialog) {
    const PreviewDialog* d = PreviewDialogOf(dialog);
    return d ? Rd<Ptr>(dialog + d->preview) : 0;
}

bool IsOutfitArrow(Ptr dialog, Ptr button) {
    const PreviewDialog* d = PreviewDialogOf(dialog);
    return button && d && (button == Rd<Ptr>(dialog + d->prev) || button == Rd<Ptr>(dialog + d->next));
}

bool IsPetDialog(Ptr dialog) { return IsKindOf(dialog, "cWinPetSelectDlg"); }

bool CurrentOutfit(Ptr dialog, Look* out) {
    typedef int(__fastcall * IndexFn)(void* preview);
    Ptr preview = OutfitPreview(dialog);
    if (!preview || !FunctionOk(addr::clothing::kCurrentIndex)) return false;
    Ptr selector = Rd<Ptr>(preview + addr::clothing::kSelector);
    if (!selector) return false;
    int listOffset;
    switch (Rd<int>(preview + addr::clothing::kKind)) {
    case 0: listOffset = 0x7c; break;
    case 2: listOffset = 0x84; break;
    case 4: listOffset = 0x80; break;
    case 5: listOffset = 0x88; break;
    case 0x11: listOffset = 0x8c; break;
    case 0x12: listOffset = 0x90; break;
    default: return false;
    }
    // The game's lookup builds the kind's list on first use, so it runs first.
    int index = -1;
    if (!Guarded([&] { index = ((IndexFn)Va(addr::clothing::kCurrentIndex))((void*)preview); })) return false;
    Ptr list = Rd<Ptr>(selector + listOffset);
    if (!list) return false;
    Ptr begin = Rd<Ptr>(list), end = Rd<Ptr>(list + 4);
    if (!begin || end < begin || (end - begin) % addr::designer::kEntrySize) return false;
    out->count = (int)((end - begin) / addr::designer::kEntrySize);
    out->index = index;  // -1: the Sim wears an outfit the list does not hold (before the first arrow)
    out->mesh.clear();
    if (index < 0 || index >= out->count) return index == -1;
    char name[addr::designer::kMeshNameSize + 1] = {};
    Read(begin + index * addr::designer::kEntrySize + addr::designer::kMeshName, name, addr::designer::kMeshNameSize);
    out->mesh = name;
    size_t eq = out->mesh.rfind('=');
    if (eq != std::string::npos) out->mesh.erase(0, eq + 1);
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
