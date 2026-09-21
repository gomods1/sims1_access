#include "cheats.h"

#include "../util/log.h"
#include "../util/text.h"
#include "addresses.h"
#include "rtti.h"
#include "ui.h"

namespace game {
namespace {

Ptr Manager() {
    Ptr manager = Rd<Ptr>(MainWindow() + addr::cheat::kManager);
    return IsKindOf(manager, "cTSCheatCodeManager") ? manager : 0;
}

// MSVC std::map nodes: left +0, parent +4, right +8, is-nil byte +0xd, the key (std::string) at
// +0x10 and the value right after it at +0x28. The head's parent is the root.
void Walk(Ptr node, int depth, std::vector<Ptr>* out) {
    if (!node || depth > 32 || out->size() > 500 || Rd<uint8_t>(node + 0xd)) return;
    Walk(Rd<Ptr>(node), depth + 1, out);
    out->push_back(node);
    Walk(Rd<Ptr>(node + 8), depth + 1, out);
}

std::vector<Ptr> MapNodes(Ptr head) {
    std::vector<Ptr> nodes;
    if (head) Walk(Rd<Ptr>(head + 4), 0, &nodes);
    return nodes;
}

// cp1252 for the game; the mod's text is UTF-8 (cheat lines are ASCII in practice).
std::string ToGame(const std::string& utf8) {
    std::string out;
    for (size_t i = 0; i < utf8.size(); i++) {
        unsigned char c = (unsigned char)utf8[i];
        if (c < 0x80) out += (char)c;
        else if ((c & 0xe0) == 0xc0 && i + 1 < utf8.size()) {
            out += (char)(((c & 0x1f) << 6) | ((unsigned char)utf8[i + 1] & 0x3f));
            i++;
        }
    }
    return out;
}

}  // namespace

Ptr CheatField() {
    Ptr field = Rd<Ptr>(MainWindow() + addr::cheat::kField);
    return IsKindOf(field, "cTSWinTextEdit2") ? field : 0;
}

bool CheatConsoleOpen() {
    Ptr field = CheatField();
    return field && Visible(field) && KeyboardFocus() == field;
}

bool OpenCheatConsole() {
    typedef void(__thiscall * Fn)(void*, char);
    Ptr main = MainWindow();
    Ptr fn = Rd<Ptr>(Rd<Ptr>(main) + addr::cheat::kShowSlot * 4);
    if (!CheatField() || fn != Va(addr::cheat::kShow) || !FunctionOk(addr::cheat::kShow)) {
        Log("cheats: cannot open the console (slot %p)", (void*)fn);
        return false;
    }
    return Guarded([&] { ((Fn)fn)((void*)main, 1); }) && CheatConsoleOpen();
}

std::vector<CheatCommand> CheatCommands() {
    std::vector<CheatCommand> out;
    Ptr manager = Manager();
    if (!manager) return out;
    for (Ptr node : MapNodes(Rd<Ptr>(manager + addr::cheat::kCommands)))
        out.push_back({ReadStdStringUtf8(node + 0x10), ""});
    for (Ptr node : MapNodes(Rd<Ptr>(manager + addr::cheat::kUsages))) {
        std::string name = ReadStdStringUtf8(node + 0x10);
        for (CheatCommand& c : out)
            if (c.name == name) c.usage = util::CleanForSpeech(ReadStdStringUtf8(node + 0x28));
    }
    return out;
}

bool SetCheatText(const std::string& utf8) {
    typedef void(__thiscall * Fn)(void*, const char*, char, char);
    Ptr field = CheatField();
    if (!field || !FunctionOk(addr::cheat::kSetText)) return false;
    std::string text = ToGame(utf8);
    return Guarded([&] { ((Fn)Va(addr::cheat::kSetText))((void*)field, text.c_str(), 0, 0); });
}

bool RunCheat(const std::string& utf8) {
    typedef bool(__thiscall * Fn)(void*, const char*);
    Ptr manager = Manager();
    if (!manager || !FunctionOk(addr::cheat::kRun)) return false;
    std::string line = ToGame(utf8);
    if (line.size() > 255) line.resize(255);
    bool ran = false;
    Guarded([&] { ran = ((Fn)Va(addr::cheat::kRun))((void*)manager, line.c_str()); });
    return ran;
}

}  // namespace game
