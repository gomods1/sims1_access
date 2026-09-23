#include "keyboard.h"

#include <cstring>

#include "../game/buy.h"
#include "../game/hooks.h"
#include "../game/pie.h"
#include "../game/ui.h"
#include "../nav/buffers.h"
#include "../nav/buy_mode.h"
#include "../nav/camera.h"
#include "../nav/cheat_console.h"
#include "../nav/narrator.h"
#include "../nav/navigator.h"
#include "../nav/text_entry.h"
#include "../nav/tutorial.h"
#include "../speech/speech.h"
#include "../util/strings.h"
#include "../nav/world.h"
#include "../util/log.h"

namespace input {
namespace {

// Keys the mod owns (none of them is a game hotkey; see CLAUDE.md "Game key bindings"):
//   Up/Left, Down/Right            previous/next item in the group
//                                  lot view (the HUD): Up/Down previous/next group (status,
//                                  needs, family, action queue, panels), Left/Right the items;
//                                  on a settings page (options) Up/Down walk its rows first
//                                  on a slider, value picker or personality bar: Left/Right change it
//   Home, End                      first/last item in the group
//   Tab, Shift+Tab                 next/previous group
//   Space                          lot view: the game's next-Sim key (the selection is announced)
//   Enter, Space                   press the focused item, or in the lot view after PageUp/Down
//                                  open the focused object's interaction menu (Space is typing
//                                  while a text field is shown)
//   R                              repeat the focused item (typing while a text field is shown)
//   Ctrl+Up / Ctrl+Down            review buffers (nav/buffers.h): the next / previous line of the
//                                  current buffer (details of what was spoken, or events)
//   Ctrl+Left / Ctrl+Right         previous / next review buffer
//   buy mode, while placing an object (nav/buy_mode.h): PageUp/PageDown the spots, Ctrl+ the rooms,
//   Shift+ the directions on a spot, Enter buys, Escape cancels, R repeats
//   PageUp/PageDown                a value control (slider, value picker, personality bar): one
//                                  step up/down, like Right/Left
//                                  lot view: previous/next object in the category
//   Shift+PageUp/PageDown          lot view: previous/next object category
//   Ctrl+PageUp/PageDown           lot view: previous/next room
//   Ctrl+Enter                     lot view, after PageUp/PageDown: walk to the focused object
//   Escape                         close the interaction menu (otherwise the game's Escape)
//   Ctrl+F1                        say the mod's keys for this screen
//   Ctrl+1 .. Ctrl+9               lot view: select the n-th Sim of the family (its portrait)
//   Ctrl+F                         lot view: follow the selected Sim with the camera on/off
//   Ctrl+H                         lot view: open the help system's waiting tip
//   Ctrl+E                         lot view: the selected Sim's state (room, actions, needs, mood)
//   Ctrl+N                         the narrator's verbosity: everything, warnings only, off
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

bool WorldTarget() { return nav::WorldFocused() && nav::LotViewActive(); }

// The keys that work where the player is.
void SayKeyHelp() {
    if (game::IsPieMenu(game::InputRoot())) speech::Say(S(Str::KeysMenu));
    else if (nav::cheats::Entering()) speech::Say(S(Str::CheatHint));
    else if (nav::buy::Placing()) speech::Say(S(Str::KeysPlacing));
    else if (nav::LotViewActive() && game::InBuyTool()) speech::Say(S(Str::KeysBuy));
    else if (nav::LotViewActive()) speech::Say(S(Str::KeysLot));
    else speech::Say(S(Str::KeysScreen));
}

// The lot view's HUD is a list of groups read top to bottom, each read left to right. Left/Right
// change a slider or value picker instead of leaving it.
void Arrow(bool vertical, int delta) {
    nav::FocusWorld(false);
    if (!vertical && nav::Adjust(delta)) return;
    if (vertical && nav::LotViewActive()) nav::NextRowOrGroup(delta);
    else nav::Move(delta);
}

// Runs the action for a key; false when the key is not the mod's in this situation. The
// character to swallow is set before the action runs: an action can open a menu that runs
// modally (and processes that character) before returning.
bool Dispatch(WPARAM vk, bool shift, bool ctrl) {
    bool typing = nav::TextEntryActive();
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
        else {
            nav::FocusWorld(false);
            nav::MoveToEdge(vk == VK_END);
        }
        return true;
    case VK_TAB:
        g_swallowChar = '\t';
        nav::FocusWorld(false);
        nav::NextGroup(shift ? -1 : +1);
        return true;
    case VK_PRIOR: case VK_NEXT:
        if (!ctrl && !WorldTarget() && nav::Adjust(vk == VK_PRIOR ? +1 : -1)) return true;
        if (!nav::LotViewActive()) return false;
        nav::FocusWorld(true);
        if (nav::buy::Placing()) {  // the spots where the held object can go
            if (ctrl) nav::buy::NextArea(vk == VK_NEXT ? +1 : -1);
            else if (shift) nav::buy::NextTurn(vk == VK_NEXT ? +1 : -1);
            else nav::buy::NextSpot(vk == VK_NEXT ? +1 : -1);
            return true;
        }
        if (ctrl) nav::world::NextRoom(vk == VK_NEXT ? +1 : -1);
        else if (shift) nav::world::NextCategory(vk == VK_NEXT ? +1 : -1);
        else nav::world::NextObject(vk == VK_NEXT ? +1 : -1);
        return true;
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        if (!ctrl || shift) return false;
        g_swallowChar = vk;  // a posted digit still makes its character (a game speed key)
        nav::camera::SelectSim((int)vk - '1');
        return true;
    case 'H':  // Ctrl+H: the help system's waiting tip
        if (!ctrl || shift) return false;
        g_swallowChar = 'h';
        nav::tutorial::OpenTip();
        return true;
    case 'E':
        if (!ctrl || shift) return false;
        g_swallowChar = 'e';
        nav::narrator::SayStatus();
        return true;
    case 'N':
        if (!ctrl || shift) return false;
        g_swallowChar = 'n';
        nav::narrator::CycleVerbosity();
        return true;
    case 'F':
        if (!ctrl || shift) return false;
        g_swallowChar = 'f';
        nav::camera::ToggleFollow();
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
        if (ctrl) {
            if (!WorldTarget() || nav::buy::Placing()) return false;
            nav::world::WalkToFocus();
        } else if (WorldTarget() && nav::buy::Placing()) {
            nav::buy::Confirm();
        } else if (WorldTarget() && game::InBuyTool()) {
            nav::buy::PickUpFocused();
        } else if (WorldTarget()) {
            nav::world::Interact();
        } else {
            nav::Activate();
        }
        return true;
    case VK_DELETE:  // the game sells a held object too, but says nothing
        if (!nav::buy::Placing()) return false;
        nav::buy::Sell();
        return true;
    case VK_F1:
        if (!ctrl) return false;  // F1 alone is the game's Live mode
        SayKeyHelp();
        return true;
    case VK_ESCAPE:
        g_swallowChar = 0x1b;
        if (nav::Cancel() || nav::buy::Cancel() || nav::tutorial::GuardEscape()) return true;
        g_swallowChar = 0;
        return false;
    case 'R':
        if (typing || shift) return false;
        g_swallowChar = 'r';
        if (WorldTarget() && nav::buy::Placing()) nav::buy::Repeat();
        else if (WorldTarget()) nav::world::Repeat();
        else nav::RepeatFocus();
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
    if (vk == VK_BACK) {  // typing: the game gets the key, the mod only speaks what it deletes
        nav::text::OnBackspace();
        return false;
    }
    bool ctrl = Held(VK_CONTROL);
    bool ctrlKey = vk == VK_PRIOR || vk == VK_NEXT || vk == VK_RETURN || vk == VK_F1 || vk == VK_UP ||
                   vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT || (vk >= '1' && vk <= '9') || vk == 'F' || vk == 'H' || vk == 'E' || vk == 'N' ||
                   (vk == 'C' && Held(VK_SHIFT));  // the mod's Ctrl combinations
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
    if (!swallow) nav::text::OnChar(ch);
    return swallow;
}

void OnFocusLost() { memset(g_syntheticDown, 0, sizeof g_syntheticDown); }

}  // namespace input
