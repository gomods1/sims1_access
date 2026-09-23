#include "sim_watch.h"

#include <windows.h>

#include <cstdlib>
#include <algorithm>
#include <map>
#include <vector>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/people.h"
#include "../game/simantics.h"
#include "../game/view.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "cheat_console.h"
#include "narrator.h"
#include "room_names.h"

namespace nav::simwatch {
namespace {

using game::Need;
using game::Ptr;

constexpr ULONGLONG kRoomPollMs = 250;
// The Sim is "in" a room once it has been there this long and stopped walking (or stayed this
// long anyway): walking through a hallway or a doorway says nothing.
constexpr ULONGLONG kRoomStayMs = 1200;
constexpr ULONGLONG kRoomStillMs = 600;
constexpr ULONGLONG kRoomLongStayMs = 4000;
constexpr ULONGLONG kActionPollMs = 300;
// An action is spoken once it has been running this long (a Sim switching between two
// interactions within a frame or two says nothing), its end once nothing followed for this long.
constexpr ULONGLONG kActionStartMs = 700;
constexpr ULONGLONG kActionEndMs = 1500;
constexpr ULONGLONG kNeedPollMs = 1000;
constexpr ULONGLONG kRelationPollMs = 1000;
constexpr ULONGLONG kRelationSocialMs = 4000;  // a + / - balloon this recent: a social made the change
constexpr int kRelationDrift = 2;              // the game's own drift, a point or two
// Money: polled often, spoken once it has not moved for this long, so a bill paid in parts or
// several purchases in a row become one line.
constexpr ULONGLONG kFundsPollMs = 250;
constexpr ULONGLONG kFundsSettleMs = 800;

// Need thresholds (-100 empty .. 100 full) with a margin, so a value hovering at a threshold
// does not say the line again and again.
constexpr int kLowAt = -30, kLowClearAt = -20;
constexpr int kUrgentAt = -70, kUrgentClearAt = -60;
// The mood meter is red below zero bars; it is good again from one green bar.
constexpr int kMoodRedBelow = 0, kMoodClearFrom = 1;

const Need kNeeds[] = {Need::Hunger, Need::Comfort, Need::Hygiene, Need::Bladder,
                       Need::Energy, Need::Fun,     Need::Social,  Need::Room};

Str NeedName(Need n) {
    switch (n) {
    case Need::Hunger: return Str::RatingHunger;
    case Need::Comfort: return Str::RatingComfort;
    case Need::Hygiene: return Str::RatingHygiene;
    case Need::Bladder: return Str::RatingBladder;
    case Need::Energy: return Str::RatingEnergy;
    case Need::Fun: return Str::RatingFun;
    case Need::Social: return Str::NeedSocial;
    case Need::Room: return Str::RatingRoom;
    }
    return Str::RatingHunger;
}

enum Level { kOk = 0, kLow = 1, kUrgent = 2 };

struct SimNeeds {
    std::map<int, Level> level;  // by Need
    bool moodRed = false;
    bool seeded = false;
};

struct State {
    uintptr_t lot = 0;  // game::LotIdentity
    int selected = -2;  // neighbor id the room and action baselines belong to (-2: none yet)

    // Room of the selected Sim.
    ULONGLONG roomPolled = 0;
    int roomSeen = game::kNoRoom, levelSeen = 0;  // where the Sim is now
    ULONGLONG roomSince = 0;                      // ... since
    int lastX = 0, lastY = 0;
    ULONGLONG stillSince = 0;                     // not moved since
    int roomSaid = game::kNoRoom, levelSaid = 0;  // the room last spoken (or the starting one)

    // Running action of the selected Sim.
    ULONGLONG actionPolled = 0;
    std::string running;          // running now ("" none)
    ULONGLONG runningSince = 0;
    bool actionSeeded = false;
    std::string actionSaid;       // the running action last spoken (or the starting one)
    bool saidByPlayer = false;    // ... was queued by the player
    bool endSaid = true;          // its end was said (or there was nothing to end)
    ULONGLONG idleSince = 0;      // nothing running and nothing queued since
    std::vector<std::string> waiting;  // the player's queued actions not running yet
    std::string runningByPlayer;       // the running action when the player queued it

    // How the selected Sim feels about each Sim on the lot (daily value), by neighbor id.
    ULONGLONG relationsPolled = 0;
    std::map<int, int> daily;
    std::map<int, ULONGLONG> relationSaid;  // by neighbor id (both Sims of the line)

    // Family funds.
    ULONGLONG fundsPolled = 0;
    bool fundsSeeded = false;
    int funds = 0;             // last spoken (or first read)
    int fundsSeen = 0;         // last read
    ULONGLONG fundsChanged = 0;  // when fundsSeen last moved; 0 = settled

    // Needs of every family member.
    ULONGLONG needsPolled = 0;
    std::map<int, SimNeeds> needs;  // by neighbor id
};

State g;

std::string SimName(int neighborId) {
    std::string name = game::NeighborName(neighborId);
    return name.empty() ? S(Str::Unnamed) : name;
}

// The names say the floor when the house has more than one.
std::string RoomName(int room) { return rooms::NameOf(rooms::NameRooms(game::LotObjects()), room); }

void ResetSelected(int id) {
    g.selected = id;
    g.roomSeen = g.roomSaid = game::kNoRoom;
    g.levelSeen = g.levelSaid = 0;
    g.roomSince = g.stillSince = 0;
    g.actionSeeded = false;
    g.running.clear();
    g.actionSaid.clear();
    g.endSaid = true;
    g.idleSince = 0;
    g.runningSince = 0;
    g.roomPolled = g.actionPolled = 0;
    g.daily.clear();
    g.relationsPolled = 0;
}

// ---- Rooms ---------------------------------------------------------------------------------

void PollRoom(Ptr sim, ULONGLONG now) {
    if (now - g.roomPolled < kRoomPollMs) return;
    g.roomPolled = now;
    game::LotObject o;
    if (!game::GetObject(sim, &o) || o.room == game::kNoRoom) return;
    if (o.x != g.lastX || o.y != g.lastY) {
        g.lastX = o.x;
        g.lastY = o.y;
        g.stillSince = now;
    }
    if (o.room != g.roomSeen || o.level != g.levelSeen) {
        bool first = g.roomSeen == game::kNoRoom;
        g.roomSeen = o.room;
        g.levelSeen = o.level;
        g.roomSince = now;
        if (first) {  // where the Sim was when the watch started: not news
            g.roomSaid = o.room;
            g.levelSaid = o.level;
        }
        return;
    }
    if (o.room == g.roomSaid && o.level == g.levelSaid) return;
    ULONGLONG stayed = now - g.roomSince;
    bool still = now - g.stillSince >= kRoomStillMs;
    if (stayed < kRoomStayMs || (!still && stayed < kRoomLongStayMs)) return;
    g.roomSaid = o.room;
    g.levelSaid = o.level;
    std::string room = RoomName(o.room);
    if (room.empty()) return;
    narrator::Announce(util::Format(S(Str::SimNowIn), SimName(g.selected).c_str(), room.c_str()), false);
}

std::string g_cancelled;  // the queued action the player last cancelled (NoteCancelled)
ULONGLONG g_cancelledAt = 0;
constexpr ULONGLONG kCancelMs = 5000;

// ---- Actions -------------------------------------------------------------------------------

void PollAction(ULONGLONG now) {
    if (now - g.actionPolled < kActionPollMs) return;
    g.actionPolled = now;
    std::vector<game::SimAction> actions = game::SimActionList(g.selected);
    std::string running;
    bool byPlayer = false;
    for (const game::SimAction& a : actions)
        if (a.running) {
            running = a.name;
            byPlayer = a.byPlayer;
            break;
        }
    // The player's queued actions that left the queue without starting: the Sim refused (a
    // stomp when the mood is too low) or something took them away. A queued action that starts
    // becomes the running one (its name can change then: "Llamar taxi.../Ir al Centro" runs as
    // "Llamar taxi"), so a player's action running now that was not before counts as started.
    std::vector<std::string> waiting;
    for (const game::SimAction& a : actions)
        if (a.byPlayer && !a.running) waiting.push_back(a.name);
    std::string nowByPlayer = byPlayer ? running : "";
    if (g.actionSeeded) {
        std::vector<std::string> gone = g.waiting;
        for (const std::string& w : waiting) {
            auto it = std::find(gone.begin(), gone.end(), w);
            if (it != gone.end()) gone.erase(it);
        }
        bool started = !nowByPlayer.empty() && nowByPlayer != g.runningByPlayer;
        if (started && !gone.empty()) gone.erase(gone.begin());
        for (const std::string& name : gone) {
            if (name == g_cancelled && now - g_cancelledAt < kCancelMs) continue;
            int mood = 0;
            bool bad = game::SimMood(g.selected, &mood) && game::MoodBars(mood) < 0;
            narrator::Announce(util::Format(S(bad ? Str::ActionDroppedMood : Str::ActionDropped),
                                            SimName(g.selected).c_str(), name.c_str()),
                               true);
        }
    }
    g.waiting = waiting;
    g.runningByPlayer = nowByPlayer;
    if (!g.actionSeeded) {  // what the Sim was doing when the watch started: not news
        g.actionSeeded = true;
        g.running = g.actionSaid = running;
        g.saidByPlayer = byPlayer;
        g.endSaid = running.empty();
        return;
    }
    if (running != g.running) {
        g.running = running;
        g.runningSince = now;
    }
    g.idleSince = actions.empty() ? (g.idleSince ? g.idleSince : now) : 0;
    if (!running.empty()) {
        if (running == g.actionSaid || now - g.runningSince < kActionStartMs) return;
        g.actionSaid = running;
        g.saidByPlayer = byPlayer;
        g.endSaid = false;
        narrator::Announce(util::Format(S(byPlayer ? Str::ActionStarted : Str::ActionStartedOwn),
                                        SimName(g.selected).c_str(), running.c_str()),
                           false);
        return;
    }
    if (g.endSaid || g.actionSaid.empty() || !g.idleSince || now - g.idleSince < kActionEndMs) return;
    g.endSaid = true;
    // Only what the player asked for is worth an end: free will goes from one thing to the next.
    std::string line = util::Format(S(Str::ActionFinished), SimName(g.selected).c_str(), g.actionSaid.c_str());
    if (g.saidByPlayer) narrator::Announce(line, false);
    else narrator::Record(line);
    g.actionSaid.clear();
}

// ---- Relationships -------------------------------------------------------------------------

// A social changes the daily relationship (the game shows + or - over the Sim, which the balloon
// narration speaks for pairs without the selected Sim); the number says how much.
void PollRelationships(ULONGLONG now) {
    if (now - g.relationsPolled < kRelationPollMs) return;
    bool seeding = g.relationsPolled == 0;
    g.relationsPolled = now;
    for (Ptr person : game::LotPeople()) {
        int other = game::SimNeighborId(person);
        if (other < 0 || other == g.selected) continue;
        game::Relationship r;
        if (!game::SimRelationship(g.selected, other, &r)) continue;
        auto it = g.daily.find(other);
        if (it == g.daily.end() || seeding) {
            g.daily[other] = r.daily;
            continue;
        }
        int delta = r.daily - it->second;
        it->second = r.daily;
        if (!delta) continue;
        std::string amount = (delta > 0 ? "+" : "") + std::to_string(delta);
        std::string line = util::Format(S(Str::RelationChanged), SimName(g.selected).c_str(), SimName(other).c_str(),
                                        amount.c_str(), r.daily);
        // The game also moves relationships by a point or two on its own every few hours, with
        // no social (said every half minute at ultra speed): those only go to the buffer.
        bool social = narrator::RelationSignSince(other, now - kRelationSocialMs) ||
                      narrator::RelationSignSince(g.selected, now - kRelationSocialMs);
        if (!social && delta >= -kRelationDrift && delta <= kRelationDrift) {
            narrator::Record(line);
            continue;
        }
        g.relationSaid[other] = g.relationSaid[g.selected] = now;
        narrator::Announce(line, false);
    }
}

// ---- Money ---------------------------------------------------------------------------------

// The game only plays a sound when money is paid or received (the pizza, bills, a job's pay, a
// sale): the amount and what is left are spoken. Buy mode says its own prices and refunds.
void PollFunds(ULONGLONG now) {
    if (now - g.fundsPolled < kFundsPollMs) return;
    g.fundsPolled = now;
    int funds = 0;
    if (!game::GetFunds(&funds)) return;
    // The cheat console says its own result ("rosebud ... +$1.000").
    if (!g.fundsSeeded || game::InBuyTool() || cheats::Entering()) {
        g.fundsSeeded = true;
        g.funds = g.fundsSeen = funds;
        g.fundsChanged = 0;
        return;
    }
    if (funds != g.fundsSeen) {
        g.fundsSeen = funds;
        g.fundsChanged = now;
        return;
    }
    if (!g.fundsChanged || now - g.fundsChanged < kFundsSettleMs) return;
    g.fundsChanged = 0;
    int delta = funds - g.funds;
    g.funds = funds;
    if (!delta) return;
    std::string amount = (delta > 0 ? "+" : "-") + util::Money(delta > 0 ? delta : -delta);
    narrator::Announce(util::Format(S(Str::FundsChanged), amount.c_str(), util::Money(funds).c_str()), true);
}

// ---- Needs and mood ------------------------------------------------------------------------

Level NextLevel(Level was, int value) {
    if (value < kUrgentAt) return kUrgent;
    if (was == kUrgent && value < kUrgentClearAt) return kUrgent;
    if (value < kLowAt) return kLow;
    if (was >= kLow && value < kLowClearAt) return kLow;
    return kOk;
}

std::string JoinNames(const std::vector<Need>& needs) {
    std::string out;
    for (Need n : needs) {
        if (!out.empty()) out += ", ";
        out += S(NeedName(n));
    }
    return out;
}

void PollNeeds(int selected, ULONGLONG now) {
    if (now - g.needsPolled < kNeedPollMs) return;
    g.needsPolled = now;
    std::vector<int> family = game::FamilyNeighborIds();
    for (int id : family) {
        SimNeeds& sn = g.needs[id];
        std::vector<Need> low, urgent, recovered;
        for (Need n : kNeeds) {
            int value = 0;
            if (!game::SimNeed(id, n, &value)) continue;
            Level was = sn.level.count((int)n) ? sn.level[(int)n] : kOk;
            Level now_ = NextLevel(was, value);
            sn.level[(int)n] = now_;
            if (!sn.seeded || now_ == was) continue;
            if (now_ == kUrgent) urgent.push_back(n);
            else if (now_ == kLow && was == kOk) low.push_back(n);
            else if (now_ == kOk) recovered.push_back(n);
        }
        int mood = 0;
        bool red = sn.moodRed;
        if (game::SimMood(id, &mood)) {
            int bars = game::MoodBars(mood);
            red = sn.moodRed ? bars < kMoodClearFrom : bars < kMoodRedBelow;
        }
        bool moodChanged = red != sn.moodRed;
        sn.moodRed = red;
        if (!sn.seeded) {  // what the needs were when the watch started: not news
            sn.seeded = true;
            continue;
        }
        std::string name = SimName(id);
        bool mine = id == selected;
        if (!urgent.empty())
            narrator::Announce(util::Format(S(urgent.size() > 1 ? Str::NeedsUrgent : Str::NeedUrgent), name.c_str(),
                                            JoinNames(urgent).c_str()),
                               true);
        if (!low.empty()) {
            std::string line = util::Format(S(low.size() > 1 ? Str::NeedsLow : Str::NeedLow), name.c_str(),
                                            JoinNames(low).c_str());
            if (mine) narrator::Announce(line, true);
            else narrator::Record(line);  // the others: only urgent needs are spoken
        }
        if (!recovered.empty())
            narrator::Record(util::Format(S(Str::NeedRecovered), name.c_str(), JoinNames(recovered).c_str()));
        if (moodChanged) {
            std::string line = util::Format(S(red ? Str::MoodTurnedBad : Str::MoodGoodAgain), name.c_str());
            if (red && mine) narrator::Announce(line, true);
            else narrator::Record(line);
        }
    }
}

}  // namespace

void Reset() { g = State{}; }

void Tick() {
    uintptr_t lot = game::LotIdentity();
    if (lot != g.lot) {
        g = State{};
        g.lot = lot;
    }
    if (!lot) return;
    ULONGLONG now = GetTickCount64();
    int selected = game::SelectedNeighborId();
    if (selected != g.selected) ResetSelected(selected);
    PollNeeds(selected, now);
    PollFunds(now);
    Ptr sim = game::SelectedSim();
    if (selected < 0 || !sim) return;
    PollRoom(sim, now);
    PollAction(now);
    PollRelationships(now);
}

void NoteCancelled(const std::string& action) {
    g_cancelled = action;
    g_cancelledAt = GetTickCount64();
}

bool RelationSaidSince(int neighborId, unsigned long long since) {
    auto it = g.relationSaid.find(neighborId);
    return it != g.relationSaid.end() && it->second >= since;
}

std::string Status() {
    int id = game::SelectedNeighborId();
    Ptr sim = game::SelectedSim();
    if (id < 0 || !sim) return S(Str::NoSimSelected);
    std::string line = SimName(id);
    game::LotObject o;
    if (game::GetObject(sim, &o) && o.room != game::kNoRoom) {
        std::string room = RoomName(o.room);
        if (!room.empty()) line += ", " + util::Format(S(Str::StatusInRoom), room.c_str());
    }
    std::vector<game::SimAction> actions = game::SimActionList(id);
    if (actions.empty()) line += ", " + std::string(S(Str::StatusIdle));
    for (size_t i = 0; i < actions.size(); i++)
        line += ", " + (i == 0 ? actions[i].name : util::Format(S(Str::StatusThen), actions[i].name.c_str()));
    std::vector<Need> low, urgent;
    for (Need n : kNeeds) {
        int value = 0;
        if (!game::SimNeed(id, n, &value)) continue;
        if (value < kUrgentAt) urgent.push_back(n);
        else if (value < kLowAt) low.push_back(n);
    }
    if (!urgent.empty())
        line += "; " + util::Format(S(urgent.size() > 1 ? Str::StatusNeedsUrgent : Str::StatusNeedUrgent),
                                    JoinNames(urgent).c_str());
    if (!low.empty())
        line += "; " + util::Format(S(low.size() > 1 ? Str::StatusNeedsLow : Str::StatusNeedLow), JoinNames(low).c_str());
    if (low.empty() && urgent.empty()) line += "; " + std::string(S(Str::StatusNeedsFine));
    int mood = 0;
    if (game::SimMood(id, &mood)) {
        int bars = game::MoodBars(mood);
        std::string label = S(Str::StatusMood);
        if (bars > 0) line += "; " + util::Format(S(Str::MoodGood), label.c_str(), bars);
        else if (bars < 0) line += "; " + util::Format(S(Str::MoodBad), label.c_str(), -bars);
        else line += "; " + util::Format(S(Str::MoodNeutral), label.c_str());
    }
    return line;
}

}  // namespace nav::simwatch
