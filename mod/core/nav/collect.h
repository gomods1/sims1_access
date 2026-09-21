// Turns the live widget tree under the input root into groups of navigable items, in the order a
// sighted player reads the screen.
#pragma once

#include <vector>

#include "model.h"

namespace nav {

struct Screen {
    std::vector<Group> groups;
    bool textEntryVisible = false;  // a text field is on screen: letters and Space are typing
};

Screen Collect(Ptr root);

}  // namespace nav
