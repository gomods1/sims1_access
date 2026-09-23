#include "narrator.h"

#include <windows.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../game/lot.h"
#include "../game/people.h"
#include "../game/simantics.h"
#include "../game/sounds.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/settings.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "navigator.h"
#include "sim_watch.h"
#include "world.h"

namespace nav::narrator {
namespace {

using game::Balloon;
using game::BalloonKind;
using game::Ptr;
namespace grp = game::balloon;

constexpr ULONGLONG kSimRepeatMs = 15000;     // the same balloon over the same Sim: once per this
constexpr ULONGLONG kObjectRepeatMs = 60000;  // over an object (flies over the trash come and go)
// A tree that keeps showing balloons over the same Sim is spoken once and then stays quiet until
// it has been silent this long: a sleeping Sim's dreams (the bed's "Dream Topic", "Dream Person",
// "Dream Motive" trees cycle an icon every second or two), "Relationship Impact for Join" giving
// a "-" again and again, the idle thought about the same need. Route failures and shouts are
// always spoken.
constexpr ULONGLONG kTreeQuietMs = 30000;
constexpr ULONGLONG kVisitorPollMs = 100;
// A Sim is announced once it has stayed this long: the Unleashed raccoon ("El Bandito") is
// created and removes itself within half a second when nothing tempts it.
constexpr ULONGLONG kVisitorStayMs = 1000;

struct Shown {
    game::ShownBalloon record;  // what the game drew when the balloon was reported
    std::string text;           // "piensa: hambre"
};

struct Visitor {
    std::string name;
    std::string burglarShout;  // the game's cry when it is the burglar, else empty
    bool ghost = false;        // a grave brought it out
    ULONGLONG since = 0;       // first seen
    bool announced = false;    // its arrival was said (or it was already there)
};

struct State {
    Ptr lot = 0;                                  // the ObjectModule the state belongs to
    std::map<int, Shown> shown;                   // by owner id
    std::map<std::string, ULONGLONG> spokenAt;    // "<owner id>|<line>" -> when
    std::map<std::pair<int, int>, ULONGLONG> treeSeen;  // (owner id, tree) -> its last balloon
    std::map<int, ULONGLONG> alertSaid;           // by game::SoundAlert: when it was last spoken
    std::map<int, Visitor> visitors;              // by object id: Sims from outside the family
    bool visitorsSeeded = false;
    ULONGLONG visitorsPolled = 0;
};

State g;

// Verbosity levels as saved in SimsAccess.ini ("narrator").
enum Verbosity { kOff = 0, kWarnings = 1, kEverything = 2 };
int g_verbosity = -1;  // read from the settings on first use

int CurrentVerbosity() {
    if (g_verbosity < 0) {
        g_verbosity = util::GetSetting("narrator", kEverything);
        if (g_verbosity < kOff || g_verbosity > kEverything) g_verbosity = kEverything;
    }
    return g_verbosity;
}

bool Contains(const std::vector<int>& ids, int id) { return std::find(ids.begin(), ids.end(), id) != ids.end(); }

// ---- What a balloon shows, in words --------------------------------------------------------

// Conversation topics: three icons per interest, in the order of the game's interests (Live.iff
// STR# 140; the sprites are labelled the same way), then the careers.
const Str kInterests[] = {Str::InterestTravel, Str::InterestMoney,    Str::InterestPolitics, Str::InterestSixties,
                          Str::InterestWeather, Str::InterestSports,  Str::InterestMusic,    Str::InterestOutdoors,
                          Str::InterestToys,    Str::InterestAliens,  Str::InterestPets,     Str::InterestSchool,
                          Str::InterestExercise, Str::InterestFood,   Str::InterestParties,  Str::InterestStyle,
                          Str::InterestHollywood, Str::InterestTechnology, Str::InterestRomance};
const Str kCareers[] = {Str::CareerBusiness, Str::CareerEntertainment, Str::CareerLaw,         Str::CareerCrime,
                        Str::CareerMedicine, Str::CareerMilitary,      Str::CareerPolitics,    Str::CareerAthletics,
                        Str::CareerScience,  Str::CareerXtreme,        Str::CareerMusic,       Str::CareerSlacker,
                        Str::CareerParanormal, Str::CareerJournalism,  Str::CareerHacker,      Str::CareerUnemployed};
const Str kHeadlines[] = {Str::HeadlineHate, Str::HeadlineStress, Str::HeadlineSurprise,
                          Str::HeadlineIdea, Str::HeadlineLove,   Str::HeadlineDrunk,
                          Str::HeadlineHurt, Str::HeadlineSmell,  Str::HeadlineFlies};
const Str kRouteFailures[] = {Str::RouteNoChair, Str::RouteNoDoor, Str::RouteWall, Str::RouteNotLevel,
                              Str::RouteNoLadder};

// The social group (Behavior.iff STR# 506, "Relationship/Social/Debug"). Its routing icons
// (moving out of the way, waiting, ...) are debugging aids of the route_balloons cheat.
bool SocialIcon(int index, Str* out) {
    static const Str kInteractions[] = {Str::SocialHug,  Str::SocialGift,  Str::SocialBackRub, Str::SocialCheerUp,
                                        Str::SocialEntertain, Str::SocialTickle, Str::SocialBrag, Str::SocialSlap,
                                        Str::SocialScare, Str::SocialTease, Str::SocialAttack};
    if (index >= 1 && index <= 11) {
        *out = kInteractions[index - 1];
        return true;
    }
    switch (index) {
    case 12: *out = Str::BalloonNeedsChair; return true;
    case 13: *out = Str::BalloonInUse; return true;
    case 14: *out = Str::BalloonCantGetThere; return true;
    case 15: *out = Str::BalloonRelationUpMuch; return true;
    case 16: *out = Str::BalloonRelationUp; return true;
    case 17: *out = Str::BalloonRelationDown; return true;
    case 18: *out = Str::BalloonRelationDownMuch; return true;
    case 19: *out = Str::BalloonPizza; return true;
    case 29: *out = Str::RouteWall; return true;
    case 30: *out = Str::BalloonFriends; return true;
    case 31: *out = Str::BalloonRomance; return true;
    case 33: *out = Str::BalloonCrush; return true;
    case 34: *out = Str::BalloonFameUpMuch; return true;
    case 35: *out = Str::BalloonFameUp; return true;
    case 36: *out = Str::BalloonFameDown; return true;
    case 37: *out = Str::BalloonFameDownMuch; return true;
    }
    return false;
}

bool MotiveIcon(int index, Str* out) {
    switch (index) {  // the motive icons that exist (Sprites.iff 200-213)
    case 0: *out = Str::RatingHunger; return true;
    case 1: *out = Str::RatingBladder; return true;
    case 3: *out = Str::RatingHygiene; return true;
    case 4: *out = Str::RatingEnergy; return true;
    case 5: *out = Str::RatingComfort; return true;
    case 6: *out = Str::RatingFun; return true;
    case 10: *out = Str::NeedSocial; return true;
    case 13: *out = Str::RatingRoom; return true;
    }
    return false;
}

std::string ObjectNameById(int id) {
    game::LotObject o;
    if (!game::GetObject(game::ObjectById(id), &o)) return "";
    return o.person ? game::ObjectName(o) : world::DisplayName(o);
}

// The icon's meaning ("hambre", "Deportes", "Silla de comedor"); empty for icons without one.
std::string Content(const Balloon& b) {
    Str s;
    switch (b.group) {
    case grp::kConversation:
        if (b.index >= 0 && b.index < 57) return S(kInterests[b.index / 3]);
        if (b.index >= 57 && b.index < 73) return util::Format(S(Str::TopicWork), S(kCareers[b.index - 57]));
        if (b.index == 73) return S(Str::TopicBored);
        if (b.index == 74 || b.index == 75) return S(Str::TopicBadMood);
        return "";
    case grp::kConversation2: return b.index >= 0 && b.index < 3 ? S(Str::TopicMagic) : "";
    case grp::kMotive: return MotiveIcon(b.index, &s) ? S(s) : "";
    case grp::kRelationship:
        if (b.index == 0) return S(Str::BalloonRomance);
        if (b.index == 1) return S(Str::BalloonFriends);
        return "";
    case grp::kHeadline: return b.index >= 0 && b.index < 9 ? S(kHeadlines[b.index]) : "";
    case grp::kSocial: return SocialIcon(b.index, &s) ? S(s) : "";
    case grp::kPicture:
        if (b.pictureNeighborId >= 0) return game::NeighborName(b.pictureNeighborId);
        return ObjectNameById(b.pictureObjectId);
    case grp::kRouteFailure: return b.index >= 0 && b.index < 5 ? S(kRouteFailures[b.index]) : "";
    case grp::kProgress: {
        static const Str kKinds[] = {Str::ProgressSkill, Str::ProgressRepair, Str::ProgressInterest};
        if (b.index < 0 || b.index >= 30) return "";
        return util::Format(S(kKinds[b.index / 10]), b.index % 10 * 10);
    }
    }
    return "";
}

bool RouteTrouble(const Balloon& b) { return b.routeFailure || b.group == grp::kRouteFailure; }

// "Bella piensa: hambre" (named) or "piensa: hambre" (for the object browser).
std::string Line(const Balloon& b, const std::string& content, const std::string& name) {
    bool named = !name.empty();
    const char* n = name.c_str();
    const char* c = content.c_str();
    if (RouteTrouble(b)) {
        if (b.group == grp::kPicture)
            return named ? util::Format(S(Str::BalloonRouteBlocked), n, c) : util::Format(S(Str::ShownRouteBlocked), c);
        return named ? util::Format(S(Str::BalloonRouteFailed), n, c) : util::Format(S(Str::ShownRouteFailed), c);
    }
    switch (b.kind) {
    case BalloonKind::Thought:
        return named ? util::Format(S(Str::BalloonThinks), n, c) : util::Format(S(Str::ShownThinks), c);
    case BalloonKind::Scream:
        return named ? util::Format(S(Str::BalloonShouts), n, c) : util::Format(S(Str::ShownShouts), c);
    case BalloonKind::Speak:
        return named ? util::Format(S(Str::BalloonSays), n, c) : util::Format(S(Str::ShownSays), c);
    case BalloonKind::None: break;
    }
    return named ? util::Format(S(Str::BalloonShows), n, c) : content;
}

// Whose balloons are spoken: everything over the family and over objects; a visitor's only when
// it concerns the family (a social interaction with one of them) or is a shout. Skill progress
// bars are not spoken (every tenth of a skill point; the game announces each point gained
// itself) but stay in the object browser's description. The others are logged.
bool Speak(const Balloon& b, bool person, bool family, bool withFamily) {
    if (b.group == grp::kProgress) return false;
    if (!person || family) return true;
    return withFamily || b.kind == BalloonKind::Scream;
}

std::string Trimmed(const std::string& s) {
    size_t first = s.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    return s.substr(first, s.find_last_not_of(' ') - first + 1);
}

void Report(const Balloon& b, const std::vector<int>& family) {
    Ptr owner = game::ObjectById(b.ownerId);
    game::LotObject o;
    if (!game::GetObject(owner, &o)) return;
    std::string content = Trimmed(Content(b));  // some catalog names start with a space
    if (content.empty()) {
        Log("balloon over #%d: group %d icon %d kind %d (tree 0x%x) has no words", b.ownerId, b.group, b.index,
            (int)b.kind, b.tree);
        return;
    }
    if (b.crossed) content = util::Format(S(Str::BalloonCrossedOut), content.c_str());
    Shown shown;
    game::BalloonShownOver(owner, &shown.record);
    shown.text = Line(b, content, "");
    g.shown[b.ownerId] = shown;

    std::string name = Trimmed(o.person ? game::ObjectName(o) : world::DisplayName(o));
    std::string line = Line(b, content, name);
    bool inFamily = o.person && Contains(family, game::SimNeighborId(owner));
    game::LotObject other;
    bool withFamily = b.otherId != b.ownerId && game::GetObject(game::ObjectById(b.otherId), &other) &&
                      other.person && Contains(family, game::SimNeighborId(other.object));
    bool speak = Speak(b, o.person, inFamily, withFamily);
    // The + or - after a social with the selected Sim: the family watch says it with the number.
    Ptr selected = game::SelectedSim();
    game::LotObject sel;
    if (speak && b.group == grp::kSocial && b.index >= 15 && b.index <= 18 && selected &&
        game::GetObject(selected, &sel) && (sel.id == b.ownerId || sel.id == b.otherId))
        speak = false;
    ULONGLONG now = GetTickCount64();
    const char* quiet = speak ? "" : ", not spoken";
    if (speak && o.person && !RouteTrouble(b) && b.kind != BalloonKind::Scream) {
        auto seen = g.treeSeen.find({b.ownerId, b.tree});
        if (seen != g.treeSeen.end() && now - seen->second < kTreeQuietMs) {
            speak = false;
            quiet = ", repeating";
        }
    }
    g.treeSeen[{b.ownerId, b.tree}] = now;
    std::string key = std::to_string(b.ownerId) + "|" + line;
    auto last = g.spokenAt.find(key);
    if (speak && last != g.spokenAt.end() && now - last->second < (o.person ? kSimRepeatMs : kObjectRepeatMs)) {
        speak = false;
        quiet = ", said just now";
    }
    Log("balloon (tree 0x%x, group %d icon %d, over #%d, stack object #%d%s): %s", b.tree, b.group, b.index,
        b.ownerId, b.otherId, quiet, line.c_str());
    if (!speak) return;
    g.spokenAt[key] = now;
    Announce(line, RouteTrouble(b) || b.kind == BalloonKind::Scream);
}

// ---- Visitors ------------------------------------------------------------------------------

void PollVisitors(const std::vector<int>& family) {
    ULONGLONG now = GetTickCount64();
    if (now - g.visitorsPolled < kVisitorPollMs) return;
    g.visitorsPolled = now;
    if (family.empty()) return;  // the lot is still loading (or no family lives here)
    std::map<int, Visitor> present;
    for (Ptr person : game::LotPeople()) {
        if (Contains(family, game::SimNeighborId(person))) continue;
        game::LotObject o;
        if (!game::GetObject(person, &o)) continue;
        auto known = g.visitors.find(o.id);
        if (known != g.visitors.end()) {
            present[o.id] = known->second;
            continue;
        }
        Visitor v;  // new here; those present when the lot was first seen count as announced
        v.name = Trimmed(game::ObjectName(o));
        v.ghost = game::IsGhost(person);
        v.burglarShout = Trimmed(game::BurglarShout(person));  // the game's own "¡Ladrón!"
        v.since = now;
        v.announced = !g.visitorsSeeded;
        present[o.id] = v;
    }
    for (auto& [id, v] : present) {
        if (v.announced || now - v.since < kVisitorStayMs) continue;
        v.announced = true;
        if (v.name.empty()) continue;
        if (v.ghost) {
            Announce(util::Format(S(Str::GhostAppeared), v.name.c_str()), false);
            continue;
        }
        std::string line = util::Format(S(Str::VisitorArrived), v.name.c_str());
        if (v.burglarShout.empty()) Announce(line, false);
        else Announce(v.burglarShout + " " + line, true);
    }
    for (const auto& [id, v] : g.visitors)
        if (!present.count(id) && v.announced && !v.name.empty())
            Announce(util::Format(S(v.ghost ? Str::GhostLeft : Str::VisitorLeft), v.name.c_str()), false);
    g.visitors = std::move(present);
    g.visitorsSeeded = true;
}

void ReportTaken(const game::TakenObject& t) {
    std::string value = util::Money(t.value);
    std::string taker = Trimmed(t.taker), object = Trimmed(t.object);
    std::string line;
    if (!taker.empty())
        line = t.value > 0 ? util::Format(S(Str::ObjectTakenBy), taker.c_str(), object.c_str(), value.c_str())
                           : util::Format(S(Str::ObjectTakenByNoValue), taker.c_str(), object.c_str());
    else
        line = t.value > 0 ? util::Format(S(Str::ObjectGone), object.c_str(), value.c_str())
                           : util::Format(S(Str::ObjectGoneNoValue), object.c_str());
    Announce(line, true);
}

// ---- Sounds whose meaning the sound does not give ---------------------------------------------

// A car honks again and again while it waits: once per this long.
constexpr ULONGLONG kAlertRepeatMs = 20000;

Str AlertText(game::SoundAlert a) {
    using game::SoundAlert;
    switch (a) {
    case SoundAlert::Carpool: return Str::SoundCarpool;
    case SoundAlert::SchoolBus: return Str::SoundSchoolBus;
    case SoundAlert::PoliceCar: return Str::SoundPoliceCar;
    }
    return Str::SoundCarpool;
}

void ReportSound(const game::SoundEvent& e) {
    ULONGLONG now = GetTickCount64();
    auto last = g.alertSaid.find((int)e.alert);
    if (last != g.alertSaid.end() && now - last->second < kAlertRepeatMs) return;
    g.alertSaid[(int)e.alert] = now;
    std::string line = S(AlertText(e.alert));
    Log("sound '%s' from #%d: %s", e.name.c_str(), e.source, line.c_str());
    Announce(line, true);
}

}  // namespace

void Reset() {
    g = State{};
    game::TakeBalloons();
    game::TakeTakenObjects();
    game::TakeSoundEvents();
    simwatch::Reset();
}

void Announce(const std::string& line, bool important) {
    int v = CurrentVerbosity();
    if (v == kEverything || (important && v == kWarnings)) buffers::Event(line);
    else Record(line);
}

void Record(const std::string& line) {
    Log("narrator (not spoken): %s", line.c_str());
    buffers::Record(line);
}

void CycleVerbosity() {
    int v = CurrentVerbosity();
    g_verbosity = v == kEverything ? kWarnings : v == kWarnings ? kOff : kEverything;
    util::SetSetting("narrator", g_verbosity);
    speech::Say(S(g_verbosity == kEverything ? Str::NarratorEverything
                  : g_verbosity == kWarnings ? Str::NarratorWarnings
                                             : Str::NarratorOff));
}

void SayStatus() {
    if (!LotViewActive() || !game::LotModule()) {
        speech::Say(S(Str::NotOnLot));
        return;
    }
    speech::Say(simwatch::Status());
}

void Tick() {
    Ptr lot = game::LotModule();
    if (lot != g.lot) {  // another lot, or none: nothing seen before applies
        Reset();
        g.lot = lot;
    }
    std::vector<Balloon> balloons = game::TakeBalloons();
    std::vector<game::TakenObject> taken = game::TakeTakenObjects();
    std::vector<game::SoundEvent> sounds = game::TakeSoundEvents();
    if (!lot) return;
    if (g.spokenAt.size() + g.treeSeen.size() > 500) {  // forget what is long past its quiet time
        ULONGLONG now = GetTickCount64();
        for (auto it = g.spokenAt.begin(); it != g.spokenAt.end();)
            it = now - it->second > kObjectRepeatMs ? g.spokenAt.erase(it) : std::next(it);
        for (auto it = g.treeSeen.begin(); it != g.treeSeen.end();)
            it = now - it->second > kTreeQuietMs ? g.treeSeen.erase(it) : std::next(it);
    }
    std::vector<int> family = game::FamilyNeighborIds();
    for (const Balloon& b : balloons) Report(b, family);
    for (const game::TakenObject& t : taken) ReportTaken(t);
    for (const game::SoundEvent& e : sounds) ReportSound(e);
    PollVisitors(family);
    simwatch::Tick();
}

std::string ShownBalloon(Ptr object) {
    game::ShownBalloon now;
    if (!game::BalloonShownOver(object, &now)) return "";
    game::LotObject o;
    if (!game::GetObject(object, &o)) return "";
    auto it = g.shown.find(o.id);
    return it != g.shown.end() && it->second.record == now ? it->second.text : "";
}

}  // namespace nav::narrator
