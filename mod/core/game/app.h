// State of the game's app object ([kSimRoot]) that is not about the lot: the language the game
// chose at startup (util/language.h has the numbers and the order of the sources: registry,
// Steam, -l<Language> switch).
#pragma once

namespace game {

// The game's language number, 1 (US English) to 20 (Korean); 0 before the app object exists.
int LanguageNumber();

}  // namespace game
