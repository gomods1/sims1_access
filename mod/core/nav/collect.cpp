#include "collect.h"

#include <algorithm>
#include <map>
#include <set>

#include "../game/addresses.h"
#include "../game/buy.h"
#include "../game/cheats.h"
#include "../game/design.h"
#include "../game/house.h"
#include "../game/lot.h"
#include "../game/neighborhood.h"
#include "../game/people.h"
#include "../game/pie.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "arrange.h"
#include "screens.h"

namespace nav {
namespace {

constexpr int kMaxDepth = 24;

// A button that shows one page of its panel: the lot view's modes, a mode panel's own toggles
// (options pages, catalog categories, build tools) and a live mode subpanel's filters
// (inventory, relationships). Push buttons (save, back, page arrows) only act.
bool IsTab(Ptr w) {
    using game::IsKindOf;
    if (game::GetButtonType(w) == game::ButtonType::Push) return false;
    Ptr parent = game::Parent(w);
    if (IsKindOf(parent, "cWinViewControl")) return game::IsModeButton(w);
    if (IsKindOf(parent, "cWinOptions") || IsKindOf(parent, "cWinCatalog") || IsKindOf(parent, "cWinArch"))
        return true;
    return game::ClassName(parent) == "cTSWin" && IsKindOf(game::Parent(parent), "cWinPeople");
}

// A button whose meaning its panel defines (a skill of the job panel, a family portrait...).
struct Registered {
    Role role;
    Ptr group;   // 0: the group Role implies (GroupKey)
    int number;
};

struct Walker {
    Screen& screen;
    std::vector<Item> items;
    std::set<Ptr> claimed;               // widgets already represented by a status item
    std::map<Ptr, Registered> registered;
    Ptr needsBox = 0;                    // the family panel's needs container, when shown
    bool moodAdded = false;              // the mood meter has two halves; one item reads it

    void Add(Ptr w, Role role, std::string label = "", int number = 0, Ptr group = 0) {
        Item it;
        it.widget = w;
        it.role = role;
        it.label = std::move(label);
        it.rect = game::Rect(w);
        it.group = group;
        it.number = role == Role::Lot ? game::LotNumber(w) : number;
        items.push_back(std::move(it));
    }

    // Panels are visited before their children, so they register their buttons first.
    void Register(const std::vector<Ptr>& buttons, Role role, Ptr group, int firstNumber = 0) {
        for (int i = 0; i < (int)buttons.size(); i++) registered[buttons[i]] = Registered{role, group, firstNumber + i};
    }

    // The lot view's status line, from the control panel: time, funds, speed, friends. Funds and
    // friends are shown on buttons that are only listed here; pause also stays a control.
    void AddStatus(Ptr vc) {
        if (!game::ClockText(vc).empty()) Add(vc, Role::Status, "", (int)StatusField::Clock);
        auto add = [&](Ptr w, StatusField field, bool claim) {
            if (!w || !game::Visible(w)) return;
            Add(w, Role::Status, game::Tooltip(w), (int)field);
            if (claim) claimed.insert(w);
        };
        add(game::FundsButton(vc), StatusField::Funds, true);
        add(game::PauseButton(vc), StatusField::Speed, false);
        add(game::FriendsButton(vc), StatusField::Friends, true);
    }

    // Create a Sim: the appearance controls in a group of their own, in the game's order (the
    // screen's rows mix them with the personality bars); the zodiac sign joins the bars.
    void RegisterDesignScreen(Ptr screen) {
        std::vector<Ptr> looks = game::LookControls(screen);
        for (int i = 0; i < (int)looks.size(); i++) {
            Role role = game::IsKindOf(looks[i], "cWinVitaBtn") ? Role::SimPreview : Role::Button;
            registered[looks[i]] = Registered{role, kLooksGroup, i};
        }
        registered[game::ZodiacButton(screen)] = Registered{Role::Button, kPersonalityGroup, 0};
    }

    // Screens (input roots) whose buttons mean more than their labels say.
    void RegisterScreen(Ptr root) {
        if (game::IsKindOf(root, "cWinDesignCharacter")) RegisterDesignScreen(root);
        // The wardrobe's clothing choice and the pet picker: the unlabeled preview says what it shows.
        if (Ptr preview = game::OutfitPreview(root)) registered[preview] = Registered{Role::SimPreview, 0, 0};
    }

    // The live mode family panel and its subpanels. true when w was handled here.
    bool VisitFamilyPanel(Ptr w, bool* descend) {
        using game::IsKindOf;
        if (IsKindOf(w, "cWinPeople")) {
            Register(game::FamilyMemberButtons(w), Role::FamilyMember, 0);
            Register(game::SubpanelButtons(w), Role::Tab, kSimPanelsGroup, 1);  // the mood is 0
            Ptr box = game::NeedsBox(w);
            needsBox = box && game::Visible(box) ? box : 0;
            return true;
        }
        if (IsKindOf(w, "cWinSubpanelJob")) {
            Register(game::JobPanelInfoButtons(w), Role::JobInfo, kJobGroup);
            Register(game::JobPanelSkillButtons(w), Role::Skill, kJobGroup);
            return true;
        }
        if (IsKindOf(w, "cWinSubpanelDogSkills") || IsKindOf(w, "cWinSubpanelCatSkills")) {
            Register(game::PetSkillButtons(w), Role::Skill, 0, game::kPetSkillSlot);
            return true;
        }
        if (IsKindOf(w, "cWinSubpanelHouse")) {
            std::vector<Ptr> buttons = game::HousePanelButtons(w);
            size_t ratings = std::min(buttons.size(), (size_t)game::kHouseRatingCount);
            Register({buttons.begin(), buttons.begin() + ratings}, Role::HouseRating, kHouseGroup);
            Register({buttons.begin() + ratings, buttons.end()}, Role::Info, kHouseGroup, (int)ratings);
            return true;
        }
        if (game::IsMoodMeter(w)) {
            // Leads the needs while they are shown, else the Sim panels.
            if (!moodAdded) Add(w, Role::Mood, game::Tooltip(w), 0, needsBox ? needsBox : kSimPanelsGroup);
            moodAdded = true;
            return true;
        }
        if (IsKindOf(w, "cWinPersonality")) { Add(w, Role::Trait, game::Caption(w)); *descend = false; return true; }
        if (IsKindOf(w, "cWinInterest")) { Add(w, Role::Interest, game::Tooltip(w)); *descend = false; return true; }
        if (IsKindOf(w, "cWinRelationship")) { Add(w, Role::Relationship, game::Tooltip(w)); *descend = false; return true; }
        return false;
    }

    // Registered buttons read their tooltip, else their caption. Text lines a panel has not drawn
    // yet (the game is minimized) have neither and are left out.
    void AddRegistered(Ptr w, const Registered& r) {
        std::string label = game::Tooltip(w);
        if (label.empty()) label = game::Caption(w);
        if (r.role == Role::Info && label.empty()) return;
        Add(w, r.role, label, r.number, r.group);
    }

    // Classifies one visible widget; returns false when its children must not be walked (they are
    // part of this item's description).
    bool Visit(Ptr w, bool hasChildren) {
        using game::IsKindOf;
        if (claimed.count(w)) return true;
        auto reg = registered.find(w);
        if (reg != registered.end()) {
            AddRegistered(w, reg->second);
            return true;
        }
        if (IsKindOf(w, "cWinViewControl")) { AddStatus(w); return true; }
        bool descend = true;
        if (VisitFamilyPanel(w, &descend)) return descend;
        if (game::IsTraitBar(w)) {
            Add(w, Role::TraitBar, game::TraitBarName(w), 0, kPersonalityGroup);
            return false;
        }
        if (IsKindOf(w, "cTSWinCheck")) { Add(w, Role::CheckBox); return false; }
        if (IsKindOf(w, "cTSWinSlider")) { Add(w, Role::Slider); return false; }
        if (IsKindOf(w, "cPickFamilyItem")) {
            Add(w, Role::FamilyRow);
            // The selected family's members get a group: the game shows them on hover cards.
            if (game::ButtonChecked(w))
                for (Ptr member : game::FamilyRowMembers(w))
                    Add(member, Role::PickerMember, "", game::FamilyMemberId(member), kMembersGroup);
            return false;
        }
        if (IsKindOf(w, "cWinLotBtn")) { Add(w, Role::Lot); return true; }
        if (IsKindOf(w, "cWinMotive")) { Add(w, Role::Motive, game::Caption(w)); return false; }
        if (IsKindOf(w, "cActionIcon")) {
            std::string action = game::Tooltip(w);
            if (!action.empty()) Add(w, Role::QueuedAction, action);
            return true;
        }
        if (IsKindOf(w, "cTutorialIcon")) { Add(w, Role::TutorialIcon); return true; }
        if (IsKindOf(w, "cWinBudgetRow")) { Add(w, Role::BudgetRow, game::Caption(w)); return false; }
        if (game::IsProductButton(w)) {
            game::ProductInfo p;
            Add(w, Role::Product, game::ReadProductOfButton(w, &p) ? p.name : game::Tooltip(w));
            return false;
        }
        if (IsKindOf(w, "cTSWinTextEdit2")) {
            screen.textEntryVisible = true;
            // A Sim's card (Ctrl+Shift+1..9) is just its biography: "Bio de Cris".
            Ptr parent = game::Parent(w);
            std::string name = IsKindOf(parent, "cWinEditBio") ? game::NeighborName(game::Rd<int>(parent + game::addr::editbio::kNeighborId, -1)) : "";
            if (!name.empty()) Add(w, Role::TextField, util::Format(S(Str::BioFor), name.c_str()) + ":");
            else Add(w, Role::TextField);
            return true;
        }
        if (IsKindOf(w, "cTSWinText") || IsKindOf(w, "cWinTextWrap")) {
            std::string text = util::CleanForSpeech(game::Caption(w));  // some titles are a lone space
            if (!text.empty()) Add(w, Role::Text, text);
            return true;
        }
        std::string label = game::Tooltip(w);
        if (IsKindOf(w, "cTSWinBtn")) {
            if (label.empty()) label = game::Caption(w);
            // An unlabeled button holding other widgets is a panel background, not a control.
            if (!label.empty()) Add(w, IsTab(w) ? Role::Tab : Role::Button, label);
            else if (!hasChildren && game::ButtonEnabled(w)) Add(w, Role::Button, label);
            return true;
        }
        // Any other widget with hover text is something a sighted player can point at.
        if (!label.empty()) Add(w, Role::Button, label);
        return true;
    }

    void Walk(Ptr w, int depth) {
        // The help system's tip icon blinks (hidden half the time) while a tip waits: it is
        // listed as long as it exists, so the player can reach it.
        if (depth > kMaxDepth || (!game::Visible(w) && !(game::IsKindOf(w, "cTutorialIcon") && game::HelpTipPending()))) return;
        std::vector<Ptr> children = game::Children(w);
        if (!Visit(w, !children.empty())) return;
        for (Ptr c : children) Walk(c, depth + 1);
    }
};

RECT Bounds(const std::vector<Item>& items) {
    RECT b = items.front().rect;
    for (const Item& it : items) {
        b.left = std::min(b.left, it.rect.left);
        b.top = std::min(b.top, it.rect.top);
        b.right = std::max(b.right, it.rect.right);
        b.bottom = std::max(b.bottom, it.rect.bottom);
    }
    return b;
}

Ptr GroupKey(const Item& it) {
    if (it.group) return it.group;
    switch (it.role) {
    case Role::Lot: return kHousesGroup;
    case Role::Status: return kStatusGroup;
    case Role::FamilyMember: return kFamilyGroup;
    case Role::QueuedAction: return kActionQueueGroup;
    case Role::TutorialIcon: return kTipGroup;
    default: return game::Parent(it.widget);
    }
}

// Items are grouped by their container, so a panel's title and a dialog's message sit with the
// controls they belong to. Houses form one group sorted by house number. Groups come in the
// screen's order of importance (GroupOrder), then top to bottom, left to right.
std::vector<Group> MakeGroups(std::vector<Item>& items) {
    std::vector<Group> groups;
    for (Item& it : items) {
        Ptr key = GroupKey(it);
        auto g = std::find_if(groups.begin(), groups.end(), [&](const Group& x) { return x.key == key; });
        if (g == groups.end()) {
            groups.push_back(Group{key, {}, {}});
            g = groups.end() - 1;
        }
        g->items.push_back(std::move(it));
    }
    for (Group& g : groups) {
        ArrangeGroup(g);
        g.bounds = Bounds(g.items);
    }
    std::stable_sort(groups.begin(), groups.end(), [](const Group& a, const Group& b) {
        int oa = GroupOrder(a), ob = GroupOrder(b);
        if (oa != ob) return oa < ob;
        if (a.bounds.top != b.bounds.top) return a.bounds.top < b.bounds.top;
        return a.bounds.left < b.bounds.left;
    });
    return groups;
}

}  // namespace

// The interaction menu draws its entries itself; they are items of one group, in menu order.
Screen CollectPieMenu(Ptr pie) {
    Screen screen;
    Group group{pie, {}, game::Rect(pie)};
    std::vector<std::string> labels = game::PieItems(pie);
    for (int i = 0; i < (int)labels.size(); i++) {
        Item it;
        it.widget = pie;
        it.role = Role::PieItem;
        it.label = labels[i];
        it.rect = group.bounds;
        it.number = i;
        group.items.push_back(std::move(it));
    }
    if (!group.items.empty()) screen.groups.push_back(std::move(group));
    return screen;
}

// The cheat console is a text field of the main window: the field itself owns input.
Screen CollectLoneField(Ptr field) {
    Screen screen;
    screen.textEntryVisible = true;
    Item it;
    it.widget = field;
    it.role = Role::TextField;
    it.label = field == game::CheatField() ? S(Str::CheatConsole) : "";
    it.rect = game::Rect(field);
    Group group{field, {}, it.rect};
    group.items.push_back(std::move(it));
    screen.groups.push_back(std::move(group));
    return screen;
}

Screen Collect(Ptr root) {
    if (game::IsPieMenu(root)) return CollectPieMenu(root);
    if (game::IsKindOf(root, "cTSWinTextEdit2")) return CollectLoneField(root);
    Screen screen;
    Walker walker{screen, {}};
    walker.RegisterScreen(root);  // the root itself is not an item, but may define its buttons
    for (Ptr c : game::Children(root)) walker.Walk(c, 0);
    screen.groups = MakeGroups(walker.items);
    return screen;
}

}  // namespace nav
