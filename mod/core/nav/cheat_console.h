// The cheat console by keyboard. Ctrl+Shift+C opens it (the mod calls the game's own opener, so
// it works whatever way the keys arrive).
//   Up / Down, Home / End   pick a cheat from the ones this build registers (read from the game,
//                           with its usage text when it has one); its name goes into the field
//   typing                  edits the line (echoed; add parameters after the name: "set_hour 12")
//   Enter                   runs the line and keeps the console open, so Enter again repeats it
//   Escape                  closes the console (the game's)
// What a cheat did (the money it brought) is said and kept in the events buffer; the game's own
// errors come as message boxes.
#pragma once

namespace nav::cheats {

void Open();
bool Entering();         // the console takes the typing now
void Select(int delta);  // the previous / next registered cheat
void SelectEdge(bool last);
void Run();              // Enter
void Tick();

}  // namespace nav::cheats
