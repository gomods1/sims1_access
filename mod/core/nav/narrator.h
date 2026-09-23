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
//   The family (nav/sim_watch.h): the selected Sim's room and actions, need and mood warnings.
// Verbosity (Ctrl+N cycles it, kept in SimsAccess.ini): everything; warnings only (needs, mood,
// route failures, shouts, the burglar, thefts); off. What is not spoken still goes to the events
// buffer. Ctrl+E says the selected Sim's state.
// Nothing here changes game state.
#pragma once

#include <string>

#include "../game/memory.h"

namespace nav::narrator {

void Reset();
void Tick();

// A line of the narrator: spoken when the verbosity lets it through, always kept in the events
// buffer. important = a warning (spoken unless the narrator is off).
void Announce(const std::string& line, bool important);
void Record(const std::string& line);  // only kept in the buffer
// A lot was chosen on a travel map: its name is said when the family is on the next lot.
void NoteDestination(const std::string& name);
// A + or - balloon (a social's effect) showed over the Sim since then.
bool RelationSignSince(int neighborId, unsigned long long since);

void CycleVerbosity();  // Ctrl+N: everything -> warnings only -> off -> everything
void SayStatus();       // Ctrl+E: the selected Sim's room, actions, low needs and mood

// The balloon shown over a Sim or object right now, in a few words ("piensa: hambre"); empty
// when none (or one the mod has not seen appear).
std::string ShownBalloon(game::Ptr object);

}  // namespace nav::narrator
