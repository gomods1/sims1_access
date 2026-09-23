// Makin' Magic's book (CWinMagicBook): the family spellbook, or the cookbook with the same window.
// A page lists four entries (cPickSpellItem), each drawn as its three ingredients (buttons whose
// tooltips name them); pressing an entry shows its name and description on the right, in a title
// (cTSWinText) above a read-only cTSWinTextEdit2. Undiscovered spells read "Conjuro desconocido".
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

bool IsMagicBook(Ptr w);
bool IsBookEntry(Ptr w);  // a cPickSpellItem
// The book w belongs to (w itself or an ancestor), 0 when none.
Ptr MagicBookOf(Ptr w);
// Hypothesis: the byte the window's setup (0x494c90) tests before the child-age check is set for
// the cookbook.
bool IsCookbook(Ptr book);
// The entry's ingredients, left to right, as their tooltips name them.
std::vector<std::string> EntryIngredients(Ptr entry);
// What the right-hand page shows for the chosen entry: its name and description (UTF-8).
std::string ChosenEntryTitle(Ptr book);
std::string ChosenEntryText(Ptr book);

}  // namespace game
