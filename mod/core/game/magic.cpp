#include "magic.h"

#include <algorithm>

#include "../util/text.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

constexpr int kCookbookFlag = 0x46a;  // byte (see IsCookbook)

Ptr DescriptionField(Ptr book) {
    for (Ptr c : Children(book))
        if (IsKindOf(c, "cTSWinTextEdit2")) return c;
    return 0;
}

}  // namespace

bool IsMagicBook(Ptr w) { return IsKindOf(w, "CWinMagicBook"); }
bool IsBookEntry(Ptr w) { return IsKindOf(w, "cPickSpellItem"); }

Ptr MagicBookOf(Ptr w) {
    for (int depth = 0; w && depth < 6; depth++, w = Parent(w))
        if (IsMagicBook(w)) return w;
    return 0;
}

bool IsCookbook(Ptr book) { return IsMagicBook(book) && Rd<uint8_t>(book + kCookbookFlag) != 0; }

std::vector<std::string> EntryIngredients(Ptr entry) {
    std::vector<Ptr> buttons;
    for (Ptr c : Children(entry))
        if (IsKindOf(c, "cTSWinBtn")) buttons.push_back(c);
    std::sort(buttons.begin(), buttons.end(), [](Ptr a, Ptr b) { return Rect(a).left < Rect(b).left; });
    std::vector<std::string> names;
    for (Ptr b : buttons) {
        std::string name = util::CleanForSpeech(Tooltip(b));
        if (!name.empty()) names.push_back(name);
    }
    return names;
}

// The title is the text standing right above the description, at the same left edge.
std::string ChosenEntryTitle(Ptr book) {
    Ptr field = DescriptionField(book);
    if (!field) return "";
    RECT f = Rect(field);
    for (Ptr c : Children(book))
        if (IsKindOf(c, "cTSWinText") && Rect(c).left == f.left && Rect(c).bottom <= f.top)
            return util::CleanForSpeech(Caption(c));
    return "";
}

std::string ChosenEntryText(Ptr book) {
    Ptr field = DescriptionField(book);
    return field ? util::CleanForSpeech(TextFieldText(field)) : "";
}

}  // namespace game
