// The house at a glance, for a builder who cannot see it: spoken when build mode opens, and on
// Ctrl+Shift+L anywhere on a lot. Its floors and rooms (named, with sizes) and the yard; the
// game's house ratings; what a household needs and the house lacks (toilet, shower or bath,
// beds for everyone, fridge, phone, smoke alarm); what is wrong (rooms nobody can get into,
// objects Sims cannot reach, dark rooms, rooms the Sims dislike). The first lines are spoken,
// everything is in the details buffer (Ctrl+Up/Down). In the neighborhood the same overview of a
// house comes from its file (nav/house_plan.h), in the details of the focused house.
#pragma once

#include <string>
#include <vector>

namespace nav::house {

void Reset();
void Tick();          // build mode opening: the overview
void SayOverview();   // Ctrl+Shift+L

// The overview's lines for a house of the neighborhood from its file: floors and rooms, what is
// missing, rooms with no way in (the engine's own checks, ratings, light, unreachable objects,
// need the lot loaded). Empty when the house has no readable file.
std::vector<std::string> FileOverview(int houseNumber);

}  // namespace nav::house
