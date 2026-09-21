// The mod's own spoken strings in the game's language (SIMS_LANGUAGE registry value).
#pragma once

#include "strings_table.h"

enum class Str {
#define SA_STRING_ID(id, en, es) id,
    SA_STRINGS(SA_STRING_ID)
#undef SA_STRING_ID
    Count
};

namespace util {

void InitStrings();
bool LanguageIsSpanish();

}  // namespace util

// UTF-8 text of a mod string in the current language.
const char* S(Str id);
