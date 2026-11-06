#include "game_speed.h"

#include <windows.h>

#include "buffers.h"

#include "../game/buy.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/settings.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "describe.h"
#include "navigator.h"

namespace nav::speed {
namespace {

// A running speed is spoken once it has held this long: the game sometimes switches between
// high and ultra by itself every half second (seen at 3 AM with no key pressed, 2026-09-23),
// which would otherwise be a stream of "ultra, alta, ultra". A pause is spoken at once.
constexpr ULONGLONG kSpeedSettleMs = 700;

struct State {
    Ptr viewControl = 0;  // the panel the last reading came from; 0 when not in the lot view
    game::GameSpeed last;       // last spoken (or first read)
    game::GameSpeed pending;    // a running speed waiting to settle
    ULONGLONG pendingSince = 0; // 0 when nothing waits
};

State g;

// Every new hour of the game's clock, spoken when the player asked for it (Ctrl+T; a tester went
// past the hour he wanted at ultra speed): "son las 14:00".
int g_hours = -1;  // -1: not read from the settings yet
std::string g_hourSeen;  // the clock's hour part ("14", "2 PM") last read

bool HoursOn() {
    if (g_hours < 0) g_hours = util::GetSetting("hours", 0) ? 1 : 0;
    return g_hours == 1;
}

// The clock's text without its minutes: "11:46" -> "11", "2:05 PM" -> "2 PM".
std::string HourPart(const std::string& clock) {
    size_t colon = clock.find(':');
    if (colon == std::string::npos) return clock;
    size_t end = colon + 1;
    while (end < clock.size() && clock[end] >= '0' && clock[end] <= '9') end++;
    return clock.substr(0, colon) + clock.substr(end);
}

void WatchHour(Ptr vc, bool fresh) {
    std::string clock = game::ClockText(vc);
    if (clock.empty()) return;
    std::string hour = HourPart(clock);
    bool changed = !g_hourSeen.empty() && hour != g_hourSeen;
    g_hourSeen = hour;
    if (fresh || !changed || !HoursOn()) return;
    buffers::Event(util::Format(S(Str::HourChanged), clock.c_str()));
}

// "Pausa. 11:46, $9.500": what a player wants first when stopping to look around.
void AnnouncePause(Ptr vc) {
    std::string line = GameSpeedName(vc);
    std::string clock = game::ClockText(vc);
    Ptr funds = game::FundsButton(vc);
    if (!clock.empty()) line += ". " + clock;
    if (funds) line += ", " + (game::UnlimitedFunds() ? std::string(S(Str::NoFamilyFunds)) : game::Caption(funds));
    FocusStatus();
    buffers::Event(line);
}

}  // namespace

void Reset() {
    g = State{};
    g_hourSeen.clear();
}

void ToggleHours() {
    g_hours = HoursOn() ? 0 : 1;
    util::SetSetting("hours", g_hours);
    speech::Say(S(g_hours ? Str::HoursOn : Str::HoursOff));
}

// A new panel (entering a lot, or coming back from a dialog or menu) is read silently: only
// changes seen on the same panel are announced.
void Tick(Ptr root) {
    Ptr vc = game::IsKindOf(root, "cDDDSimsView") ? game::FindViewControl(root) : 0;
    if (!vc || !game::Visible(vc)) {
        g.viewControl = 0;
        return;
    }
    game::GameSpeed now = game::ReadGameSpeed(vc);
    WatchHour(vc, vc != g.viewControl);
    if (vc != g.viewControl) {
        g.viewControl = vc;
        g.last = now;
        return;
    }
    ULONGLONG t = GetTickCount64();
    if (now == g.last) {
        g.pendingSince = 0;
        return;
    }
    if (!now.paused) {
        if (!g.pendingSince || !(now == g.pending)) {
            // The game's own flicker brings the same level back every second: logged once.
            if (!(now == g.pending)) LogVerbose("game speed: running level %d (settling)", now.level);
            g.pending = now;
            g.pendingSince = t;
            return;
        }
        if (t - g.pendingSince < kSpeedSettleMs) return;
    }
    g.pendingSince = 0;
    g.last = now;
    Log("game speed: %s level %d", now.paused ? "paused" : "running", now.level);
    if (now.paused) AnnouncePause(vc);
    else buffers::Event(GameSpeedName(vc));
}

}  // namespace nav::speed
