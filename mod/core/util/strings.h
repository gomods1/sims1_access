// The mod's own spoken strings in the game's language (util/language.h). English is the source
// (strings_table.h); each translation is a file in lang/, and a string it lacks is spoken in English.
#pragma once

#include "strings_table.h"

enum class Str {
#define SA_STRING_ID(id, english) id,
    SA_STRINGS(SA_STRING_ID)
#undef SA_STRING_ID
    Count
};

namespace lang {
struct Translations;
}

namespace util {

// Speaks `translations` from now on; nullptr speaks English. Called by util/language.cpp.
void LoadStrings(const lang::Translations* translations);

}  // namespace util

// UTF-8 text of a mod string in the current language.
const char* S(Str id);
