// Ctrl+F in buy mode: the whole catalog searched by name, every category at once (a tester,
// 2026-11-10: "I still haven't managed to find a bookshelf"). The matches are a group of the lot
// view, "catalog search: <text>", placed before the lot's things; Enter on one starts placing it,
// as Enter on the catalog's product does (game::ChooseProduct: whatever page is shown). The
// typing itself is the navigator's Find (navigator.h); this holds what it found.
#pragma once

#include <string>
#include <vector>

#include "model.h"

namespace nav::catalog_find {

// The products whose name holds the text (accents and case aside): those starting with it first,
// then a word starting with it, then containing it, each in the catalog's order. Returns how many
// match; with none the list stays as it was.
int SetQuery(const std::string& utf8);
void Clear();
bool Shown();          // there is a list, and buy mode's catalog is on screen
Group ResultsGroup();  // kCatalogFindGroup; empty when not Shown()
std::string GroupName();
std::string Describe(int number);
std::vector<std::string> Details(int number);
void Choose(int number);  // Enter: the product in hand (buy mode then finds its spots)

}  // namespace nav::catalog_find
