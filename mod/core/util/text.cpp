#include "text.h"

#include "strings.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <vector>

namespace util {

std::string Cp1252ToUtf8(const std::string& s) {
    if (s.empty()) return s;
    int wide = MultiByteToWideChar(1252, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(wide, L'\0');
    MultiByteToWideChar(1252, 0, s.data(), (int)s.size(), w.data(), wide);
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), wide, nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), wide, out.data(), n, nullptr, nullptr);
    return out;
}

namespace {

bool EndsWithPunctuation(const std::string& s) {
    if (s.empty()) return false;
    switch (s.back()) {
    case '.': case ',': case ':': case ';': case '!': case '?': return true;
    default: return false;
    }
}

std::string CollapseSpaces(const std::string& line) {
    std::string out;
    bool pendingSpace = false;
    for (char c : line) {
        if (c == ' ' || c == '\t') {
            pendingSpace = !out.empty();
            continue;
        }
        if (pendingSpace) out += ' ';
        pendingSpace = false;
        out += c;
    }
    return out;
}

}  // namespace

std::string CleanForSpeech(const std::string& in) {
    std::vector<std::string> lines(1);
    for (char c : in) {
        if (c == '\r' || c == '\n') lines.emplace_back();
        else lines.back() += (unsigned char)c < 0x20 ? ' ' : c;
    }
    std::string out;
    for (const std::string& raw : lines) {
        std::string line = CollapseSpaces(raw);
        if (line.empty()) continue;
        if (!out.empty()) out += EndsWithPunctuation(out) ? " " : ". ";
        out += line;
    }
    return out;
}

std::string WithoutHotkey(const std::string& label) {
    size_t dash = label.rfind(" - ");
    if (dash == std::string::npos || dash == 0) return label;
    size_t hotkeyLength = label.size() - (dash + 3);
    return hotkeyLength >= 1 && hotkeyLength <= 3 ? label.substr(0, dash) : label;
}

std::string Money(int amount) {
    std::string digits = std::to_string(amount < 0 ? -amount : amount);
    char separator = LanguageIsSpanish() ? '.' : ',';
    for (int i = (int)digits.size() - 3; i > 0; i -= 3) digits.insert(digits.begin() + i, separator);
    return (amount < 0 ? "-$" : "$") + digits;
}

std::string Format(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    return buf;
}

}  // namespace util
