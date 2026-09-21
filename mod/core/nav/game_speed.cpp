#include "game_speed.h"

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

struct State {
    Ptr viewControl = 0;  // the panel the last reading came from; 0 when not in the lot view
    game::GameSpeed last;
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
    if (now == g.last) return;
    g.last = now;
    Log("game speed: %s level %d", now.paused ? "paused" : "running", now.level);
    if (now.paused) AnnouncePause(vc);
    else buffers::Event(GameSpeedName(vc));
}

}  // namespace nav::speed
