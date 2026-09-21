#include "screens.h"

#include "../game/lot.h"
#include "../game/pie.h"
#include "../game/view_control.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../util/game_settings.h"
#include "../util/strings.h"
#include "../util/text.h"

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
};

constexpr size_t kMaxTitleLength = 40;  // longer text is a message, not a title

}  // namespace

// The neighborhood says its number ("Barrio 1"), as the game shows it between its arrows.
std::string ScreenName(Ptr root) {
    if (game::IsKindOf(root, "cWinNeighborhood")) {
        std::string number = util::ReadGameSetting("SIMS_CURRENT_NEIGHBORHOOD_NUM");
        if (!number.empty()) return util::Format(S(Str::ScreenNeighborhoodNumber), number.c_str());
    }
    for (const Named& s : kScreens)
        if (game::IsKindOf(root, s.cls)) return S(s.name);
    return "";
}

bool IsScreen(Ptr root) { return !ScreenName(root).empty(); }

uintptr_t ScreenInstance(Ptr root) { return game::IsKindOf(root, "cDDDSimsView") ? game::LotModule() : 0; }

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
    if (game::IsPieMenu(g.key)) return S(Str::PieMenu);
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
    return true;
}

}  // namespace nav
