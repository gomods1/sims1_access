// Turns a group's collected items into what a player moves through: labels merge into the
// controls they name (text fields, sliders, check boxes, value pickers), and items take the
// order a sighted player reads them in (or the game's own order for lists it defines).
#pragma once

#include "model.h"

namespace nav {

void ArrangeGroup(Group& g);

// Page arrows ("Siguiente página") are thin, tall buttons on a panel's edges; ArrangeGroup puts
// them at the end of their group.
bool IsPageArrow(const Item& it);

}  // namespace nav
