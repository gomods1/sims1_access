#include "strings.h"

#include <algorithm>
#include <iterator>

#include "../lang/translations.h"
#include "log.h"

namespace util {
namespace {

const char* const kEnglish[] = {
#define SA_STRING_ENGLISH(id, english) english,
    SA_STRINGS(SA_STRING_ENGLISH)
#undef SA_STRING_ENGLISH
};
static_assert(std::size(kEnglish) == (size_t)Str::Count, "string table size");

const char* g_strings[(size_t)Str::Count] = {};  // the current language; null: English

}  // namespace

void LoadStrings(const lang::Translations* translations) {
    std::fill(std::begin(g_strings), std::end(g_strings), nullptr);
    if (!translations) return;
    for (size_t i = 0; i < translations->count; i++) {
        const lang::Translation& t = translations->entries[i];
        if ((size_t)t.id < (size_t)Str::Count) g_strings[(size_t)t.id] = t.text;
    }
    size_t missing = std::count(std::begin(g_strings), std::end(g_strings), nullptr);
    if (missing) Log("%zu mod strings are not translated; they are spoken in English", missing);
}

}  // namespace util

const char* S(Str id) {
    size_t i = (size_t)id;
    return util::g_strings[i] ? util::g_strings[i] : util::kEnglish[i];
}
