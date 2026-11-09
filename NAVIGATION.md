# Navigation: the one model every screen follows

The user's rule (2026-09-29): keyboard navigation follows what things **mean**, the way a screen
reader user moves through an application, never how the screen **looks**. Every screen, panel,
dialog and mode of the mod, today and later, uses this model (the same rule as the Sims 2 mod,
`..\sims2\NAVIGATION.md`; the Sims 4 mod is someone else's now). A new feature that navigates
anything adds groups or items to this model.

**Keys** (the user, 2026-11-09, dropping the old "Left/Right do what Up/Down do everywhere" rule:
"I never meant to use the same 4 keys. Just don't use super obscure keys that are unexplained"):
a screen may give a key pair its own job where that helps (Left/Right turn the catalog's pages),
as long as the keys are common ones and every key is explained: Ctrl+F1 on that screen, the
manual, and the details where it applies. No obscure combinations nobody is told about.

## The model

- What the player moves through is a ring of **groups** (sections): a panel of the game (a
  dialog's buttons, the control panel, an options page, the catalog), or a group the mod makes
  by meaning (the status line, the needs, the family, the action queue, the houses, the lot's
  things by kind and by room).
- A group holds **items**. An item may open a deeper **level**: a kind or a room of the lot's
  things opens its things (`Role::Level`, `nav/world.cpp`).
- What belongs together comes from the game's own structure (the item's container, a panel's
  registered buttons: `nav/collect.cpp`) or the mod's own data. **Groups come in the order of
  importance (`GroupOrder`: status, needs, family, queue, a screen's main list), then in the
  window tree's order**; screen position never decides what belongs together nor the order of
  groups. Position only orders the items inside one group (`ArrangeGroup`: a panel's rows).

## The keys

| Key | What it does |
|---|---|
| Tab / Shift+Tab | next / previous group (wraps); says the group's name and size, then the item |
| Up / Down | previous / next item of the group (the lot view's HUD too); at the ends the item is said again (a catalog page turns) |
| Left / Right | the same as Up / Down, except: on a slider, value picker or personality bar, its value (PageUp / PageDown too); in buy and build mode's catalogs (a group with page arrows), the previous / next page, landing on its first product ("última página" at the end) |
| Home / End | first / last item of the group |
| Enter (Space outside the lot view) | press; on a kind or a room, open its things; on an entry of identical things ("2 alike"), open them one by one (a level, as a kind is); on a thing, its interactions (buy mode: pick it up) |
| Backspace | back up a level (a kind's or room's things to the kinds or rooms); a text field's is the game's |
| PageDown / PageUp (lot view, every mode) | next / previous thing of the kind or room chosen last, from anywhere in the lot view; focus goes to it in its group |
| Shift+PageDown / Shift+PageUp | next / previous kind of things, on its nearest thing |
| Ctrl+PageDown / Ctrl+PageUp | next / previous room (the first press: the selected Sim's), on its nearest thing |
| Ctrl+Enter | walk the selected Sim to the thing last focused |
| Ctrl+F (lot view, every mode) | find a thing by typing part of its name: the matches are a list of "things by kind", best first; Up / Down (Page keys, Home / End) go through them, Enter acts, Ctrl+Enter walks, Delete removes a door or window, Tab leaves with the list kept, Escape goes back to where the focus was |
| Delete (buy and build mode) | on a door or window of the lot's things: removes it |
| R | the item again, with its group |
| Escape | close the interaction menu; otherwise the game's |
| Ctrl+Up / Ctrl+Down, Ctrl+Left / Ctrl+Right | the review buffers (details, events) |
| Ctrl+F1 / Ctrl+Shift+F1 | this screen's keys / the manual; both are also the buttons of every screen's last group, "mod help" |

- Entering a group says its name and size ("objetos por tipo, 10 elementos. Sims, 3 elementos");
  moving inside says the item only; the position ("3 de 12, group") is in the details buffer.
- **The lot's things on the Page keys** (the user's rule, 2026-10-29: "there is an overuse of
  arrow keys"): in live, buy and build mode PageDown/PageUp walk the list of things, Shift+ and
  Ctrl+ choose what the list holds (a kind, a room). They drive the same "objetos por tipo /
  por habitación" groups (`nav::BrowseThings`), so Tab, the arrows, Enter and Backspace go on
  from where the Page keys left the focus. A focused slider or value picker keeps PageUp/PageDown
  for its value. Any later list of things on the lot follows this: Page for the items, a modifier
  for the filter.
- **Find owns the keyboard while it is on** (the user's request, 2026-11-02): what types goes to the
  search, so letters reach neither the game's hotkeys nor R. Its results are ordinary items of the
  kinds group, so everything after it (Page keys, Backspace, Tab) goes on as usual.
- **Build mode's tools and styles are two groups** (`kBuildStylesGroup`, 2026-11-02): by position
  they were mixed, and a page turn moved the tabs around the styles. Pressing a tool's tab moves
  focus to its styles and puts away the object the game brings back with it (`buy::HoldOff`);
  otherwise the arrows would start moving that object with the styles out of reach. Only Enter on
  a style picks one up.
- When the focused thing goes away, focus stays in its group; after an interaction menu the focus
  is back where it was (the screen's remembered focus).

## Spatial tools (the only exception)

When the task is a place, the arrows move in space:

| Tool | Arrows | PageDown / PageUp | Ctrl+PageDown / PageUp | Shift+PageDown / PageUp | Tab / Shift+Tab | comma / period | Other |
|---|---|---|---|---|---|---|---|
| Placing (buy mode, `nav/buy_mode.cpp`, `PlacingKey`) | one tile, Shift five; walls, fences and windows stop it, Alt+arrows go through | the spots where it fits | the rooms with spots | the other directions on the tile | the spots too | the directions too | Enter buys, Delete sells, Escape stops, R repeats |
| Build cursor (`nav/build_mode.cpp`) | one corner or tile, Shift five | the floor above / below | the rooms (their north-west corner) | | the rooms too | the ready-made room sizes (Insert builds one) | Space marks, Enter builds, Ctrl+Enter or Delete removes, Home back to the Sim, R, Escape leaves |
| Live map (`nav/map_mode.cpp`, Ctrl+M) | one tile, Shift five; walls, fences and windows stop it, Alt+arrows go through | the lot's things (the map goes to each) | the rooms, as the lot's things | the kinds, as the lot's things | the HUD's groups as usual | the layer each step says: everything, objects, floors, walls, rooms | Ctrl+Shift+PageUp/Down the floors, Enter the menu of what stands there, Ctrl+Enter walks there, Home back to the Sim, R, Escape or Ctrl+M leaves |

A spatial tool that walks the house stops at walls (the tester's "Zones" idea, 2026-11-06: a room
was left by accident); Alt with the arrows goes through, since Ctrl+arrows are the review buffers
everywhere. The build cursor keeps crossing walls: walls are its work. Its comma / period stay what
it holds (sizes); the map's are its layers (from Oxygen Not Included's overlays: one kind of
thing per step on a furnished lot).

Every spatial tool also plays the sound cues of `nav/cues.h` on each step (Ctrl+Shift+S turns them
off): the floor, each side's wall, fence, door or window, what stands on the tile.
