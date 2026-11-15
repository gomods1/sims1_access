#include "magic.h"

#include <algorithm>

#include "../util/text.h"
#include "addresses.h"
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
    std::vector<std::string> names;
    for (const Ingredient& i : EntryIngredientList(entry)) names.push_back(i.name);
    return names;
}

std::vector<Ingredient> EntryIngredientList(Ptr entry) {
    // Button i shows GUID i (the setter 0x4436f0); the buttons are then sorted as drawn.
    std::vector<std::pair<Ptr, uint32_t>> buttons;
    Ptr begin = Rd<Ptr>(entry + addr::spellitem::kGuids), end = Rd<Ptr>(entry + addr::spellitem::kGuids + 4);
    bool guids = begin && end - begin == addr::spellitem::kButtonCount * 4;
    for (int i = 0; i < addr::spellitem::kButtonCount; i++) {
        Ptr b = Rd<Ptr>(entry + addr::spellitem::kButtons + i * 4);
        if (b && IsKindOf(b, "cTSWinBtn")) buttons.push_back({b, guids ? Rd<uint32_t>(begin + i * 4) : 0});
    }
    if (buttons.empty())  // not laid out as the setter does: the child buttons, names only
        for (Ptr c : Children(entry))
            if (IsKindOf(c, "cTSWinBtn")) buttons.push_back({c, 0});
    std::sort(buttons.begin(), buttons.end(),
              [](const auto& a, const auto& b) { return Rect(a.first).left < Rect(b.first).left; });
    std::vector<Ingredient> list;
    for (const auto& [b, guid] : buttons) {
        std::string name = util::CleanForSpeech(Tooltip(b));
        if (!name.empty()) list.push_back({name, guid});
    }
    return list;
}

int InventoryCount(int neighborId, uint32_t guid) {
    if (neighborId < 0 || !guid) return 0;
    Ptr root = Rd<Ptr>(Va(addr::kSimRoot));
    Ptr hood = root ? Rd<Ptr>(root + addr::inventory::kNeighborhood) : 0;
    if (!hood) return 0;
    Ptr begin = Rd<Ptr>(hood + addr::inventory::kList), end = Rd<Ptr>(hood + addr::inventory::kList + 4);
    if (!begin || end < begin || end - begin > 4 * 4000) return 0;
    for (Ptr p = begin; p < end; p += 4) {
        Ptr inv = Rd<Ptr>(p);
        if (!inv || Rd<int16_t>(inv) != neighborId) continue;
        Ptr r = Rd<Ptr>(inv + addr::inventory::kRecords), rEnd = Rd<Ptr>(inv + addr::inventory::kRecords + 4);
        if (!r || rEnd < r || rEnd - r > addr::inventory::kRecordSize * 1000) return 0;
        int count = 0;
        for (; r < rEnd; r += addr::inventory::kRecordSize)
            if (Rd<uint32_t>(r + addr::inventory::kRecordGuid) == guid)
                count += Rd<int16_t>(r + addr::inventory::kRecordCount);
        return count;
    }
    return 0;
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
