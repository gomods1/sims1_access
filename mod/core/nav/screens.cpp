#include "screens.h"

#include "../game/lot.h"
#include "../game/pie.h"
#include "../game/view_control.h"
#include "../game/neighborhood.h"
#include "../game/magic.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../util/game_settings.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "tutorial.h"
#include "world.h"

namespace nav {
namespace {

struct Named {
    const char* cls;
    Str name;
};

const Named kScreens[] = {
    {"cWinNeighborhood", Str::ScreenNeighborhood},
    {"cDDDSimsView", Str::ScreenLot},
    {"cWinPickFamily", Str::ScreenPickFamily},
    {"cWinDesignFamily", Str::ScreenDesignFamily},
    {"cWinDesignCharacter", Str::ScreenDesignCharacter},
};

// Groups named after the panel that holds them.
const Named kPanels[] = {
    {"cWinViewControl", Str::GroupControls},
    {"cWinOptions", Str::GroupOptions},
    {"cWinCatalog", Str::GroupCatalog},
    {"cWinArch", Str::GroupBuild},
    {"cWinCamPanel", Str::GroupCamera},
    {"cWinPeople", Str::GroupFamilyPanel},
    {"cPickFamilyList", Str::GroupFamilies},
    {"cWinSubpanelDogSkills", Str::GroupPetSkills},
    {"cWinSubpanelCatSkills", Str::GroupPetSkills},
    {"cWinSubpanelReportCard", Str::GroupReportCard},
};

constexpr size_t kMaxTitleLength = 40;  // longer text is a message, not a title

}  // namespace

// The neighborhood says its number ("Barrio 1"), as the game shows it between its arrows.
std::string ScreenName(Ptr root) {
    if (game::TravelMap(root) == root && root) {
        // Opened from the neighborhood's own buttons (to build on its lots) the map has no
        // filter bar: it is a view, not a trip's "where to?".
        bool trip = false;
        for (const game::MapFilter& f : game::MapFilters(root)) trip |= game::Visible(f.button);
        if (!trip)
            return S(game::IsStudioTown(root) ? Str::ViewStudioTown
                     : game::IsKindOf(root, "cWinMagicland") ? Str::ViewMagicTown
                     : game::IsKindOf(root, "cWinVacation") ? Str::ViewVacation
                     : game::IsKindOf(root, "cWinDowntown") ? Str::ViewDowntown : Str::ScreenOldTown);
        return S(game::IsStudioTown(root) ? Str::ScreenStudioTown
                 : game::IsKindOf(root, "cWinMagicland") ? Str::ScreenMagicTown
                 : game::IsKindOf(root, "cWinVacation") ? Str::ScreenVacation
                 : game::IsKindOf(root, "cWinDowntown") ? Str::ScreenDowntown : Str::ScreenOldTown);
    }
    if (game::IsKindOf(root, "cWinNeighborhood")) {
        std::string number = util::ReadGameSetting("SIMS_CURRENT_NEIGHBORHOOD_NUM");
        if (!number.empty()) return util::Format(S(Str::ScreenNeighborhoodNumber), number.c_str());
    }
    for (const Named& s : kScreens)
        if (game::IsKindOf(root, s.cls)) return S(s.name);
    return "";
}

bool IsScreen(Ptr root) { return !ScreenName(root).empty(); }

uintptr_t ScreenInstance(Ptr root) { return game::IsKindOf(root, "cDDDSimsView") ? game::LotIdentity() : 0; }

bool PrepareScreen(Ptr root) {
    if (!game::IsKindOf(root, "cDDDSimsView") || game::ModePanelShown(root)) return false;
    Ptr live = game::LiveModeButton(game::FindViewControl(root));
    if (!live) return false;
    game::Press(live);
    game::Unhover(live);
    return true;
}

// An options page (game, sound, graphics) is named after the page button that is selected.
std::string OptionsPageName(Ptr page) {
    Ptr options = game::Parent(page);
    if (!game::IsKindOf(options, "cWinOptions")) return "";
    for (Ptr b : game::Children(options))
        if (game::IsKindOf(b, "cTSWinBtn") && game::Visible(b) && game::GetButtonType(b) != game::ButtonType::Push &&
            game::ButtonChecked(b))
            return game::Tooltip(b);
    return "";
}

// A group with its own short title ("Necesidades") is named after it, else after its panel.
// Options pages go by their page button first: their texts are values ("Auto").
std::string GroupName(const Group& g) {
    std::string page = OptionsPageName(g.key);
    if (!page.empty()) return page;
    if (g.key == kHousesGroup) return S(Str::GroupHouses);
    if (g.key == kStatusGroup) return S(Str::GroupStatus);
    if (g.key == kFamilyGroup) return S(Str::GroupFamily);
    if (g.key == kSimPanelsGroup) return S(Str::GroupFamilyPanel);
    if (g.key == kJobGroup) return S(Str::GroupJob);
    if (g.key == kHouseGroup) return S(Str::GroupHouse);
    if (g.key == kPersonalityGroup) return S(Str::GroupPersonality);
    if (g.key == kLooksGroup) return S(Str::GroupLooks);
    if (g.key == kMembersGroup) return S(Str::GroupMembers);
    if (g.key == kTipGroup) return S(Str::GroupTip);
    if (g.key == kKindsGroup || g.key == kRoomsGroup) return world::LevelName(g.key);
    // The phone book: families on the left, the chosen family's members on the right.
    if (game::IsKindOf(g.key, "cTSWinTextList") && game::IsKindOf(game::Parent(g.key), "CWinPhoneBook")) {
        RECT mine = game::Rect(g.key);
        bool left = true;
        for (Ptr other : game::Children(game::Parent(g.key)))
            if (other != g.key && game::IsKindOf(other, "cTSWinTextList") && game::Rect(other).left < mine.left) left = false;
        return S(left ? Str::GroupFamilies : Str::GroupMembers);
    }
    if (g.key == kBookEntriesGroup)
        return S(!g.items.empty() && game::IsCookbook(game::MagicBookOf(g.items[0].widget)) ? Str::GroupRecipes
                                                                                             : Str::GroupSpells);
    if (game::IsPieMenu(g.key)) return S(Str::PieMenu);
    if (tutorial::IsStepGroup(g)) return S(Str::GroupTutorial);
    for (const Item& it : g.items)
        if (it.role == Role::Text && it.label.size() <= kMaxTitleLength) return it.label;
    for (const Named& p : kPanels)
        if (game::IsKindOf(g.key, p.cls)) return S(p.name);
    if (IsActionQueue(g)) return S(Str::GroupActionQueue);
    return IsTextOnly(g) ? S(Str::GroupText) : S(Str::GroupButtons);
}

int GroupOrder(const Group& g) {
    if (g.key == kStatusGroup) return 0;
    if (HasRole(g, Role::Motive)) return 1;
    if (g.key == kFamilyGroup) return 2;
    if (IsActionQueue(g)) return 3;
    // A screen's main content before its other panels (NAVIGATION.md): the neighborhood's houses,
    // the family picker's list.
    if (g.key == kHousesGroup || IsPrimaryGroup(g)) return 3;
    if (g.key == kLooksGroup) return 5;  // after the personality bars
    return 4;
}

int EntryItem(const Group& g) {
    if (g.key == kFamilyGroup)
        for (int i = 0; i < (int)g.items.size(); i++)
            if (game::ButtonChecked(g.items[i].widget)) return i;
    return FirstControl(g);
}

bool IsPrimaryGroup(const Group& g) { return game::IsKindOf(g.key, "cPickFamilyList"); }

bool IsActionQueue(const Group& g) { return g.key == kActionQueueGroup; }

// The HUD's own groups (status, needs, family, queue, Sim panels) come and go with Live mode;
// only panels a player opened (options, a subpanel, the catalog) take focus.
bool TakesFocusWhenOpened(const Group& g) {
    if (g.items.size() < 2 || IsTextOnly(g) || GroupOrder(g) < 4 || g.key == kSimPanelsGroup) return false;
    if (g.key == kKindsGroup || g.key == kRoomsGroup) return false;  // the mod's own, always there
    if (tutorial::IsStepGroup(g)) return false;  // announced by the tutorial watcher instead
    return true;
}

bool IsSettingsPage(const Group& g) {
    if (game::IsKindOf(game::Parent(g.key), "cWinOptions")) return true;
    for (const Item& it : g.items)
        if (IsValueControl(it)) return true;
    return false;
}

}  // namespace nav
