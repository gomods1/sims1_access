#include "cheat_console.h"

#include <windows.h>

#include <algorithm>
#include <string>

#include "../game/buy.h"
#include "../game/cheats.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"

namespace nav::cheats {
namespace {

constexpr ULONGLONG kHintMs = 700;  // after the navigator has announced the console

struct State {
    int index = -1;            // the cheat picked with Up/Down, -1 none yet
    bool open = false;
    ULONGLONG openedAt = 0;
    bool hinted = false;       // the keys were said once this session
} g;

// "grid [on|off] - enables/disables terrain grid drawing" -> the part after the name, if any.
std::string UsageOf(const game::CheatCommand& c) {
    std::string usage = c.usage;
    if (usage.rfind(c.name, 0) == 0) usage = usage.substr(c.name.size());
    size_t start = usage.find_first_not_of(" -");
    return start == std::string::npos ? "" : usage.substr(start);
}

void Say(const std::vector<game::CheatCommand>& list) {
    const game::CheatCommand& c = list[g.index];
    std::string line = util::Format(S(Str::CheatOf), c.name.c_str(), g.index + 1, (int)list.size());
    std::string usage = UsageOf(c);
    if (!usage.empty()) line += ". " + usage;
    buffers::SetDetails({c.name, usage});
    speech::Say(line);
}

void Pick(int index) {
    std::vector<game::CheatCommand> list = game::CheatCommands();
    if (list.empty()) {
        speech::Say(S(Str::CheatsUnavailable));
        return;
    }
    g.index = std::clamp(index, 0, (int)list.size() - 1);  // the ends repeat
    game::SetCheatText(list[g.index].name);
    Say(list);
}

}  // namespace

void Open() {
    if (game::CheatConsoleOpen()) return;
    if (!game::OpenCheatConsole()) speech::Say(S(Str::CheatsUnavailable));
    // The navigator announces the console as the field that now takes the typing.
}

bool Entering() { return game::CheatConsoleOpen(); }

void Select(int delta) { Pick(g.index < 0 ? (delta > 0 ? 0 : 1 << 20) : g.index + delta); }

void SelectEdge(bool last) { Pick(last ? 1 << 20 : 0); }

// The line runs in the game's interpreter; the console stays open with it, so Enter repeats it.
void Run() {
    std::string line = util::CleanForSpeech(game::TextFieldText(game::CheatField()));
    if (line.empty()) {
        speech::Say(S(Str::CheatEmpty));
        return;
    }
    int before = 0, after = 0;
    bool counted = game::GetFunds(&before);
    Log("cheats: running \"%s\"", line.c_str());
    if (!game::RunCheat(line)) return;  // the game said why in a message box
    std::string text = util::Format(S(Str::CheatRan), line.c_str());
    if (counted && game::GetFunds(&after) && after != before) {
        std::string change = (after > before ? "+" : "") + util::Money(after - before);
        text += ". " + util::Format(S(Str::FundsNow), util::Money(after).c_str(), change.c_str());
    }
    buffers::Event(text);
}

void Tick() {
    bool open = game::CheatConsoleOpen();
    if (open && !g.open) g.openedAt = GetTickCount64();
    g.open = open;
    if (open && !g.hinted && GetTickCount64() - g.openedAt > kHintMs) {
        g.hinted = true;
        speech::Say(S(Str::CheatHint));
    }
}

}  // namespace nav::cheats
