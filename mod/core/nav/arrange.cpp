#include "arrange.h"

#include <algorithm>

namespace nav {
namespace {

int CenterY(const RECT& r) { return (r.top + r.bottom) / 2; }

// Reading order: rows by vertical centre (an item joins the row when its centre is within half
// the row starter's height), each row left to right.
void SortReadingOrder(std::vector<Item>& items) {
    std::stable_sort(items.begin(), items.end(),
                     [](const Item& a, const Item& b) { return CenterY(a.rect) < CenterY(b.rect); });
    size_t start = 0;
    while (start < items.size()) {
        int top = CenterY(items[start].rect);
        long tolerance = std::max(4L, (items[start].rect.bottom - items[start].rect.top) / 2);
        size_t end = start + 1;
        while (end < items.size() && CenterY(items[end].rect) - top <= tolerance) end++;
        std::stable_sort(items.begin() + start, items.begin() + end,
                         [](const Item& a, const Item& b) { return a.rect.left < b.rect.left; });
        start = end;
    }
}

bool SameRow(const RECT& a, const RECT& b) { return CenterY(a) >= b.top - 4 && CenterY(a) <= b.bottom + 4; }

// "< Alta >": a value text between two unlabeled arrow buttons on its row becomes one spinner,
// named by the label just above it ("Detalle del terreno"). Arrows and label are absorbed.
void MakeSpinners(Group& g) {
    auto unlabeledArrow = [](const Item& t) { return t.role == Role::Button && t.label.empty(); };
    for (Item& value : g.items) {
        if (value.role != Role::Text) continue;
        auto left = std::find_if(g.items.begin(), g.items.end(), [&](const Item& t) {
            return unlabeledArrow(t) && SameRow(t.rect, value.rect) && t.rect.right <= value.rect.left + 4 &&
                   t.rect.right >= value.rect.left - 120;
        });
        auto right = std::find_if(g.items.begin(), g.items.end(), [&](const Item& t) {
            return unlabeledArrow(t) && SameRow(t.rect, value.rect) && t.rect.left >= value.rect.right - 4 &&
                   t.rect.left <= value.rect.right + 120;
        });
        if (left == g.items.end() || right == g.items.end()) continue;
        value.role = Role::Spinner;
        value.previous = left->widget;
        value.next = right->widget;
        value.label.clear();
        auto label = std::find_if(g.items.begin(), g.items.end(), [&](const Item& t) {
            return (t.role == Role::Button || t.role == Role::Text) && !t.label.empty() &&
                   t.rect.bottom <= value.rect.top + 6 && t.rect.bottom >= value.rect.top - 30 &&
                   t.rect.right >= left->rect.left && t.rect.left <= right->rect.right;
        });
        Ptr absorbed[3] = {left->widget, right->widget, label != g.items.end() ? label->widget : 0};
        if (label != g.items.end()) value.label = label->label;
        g.items.erase(std::remove_if(g.items.begin(), g.items.end(),
                                     [&](const Item& t) {
                                         return t.widget == absorbed[0] || t.widget == absorbed[1] ||
                                                (absorbed[2] && t.widget == absorbed[2]);
                                     }),
                      g.items.end());
        return MakeSpinners(g);  // iterators are invalid now; look for the others afresh
    }
}

// A check box is named by the label button just right of it on its row ("Autocentrado"), which
// then reads as part of the box.
void LabelCheckBoxes(Group& g) {
    for (Item& box : g.items) {
        if (box.role != Role::CheckBox || !box.label.empty()) continue;
        auto label = std::find_if(g.items.begin(), g.items.end(), [&](const Item& t) {
            return t.role == Role::Button && !t.label.empty() && t.rect.left >= box.rect.right - 4 &&
                   t.rect.left <= box.rect.right + 24 && CenterY(t.rect) >= box.rect.top &&
                   CenterY(t.rect) <= box.rect.bottom;
        });
        if (label == g.items.end()) continue;
        box.label = label->label;
        g.items.erase(label);
        return LabelCheckBoxes(g);  // iterators are invalid now; label the others afresh
    }
}

// A text field or slider is named by the text or label button just left of it on its row
// ("Introduce apellido:", "Efectos"), which then reads as part of it instead of on its own.
void LabelTextFields(Group& g) {
    for (Item& field : g.items) {
        if ((field.role != Role::TextField && field.role != Role::Slider) || !field.label.empty()) continue;
        auto prompt = std::find_if(g.items.begin(), g.items.end(), [&](const Item& t) {
            return (t.role == Role::Text || (field.role == Role::Slider && t.role == Role::Button)) &&
                   !t.label.empty() && t.rect.right <= field.rect.left + 8 &&
                   CenterY(t.rect) >= field.rect.top - 4 && CenterY(t.rect) <= field.rect.bottom + 4;
        });
        if (prompt == g.items.end()) continue;
        field.label = prompt->label;
        g.items.erase(prompt);
        return LabelTextFields(g);  // iterators are invalid now; label the others afresh
    }
}

// Groups built from the game's own lists keep the game's order; the rest read like the screen.
void SortItems(Group& g) {
    auto byNumber = [](const Item& a, const Item& b) { return a.number < b.number; };
    if (g.key == kJobGroup)  // job info first, then the skills, each in the game's order
        std::stable_sort(g.items.begin(), g.items.end(), [&](const Item& a, const Item& b) {
            if ((a.role == Role::Skill) != (b.role == Role::Skill)) return b.role == Role::Skill;
            return byNumber(a, b);
        });
    else if (g.key == kHousesGroup || g.key == kStatusGroup || g.key == kSimPanelsGroup || g.key == kHouseGroup ||
             g.key == kLooksGroup)
        std::stable_sort(g.items.begin(), g.items.end(), byNumber);
    else
        SortReadingOrder(g.items);
    // The mood leads the group it joins (the needs): it sums up the rest.
    std::stable_partition(g.items.begin(), g.items.end(), [](const Item& it) { return it.role == Role::Mood; });
    std::stable_partition(g.items.begin(), g.items.end(), [](const Item& it) { return !IsPageArrow(it); });
}

}  // namespace

// By position they would land in the middle of the list they turn.
bool IsPageArrow(const Item& it) {
    return it.role == Role::Button && it.rect.right - it.rect.left <= 12 && it.rect.bottom - it.rect.top >= 40;
}

void ArrangeGroup(Group& g) {
    LabelTextFields(g);
    LabelCheckBoxes(g);
    MakeSpinners(g);
    SortItems(g);
}

}  // namespace nav
