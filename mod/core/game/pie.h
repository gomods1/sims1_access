// The interaction (pie) menu while it runs modally: its items and choosing one by index.
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

bool IsPieMenu(Ptr w);
std::vector<std::string> PieItems(Ptr pie);  // labels of the shown page, UTF-8
void PieHighlight(Ptr pie, int index);       // the highlight a sighted player sees
void PieChoose(Ptr pie, int index);          // chooses the item: queues it or opens its sub-level
void PieEnd(Ptr pie, int result);            // ends the menu's modal loop with a raw result
bool PieIsSubMenu(Ptr pie);
// Escape: a sub-level goes back to the top level, the top level closes the menu.
void PieCancel(Ptr pie);

}  // namespace game
