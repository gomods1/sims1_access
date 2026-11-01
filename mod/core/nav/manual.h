// The player's manual: docs/manual/<tag>.md, built by tools/build_docs.py into
// SimsAccess\docs\<tag>.html. Control+Shift+F1 and the "mod help" group's button (last group of
// every screen) open it in the player's browser, in the game's language (English when that page
// is missing).
#pragma once

#include <string>

namespace nav::manual {

void Init(const wchar_t* modDir);

// Opens the manual in the default browser; says so, or that the manual is missing.
void Open();

}  // namespace nav::manual
