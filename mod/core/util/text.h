// Text helpers. Game strings are Windows-1252; everything the mod speaks is UTF-8.
#pragma once

#include <string>

namespace util {

std::string Cp1252ToUtf8(const std::string& s);

// Line breaks become a spoken pause (". " unless the line already ends in punctuation), runs of
// whitespace collapse to one space, control characters are dropped. UTF-8 in and out.
std::string CleanForSpeech(const std::string& utf8);

// Drops a trailing hotkey hint from a game label: "Velocidad normal - 1" -> "Velocidad normal",
// "Pausa - P" -> "Pausa". Labels without one ("Ver de cerca +") are returned unchanged.
std::string WithoutHotkey(const std::string& label);

// Money as the game writes it: "$1.200" in Spanish, "$1,200" in English.
std::string Money(int amount);

// snprintf into a std::string.
std::string Format(const char* fmt, ...);

}  // namespace util
