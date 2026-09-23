#include "credits.h"

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>

#include "../game/app.h"
#include "../game/rtti.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"

namespace nav::credits {
namespace {

// GameData\Credits.iff keeps one STR# per game (163 The Sims ... 171 Makin' Magic, 172 the Legacy
// Collection), language tagged (format -3). The screen scrolls them as drawn text only.
constexpr int kFirstTable = 163, kLastTable = 172;
constexpr size_t kIffHeader = 64, kChunkHeader = 76;

uint32_t Be32(const std::string& d, size_t at) {
    return (uint32_t)(uint8_t)d[at] << 24 | (uint32_t)(uint8_t)d[at + 1] << 16 | (uint32_t)(uint8_t)d[at + 2] << 8 |
           (uint8_t)d[at + 3];
}

std::string ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

std::string GameDataPath() {
    char exe[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string dir = exe;
    size_t slash = dir.find_last_of("\\/");
    return (slash == std::string::npos ? std::string() : dir.substr(0, slash + 1)) + "GameData\\Credits.iff";
}

// The entries of one STR# -3 table in `language`, in order (raw game text).
std::vector<std::string> TableLines(const std::string& body, int language) {
    std::vector<std::string> out;
    if (body.size() < 4 || (uint8_t)body[0] != 0xfd || (uint8_t)body[1] != 0xff) return out;
    size_t count = (uint8_t)body[2] | (uint8_t)body[3] << 8;
    size_t pos = 4;
    for (size_t i = 0; i < count && pos < body.size(); i++) {
        int lang = (uint8_t)body[pos++];
        size_t end = body.find('\0', pos);
        if (end == std::string::npos) break;
        std::string value = body.substr(pos, end - pos);
        pos = body.find('\0', end + 1);  // the comment
        if (pos == std::string::npos) break;
        pos++;
        if (lang == language) out.push_back(value);
    }
    return out;
}

std::vector<std::string> CreditLines() {
    std::string data = ReadFile(GameDataPath());
    std::vector<std::string> lines;
    if (data.size() < kIffHeader || data.compare(0, 4, "IFF ") != 0) {
        Log("credits: cannot read %s", GameDataPath().c_str());
        return lines;
    }
    int language = game::LanguageNumber();
    std::vector<std::pair<int, std::string>> tables;  // id -> body
    for (size_t pos = kIffHeader; pos + kChunkHeader <= data.size();) {
        uint32_t size = Be32(data, pos + 4);
        if (size < kChunkHeader || pos + size > data.size()) break;
        int id = (uint8_t)data[pos + 8] << 8 | (uint8_t)data[pos + 9];
        if (data.compare(pos, 4, "STR#") == 0 && id >= kFirstTable && id <= kLastTable)
            tables.push_back({id, data.substr(pos + kChunkHeader, size - kChunkHeader)});
        pos += size;
    }
    // The collection's own team first, then the games in order.
    std::stable_sort(tables.begin(), tables.end(), [](const auto& a, const auto& b) {
        return (a.first == kLastTable) != (b.first == kLastTable) ? a.first == kLastTable : a.first < b.first;
    });
    for (const auto& [id, body] : tables) {
        std::vector<std::string> entries = TableLines(body, language);
        if (entries.empty()) entries = TableLines(body, 1);  // US English when the language has none
        for (const std::string& e : entries) {
            std::string line = util::CleanForSpeech(util::GameTextToUtf8(e));
            if (!line.empty()) lines.push_back(line);
        }
    }
    return lines;
}

game::Ptr g_seen = 0;

}  // namespace

bool OnScreen(game::Ptr root) {
    if (!game::IsKindOf(root, "cWinCredits")) {
        g_seen = 0;
        return false;
    }
    if (root == g_seen) return true;
    g_seen = root;
    std::vector<std::string> lines = CreditLines();
    Log("credits: %d lines", (int)lines.size());
    buffers::SetDetails(lines);
    return true;
}

}  // namespace nav::credits
