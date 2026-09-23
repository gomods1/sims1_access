// The house at a glance, for a builder who cannot see it: spoken when build mode opens, and on
// Ctrl+Shift+L anywhere on a lot. Its floors and rooms (named, with sizes) and the yard; the
// game's house ratings; what a household needs and the house lacks (toilet, shower or bath,
// beds for everyone, fridge, phone, smoke alarm); what is wrong (rooms nobody can get into,
// objects Sims cannot reach, dark rooms, rooms the Sims dislike). The first lines are spoken,
// everything is in the details buffer (Ctrl+Up/Down).
#pragma once

namespace nav::house {

void Reset();
void Tick();          // build mode opening: the overview
void SayOverview();   // Ctrl+Shift+L

}  // namespace nav::house
