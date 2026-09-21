// The house the family lives in, as the family panel's house subpanel (cWinSubpanelHouse)
// shows it: five ratings drawn as bars, and text lines for area, bedrooms, bathrooms, lot size.
#pragma once

#include <vector>

#include "memory.h"

namespace game {

constexpr int kHouseRatingCount = 5;  // size, furnishings, yard, upkeep, layout (the panel's order)

// Ratings 0..10 (bars), computed the way the panel does it; false when no lot is loaded.
bool HouseRatings(int out[kHouseRatingCount]);

// The subpanel's buttons: 0..4 are the ratings (caption = rating name), 5..8 the text lines
// (captions set while the panel is drawn: "Área: 120 m2", "Dormitorios: 1", ...).
std::vector<Ptr> HousePanelButtons(Ptr housePanel);

}  // namespace game
