# Sims Accessibility

A screen reader accessibility mod for **The Sims Legacy Collection** (Steam, 2025): every screen,
dialog and panel spoken through NVDA, JAWS or Narrator, one set of keys everywhere, and the tools
a blind player needs (the lot's things, a house map, a narrator, furniture spots, a build cursor).

## For players

- Install with the offline installer, or copy `version.dll` and the `SimsAccess` folder into the
  game's folder (the one with `Sims.exe`).
- In the game: **Control+F1** says the keys of the screen you are on, **Control+Shift+F1** opens
  the manual in your browser, in the game's language. Every screen also ends with a "mod help"
  group holding both.
- The manual: [docs/manual/en.md](docs/manual/en.md) (English source; the other languages sit next
  to it as `<language tag>.md`).

## For developers

- `CLAUDE.md`: how the mod is built, what is known about the game, the working rules.
- `NAVIGATION.md`: the one key grammar every screen follows.
- `mod\build.bat [deploy]` builds the mod and the manual (pandoc); `python tools/package.py
  vX.Y.Z` makes a release.
- **Keys and docs change together**: a change to a key or a feature edits, in the same change, the
  code, the `Keys*` strings Control+F1 speaks (`mod/core/util/strings_table.h` and every
  `mod/core/lang/*.cpp`), and the manual in every language (`docs/manual/*.md`).
  `python tools/build_docs.py --check` and `python tools/mod_strings.py check` find what a
  translation left out; a release stops when the manual's translations differ from the English.
