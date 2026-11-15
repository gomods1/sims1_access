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
// The cookbook: the constructor (0x494390) keeps its second argument at +0x46a; the cheat handler
// passes 0 for "magic_book" and 1 for "cook_book", and magic.iff STR# 1 titles them in that order.
bool IsCookbook(Ptr book);
// The entry's ingredients, left to right, as their tooltips name them.
std::vector<std::string> EntryIngredients(Ptr entry);

struct Ingredient {
    std::string name;   // as its button's tooltip names it
    uint32_t guid = 0;  // its object type; 0 when the entry's GUIDs could not be read
};
// The same, with the GUID of each (left to right).
std::vector<Ingredient> EntryIngredientList(Ptr entry);
// How many of the object type with this GUID the Sim carries (its inventory), 0 when none.
int InventoryCount(int neighborId, uint32_t guid);
// What the right-hand page shows for the chosen entry: its name and description (UTF-8).
std::string ChosenEntryTitle(Ptr book);
std::string ChosenEntryText(Ptr book);

}  // namespace game
