// The cheat console (Ctrl+Shift+C): a text field of the main window (cTSMainWindowW95 + 0x140).
// Enter in it runs the line through the cheat manager (main window + 0x144, cTSCheatCodeManager,
// interpreter 0x6e67d0) and closes it; Escape closes it (the main window's handler 0x6f0e70).
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

Ptr CheatField();          // the console's text field, 0 if the game has none
bool CheatConsoleOpen();   // shown and taking the typing
bool OpenCheatConsole();   // what the game's Ctrl+Shift+C does (main window slot 107)

// The cheats this build registers, by name, with the usage text the game keeps for each
// ("grid [on|off] - enables/disables terrain grid drawing"; many have none). UTF-8.
struct CheatCommand {
    std::string name;
    std::string usage;
};
std::vector<CheatCommand> CheatCommands();

bool SetCheatText(const std::string& utf8);  // what the console's field shows
// Runs a line through the game's cheat interpreter, as Enter in the console does, but leaves the
// console open. The game shows its own errors (a message box). true when it ran.
bool RunCheat(const std::string& utf8);

}  // namespace game
