// What the Sims' behavior code (SimAntics) shows or does that the mod reports: the balloons the
// game puts over Sims and objects (thoughts, shouts, conversation topics, the icons over heads),
// and objects that someone outside the family takes off the lot (the burglar, the repo man). The
// hooks in hooks.cpp call the On* functions while the game runs the primitive; the narrator
// takes the queued events on its tick.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "memory.h"

namespace game {

enum class BalloonKind { Thought, Scream, Speak, None };

// Icon groups (Behavior.iff STR# 500). Old style icons (group 0) are reported as the motive or
// conversation icon they show.
namespace balloon {
constexpr int kConversation = 2;   // topics: index / 3 = interest (Live.iff STR# 140 order), then
                                   // careers, bored, bad mood
constexpr int kMotive = 3;         // needs
constexpr int kRelationship = 4;   // romance, friends
constexpr int kHeadline = 5;       // hate, stress, surprise, idea, love, drunk, hurt, smell, flies
constexpr int kSocial = 6;         // interactions, "+"/"-" after a social, routing trouble, fame
constexpr int kPicture = 7;        // an object's picture or a Sim's face
constexpr int kRouteFailure = 8;   // chair / door not found, wall in the way, ...
constexpr int kProgress = 9;       // skill, repair, interest progress in tenths
constexpr int kConversation2 = 10; // magic
}  // namespace balloon

struct Balloon {
    int ownerId = 0;              // the Sim or object it is shown over
    int group = 0;                // balloon::
    int index = 0;                // icon in the group
    BalloonKind kind = BalloonKind::None;
    bool crossed = false;         // crossed out
    bool routeFailure = false;    // set by the router's feedback: the Sim cannot get where it was going
    int pictureObjectId = 0;      // kPicture: the object whose picture is shown
    int pictureNeighborId = -1;   // kPicture: the Sim whose face is shown
    int otherId = 0;              // the tree's stack object: the other Sim of a social interaction
    int tree = 0;                 // the tree that set it (for the log)
};

// An object someone outside the family took off the lot.
struct TakenObject {
    std::string taker;   // UTF-8; empty when the object was not taken by a Sim
    std::string object;  // UTF-8
    int value = 0;       // what it was worth (its sale value), 0 when unknown
};

std::vector<Balloon> TakeBalloons();        // new since the last call, oldest first
std::vector<TakenObject> TakeTakenObjects();

// The balloon shown over an object right now, by the record the game draws from: true when one
// is shown. Enough to tell whether a reported balloon is still up.
struct ShownBalloon {
    int sprite = 0;      // -1 = a picture
    Ptr picture = 0;
    int balloon = 0;
    int crossed = 0;
    bool operator==(const ShownBalloon& o) const {
        return sprite == o.sprite && picture == o.picture && balloon == o.balloon && crossed == o.crossed;
    }
};
bool BalloonShownOver(Ptr object, ShownBalloon* out);

// Neighbor IDs of the family living on the lot (what the family panel shows).
std::vector<int> FamilyNeighborIds();

// A Sim a grave (urn or tombstone) brought out: a ghost. Known from the moment the grave's
// script creates it, so a ghost already out when the mod (re)loaded is not recognised.
bool IsGhost(Ptr person);
// The Grim Reaper's errand: the name of the Sim who is dying (who brought him), "" otherwise.
std::string ReaperFor(Ptr person);

// ---- For hooks.cpp (main thread, inside the game's primitive) ----------------------------
// Plain data only: the game's SimAntics throws C++ exceptions through the hook.
struct BalloonCall {
    bool watch;
    Ptr owner;
    ShownBalloon before;
    bool shownBefore;
    int group, index, kind, tree, otherId, pictureObjectId, pictureNeighborId;
    bool crossed;
};
void BeforeSetBalloon(Ptr runner, Ptr frame, const uint8_t* operand, BalloonCall* call);
void AfterSetBalloon(const BalloonCall& call);
void BeforePrimitive(Ptr runner, Ptr frame, const uint8_t* instruction);
void AfterCreateObject(Ptr runner, Ptr frame);  // diagnostics: Sims created by the game's scripts

}  // namespace game
