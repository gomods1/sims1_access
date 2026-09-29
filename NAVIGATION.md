# Navigation: the one model every screen follows

The user's rule (2026-09-29): keyboard navigation follows what things **mean**, the way a screen
reader user moves through an application, never how the screen **looks**. Every screen, panel,
dialog and mode of the mod, today and later, uses this model and these keys (the same rule and
keys as the Sims 2 and Sims 4 mods, `..\sims2\NAVIGATION.md`, `..\sims4\NAVIGATION.md`). A new
feature that navigates anything adds groups or items to this model; it never invents its own keys
for moving.

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
| Left / Right | the same as Up / Down; on a slider, value picker or personality bar, its value (PageUp / PageDown too) |
| Home / End | first / last item of the group |
| Enter (Space outside the lot view) | press; on a kind or a room, open its things; on a thing, its interactions (buy mode: pick it up) |
| Backspace | back up a level (a kind's or room's things to the kinds or rooms); a text field's is the game's |
| Ctrl+Enter | walk the selected Sim to the thing last focused |
| R | the item again, with its group |
| Escape | close the interaction menu; otherwise the game's |
| Ctrl+Up / Ctrl+Down, Ctrl+Left / Ctrl+Right | the review buffers (details, events) |

- Entering a group says its name and size ("objetos por tipo, 10 elementos. Sims, 3 elementos");
  moving inside says the item only; the position ("3 de 12, group") is in the details buffer.
- When the focused thing goes away, focus stays in its group; after an interaction menu the focus
  is back where it was (the screen's remembered focus).

## Spatial tools (the only exception)

When the task is a place, the arrows move in space:

| Tool | Arrows | Tab / Shift+Tab | PageUp / PageDown | comma / period | Other |
|---|---|---|---|---|---|
| Placing (buy mode, `nav/buy_mode.cpp`, `PlacingKey`) | one tile, Shift five | the spots where it fits | the rooms with spots | the other directions on the tile | Enter buys, Delete sells, Escape stops, R repeats |
| Build cursor (`nav/build_mode.cpp`) | one corner or tile, Shift five | the rooms (their north-west corner) | the floor above / below | the ready-made room sizes (Insert builds one) | Space marks, Enter builds, Ctrl+Enter or Delete removes, Home back to the Sim, R, Escape leaves |
