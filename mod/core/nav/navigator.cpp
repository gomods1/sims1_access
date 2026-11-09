#include "navigator.h"

#include <windows.h>

#include <algorithm>
#include <map>
#include <set>

#include "../game/buy.h"
#include "../game/design.h"
#include "../game/pie.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/settings.h"
#include "../util/text.h"
#include "buy_mode.h"
#include "build_mode.h"
#include "cues.h"
#include "map_mode.h"
#include "house_overview.h"
#include "camera.h"
#include "cheat_console.h"
#include "collect.h"
#include "credits.h"
#include "../game/neighborhood.h"
#include "describe.h"
#include "arrange.h"
#include "buffers.h"
#include "game_speed.h"
#include "narrator.h"
#include "screens.h"
#include "sim_watch.h"
#include "text_entry.h"
#include "tutorial.h"
#include "../input/keyboard.h"
#include "manual.h"
#include "world.h"

namespace nav {
namespace {

constexpr ULONGLONG kSettleMs = 350;        // a new root must stay this long before it is announced
constexpr ULONGLONG kEmptyGraceMs = 1500;   // wait this long for widgets on a root that has none yet
constexpr ULONGLONG kRefreshMs = 250;       // re-collect the tree this often
constexpr ULONGLONG kPressFeedbackMs = 120; // read a toggle's new state this long after a press
constexpr ULONGLONG kPrepareQuietMs = 1000; // panels shown by PrepareScreen appear over a few frames
constexpr ULONGLONG kAutoChooseMs = 3000;   // how long ChooseInNextMenu waits for its menu
constexpr ULONGLONG kPageTurnMs = 250;      // a page arrow's new page is shown by then
constexpr unsigned kToolTabHoldMs = 700;    // a build tool's tab brings back its last object by then

struct PendingPress {
    ItemId item;
    bool wasChecked = false;
    ULONGLONG at = 0;
    std::vector<std::pair<Ptr, std::string>> texts;  // the screen's texts before the press
    bool reshowPanel = false;  // a selected mode tab was pressed: it hides the panel, press again
    bool helpSpoken = false;   // a live panel item's hover help was said: no "seleccionado" after it
};

// The on/off state a press can change, for the feedback after it.
bool CheckedState(const Item& it) {
    if (it.role == Role::CheckBox) return game::CheckBoxChecked(it.widget);
    return game::IsKindOf(it.widget, "cTSWinBtn") && game::ButtonChecked(it.widget);
}

struct Remembered {
    ItemId focus;
    uintptr_t instance = 0;  // ScreenInstance when it was remembered
};

struct State {
    Ptr root = 0;
    Screen screen;
    ItemId focus;           // focused item
    int group = -1;         // indices of `focus` in screen.groups
    int index = -1;
    Ptr focusGroup = 0;     // the key of the focused group (its index changes when groups come and go)
    std::vector<Ptr> focusPanels;  // that group's window and its parents, below the screen's root
    std::map<Ptr, Remembered> focusByScreen;  // focus to restore when coming back to a screen
    std::set<Ptr> knownGroups;         // group keys present at the last refresh
    ULONGLONG rootSince = 0;
    ULONGLONG lastCollect = 0;
    bool entering = false;  // root changed; announce once its content has settled
    bool returning = false; // entering a screen seen before: only say the restored focus
    PendingPress press;
    ULONGLONG quietUntil = 0;  // panels appearing before this are the mod's own doing (PrepareScreen)
    int autoChoose = -1;       // ChooseInNextMenu: the entry to choose, until autoChooseUntil
    ULONGLONG autoChooseUntil = 0;
    Ptr pageGroup = 0;         // a page arrow was pressed in this group (moving past its end)...
    int pageDirection = 0;     // ...toward the next (+1) or previous (-1) page...
    Role pageRole = Role::Button;  // ...from an item of this kind (a product)...
    ULONGLONG pageAt = 0;      // ...at this time
    bool pageFiltered = false; // ...looking for a product of the room filter
    int pageTurns = 0;         // pages turned in a row looking for one
    bool pageToFirst = false;  // ...and lands on the page's first item either way (Left / Right)
    bool leftPie = false;      // the root before this one was an interaction menu
    ULONGLONG pieSeenAt = 0;   // when an interaction menu was last the root
    bool choseInPie = false;   // ...and the player chose an entry in it (not cancelled)
};

State g;

std::vector<Group>& Groups() { return g.screen.groups; }

// ---- Focus --------------------------------------------------------------------------------

const Item* Focused() {
    if (g.group < 0 || g.group >= (int)Groups().size()) return nullptr;
    const auto& items = Groups()[g.group].items;
    return g.index >= 0 && g.index < (int)items.size() ? &items[g.index] : nullptr;
}

bool Locate(const ItemId& id, int* group, int* index) {
    for (int gi = 0; gi < (int)Groups().size(); gi++)
        for (int ii = 0; ii < (int)Groups()[gi].items.size(); ii++)
            if (IdOf(Groups()[gi].items[ii]) == id) {
                *group = gi;
                *index = ii;
                return true;
            }
    return false;
}

// ---- The focus on screen, for a sighted helper (Ctrl+Shift+V) ---------------------------------
// The focused control gets the mouse-enter a real pointer would give it, so the game lights it
// as under the mouse (a tester's partner could not tell what the mod had selected). Off unless
// asked for: hovering can show tooltips.

int g_showFocus = -1;  // -1 not read yet
Ptr g_hovered = 0;

bool ShowFocus() {
    if (g_showFocus < 0) g_showFocus = util::GetSetting("show_focus", 0) ? 1 : 0;
    return g_showFocus == 1;
}

bool StillListed(Ptr w) {
    for (const Group& gr : Groups())
        for (const Item& it : gr.items)
            if (it.widget == w && !IsVirtual(it)) return true;
    return false;
}

// A thing of the lot focused while the focus is shown: the camera centres on it, and the mouse
// pointer goes to the middle of the game's window, over it. The partner of a tester saw the
// pointer resting on a vegetable cart while the mod had the cactus beside it. Only while the
// game's window is in front, and never in buy or build mode (the held object follows the pointer).
void PointAtThing(const Item* it) {
    if (!ShowFocus() || !it || it->role != Role::LotObject || game::InBuyTool() || !LotViewActive() ||
        build::Active() || game::BuildPanelShown(g.root))
        return;
    HWND front = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(front, &pid);
    if (!front || pid != GetCurrentProcessId()) return;
    RECT r;
    if (!GetClientRect(front, &r)) return;
    POINT p{(r.right - r.left) / 2, (r.bottom - r.top) / 2};
    ClientToScreen(front, &p);
    SetCursorPos(p.x, p.y);
}

void HoverFocused(const Item* it) {
    PointAtThing(it);
    Ptr want = ShowFocus() && it && !IsVirtual(*it) && it->role != Role::PieItem ? it->widget : 0;
    if (want == g_hovered) return;
    if (g_hovered && StillListed(g_hovered)) game::Unhover(g_hovered);  // a widget gone may be freed
    g_hovered = want;
    if (want && game::Visible(want)) game::Hover(want);
}

// A group's window and its parents up to (not including) the screen's root; empty for the mod's
// own groups, whose keys are small numbers.
std::vector<Ptr> PanelChain(Ptr key) {
    std::vector<Ptr> chain;
    if (key < 0x10000) return chain;
    for (Ptr w = key; w && w != g.root && chain.size() < 8; w = game::Parent(w)) chain.push_back(w);
    return chain;
}

void SetFocus(int group, int index) {
    g.group = group;
    g.index = index;
    const Item* it = Focused();
    g.focus = it ? IdOf(*it) : ItemId{};
    g.focusGroup = group >= 0 && group < (int)Groups().size() ? Groups()[group].key : 0;
    g.focusPanels = PanelChain(g.focusGroup);
    if (it && it->role == Role::PieItem) game::PieHighlight(it->widget, it->number);
    if (it && it->role == Role::LotObject) world::Focus(it->number);  // the camera looks at it
    if (it && it->role == Role::TextField) text::TakeTyping(it->widget);  // typing follows the focus
    HoverFocused(it);
    if (g.root && IsScreen(g.root)) g.focusByScreen[g.root] = Remembered{g.focus, ScreenInstance(g.root)};
}

// First control of the screen's primary list, else of the first group that is not text or the
// action queue, else of any group.
void FocusDefault() {
    auto pick = [](auto accept) {
        for (int gi = 0; gi < (int)Groups().size(); gi++)
            if (!IsTextOnly(Groups()[gi]) && accept(Groups()[gi])) {
                SetFocus(gi, FirstControl(Groups()[gi]));
                return true;
            }
        return false;
    };
    if (pick(IsPrimaryGroup)) return;
    if (pick([](const Group& gr) { return !IsActionQueue(gr) && gr.key != kModHelpGroup; })) return;
    if (pick([](const Group&) { return true; })) return;
    SetFocus(Groups().empty() ? -1 : 0, Groups().empty() ? -1 : 0);
}

// ---- Announcements ------------------------------------------------------------------------

void AnnounceFocus(bool withGroup) {
    const Item* it = Focused();
    if (!it) {
        speech::Say(S(Str::NothingHere));
        return;
    }
    const Group& group = Groups()[g.group];
    // entering a group says its name and size, even of one item (NAVIGATION.md)
    std::string prefix = withGroup ? GroupSummary(group) + ". " : "";
    buffers::SetDetails(DetailLines(*it, group, g.index));
    speech::Say(prefix + Describe(*it));
    // Nothing on this panel places a roof: a player looking for the map to put it with never finds it.
    if (withGroup && game::IsKindOf(group.key, "cWinRoofPanel")) speech::Say(S(Str::RoofHint));
}

void LogScreen() {
    Log("screen: %s %p, %d groups%s", game::ClassName(g.root).c_str(), (void*)g.root, (int)Groups().size(),
        g.returning ? " (returning)" : "");
    for (const Group& group : Groups()) {
        Log("  group %s, %d items", GroupName(group).c_str(), (int)group.items.size());
        for (const Item& it : group.items)
            if (it.role == Role::Button && it.label.empty())
                Log("    unlabeled %s at %ld,%ld", game::ClassName(it.widget).c_str(), it.rect.left, it.rect.top);
    }
}

// Back from an interaction menu: the chosen action reads as queued ("Hablar, en cola"), the
// newest icon being the last; a cancel was already said. Nothing else is repeated.
void AnnounceAfterPie() {
    if (!g.choseInPie) return;
    g.choseInPie = false;
    for (const Group& group : Groups())
        if (IsActionQueue(group) && !group.items.empty())
            buffers::Event(util::Format(S(Str::QueuedAction), QueuedLabel(group.items.back().label).c_str()));
}

// Screen: its name, then the focused item with its group. Dialog: its text (title, message),
// then the focused button (a dialog's group summary would only repeat its title). Loading
// screens and empty roots stay silent.
void AnnounceScreen() {
    LogScreen();
    if (g.returning) {
        AnnounceFocus(false);
        return;
    }
    if (game::IsPieMenu(g.root)) {
        if (Groups().empty()) speech::Say(S(Str::NoInteractions));
        else speech::Say(GroupSummary(Groups()[0]) + ". " + Describe(*Focused()));
        return;
    }
    if (credits::OnScreen(g.root)) {  // the details buffer holds the credits: focus must not replace them
        speech::Say(S(Str::CreditsScreen));
        return;
    }
    std::string name = ScreenName(g.root);
    if (Groups().empty() && name.empty()) return;
    if (!name.empty()) {
        speech::Say(name);
        AnnounceFocus(true);
        return;
    }
    for (const Group& group : Groups())
        for (const Item& it : group.items)
            if (it.role == Role::Text) buffers::Event(it.label);  // a dialog's title and message
    AnnounceFocus(false);
}

// ---- Keeping the model current ------------------------------------------------------------

void RememberGroups() {
    g.knownGroups.clear();
    for (const Group& group : Groups()) g.knownGroups.insert(group.key);
}

// A panel that opened inside the same screen (options, catalog...) takes focus and is announced.
bool FocusOpenedPanel() {
    if (GetTickCount64() < g.quietUntil) {
        RememberGroups();
        return false;
    }
    int opened = -1;
    for (int gi = 0; gi < (int)Groups().size() && opened < 0; gi++)
        if (!g.knownGroups.count(Groups()[gi].key) && TakesFocusWhenOpened(Groups()[gi])) opened = gi;
    RememberGroups();
    if (opened < 0) return false;
    SetFocus(opened, FirstControl(Groups()[opened]));
    AnnounceFocus(true);
    return true;
}

// After a refresh: find the focused widget again, or move focus to its closest replacement and
// say where it went. The replacement is in the same group (by its key: groups that come and go
// shift the indices), else in a panel that has just appeared (a panel that rebuilds its page as
// it opens: the options page did, and focus fell on "mod help", the group then at the old index),
// else the screen's default.
void RelocateFocus(const std::set<Ptr>& before) {
    int gi, ii;
    if (g.focus && Locate(g.focus, &gi, &ii)) {
        g.group = gi;
        g.index = ii;
        g.focusGroup = Groups()[gi].key;
        return;
    }
    bool hadFocus = (bool)g.focus;
    int same = -1, opened = -1;
    for (int i = 0; i < (int)Groups().size(); i++) {
        if (Groups()[i].key == g.focusGroup && !Groups()[i].items.empty()) same = i;
        if (opened < 0 && !before.count(Groups()[i].key) && TakesFocusWhenOpened(Groups()[i])) opened = i;
    }
    // a group of the same panel: the one sharing the nearest parent with the lost group (the
    // options panel's buttons, when the page that had focus closed)
    int related = -1;
    size_t nearest = SIZE_MAX;
    if (same < 0 && opened < 0 && !g.focusPanels.empty())
        for (int i = 0; i < (int)Groups().size(); i++) {
            if (Groups()[i].items.empty() || IsTextOnly(Groups()[i])) continue;
            for (Ptr w : PanelChain(Groups()[i].key)) {
                auto hit = std::find(g.focusPanels.begin(), g.focusPanels.end(), w);
                if (hit == g.focusPanels.end()) continue;
                size_t depth = hit - g.focusPanels.begin();
                if (depth < nearest) {
                    nearest = depth;
                    related = i;
                }
                break;
            }
        }
    if (same >= 0)
        SetFocus(same, std::clamp(g.index, 0, (int)Groups()[same].items.size() - 1));
    else if (opened >= 0)
        SetFocus(opened, FirstControl(Groups()[opened]));
    else if (related >= 0)
        SetFocus(related, FirstControl(Groups()[related]));
    else
        FocusDefault();
    if (hadFocus) {
        LogVerbose("focus lost (group key %p); now group %d (%s)", (void*)g.focusGroup, g.group,
                   same >= 0 ? "same" : opened >= 0 ? "opened" : related >= 0 ? "same panel" : "default");
        for (const Group& gr : Groups())
            LogVerbose("  %s%s %p, %d items", before.count(gr.key) ? "" : "new ", GroupName(gr).c_str(), (void*)gr.key,
                       (int)gr.items.size());
    }
    if (hadFocus && !g.entering && g.focus && !g.pageAt) AnnounceFocus(true);  // a page turn says its own
}

void Refresh() {
    g.lastCollect = GetTickCount64();
    std::set<Ptr> before = g.knownGroups;
    g.screen = Collect(g.root);
    if (!g.entering && FocusOpenedPanel()) return;
    RelocateFocus(before);
}

void CheckRootChange() {
    Ptr root = game::InputRoot();
    if (!root || root == g.root) return;
    g.leftPie = game::IsPieMenu(g.root) && !game::IsPieMenu(root);
    if (game::IsPieMenu(root)) g.pieSeenAt = GetTickCount64();
    if (g.root && !IsScreen(g.root)) g.focusByScreen.erase(g.root);
    g.root = root;
    g.rootSince = GetTickCount64();
    g.entering = true;
    ClearDescriptionCache();
    buffers::ClearDetails();
    auto remembered = g.focusByScreen.find(root);
    g.returning = IsScreen(root) && remembered != g.focusByScreen.end() &&
                  remembered->second.instance == ScreenInstance(root);
    g.focus = g.returning ? remembered->second.focus : ItemId{};
    g.group = g.index = -1;
}

// Completes entering a new root after a refresh. `remembered` is the focus restored for a screen
// seen before; if the refresh could not find it, this is announced like a fresh screen.
void FinishEntering(const ItemId& remembered) {
    g.entering = false;
    if (g.returning && (!remembered || g.focus != remembered)) g.returning = false;
    int gi, ii;
    if (!g.focus || !Locate(g.focus, &gi, &ii)) FocusDefault();
    RememberGroups();
}

// The screen's texts, to tell afterwards what a press changed (a help topic's text).
std::vector<std::pair<Ptr, std::string>> TextSnapshot() {
    std::vector<std::pair<Ptr, std::string>> texts;
    for (const Group& group : Groups())
        for (const Item& it : group.items)
            if (it.role == Role::Text) texts.push_back({it.widget, game::Caption(it.widget)});
    return texts;
}

// Texts that changed since the snapshot are read: what the press showed.
bool SayChangedTexts(const std::vector<std::pair<Ptr, std::string>>& before) {
    bool said = false;
    for (const auto& entry : before) {
        std::string now = game::Caption(entry.first);
        if (now == entry.second) continue;
        std::string text = util::CleanForSpeech(now);
        if (text.empty()) continue;
        speech::Say(text);
        said = true;
    }
    return said;
}

// A build tool's tab was pressed: focus goes to what the tool offers (its roof panel, or its
// styles, on the one chosen). false when it offers nothing.
bool FocusToolOptions() {
    Refresh();
    for (int gi = 0; gi < (int)Groups().size(); gi++) {
        const Group& gr = Groups()[gi];
        if (gr.items.empty() || (gr.key != kBuildStylesGroup && !game::IsKindOf(gr.key, "cWinRoofPanel"))) continue;
        if (g.group == gi) return true;  // the panel opened with focus on it: said already
        int at = FirstControl(gr);
        for (int i = 0; i < (int)gr.items.size(); i++)
            if (!IsPageArrow(gr.items[i]) && game::IsKindOf(gr.items[i].widget, "cTSWinBtn") && game::ButtonChecked(gr.items[i].widget)) {
                at = i;
                break;
            }
        SetFocus(gi, at);
        AnnounceFocus(true);
        return true;
    }
    return false;
}

// Toggles and check boxes say their new state; the press's synthetic mouse-enter is undone.
// Speed buttons stay quiet here: the speed watcher announces every speed change.
void CheckPressFeedback() {
    PendingPress p = g.press;
    if (!p.item || GetTickCount64() - p.at < kPressFeedbackMs) return;
    g.press = PendingPress{};
    if (p.item.widget == g_hovered) g_hovered = 0;  // the feedback unhovers it: the tick lights it again
    int gi, ii;
    if (!Locate(p.item, &gi, &ii)) return;  // the press closed or replaced it
    Ptr w = p.item.widget;
    const Item& item = Groups()[gi].items[ii];
    if (p.reshowPanel) {
        game::Unhover(w);
        if (!game::ModePanelShown(g.root)) {
            Log("mode tab hid the panel: pressing it again");
            game::Press(w);
            game::Unhover(w);
            g.quietUntil = GetTickCount64() + kPrepareQuietMs;
        }
        speech::Say(S(Str::TabSelected));
        return;
    }
    if (item.role == Role::Spinner) {
        speech::Say(util::CleanForSpeech(game::Caption(w)));
        return;
    }
    if (item.role == Role::Product) {  // choosing a product starts placing it; buy mode speaks
        game::Unhover(w);
        return;
    }
    if (item.role == Role::CheckBox) {
        game::Unhover(game::CheckBoxButton(w));
        if (CheckedState(item) != p.wasChecked) speech::Say(S(CheckedState(item) ? Str::Checked : Str::NotChecked));
        return;
    }
    game::Unhover(w);
    if (item.role == Role::Tab && game::IsKindOf(game::Parent(w), "cWinArch") && FocusToolOptions()) return;
    std::string after = AfterPress(g.root, w);
    if (!after.empty()) {
        speech::Say(after);
        return;
    }
    if (SayChangedTexts(p.texts)) return;  // the text says what happened; no need for "seleccionado"
    if (p.helpSpoken || p.item != g.focus || !game::IsKindOf(w, "cTSWinBtn") || game::IsSpeedButton(w)) return;
    bool checked = game::ButtonChecked(w);
    if (checked == p.wasChecked) return;
    if (item.role == Role::Tab) {
        if (checked) speech::Say(S(Str::TabSelected));
        return;
    }
    switch (game::GetButtonType(w)) {
    case game::ButtonType::Check: speech::Say(S(checked ? Str::Checked : Str::NotChecked)); break;
    case game::ButtonType::Toggle: if (checked) speech::Say(S(Str::Selected)); break;
    default: break;
    }
}

// The page arrow that turns a group's list toward `direction` (+1 next: right of the list, -1
// previous: left of it), 0 when there is none on screen (the first or last page).
Ptr PageArrow(const Group& gr, int direction) {
    long left = LONG_MAX, right = LONG_MIN;
    for (const Item& it : gr.items)
        if (!IsPageArrow(it)) {
            left = std::min(left, it.rect.left);
            right = std::max(right, it.rect.right);
        }
    long middle = left <= right ? (left + right) / 2 : 512;
    for (const Item& it : gr.items)
        if (IsPageArrow(it) && (direction > 0) == (it.rect.left > middle) && game::ButtonEnabled(it.widget))
            return it.widget;
    return 0;
}

// ---- The catalog's room filter (Ctrl+R) ------------------------------------------------------
// This build's catalog only has the function categories: products can be kept to those meant for
// one kind of room (the OBJD room flags the catalog popup lists).

struct RoomChoice {
    uint16_t flag;
    Str name;
};
const RoomChoice kRoomChoices[] = {{0, Str::RoomFilterAll},   {0x01, Str::RoomKitchen}, {0x04, Str::RoomBathroom},
                                   {0x02, Str::RoomBedroom},  {0x08, Str::RoomLiving},  {0x20, Str::RoomDining},
                                   {0x80, Str::RoomStudy},    {0x10, Str::RoomOutside}};
constexpr int kRoomChoiceCount = (int)(sizeof kRoomChoices / sizeof kRoomChoices[0]);
constexpr int kMaxFilterPages = 40;
int g_roomChoice = 0;

bool IsProduct(const Item& it) { return game::IsProductButton(it.widget); }

bool MatchesFilter(const Item& it) {
    if (!g_roomChoice || !IsProduct(it)) return true;
    game::ProductInfo p;
    return game::ReadProductOfButton(it.widget, &p) && (p.roomFlags & kRoomChoices[g_roomChoice].flag);
}

void TurnPage(const Group& gr, Ptr arrow, int direction, bool filtered, bool toFirst = false) {
    Log("page turn %s%s", direction > 0 ? "next" : "previous", filtered ? " (room filter)" : "");
    g.pageToFirst = toFirst;
    g.pageGroup = gr.key;
    g.pageDirection = direction > 0 ? 1 : -1;
    g.pageRole = Focused()->role;
    g.pageAt = GetTickCount64();
    g.pageFiltered = filtered;
    g.quietUntil = g.pageAt + kPageTurnMs + kPrepareQuietMs;  // the panel redraws itself
    game::Press(arrow);
    game::Unhover(arrow);
}

// After a page turn: focus the new page's first item (or last, going back) and say it.
void FinishPageTurn() {
    if (!g.pageAt || GetTickCount64() - g.pageAt < kPageTurnMs) return;
    Ptr key = g.pageGroup;
    int direction = g.pageDirection;
    Refresh();  // with pageAt still set: the page turn says its own line below, the refresh not
    g.pageAt = 0;
    for (int gi = 0; gi < (int)Groups().size(); gi++) {
        const Group& gr = Groups()[gi];
        if (gr.key != key) continue;
        int first = -1, last = -1;  // the new page's items of the kind the player was on
        for (int pass = 0; pass < 2 && first < 0; pass++)
            for (int i = 0; i < (int)gr.items.size(); i++)
                if (!IsPageArrow(gr.items[i]) && gr.items[i].role != Role::Text &&
                    (pass == 1 || gr.items[i].role == g.pageRole)) {
                    if (first < 0) first = i;
                    last = i;
                }
        if (first < 0) return;
        bool toFirst = g.pageToFirst;
        if (toFirst) direction = 1;  // Left / Right: the new page from its start
        if (g.pageFiltered) {  // the first product of the filter's room, in the direction of travel
            int found = -1;
            for (int i = direction > 0 ? first : last; i >= first && i <= last; i += direction)
                if (IsProduct(gr.items[i]) && MatchesFilter(gr.items[i])) {
                    found = i;
                    break;
                }
            if (found < 0) {
                Ptr arrow = PageArrow(gr, direction);
                if (toFirst) arrow = PageArrow(gr, g.pageDirection);
                if (arrow && ++g.pageTurns < kMaxFilterPages) {
                    SetFocus(gi, first);
                    TurnPage(gr, arrow, toFirst ? g.pageDirection : direction, true, toFirst);
                    return;
                }
                g.pageTurns = 0;
                SetFocus(gi, direction > 0 ? last : first);
                speech::Say(util::Format(S(Str::RoomFilterNoMore), S(kRoomChoices[g_roomChoice].name)));
                return;
            }
            g.pageTurns = 0;
            SetFocus(gi, found);
            buffers::SetDetails(DetailLines(*Focused(), gr, g.index));
            speech::Say(std::string(S(g.pageDirection > 0 ? Str::NextPage : Str::PreviousPage)) + ". " + Describe(*Focused()));
            return;
        }
        SetFocus(gi, direction > 0 ? first : last);
        buffers::SetDetails(DetailLines(*Focused(), gr, g.index));
        speech::Say(std::string(S(g.pageDirection > 0 ? Str::NextPage : Str::PreviousPage)) + ". " + Describe(*Focused()));
        return;
    }
}

// A new root has settled (or a key cannot wait for it): collect it, restore or choose the
// focus and announce it. With `mayWait`, a root that has no widgets yet is left for a later
// tick. A dialog reused by the game for one message after another keeps its pointer, so this
// is also what reads a help tip shown in a dialog seen before.
void CompleteEntering(bool mayWait) {
    ULONGLONG now = GetTickCount64();
    ItemId remembered = g.focus;
    Refresh();
    if (Groups().empty() && mayWait) {
        g.focus = remembered;  // keep what to restore until the widgets exist
        return;
    }
    FinishEntering(remembered);
    if (g.leftPie && g.returning) {
        g.leftPie = false;
        LogScreen();
        AnnounceAfterPie();
        return;
    }
    if (game::IsPieMenu(g.root) && g.autoChoose >= 0 && now < g.autoChooseUntil) {
        int index = g.autoChoose;
        g.autoChoose = -1;
        Log("pie menu: answered with entry %d", index);
        game::PieChoose(g.root, index);
        return;
    }
    if (!g.returning && PrepareScreen(g.root)) {
        g.quietUntil = now + kPrepareQuietMs;
        Refresh();
        FocusDefault();
    }
    AnnounceScreen();
}

// Before acting on a key: the model must describe what is on screen right now. A key that
// arrives while a new screen settles used to finish the entering silently, so a dialog opened
// just before it (Ctrl+H, then Tab) was never read: the screen is announced first, then the
// key acts on it.
void EnsureCurrent() {
    if (g.entering) {
        CompleteEntering(false);
        return;
    }
    Refresh();
}

}  // namespace

void Reset() {
    g = State{};
    speed::Reset();
    buy::Reset();
    build::Reset();
    map::Reset();
    cues::Reset();
    house::Reset();
    camera::Reset();
    tutorial::Reset();
    narrator::Reset();
    buffers::Reset();
}

void Tick() {
    cheats::Tick();  // before a dialog the cheat opened is announced
    narrator::Tick();  // what happens on the lot is heard even while a screen settles
    CheckRootChange();
    ULONGLONG now = GetTickCount64();
    if (g.entering) {
        if (now - g.rootSince < kSettleMs) return;
        CompleteEntering(now - g.rootSince < kEmptyGraceMs);
        return;
    }
    if (now - g.lastCollect > kRefreshMs) Refresh();
    if (FindActive() && !LotViewActive()) FindEnd();  // a dialog came up: the typing is the game's again
    FinishPageTurn();
    CheckPressFeedback();
    if (ShowFocus() && !g_hovered) HoverFocused(Focused());
    speed::Tick(g.root);
    buy::Tick();
    build::Tick();
    map::Tick();
    house::Tick();
    camera::Tick();
    tutorial::Tick();
    text::Tick();
}

bool TextEntryActive() { return g.screen.textEntryVisible; }

int FocusedLotNumber() {
    const Item* it = Focused();
    return it && it->role == Role::Lot ? it->number : -1;
}

uintptr_t FocusedWidget() {
    const Item* it = Focused();
    return it && !IsVirtual(*it) ? (uintptr_t)it->widget : 0;
}

bool LotViewActive() { return game::IsKindOf(game::InputRoot(), "cDDDSimsView"); }


void ChooseInNextMenu(int index) {
    g.autoChoose = index;
    g.autoChooseUntil = GetTickCount64() + kAutoChooseMs;
}

void FocusStatus() {
    EnsureCurrent();
    for (int gi = 0; gi < (int)Groups().size(); gi++)
        if (Groups()[gi].key == kStatusGroup) {
            SetFocus(gi, 0);
            return;
        }
}

bool WorldFocused() {
    const Item* it = Focused();
    return it && it->role == Role::LotObject && LotViewActive();
}

bool PieMenuSeenSince(ULONGLONG time) { return g.pieSeenAt >= time; }

// The item nearest to a rectangle (the tutorial arrow), by the gap between the rectangles; text
// and the arrow's own panel are skipped. Returns its description, and its group's name.
std::string NearestItemTo(const RECT& r, std::string* groupName) {
    EnsureCurrent();
    const Item* best = nullptr;
    const Group* bestGroup = nullptr;
    long bestGap = LONG_MAX;
    for (const Group& gr : Groups()) {
        if (tutorial::IsStepGroup(gr)) continue;
        for (const Item& it : gr.items) {
            if (it.role == Role::Text) continue;
            // Panels listed as one item (the clock stands for the whole control panel) would
            // contain the arrow; the arrow points at a control-sized widget.
            if (it.rect.right - it.rect.left > 120 || it.rect.bottom - it.rect.top > 120) continue;
            long dx = std::max(0L, std::max(it.rect.left - r.right, r.left - it.rect.right));
            long dy = std::max(0L, std::max(it.rect.top - r.bottom, r.top - it.rect.bottom));
            long gap = dx + dy;
            if (gap < bestGap) {
                bestGap = gap;
                best = &it;
                bestGroup = &gr;
            }
        }
    }
    if (!best || bestGap > 40) return "";
    *groupName = bestGroup->items.size() > 1 ? GroupName(*bestGroup) : "";
    return Describe(*best);
}

bool Cancel() {
    Ptr root = game::InputRoot();
    if (!game::IsPieMenu(root)) return false;
    speech::Say(S(Str::Cancelled));
    g.choseInPie = false;
    game::PieCancel(root);
    return true;
}

void Move(int delta) {
    EnsureCurrent();
    if (!Focused()) {
        speech::Say(S(Str::NothingHere));
        return;
    }
    const Group& gr = Groups()[g.group];
    // Past the last item of a page (page arrows come last), or before the first, the list goes
    // on on the next or previous page: the game's page arrow turns it.
    int contentEnd = 0;
    while (contentEnd < (int)gr.items.size() && !IsPageArrow(gr.items[contentEnd])) contentEnd++;
    bool onContent = g.index < contentEnd;
    // Going back, the page turns from the page's first item of this kind (its first product),
    // since the group's first items can be something else (the catalog's subcategories).
    bool firstOfKind = true;
    for (int i = 0; i < g.index; i++) firstOfKind &= gr.items[i].role != gr.items[g.index].role;
    // With a room filter, products of other rooms are skipped (and pages turned) on the way.
    if (delta != 0 && g_roomChoice && IsProduct(*Focused())) {
        int step = delta > 0 ? 1 : -1;
        // The page's products can sit on both sides of other buttons (the catalog's "Atrás",
        // "Otro", "Todos" come between two rows of them in reading order).
        for (int i = g.index + step; i >= 0 && i < contentEnd; i += step)
            if (IsProduct(gr.items[i]) && MatchesFilter(gr.items[i])) {
                SetFocus(g.group, i);
                AnnounceFocus(false);
                return;
            }
        if (Ptr arrow = PageArrow(gr, step)) {
            g.pageTurns = 0;
            TurnPage(gr, arrow, step, true);
            return;
        }
        speech::Say(util::Format(S(Str::RoomFilterNoMore), S(kRoomChoices[g_roomChoice].name)));
        return;
    }
    bool pastEnd = delta > 0 ? g.index + delta >= contentEnd : (g.index + delta < 0 || firstOfKind);
    if (delta != 0 && onContent && pastEnd) {
        if (Ptr arrow = PageArrow(gr, delta)) {
            TurnPage(gr, arrow, delta, false);
            return;
        }
        if (delta > 0) delta = contentEnd - 1 - g.index;  // the last item repeats; arrows stay for End
    }
    int last = (int)gr.items.size() - 1;
    SetFocus(g.group, std::clamp(g.index + delta, 0, last));  // at either end the item repeats
    AnnounceFocus(false);
}

// Left / Right in a paged list of the lot view (buy mode's catalog, build mode's styles): the
// page itself, while Up / Down go item by item (a tester: both pairs did the same and browsing
// 750 products one by one was slow).
bool TurnPageKey(int delta) {
    if (!LotViewActive() || g.group < 0 || !Focused()) return false;
    const Group& gr = Groups()[g.group];
    bool paged = false;
    for (const Item& it : gr.items) paged |= IsPageArrow(it);
    if (!paged || IsPageArrow(*Focused())) return false;
    int step = delta > 0 ? 1 : -1;
    Ptr arrow = PageArrow(gr, step);
    if (!arrow) {
        speech::Say(S(step > 0 ? Str::NoNextPage : Str::NoPreviousPage));
        return true;
    }
    g.pageTurns = 0;
    TurnPage(gr, arrow, step, g_roomChoice != 0 && IsProduct(*Focused()), true);
    g.pageRole = Role::Product;  // the new page's first product, also from a subcategory's tab
    return true;
}

void CycleRoomFilter() {
    g_roomChoice = (g_roomChoice + 1) % kRoomChoiceCount;
    std::string name = S(kRoomChoices[g_roomChoice].name);
    speech::Say(g_roomChoice ? util::Format(S(Str::RoomFilterOn), name.c_str()) : name);
}

void MoveToEdge(bool last) {
    EnsureCurrent();
    if (!Focused()) {
        speech::Say(S(Str::NothingHere));
        return;
    }
    SetFocus(g.group, last ? (int)Groups()[g.group].items.size() - 1 : 0);
    AnnounceFocus(false);
}

void NextGroup(int delta) {
    EnsureCurrent();
    int n = (int)Groups().size();
    if (n == 0) {
        speech::Say(S(Str::NothingHere));
        return;
    }
    int next = g.group < 0 ? 0 : ((g.group + delta) % n + n) % n;
    SetFocus(next, EntryItem(Groups()[next]));
    AnnounceFocus(true);
}

// The first letter of a group's name, lowercase, without its accent ("Catálogo" c, "Ñ" n);
// 0 for a name that starts with anything else (a CJK name).
char FirstLetter(const std::string& utf8) {
    if (utf8.empty()) return 0;
    unsigned char c = (unsigned char)utf8[0];
    if (c < 0x80) return (char)tolower(c);
    if ((c & 0xe0) != 0xc0 || utf8.size() < 2) return 0;
    int cp = ((c & 0x1f) << 6) | ((unsigned char)utf8[1] & 0x3f);
    if (cp < 0xc0 || cp > 0xff) return 0;
    static const char kFold[] = "aaaaaaaceeeeiiiidnooooo/ouuuuyts";  // U+00C0..U+00DF; +0x20 is the lowercase
    char f = kFold[(cp & 0xdf) - 0xc0];
    return f == '/' ? 0 : f;
}

// Ctrl+Shift+E: what a Sim looks like; the Sim focused among the lot's things, else the selected one.
void SayLook() {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    EnsureCurrent();
    const Item* it = Focused();
    Ptr person = 0;
    if (it && it->role == Role::LotObject) {
        Ptr focused = world::FocusedObject();
        if (game::IsKindOf(focused, "cXPerson")) person = focused;
    }
    if (!person) person = game::SelectedSim();
    std::string line = person ? world::SimLookLine(person) : "";
    speech::Say(line.empty() ? S(Str::NoSimSelected) : line);
}

void ToggleShowFocus() {
    g_showFocus = ShowFocus() ? 0 : 1;
    util::SetSetting("show_focus", g_showFocus);
    HoverFocused(Focused());
    speech::Say(S(g_showFocus ? Str::ShowFocusOn : Str::ShowFocusOff));
}

void JumpToGroup(char letter) {
    EnsureCurrent();
    int n = (int)Groups().size();
    for (int step = 1; step <= n; step++) {
        int gi = ((g.group < 0 ? -1 : g.group) + step + n) % n;
        const Group& gr = Groups()[gi];
        // the lot's things go by their own name too while a kind or room is open ("varios")
        bool match = FirstLetter(GroupName(gr)) == letter ||
                     (gr.key == kKindsGroup && FirstLetter(S(Str::ThingsByKind)) == letter) ||
                     (gr.key == kRoomsGroup && FirstLetter(S(Str::ThingsByRoom)) == letter);
        if (!match) continue;
        SetFocus(gi, EntryItem(Groups()[gi]));
        AnnounceFocus(true);
        return;
    }
    for (const Group& gr : Groups()) LogVerbose("group letter %c: not '%s' (key %p)", letter, GroupName(gr).c_str(), (void*)gr.key);
    speech::Say(util::Format(S(Str::NoGroupLetter), std::string(1, (char)toupper(letter)).c_str()));
}

void Activate() {
    EnsureCurrent();
    const Item* it = Focused();
    if (!it) {
        speech::Say(S(Str::NothingHere));
        return;
    }
    // The live panels show what an item means on hover, and the game's texts say to click them
    // (a tester, 2026-11-01: "they do nothing"): Enter speaks that popup.
    std::vector<std::string> help = IsValueControl(*it) ? std::vector<std::string>{} : PanelHelp(*it);
    std::string helpLine;
    for (const std::string& h : help) helpLine += (helpLine.empty() ? "" : " ") + h;
    if (IsReadOnly(*it) || IsValueControl(*it)) {
        if (helpLine.empty()) AnnounceFocus(false);
        else speech::Say(helpLine);
        return;
    }
    if (it->role == Role::Level) {
        // a kind or a room of the lot's things: its things, from the first
        Ptr key = Groups()[g.group].key;
        world::Open(key, it->number);
        g.focus = ItemId{};  // it goes away with its level: no "focus moved" line before the new one
        Refresh();
        for (int gi = 0; gi < (int)Groups().size(); gi++)
            if (Groups()[gi].key == key && !Groups()[gi].items.empty()) SetFocus(gi, 0);
        AnnounceFocus(true);
        return;
    }
    if (it->role == Role::LotObject && !game::InBuyTool() && world::OpenSame(Groups()[g.group].key, it->number)) {
        // identical things merged into one entry: each of them, nearest first
        Ptr key = Groups()[g.group].key;
        g.focus = ItemId{};
        Refresh();
        for (int gi = 0; gi < (int)Groups().size(); gi++)
            if (Groups()[gi].key == key && !Groups()[gi].items.empty()) SetFocus(gi, 0);
        AnnounceFocus(true);
        return;
    }
    if (it->role == Role::LotObject) {  // its interactions; in buy mode: pick it up to move it
        world::Focus(it->number);
        if (game::InBuyTool()) buy::PickUpFocused();
        else world::Interact();
        return;
    }
    if (it->role == Role::ModManual) {
        if (it->number) input::SayKeyHelp();
        else manual::Open();
        return;
    }
    if (it->role == Role::TextField) {  // a click would put the caret where it lands
        text::TakeTyping(it->widget);
        AnnounceFocus(false);
        return;
    }
    if (it->role == Role::PieItem) {
        Log("pie menu: choose %d (%s)", it->number, it->label.c_str());
        g.choseInPie = true;
        game::PieChoose(it->widget, it->number);
        return;
    }
    Ptr w = it->role == Role::CheckBox ? game::CheckBoxButton(it->widget) : it->widget;
    // The selected mode tab pressed again hides the whole panel (the game's F1 toggle): a trap
    // for a player who cannot see the panel vanish. The press still happens (the tutorial waits
    // for it), and the panel is pressed back when it went away.
    bool reshow = it->role == Role::Tab && game::IsModeButton(w) && game::ButtonChecked(w) && game::ModePanelShown(g.root);
    bool isButton = game::IsKindOf(w, "cTSWinBtn");
    if (isButton && !game::ButtonEnabled(w)) {
        speech::Say(S(Str::Unavailable));
        return;
    }
    if (it->role == Role::QueuedAction) simwatch::NoteCancelled(it->label);  // not a refusal
    if (it->role == Role::Tab && game::IsKindOf(game::Parent(w), "cWinArch")) buy::HoldOff(kToolTabHoldMs);
    if (it->role == Role::Lot && game::TravelMap(g.root)) narrator::NoteDestination(LotPlaceName(*it));
    g.press = PendingPress{IdOf(*it), CheckedState(*it), GetTickCount64(), TextSnapshot(), reshow};
    Log("press %s %p (%s)", game::ClassName(w).c_str(), (void*)w, it->label.c_str());
    bool sayHelp = !helpLine.empty() && it->role != Role::Button;  // a need bar, skill, job...
    g.press.helpSpoken = sayHelp;
    game::Press(w);  // may open a dialog or change screens (and run ticks): `it` may be gone after
    if (sayHelp) speech::Say(helpLine);
}

// "4 de 10, quedan 21 puntos". When nothing changed the game refused (no points left, or the
// bar is at an end) and the player hears why.
bool Adjust(int delta) {
    EnsureCurrent();
    const Item* it = Focused();
    if (it && it->role == Role::Spinner) {
        Ptr arrow = delta > 0 ? it->next : it->previous;
        g.press = PendingPress{IdOf(*it), false, GetTickCount64()};
        game::Press(arrow);
        game::Unhover(arrow);
        return true;
    }
    if (it && it->role == Role::Slider) {  // "9 de 10"
        game::SliderState s = game::ReadSlider(it->widget);
        game::SetSlider(it->widget, s.value + delta);
        s = game::ReadSlider(it->widget);
        speech::Say(util::Format(S(Str::ValueOfMax), s.value - s.min, s.max - s.min));
        return true;
    }
    if (!it || it->role != Role::TraitBar) return false;
    Ptr bar = it->widget;
    int before = game::TraitBarPoints(bar);
    game::AdjustTraitBar(bar, delta);
    int after = game::TraitBarPoints(bar);
    int left = game::PersonalityPointsLeft(bar);
    if (after == before && delta > 0 && left == 0) {
        speech::Say(S(Str::NoPointsLeft));
        return true;
    }
    std::string line = util::Format(S(Str::OutOfTen), after);
    if (left == 0) line += std::string(", ") + S(Str::NoPointsLeft);
    else if (left > 0) line += ", " + util::Format(S(left == 1 ? Str::PointLeft : Str::PointsLeft), left);
    speech::Say(line);
    return true;
}

void RepeatFocus() {
    EnsureCurrent();
    AnnounceFocus(true);
}

// Buy mode, focus in the catalog: a category's subsorts back to the categories, as the game's "Atrás" does (it sits
// after the subsorts, and neither Backspace nor Escape reached it: a tester, 2026-10-16).
bool PressCatalogBack() {
    Ptr back = game::CatalogBackButton();
    if (!back || !LotViewActive()) return false;
    int group = -1, index = -1;
    for (int gi = 0; gi < (int)Groups().size() && group < 0; gi++)
        for (int ii = 0; ii < (int)Groups()[gi].items.size(); ii++)
            if (Groups()[gi].items[ii].widget == back) {
                group = gi;
                index = ii;
                break;
            }
    if (group != g.group) return false;  // only from the catalog: elsewhere Backspace is another level
    SetFocus(group, index);
    Activate();
    return true;
}

bool Back() {
    EnsureCurrent();
    if (g.group < 0 || g.group >= (int)Groups().size()) return false;
    Ptr key = Groups()[g.group].key;
    int was = world::Close(key);
    if (!was) return PressCatalogBack();
    g.focus = ItemId{};  // the thing goes away with its level: no "focus moved" line first
    Refresh();  // back to the kinds or rooms, on the one that was open
    for (int gi = 0; gi < (int)Groups().size(); gi++) {
        if (Groups()[gi].key != key) continue;
        int at = 0;
        for (int ii = 0; ii < (int)Groups()[gi].items.size(); ii++)
            if (Groups()[gi].items[ii].number == was) at = ii;
        SetFocus(gi, at);
    }
    AnnounceFocus(true);
    return true;
}

// ---- Find (Ctrl+F) ---------------------------------------------------------------------------

namespace {

struct FindState {
    bool active = false;
    std::string query;       // UTF-8
    bool listed = false;     // the matches are the open list of the kinds group
    unsigned char lead = 0;  // the first byte of a double-byte character
    // Where things were, for Escape.
    ItemId before;
    int beforeKind = 0;
    Ptr beforeLast = 0;
};

FindState g_find;

// Back to the list and the focus from before the search.
void FindRestore() {
    world::Open(kKindsGroup, g_find.beforeKind);
    world::Open(g_find.beforeLast, world::OpenNumber(g_find.beforeLast));  // PageDown's list as it was
    world::ClearFindQuery();
    g_find.listed = false;
    g.focus = ItemId{};
    Refresh();
    int gi, ii;
    if (g_find.before && Locate(g_find.before, &gi, &ii)) SetFocus(gi, ii);
}

// The matches for the query, focus on the best: "tel: 2 encontrados. Teléfono ..."
void FindShow() {
    int count = world::SetFindQuery(g_find.query);
    if (!count) {
        speech::Say(util::Format(S(Str::FindNoMatch), g_find.query.c_str()));
        return;
    }
    world::Open(kKindsGroup, world::kFindKind);
    g_find.listed = true;
    g.focus = ItemId{};  // no "focus moved" line before the new one
    Refresh();
    for (int gi = 0; gi < (int)Groups().size(); gi++) {
        if (Groups()[gi].key != kKindsGroup || Groups()[gi].items.empty()) continue;
        SetFocus(gi, 0);
        buffers::SetDetails(DetailLines(*Focused(), Groups()[gi], 0));
        std::string found = count == 1 ? util::Format(S(Str::FindCountOne), g_find.query.c_str())
                                       : util::Format(S(Str::FindCount), g_find.query.c_str(), count);
        speech::Say(found + ". " + Describe(*Focused()));
        return;
    }
}

}  // namespace

bool FindActive() { return g_find.active; }

void FindBegin() {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    if (buy::Placing() || build::Active()) {
        speech::Say(S(Str::FindNotNow));
        return;
    }
    if (g_find.active) {  // the keys again
        speech::Say(S(Str::FindPrompt));
        return;
    }
    EnsureCurrent();
    g_find = FindState{};
    g_find.active = true;
    g_find.before = g.focus;
    g_find.beforeKind = world::OpenNumber(kKindsGroup);
    g_find.beforeLast = world::LastGroup();
    speech::Say(S(Str::FindPrompt));
}

void FindChar(WPARAM ch) {
    if (ch < 0x20 || ch >= 0x100) {
        g_find.lead = 0;
        return;
    }
    unsigned char byte = (unsigned char)ch;
    std::string raw;
    if (g_find.lead) {
        raw = {(char)g_find.lead, (char)byte};
        g_find.lead = 0;
    } else if (util::IsGameLeadByte(byte)) {
        g_find.lead = byte;
        return;
    } else {
        raw = std::string(1, (char)byte);
    }
    if (raw == " " && g_find.query.empty()) return;
    g_find.query += util::GameTextToUtf8(raw);
    FindShow();
}

void FindErase() {
    std::string& q = g_find.query;
    if (q.empty()) {
        speech::Say(S(Str::FindEmpty));
        return;
    }
    size_t at = q.size() - 1;
    while (at > 0 && ((unsigned char)q[at] & 0xc0) == 0x80) at--;  // a whole UTF-8 character
    q.resize(at);
    if (!q.empty()) {
        FindShow();
        return;
    }
    if (g_find.listed) FindRestore();
    speech::Say(S(Str::FindEmpty));
}

void FindMove(int delta) {
    if (!g_find.listed) {
        speech::Say(S(Str::FindTypeFirst));
        return;
    }
    Move(delta);
}

void FindEdge(bool last) {
    if (!g_find.listed) {
        speech::Say(S(Str::FindTypeFirst));
        return;
    }
    MoveToEdge(last);
}

void FindConfirm(bool walk) {
    if (!g_find.listed) {
        speech::Say(S(Str::FindTypeFirst));
        return;
    }
    g_find.active = false;
    if (walk) world::WalkToFocus();
    else Activate();
}

void FindCancel() {
    g_find.active = false;
    if (g_find.listed) FindRestore();
    speech::Say(S(Str::FindCancelled));
}

void FindEnd() { g_find.active = false; }

// PageUp/PageDown (nav/world.h): the lot's things from anywhere in the lot view. A kind or room
// is opened in its group and focus goes there, so Tab, the arrows, Enter and Backspace go on from
// it as usual.
void BrowseThings(Browse how, int delta) {
    EnsureCurrent();
    Ptr key = how == Browse::Kind ? kKindsGroup : how == Browse::Room ? kRoomsGroup : world::LastGroup();
    const Item* it = Focused();
    bool inGroup = it && Groups()[g.group].key == key;
    if (how == Browse::Thing && inGroup && it->role == Role::LotObject) {
        Move(delta);
        return;
    }
    // A thing: from the kind or room focused in its list, else the one open, else the first.
    int number = how != Browse::Thing ? world::Step(key, delta)
                 : inGroup && it->role == Role::Level ? it->number
                 : world::OpenNumber(key) ? world::OpenNumber(key)
                                          : world::Step(key, +1);
    // Twice: the kind or room left open may have emptied since (its group is then gone).
    for (int attempt = 0; attempt < 2 && number; attempt++) {
        bool same = number == world::OpenNumber(key);
        world::Open(key, number);
        g.focus = ItemId{};  // what was focused may go with its level: no "focus moved" line first
        Refresh();
        for (int gi = 0; gi < (int)Groups().size(); gi++) {
            const Group& gr = Groups()[gi];
            if (gr.key != key || gr.items.empty()) continue;
            int at = 0;  // back in the same kind or room: on the thing focused last
            for (int ii = 0; same && ii < (int)gr.items.size(); ii++)
                if (gr.items[ii].number == world::FocusedId()) at = ii;
            SetFocus(gi, at);
            AnnounceFocus(true);
            return;
        }
        world::Close(key);
        number = world::Step(key, delta);
    }
    speech::Say(S(Str::NoObjects));
}

}  // namespace nav
