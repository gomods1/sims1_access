#include "credits.h"

#include "../game/iff_strings.h"
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

std::vector<std::string> CreditLines() {
    std::vector<std::string> lines;
    // The collection's own team first, then the games in order.
    std::vector<int> tables = {kLastTable};
    for (int id = kFirstTable; id < kLastTable; id++) tables.push_back(id);
    for (int id : tables)
        for (const std::string& e : game::GameDataStrings("Credits.iff", id)) {
            std::string line = util::CleanForSpeech(e);
            if (!line.empty()) lines.push_back(line);
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
