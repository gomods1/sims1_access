// The lot by ear: what a sighted player notices without asking, spoken as it happens and kept in
// the events buffer.
//   Balloons: the thought balloons, shouts, conversation topics and icons the game puts over Sims
//     and objects ("Bella piensa: hambre", "Bella no puede pasar por Silla de comedor", "Mortimer:
//     la relación mejora", "Cubo de basura: moscas"). The family's are spoken (the selected Sim's
//     skill progress too); a visitor's only when it concerns the family or is a shout.
//   Visitors: Sims from outside the family arriving on the lot and leaving it (neighbors, the
//     pizza delivery, the burglar, the police).
//   Taken objects: furniture someone outside the family takes off the lot (the burglar, the repo
//     man), with what it was worth.
// Nothing here changes game state.
#pragma once

#include <string>

#include "../game/memory.h"

namespace nav::narrator {

void Reset();
void Tick();

// The balloon shown over a Sim or object right now, in a few words ("piensa: hambre"); empty
// when none (or one the mod has not seen appear).
std::string ShownBalloon(game::Ptr object);

}  // namespace nav::narrator
