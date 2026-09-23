#include "game_speed.h"

#include <windows.h>

#include "buffers.h"

#include "../game/rtti.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../speech/speech.h"
#include "../util/log.h"
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

// "Pausa. 11:46, $9.500": what a player wants first when stopping to look around.
void AnnouncePause(Ptr vc) {
    std::string line = GameSpeedName(vc);
    std::string clock = game::ClockText(vc);
    Ptr funds = game::FundsButton(vc);
    if (!clock.empty()) line += ". " + clock;
    if (funds) line += ", " + game::Caption(funds);
    FocusStatus();
    buffers::Event(line);
}

}  // namespace

void Reset() { g = State{}; }

// A new panel (entering a lot, or coming back from a dialog or menu) is read silently: only
// changes seen on the same panel are announced.
void Tick(Ptr root) {
    Ptr vc = game::IsKindOf(root, "cDDDSimsView") ? game::FindViewControl(root) : 0;
    if (!vc || !game::Visible(vc)) {
        g.viewControl = 0;
        return;
    }
    game::GameSpeed now = game::ReadGameSpeed(vc);
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
            g.pending = now;
            g.pendingSince = t;
            LogVerbose("game speed: running level %d (settling)", now.level);
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
