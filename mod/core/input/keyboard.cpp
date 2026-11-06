#include "keyboard.h"

#include <cstring>
#include <string>

#include "../game/buy.h"
#include "../game/hooks.h"
#include "../game/pie.h"
#include "../game/ui.h"
#include "../nav/buffers.h"
#include "../nav/build_mode.h"
#include "../nav/buy_mode.h"
#include "../nav/camera.h"
#include "../nav/cheat_console.h"
#include "../nav/cues.h"
#include "../nav/map_mode.h"
#include "../nav/game_speed.h"
#include "../nav/narrator.h"
#include "../nav/room_layout.h"
#include "../nav/house_overview.h"
#include "../nav/manual.h"
#include "../nav/navigator.h"
#include "../nav/text_entry.h"
#include "../nav/tutorial.h"
#include "../speech/speech.h"
#include "../util/strings.h"
#include "../nav/world.h"
#include "../util/log.h"

namespace input {
namespace {

// Keys the mod owns (none of them is a game hotkey; see CLAUDE.md "Game key bindings"). One
// grammar everywhere (NAVIGATION.md): groups on Tab, items on the arrows, levels on Enter and
// Backspace; only the spatial tools (placing, the build cursor) use the arrows for space.
//   Up/Left, Down/Right            previous/next item in the group (the lot view's HUD too)
//                                  on a slider, value picker or personality bar: Left/Right change it
//   Home, End                      first/last item in the group
//   Tab, Shift+Tab                 next/previous group (the lot view: status, needs, family, action
//                                  queue, things by kind, things by room, then the game's panels)
//   Space                          lot view: the game's next-Sim key (the selection is announced)
//   Enter, Space                   press the focused item; a kind or a room of the lot's things
//                                  opens its things; a thing opens its interaction menu (buy mode:
//                                  picks it up) (Space is typing while a text field is shown)
//   Backspace                      back up a level (a kind's or room's things to the kinds or rooms)
//   R                              repeat the focused item (typing while a text field is shown)
//   on the text field that takes typing (focus on it gives it the typing): Left/Right (Ctrl: by
//   word), Home/End and Delete go to the field and the character at the caret is spoken
//   Ctrl+Up / Ctrl+Down            review buffers (nav/buffers.h): the next / previous line of the
//                                  current buffer (details of what was spoken, or events)
//   Ctrl+Left / Ctrl+Right         previous / next review buffer
//   PageUp/PageDown                a value control (slider, value picker, personality bar): one
//                                  step up/down, like Right/Left
//                                  lot view (every mode): previous/next thing of the kind or room
//                                  chosen last (nav/world.h)
//   Shift+PageUp/PageDown          lot view: previous/next kind of things
//   Ctrl+PageUp/PageDown           lot view: previous/next room
//   buy mode, while placing an object (nav/buy_mode.h, a spatial tool): arrows move it (Shift
//   five), PageDown/PageUp (or Tab/Shift+Tab) the spots, Ctrl+ the rooms, Shift+ (or comma/period)
//   turn it, Enter buys, Delete sells, Escape cancels, R repeats
//   the build cursor (nav/build_mode.h, a spatial tool): arrows move (Shift five), Tab/Shift+Tab
//   (or Ctrl+PageDown/PageUp) the rooms, PageUp/PageDown the floors, comma/period the ready-made
//   room sizes, Insert builds one, Space marks, Enter builds, Delete or Ctrl+Enter removes, Home,
//   R, Escape
//   the live-mode map (nav/map_mode.h, a spatial tool): Ctrl+M on/off; arrows move a tile (Shift
//   five), Home back to the Sim, Ctrl+Shift+PageUp/PageDown the floors, Enter the menu of what
//   stands there, Ctrl+Enter walk there, R, Escape; the Page keys browse and the map follows
//   Ctrl+Shift+S                   the cursors' sound cues on/off (nav/cues.h)
//   Ctrl+Enter                     lot view: walk to the thing last focused
//   Escape                         close the interaction menu (otherwise the game's Escape)
//   Alt+letter                     the next group whose name starts with that letter (Alt+N needs,
//                                  Alt+F family, Alt+O the lot's things...: the names are spoken
//                                  in the game's language, so the letters follow it)
//   Ctrl+Shift+V                   light the focused control on screen for a sighted helper (on/off)
//   Ctrl+F1                        say the mod's keys for this screen
//   Ctrl+Shift+F1                  open the manual (nav/manual.h) in the browser, in the game's
//                                  language; so does the "mod help" group's button on every screen
//   Ctrl+1 .. Ctrl+9               lot view: select the n-th Sim of the family (its portrait)
//   Ctrl+F                         lot view: find a thing by typing part of its name; while it
//                                  is on, typing and Backspace edit the search, Up/Down (Page keys,
//                                  Home/End) go through the matches, Enter acts on one, Ctrl+Enter
//                                  walks there, Escape goes back (nav/navigator.h)
//   Ctrl+Shift+F                   lot view: follow the selected Sim with the camera on/off
//   Delete                         on a door or window of the lot's things (buy or build mode):
//                                  removes it, what the game does on Ctrl+click
//   Ctrl+H                         lot view: open the help system's waiting tip
//   Ctrl+E                         lot view: the selected Sim's state (room, actions, needs, mood)
//   Ctrl+N                         the narrator's verbosity: everything, warnings only, off
//   Ctrl+L                         lot view: the room's layout (the build cursor's, else the Sim's)
//   Ctrl+Shift+L                   the house: floors, rooms, ratings, what is missing or wrong
//   Ctrl+Shift+C                   the game's cheat console (nav/cheat_console.h): Up/Down/Home/End pick
//                                  a cheat, typing edits the line, Enter runs it (again: repeats)

bool g_consumed[256] = {};  // key-downs we took; their key-ups are taken too
WPARAM g_swallowChar = 0;   // character TranslateMessage will produce from the last claimed key

// Keys posted by tools/keys.py carry this reserved lParam bit. The keyboard state does not follow
// posted messages, so their modifiers are tracked from their own messages; real keys read the
// physical keyboard, which cannot go stale when a key-up goes to another window (a system
// shortcut such as Ctrl+Win+Alt did exactly that).
constexpr LPARAM kSyntheticKey = 1 << 25;
bool g_syntheticDown[256] = {};
bool g_synthetic = false;  // the key being handled was posted by the tool
bool Held(int vk) {
    return g_synthetic ? g_syntheticDown[vk] : (game::RealGetAsyncKeyState(vk) & 0x8000) != 0;
}
bool IsModifier(WPARAM vk) { return vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU; }

}  // namespace

// The keys that work where the player is.
void SayKeyHelp() {
    if (game::IsPieMenu(game::InputRoot())) speech::Say(S(Str::KeysMenu));
    else if (nav::cheats::Entering()) speech::Say(S(Str::CheatHint));
    else if (nav::buy::Placing()) speech::Say(std::string(S(Str::KeysPlacing)) + " " + S(Str::KeysPlacingFree) + " " + S(Str::KeysPlacingWalls) + " " + S(Str::KeysPlacingThings) + " " + S(Str::KeysCuesHint));
    else if (nav::FindActive()) speech::Say(S(Str::FindPrompt));
    else if (nav::LotViewActive() && game::InBuyTool()) speech::Say(std::string(S(Str::KeysBuy)) + " " + S(Str::KeysRoomFilter) + " " + S(Str::KeysFind) + " " + S(Str::KeysPortalDelete) + " " + S(Str::KeysLayoutHint) + " " + S(Str::KeysMapHint));
    else if (nav::LotViewActive()) speech::Say(std::string(S(Str::KeysLot)) + " " + S(Str::KeysLotMore) + " " + S(Str::KeysFind) + " " + S(Str::KeysSimCard) + " " + S(Str::KeysLayoutHint) + " " + S(Str::KeysMapHint));
    else if (nav::FocusedLotNumber() >= 0) speech::Say(std::string(S(Str::KeysScreen)) + " " + S(Str::KeysHouseFile));
    else speech::Say(S(Str::KeysScreen));
    speech::Say(std::string(S(Str::KeysGroupLetter)) + " " + S(Str::KeysShowFocus) + " " + S(Str::KeysShowFocusThings) + " " + S(Str::KeysManual));
}

namespace {

// Up/Down and Left/Right move through the group, the lot view's HUD included; Left/Right
// change a slider or value picker instead of leaving it.
void Arrow(bool vertical, int delta) {
    if (!vertical && nav::Adjust(delta)) return;
    nav::Move(delta);
}

// Finding a thing (Ctrl+F) owns the keyboard while it is on: what types goes to the search through
// its character. Ctrl combinations (the review buffers...) and the game's function keys go on as
// usual.
bool FindKey(WPARAM vk, bool shift, bool ctrl) {
    if (ctrl) {
        if (vk != VK_RETURN) return false;
        g_swallowChar = 0x0a;
        nav::FindConfirm(true);
        return true;
    }
    switch (vk) {
    case VK_UP: case VK_LEFT: case VK_PRIOR: nav::FindMove(-1); return true;
    case VK_DOWN: case VK_RIGHT: case VK_NEXT: nav::FindMove(+1); return true;
    case VK_HOME: nav::FindEdge(false); return true;
    case VK_END: nav::FindEdge(true); return true;
    case VK_RETURN: g_swallowChar = '\r'; nav::FindConfirm(false); return true;
    case VK_ESCAPE: g_swallowChar = 0x1b; nav::FindCancel(); return true;
    case VK_TAB: case VK_DELETE: nav::FindEnd(); return false;  // the groups, removing it: the matches stay
    }
    if (vk >= VK_F1 && vk <= VK_F12) {
        nav::FindEnd();
        return false;
    }
    (void)shift;
    return true;
}

// Placing an object (buy mode) is a spatial tool: it owns these keys while it holds one.
bool PlacingKey(WPARAM vk, bool shift, bool ctrl) {
    namespace b = nav::buy;
    if (vk == VK_NEXT || vk == VK_PRIOR) {  // the spots, as the lot's things: Ctrl the rooms, Shift the turns
        int delta = vk == VK_NEXT ? +1 : -1;
        if (ctrl && shift) b::NextThing(delta);  // beside the lot's things, nearest first
        else if (ctrl) b::NextArea(delta);
        else if (shift) b::NextTurn(delta);
        else b::NextSpot(delta);
        return true;
    }
    if (ctrl) return false;
    int n = shift ? 5 : 1;
    switch (vk) {
    case VK_UP: b::MoveFree(0, -n); return true;
    case VK_DOWN: b::MoveFree(0, n); return true;
    case VK_LEFT: b::MoveFree(-n, 0); return true;
    case VK_RIGHT: b::MoveFree(n, 0); return true;
    case VK_TAB: g_swallowChar = '\t'; b::NextSpot(shift ? -1 : +1); return true;
    case VK_OEM_COMMA: case VK_OEM_PERIOD:  // the game's own rotate keys, turning what is held
        g_swallowChar = vk == VK_OEM_COMMA ? (shift ? '<' : ',') : (shift ? '>' : '.');
        b::NextTurn(vk == VK_OEM_PERIOD ? +1 : -1);
        return true;
    case VK_RETURN: g_swallowChar = '\r'; b::Confirm(); return true;
    case 'R': g_swallowChar = 'r'; b::Repeat(); return true;
    }
    return false;
}

// The build cursor (nav/build_mode.h) owns these keys while it is on; the rest leave it.
bool BuildCursorKey(WPARAM vk, bool shift, bool ctrl) {
    namespace b = nav::build;
    int step = shift ? 5 : 1;
    switch (vk) {
    case VK_UP: if (ctrl) return false; b::Move(0, -step); return true;
    case VK_DOWN: if (ctrl) return false; b::Move(0, step); return true;
    case VK_LEFT: if (ctrl) return false; b::Move(-step, 0); return true;
    case VK_RIGHT: if (ctrl) return false; b::Move(step, 0); return true;
    case VK_SPACE: g_swallowChar = ' '; b::MarkStart(); return true;
    case VK_RETURN: g_swallowChar = ctrl ? 0x0a : 0x0d; b::Build(shift, ctrl); return true;
    case VK_DELETE: b::Build(false, true); return true;
    case VK_NEXT: if (ctrl) b::NextRoom(+1); else b::ChangeFloor(-1); return true;   // Ctrl: the rooms, as
    case VK_PRIOR: if (ctrl) b::NextRoom(-1); else b::ChangeFloor(+1); return true;  // the lot's things
    case VK_TAB: g_swallowChar = '\t'; b::NextRoom(shift ? -1 : +1); return true;  // its targets: the rooms
    case VK_OEM_COMMA: case VK_OEM_PERIOD:  // what it holds: the ready-made room sizes
        g_swallowChar = vk == VK_OEM_COMMA ? (shift ? '<' : ',') : (shift ? '>' : '.');
        b::NextPreset(vk == VK_OEM_PERIOD ? +1 : -1);
        return true;
    case VK_INSERT: b::BuildPreset(); return true;
    case VK_HOME: b::Home(); return true;
    case 'R': if (ctrl) return false; g_swallowChar = 'r'; b::Repeat(); return true;
    case VK_ESCAPE: g_swallowChar = 0x1b; b::End(); return true;
    case VK_F1: if (!ctrl || shift) return false; speech::Say(std::string(S(b::TerrainCursor() ? Str::KeysTerrainCursor : b::PoolCursor() ? Str::KeysPoolCursor : b::PaperCursor() ? Str::KeysPaperCursor : b::FloorCursor() ? Str::KeysFloorCursor : Str::KeysBuildCursor)) + " " + S(Str::KeysRoomJump) + " " + S(Str::KeysFloorChange) + " " + S(Str::KeysLayoutHint) + " " + S(Str::KeysCuesHint)); return true;
    }
    return false;
}

// The live-mode map (nav/map_mode.h) owns these keys while it is on; the rest work as usual
// (the Page keys browse the lot's things and the map goes to the thing).
bool MapKey(WPARAM vk, bool shift, bool ctrl) {
    namespace m = nav::map;
    int step = shift ? 5 : 1;
    switch (vk) {
    case VK_UP: if (ctrl) return false; m::Move(0, -step); return true;
    case VK_DOWN: if (ctrl) return false; m::Move(0, step); return true;
    case VK_LEFT: if (ctrl) return false; m::Move(-step, 0); return true;
    case VK_RIGHT: if (ctrl) return false; m::Move(step, 0); return true;
    case VK_HOME: if (ctrl) return false; m::Home(); return true;
    case VK_NEXT: case VK_PRIOR:
        if (!ctrl || !shift) return false;  // the lot's things, below
        m::ChangeFloor(vk == VK_PRIOR ? +1 : -1);
        return true;
    case VK_RETURN:
        g_swallowChar = ctrl ? 0x0a : 0x0d;
        if (ctrl) m::WalkHere();
        else m::Activate();
        return true;
    case 'R': if (ctrl || shift) return false; g_swallowChar = 'r'; m::Repeat(); return true;
    case VK_OEM_COMMA: case VK_OEM_PERIOD:  // the layer each step says
        if (ctrl) return false;
        g_swallowChar = vk == VK_OEM_COMMA ? (shift ? '<' : ',') : (shift ? '>' : '.');
        m::NextLayer(vk == VK_OEM_PERIOD ? +1 : -1);
        return true;
    case VK_ESCAPE: g_swallowChar = 0x1b; m::End(); return true;
    case VK_F1: if (!ctrl || shift) return false; speech::Say(std::string(S(Str::KeysMap)) + " " + S(Str::KeysMapMore)); return true;  // Shift: the manual
    }
    return false;
}

// Runs the action for a key; false when the key is not the mod's in this situation. The
// character to swallow is set before the action runs: an action can open a menu that runs
// modally (and processes that character) before returning.
bool Dispatch(WPARAM vk, bool shift, bool ctrl) {
    bool typing = nav::TextEntryActive();
    if (nav::FindActive() && FindKey(vk, shift, ctrl)) return true;
    if (nav::build::Active() && !typing && BuildCursorKey(vk, shift, ctrl)) return true;
    if (!typing && nav::LotViewActive() && nav::buy::Placing() && PlacingKey(vk, shift, ctrl)) return true;
    if (!typing && nav::map::Active() && nav::LotViewActive() && MapKey(vk, shift, ctrl)) return true;
    // On the field that takes typing, Left/Right/Home/End/Delete edit it (the field moves its caret).
    if (typing && !nav::cheats::Entering() && nav::text::OnCaretKey(vk, ctrl, nav::FocusedWidget())) return false;
    switch (vk) {
    case VK_UP:
        if (ctrl) nav::buffers::Deeper();
        else if (nav::cheats::Entering()) nav::cheats::Select(-1);
        else Arrow(true, -1);
        return true;
    case VK_DOWN:
        if (ctrl) nav::buffers::TowardTop();
        else if (nav::cheats::Entering()) nav::cheats::Select(+1);
        else Arrow(true, +1);
        return true;
    case VK_LEFT: if (ctrl) nav::buffers::Switch(-1); else Arrow(false, -1); return true;
    case VK_RIGHT: if (ctrl) nav::buffers::Switch(+1); else Arrow(false, +1); return true;
    case VK_HOME: case VK_END:
        if (nav::cheats::Entering()) nav::cheats::SelectEdge(vk == VK_END);
        else nav::MoveToEdge(vk == VK_END);
        return true;
    case VK_TAB:
        g_swallowChar = '\t';
        nav::NextGroup(shift ? -1 : +1);
        return true;
    case VK_PRIOR: case VK_NEXT: {
        int delta = vk == VK_NEXT ? +1 : -1;
        if (!ctrl && !shift && nav::Adjust(-delta)) return true;  // a value control's step, like Right/Left
        if (!nav::LotViewActive()) return false;
        nav::BrowseThings(ctrl ? nav::Browse::Room : shift ? nav::Browse::Kind : nav::Browse::Thing, delta);
        nav::map::FollowFocus();  // the map, when it is on, goes to the thing
        return true;
    }
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        if (!ctrl) return false;
        g_swallowChar = vk;  // a posted digit still makes its character (a game speed key)
        nav::camera::SelectSim((int)vk - '1', shift);  // Ctrl+Shift: the Sim's card
        return true;
    case 'H':  // Ctrl+H: the help system's waiting tip
        if (!ctrl || shift) return false;
        g_swallowChar = 'h';
        nav::tutorial::OpenTip();
        return true;
    case 'E':  // Ctrl+E the selected Sim's state, Ctrl+Shift+E what the Sim looks like
        if (!ctrl) return false;
        g_swallowChar = 'e';
        if (shift) nav::SayLook();
        else nav::narrator::SayStatus();
        return true;
    case 'L':  // Ctrl+L the room's layout, Ctrl+Shift+L the whole house
        if (!ctrl) return false;
        g_swallowChar = 'l';
        if (shift) nav::house::SayOverview();
        else nav::layout::SayRoomLayout();
        return true;
    case 'M':  // Ctrl+M: the live-mode map
        if (!ctrl || shift) return false;
        g_swallowChar = 'm';
        nav::map::Toggle();
        return true;
    case 'S':  // Ctrl+Shift+S: the cursors' sound cues (Ctrl+S alone is the game's save)
        if (!ctrl || !shift) return false;
        g_swallowChar = 's';
        nav::cues::Toggle();
        return true;
    case 'N':  // Ctrl+N the narrator's verbosity, Ctrl+Shift+N whose conversations it says
        if (!ctrl) return false;
        g_swallowChar = 'n';
        if (shift) nav::narrator::CycleSocial();
        else nav::narrator::CycleVerbosity();
        return true;
    case 'T':  // Ctrl+T: every new hour of the clock spoken, or not
        if (!ctrl || shift) return false;
        g_swallowChar = 't';
        nav::speed::ToggleHours();
        return true;
    case 'F':  // Ctrl+F finds a thing, Ctrl+Shift+F the camera's following
        if (!ctrl) return false;
        g_swallowChar = 'f';
        if (shift) nav::camera::ToggleFollow();
        else nav::FindBegin();
        return true;
    case 'V':  // Ctrl+Shift+V: show the focus on screen (Ctrl+V alone is the game's paste)
        if (!ctrl || !shift) return false;
        g_swallowChar = 'v';
        nav::ToggleShowFocus();
        return true;
    case 'C':  // Ctrl+Shift+C: the cheat console, opened the way the game's own key does
        if (!ctrl || !shift) return false;
        g_swallowChar = 'c';  // Ctrl+C comes as 0x03, a posted one as 'c': neither may reach the console
        nav::cheats::Open();
        return true;
    case VK_SPACE:
        if (typing) return false;
        // In the lot view Space is the game's own key: the next Sim of the family becomes the
        // selected one (the camera watcher says who); Enter presses.
        if (nav::LotViewActive() && !nav::cheats::Entering()) return false;
        [[fallthrough]];
    case VK_RETURN:
        if (vk == VK_RETURN && nav::cheats::Entering()) {  // runs the line; the console stays for repeats
            g_swallowChar = '\r';
            nav::cheats::Run();
            return true;
        }
        g_swallowChar = vk == VK_RETURN ? (ctrl ? '\n' : '\r') : ' ';  // Ctrl+Enter types a line feed
        if (ctrl) {  // walk to the thing last focused
            if (!nav::LotViewActive() || !nav::world::HasFocus()) return false;
            nav::world::WalkToFocus();
        } else {
            uintptr_t pressed = nav::FocusedWidget();
            bool product = pressed && game::IsProductButton(pressed);
            nav::Activate();
            if (product) {
                nav::buy::Renew();
                nav::build::Begin(pressed);  // a wall or fence style: the build cursor
            }
        }
        return true;
    case VK_DELETE:  // the game sells a held object too, but says nothing
        if (!nav::buy::Placing()) return nav::LotViewActive() && nav::WorldFocused() && nav::buy::RemoveFocusedPortal();
        nav::buy::Sell();
        return true;
    case VK_F1:
        if (!ctrl) return false;  // F1 alone is the game's Live mode
        if (shift) nav::manual::Open();
        else SayKeyHelp();
        return true;
    case VK_ESCAPE:
        g_swallowChar = 0x1b;
        if (nav::Cancel() || nav::buy::Cancel() || nav::tutorial::GuardEscape()) return true;
        g_swallowChar = 0;
        return false;
    case 'R':
        if (typing || shift) return false;
        g_swallowChar = 'r';
        if (ctrl && nav::LotViewActive() && game::InBuyTool()) {
            nav::CycleRoomFilter();
            return true;
        }
        nav::RepeatFocus();
        return true;
    default:
        return false;
    }
}

}  // namespace

bool OnKeyDown(WPARAM vk, LPARAM lp) {
    if (vk >= 256) return false;
    LogVerbose("key down vk=0x%02x lparam=0x%08lx", (unsigned)vk, (unsigned long)lp);
    g_synthetic = (lp & kSyntheticKey) != 0;
    if (IsModifier(vk)) {
        if (g_synthetic) g_syntheticDown[vk] = true;
        return false;
    }
    if (vk == VK_BACK) {
        if (nav::FindActive()) {
            nav::FindErase();
            g_consumed[vk] = true;
            g_swallowChar = 0x08;
            return true;
        }
        if (nav::TextEntryActive()) {  // typing: the game gets the key, the mod only speaks what it deletes
            nav::text::OnBackspace();
            return false;
        }
        if (Held(VK_CONTROL) || Held(VK_MENU)) return false;
        // Up a level (NAVIGATION.md); at the top of the lot view the key says where the focus is
        // (the game's own Backspace would pause it).
        if (!nav::Back()) {
            if (!nav::LotViewActive()) return false;
            nav::RepeatFocus();
        }
        g_consumed[vk] = true;
        g_swallowChar = 0x08;
        return true;
    }
    bool ctrl = Held(VK_CONTROL);
    bool ctrlKey = vk == VK_PRIOR || vk == VK_NEXT || vk == VK_RETURN || vk == VK_F1 || vk == VK_UP ||
                   vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT || (vk >= '1' && vk <= '9') || vk == 'F' || vk == 'H' || vk == 'E' || vk == 'N' || vk == 'T' || vk == 'L' || vk == 'R' || vk == 'M' || vk == VK_DELETE ||
                   ((vk == 'C' || vk == 'V' || vk == 'S') && Held(VK_SHIFT));  // the mod's Ctrl combinations
    if (Held(VK_MENU) || (ctrl && !ctrlKey)) {
        LogVerbose("key 0x%02x left to the game: %s held", (unsigned)vk, ctrl ? "Ctrl" : "Alt");
        return false;
    }
    g_consumed[vk] = true;  // before the action: its key-up may arrive while a menu runs inside it
    if (Dispatch(vk, Held(VK_SHIFT), ctrl)) return true;
    g_consumed[vk] = false;
    return false;
}

bool OnKeyUp(WPARAM vk, LPARAM lp) {
    if (vk >= 256) return false;
    if (IsModifier(vk)) {
        if (lp & kSyntheticKey) g_syntheticDown[vk] = false;
        return false;
    }
    if (!g_consumed[vk]) return false;
    g_consumed[vk] = false;
    return true;
}

bool OnChar(WPARAM ch) {
    // A letter's character comes in either case, or as its control code when Ctrl is held.
    bool letter = g_swallowChar >= 'a' && g_swallowChar <= 'z';
    bool swallow = g_swallowChar && (ch == g_swallowChar || (letter && (ch == g_swallowChar - 32 || ch == (g_swallowChar & 0x1f))));
    g_swallowChar = 0;
    if (swallow) return true;
    if (nav::FindActive()) {
        nav::FindChar(ch);
        return true;
    }
    nav::text::OnChar(ch);
    return false;
}

bool g_swallowSysChar = false;

bool OnSysKeyDown(WPARAM vk, LPARAM) {
    // Alt+arrows: the map's or placing's cursor through a wall (the plain arrows stop at walls).
    if (vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT) {
        bool placing = nav::LotViewActive() && nav::buy::Placing() && !nav::TextEntryActive();
        bool map = nav::map::Active() && nav::LotViewActive() && !nav::TextEntryActive();
        if (!placing && !map) return false;
        int n = Held(VK_SHIFT) ? 5 : 1;
        int dx = vk == VK_LEFT ? -n : vk == VK_RIGHT ? n : 0, dy = vk == VK_UP ? -n : vk == VK_DOWN ? n : 0;
        if (placing) nav::buy::MoveFree(dx, dy, true);
        else nav::map::Move(dx, dy, true);
        return true;
    }
    if (vk < 'A' || vk > 'Z' || Held(VK_CONTROL) || nav::buy::Placing() || nav::build::Active()) return false;
    nav::FindEnd();
    g_swallowSysChar = true;
    nav::JumpToGroup((char)(vk - 'A' + 'a'));
    return true;
}

bool OnSysChar(WPARAM) {
    bool swallow = g_swallowSysChar;
    g_swallowSysChar = false;
    return swallow;
}

void OnFocusLost() { memset(g_syntheticDown, 0, sizeof g_syntheticDown); }

}  // namespace input
