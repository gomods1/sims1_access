#include "iff_strings.h"

#include <windows.h>

#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>

#include "../util/log.h"
#include "../util/text.h"
#include "app.h"

namespace game {
namespace {

constexpr size_t kIffHeader = 64, kChunkHeader = 76;

uint32_t Be32(const std::string& d, size_t at) {
    return (uint32_t)(uint8_t)d[at] << 24 | (uint32_t)(uint8_t)d[at + 1] << 16 | (uint32_t)(uint8_t)d[at + 2] << 8 |
           (uint8_t)d[at + 3];
}

std::string GameDataPath(const char* file) {
    char exe[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string dir = exe;
    size_t slash = dir.find_last_of("\\/");
    return (slash == std::string::npos ? std::string() : dir.substr(0, slash + 1)) + "GameData\\" + file;
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

// Every STR# chunk of a file: id -> body.
std::map<int, std::string> ReadTables(const char* file) {
    std::map<int, std::string> tables;
    std::string path = GameDataPath(file);
    std::ifstream f(path, std::ios::binary);
    std::string data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (data.size() < kIffHeader || data.compare(0, 4, "IFF ") != 0) {
        Log("strings: cannot read %s", path.c_str());
        return tables;
    }
    for (size_t pos = kIffHeader; pos + kChunkHeader <= data.size();) {
        uint32_t size = Be32(data, pos + 4);
        if (size < kChunkHeader || pos + size > data.size()) break;
        int id = (uint8_t)data[pos + 8] << 8 | (uint8_t)data[pos + 9];
        if (data.compare(pos, 4, "STR#") == 0) tables[id] = data.substr(pos + kChunkHeader, size - kChunkHeader);
        pos += size;
    }
    return tables;
}

}  // namespace

const std::vector<std::string>& GameDataStrings(const char* file, int id) {
    static std::map<std::string, std::map<int, std::string>> files;  // file -> raw tables
    static std::map<std::pair<std::string, int>, std::vector<std::string>> cache;
    static int cachedLanguage = -1;
    int language = LanguageNumber();
    if (language != cachedLanguage) {  // read before the game's language was known
        cache.clear();
        cachedLanguage = language;
    }
    auto key = std::make_pair(std::string(file), id);
    auto hit = cache.find(key);
    if (hit != cache.end()) return hit->second;
    auto fileIt = files.find(file);
    if (fileIt == files.end()) fileIt = files.emplace(file, ReadTables(file)).first;
    std::vector<std::string> entries;
    auto table = fileIt->second.find(id);
    if (table != fileIt->second.end()) {
        std::vector<std::string> raw = TableLines(table->second, language);
        if (raw.empty()) raw = TableLines(table->second, 1);  // US English when the language has none
        for (const std::string& e : raw) entries.push_back(util::GameTextToUtf8(e));
    }
    return cache.emplace(key, std::move(entries)).first->second;
}

std::string GameDataString(const char* file, int id, int index) {
    const std::vector<std::string>& entries = GameDataStrings(file, id);
    return index >= 0 && index < (int)entries.size() ? entries[index] : std::string();
}

}  // namespace game
