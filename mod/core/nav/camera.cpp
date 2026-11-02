#include "camera.h"

#include "../game/view.h"
#include "../game/lot.h"
#include "../game/people.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "buy_mode.h"
#include "navigator.h"

namespace nav::camera {
namespace {

struct State {
    bool enabled = true;   // follow the selected Sim when nothing is browsed
    game::Ptr wanted = 0;  // the Sim the mod told the game to follow (0: none yet, or paused)
    game::Ptr selected = 0;  // the selected Sim last seen, to announce a change
    int zoom = 0;            // the zoom and facing last said (or first read on this lot)
    int facing = -1;
    bool browsing = false; // the camera is on a browsed object
};

State g;

bool Browsing() { return WorldFocused() || buy::Placing(); }

// Following pauses while the camera is on a browsed object; the game would otherwise scroll
// back to the Sim on the next frame.
void PauseFollow() {
    if (game::FollowedSim()) game::StopFollowing(game::InputRoot());
    g.wanted = 0;
    g.browsing = true;
}

std::string SimName(game::Ptr person) {
    std::string name = game::NeighborName(game::SimNeighborId(person));
    return name.empty() ? S(Str::Unnamed) : name;
}

}  // namespace

void Reset() { g = State{}; }

namespace {

Str ZoomWord(int zoom) { return zoom >= 3 ? Str::ZoomNear : zoom == 2 ? Str::ZoomMedium : Str::ZoomFar; }

Str CompassWord(int compass) {
    switch (compass) {
    case 1: return Str::DirNorthEast;
    case 3: return Str::DirSouthEast;
    case 5: return Str::DirSouthWest;
    default: return Str::DirNorthWest;
    }
}

// The camera's own keys and buttons change these: a player can't tell otherwise.
void WatchView() {
    int zoom = game::ViewZoom(), facing = game::ViewCompass();
    bool first = !g.zoom;
    bool zoomed = zoom != g.zoom, turned = facing != g.facing;
    g.zoom = zoom;
    g.facing = facing;
    if (first || !zoom) return;
    if (zoomed) buffers::Event(S(ZoomWord(zoom)));
    if (turned) buffers::Event(util::Format(S(Str::CameraFacing), S(CompassWord(facing))));
}

}  // namespace

std::string ViewLine() {
    int zoom = game::ViewZoom(), facing = game::ViewCompass();
    if (!zoom) return "";
    return util::Format(S(Str::CameraStatus), S(ZoomWord(zoom)), S(CompassWord(facing)));
}

void Tick() {
    if (!LotViewActive() || !game::LotModule()) {
        g.wanted = 0;
        g.browsing = false;
        g.selected = 0;
        g.zoom = 0;
        return;
    }
    WatchView();
    game::Ptr now = game::SelectedSim();
    if (now != g.selected) {
        bool first = g.selected == 0;
        g.selected = now;
        if (now && !first) {
            std::string line = util::Format(S(Str::SimSelectedNow), SimName(now).c_str());
            std::vector<std::string> actions = game::SimActions(game::SimNeighborId(now));
            if (!actions.empty()) line += ", " + actions.front();
            buffers::Event(line);
        }
    }
    if (Browsing()) return;
    if (g.browsing) {  // the browser was left: the Sim is followed again
        g.browsing = false;
        g.wanted = 0;
    }
    if (!g.enabled) return;
    game::Ptr sim = game::SelectedSim();
    if (!sim || sim == g.wanted) return;
    g.wanted = sim;
    if (game::FollowedSim() != sim && game::FollowSim(sim)) Log("camera: following %s", SimName(sim).c_str());
}

void LookAt(game::Ptr object) {
    if (!object || !LotViewActive()) return;
    PauseFollow();
    if (!game::CenterViewOn(object)) LogVerbose("camera: could not center on %p", (void*)object);
}

void ToggleFollow() {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    g.enabled = !g.enabled;
    game::Ptr sim = game::SelectedSim();
    if (!g.enabled) {
        game::StopFollowing(game::InputRoot());
        g.wanted = 0;
        speech::Say(S(Str::FollowOff));
        return;
    }
    if (!sim) {
        speech::Say(S(Str::FollowOnNoSim));
        return;
    }
    g.browsing = false;
    g.wanted = sim;
    game::FollowSim(sim);
    speech::Say(util::Format(S(Str::FollowOn), SimName(sim).c_str()));
}

void SelectSim(int index, bool card) {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    int id = game::SelectFamilyMember(game::InputRoot(), index, card);
    if (id < 0) speech::Say(util::Format(S(Str::NoSuchSim), index + 1));
    // The change itself is announced by Tick ("Isabel, seleccionado, Comer"); pressing the
    // portrait of the Sim already selected centers the camera on it, as the game does.
}

}  // namespace nav::camera
