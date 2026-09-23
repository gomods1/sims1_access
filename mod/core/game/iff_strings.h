// String tables (STR#, format -3: every entry tagged with its language) read straight from the
// game's GameData IFF files, for text the game keeps there but only draws (credits, the live
// panels' hover help).
#pragma once

#include <string>
#include <vector>

namespace game {

// The entries of STR# `id` in GameData\<file> in the game's language (US English when the table
// has none in it), converted to UTF-8, in order. Read once per file and table; empty when the file
// or the table is missing.
const std::vector<std::string>& GameDataStrings(const char* file, int id);

// One entry, "" when out of range.
std::string GameDataString(const char* file, int id, int index);

}  // namespace game
