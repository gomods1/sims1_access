# Structures

Build: `Sims.exe` 1.10.7 (`b7c8765e...`). Offsets in bytes. VAs at preferred base 0x400000.
Class names come from RTTI (`tools/rtti.py` -> `re/classes.tsv`, `re/vtables.tsv`).
Live inspection: `python tools/memtool.py "<code>"`, tree dump: `python tools/memtool.py -f tools/wintree.py`
(examples in `re/wintree_*.txt`).

## MSVC x86 `std::string` (0x18 bytes)

| Off | Type | Meaning |
|---|---|---|
| 0x00 | `char[16]` or `char*` | inline buffer when capacity <= 15, else pointer to heap text |
| 0x10 | u32 | size |
| 0x14 | u32 | capacity |

Evidence: `cDefaultTTWindow` code at `0x702a20` tests `capacity > 0xf` before dereferencing, reads
size from +0x10; the Frida reader using this layout returned correct tooltip text.
Text encoding: Windows-1252 bytes (read "Ciudad Mágica" correctly as ANSI on a Spanish system).

## MSVC `std::list<T*>` (8 bytes in the owner)

| Off | Type | Meaning |
|---|---|---|
| 0x00 | node* | head (sentinel) node |
| 0x04 | u32 | element count |

Node: `{next, prev, T* value}`. Walk from `head->next` until back at `head`.

## Globals

| VA | Type | Meaning | Evidence |
|---|---|---|---|
| `0xa7f484` | `cTSWinMgrW95*` | window manager | read live; used by tooltip code `0x6ddf30` and every button handler |
| `0xa7f464` | `cTSFrameWorkW95*` | app framework (window procedure calls its vtable) | wndproc `0x6f0e40` |
| `0xa7f488` | keyboard object | receives WM_KEYDOWN (vtable+0x1c) / WM_KEYUP (+0x20) with the VK | `0x6f1af2` |
| `0xa52074` | u8 | app active (WM_ACTIVATEAPP wParam); while 0 the input dispatcher `0x6dae40` drops all input and the main loop sleeps 100 ms per frame | `0x6f1699`, `0x6dae46`, `0x6d7ca3`; forcing it to 1 made background hover work |
| `0xa65324` | `cWinLotPopup*` | the shared lot popup (neighborhood hover card) | `0x490cc0` |
| `0xa67044` | app root | `[[0xa67044]] + 0x14` = the lot's `ObjectModule` (below); `+ 0x68` u8 = the game's language number (1 US English ... 6 Spanish ... 20 Korean, CLAUDE.md "Languages") | read live on a lot; the language byte: setter `0x45d580` and getter `0x455820` are `mov eax,[0xa67044]; mov [eax+0x68],cl` / `mov al,[eax+0x68]` |
| `0xa67108` | `cTool*` | current world tool: `cObjPickerTool` in live mode | read live; the picker's slot 30 opens pie menus |
| `0xa63570` | `cSimsApp*` | `+0xc4` `CPState*`, whose `+0x70` is the buy mode `Catalog` | read live |
| `0xa634e0` | `cPickFamilyPersonInfo*` | shared person card of the family picker | |

## `cTSWinMgrW95` (window manager, `[0xa7f484]`)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x24 | `cTSWin*` | window that owns input now: the current screen or the open modal dialog. It can also be a sub-panel (it became `cWinPeople` after a click on the family panel), so the mod walks up to the top-level child of the main window (`game::InputRoot`); an open pie menu has no parent and is its own root | modal loop `0x6d9f50` stores the dialog here and restores it after; live reads: neighborhood `cWinNeighborhoodUL`, dialog `cTSObjectDialog`, live mode `cDDDSimsView` |
| 0x28 | `cTSWin*` | same as 0x24 in every observation (focus?) | `0x6d9f50` |
| 0x2c | `cTSMainWindowW95*` | root window (1024x768) | read live |
| 0x15c | vector of 0x14-byte messages | posted UI messages `{target, type, code, sender, extra}` | `0x6dac50` |
| 0x17c | `cTSWin*` | widget under the mouse (hover) | read live after hovering |
| 0x1c0 | `cDefaultTTWindow*` | tooltip window (created lazily by `0x6da4d0`) | |
| 0x208 | `cTSWinTutorialHighlight*` | tutorial arrow | |

## `cTSWin` (base of every widget, 0x128 bytes; vtable `0x99d8c0`, 98 slots)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x0c..0x18 | int x4 | rectangle left, top, right, bottom in absolute game coordinates (1024x768 space) | matches hover positions: screen = game * 2.0833 + (213, 0) on the 2560x1600 display |
| 0x1c..0x28 | int x4 | copy of the rectangle | |
| 0x38 | `std::list<cTSWin*>` | children (count at 0x3c) | walked live; matches every button on screen |
| 0x44 | `cTSWin*` | parent | cTSWin slot 6 returns it |
| 0x4c | `cITSMessageTarget*` | owner: receives this widget's notifications (usually = parent) | button mouse-up `0x6ea2f0` |
| 0x50 | `std::string` | caption text (dialog message, title, button caption "OK") | text layout `0x6eda50` reads it; read live |
| 0xbc | `std::string` | tooltip text | tooltip routine `0x6ddf30` passes `this+0xbc` |
| 0xd4..0xe0 | int x4 | rectangle relative to the parent | diff of toolbar buttons |
| 0xf4 | u32 flags | bit 0x1 = visible (hidden "Ver páginas web" button is 0x452, visible ones 0x453; slot 37 sets 0x1) | slots 35/36 test/set flags |

Virtual slots: 0 `HandleMessage(msg*)` (dispatcher `0x6dd7a0`), 6 parent, 35 `TestFlag(mask)`,
36 `SetFlag(mask, on)`, 37 show, 46 `ShowTooltip()` (fills and returns the hover window), 53 key
down, 73 `OnNotify(code, sender)`.

### Messages (`{type, a, b, c}` into slot 0)

| Type | Slots called | Meaning | Evidence |
|---|---|---|---|
| 3 | 73 `(a=code, b=sender)` | notification from a child (button click = code 1) | dispatch table at `0x6dda9c` |
| 4 | Ctrl shortcuts / 79 | character | same; seen for `p` in live mode |
| 5 / 6 | 53 / 54 `(a=vk, b)` | key down / up | `tools/frida/msg_trace.js` with a posted `P` |
| 7 | 69 + 55 `(x, y, flags)` | left button down (flags 0x10) | recorded real click on a house |
| 11 | 71 + 59 `(x, y, flags)` | left button up | same |
| 13 | 70 + 61 | mouse move | same (stream while hovering) |
| 24 / 25 | 64 / 65 | mouse enter / leave | same (24 arrives first) |

Coordinates are relative to the widget. `game::Press` sends 24, 7, 11 (then 25 later if the widget
still exists): verified on toolbar buttons, dialog buttons, a house (enters the lot), family rows.

## `cTSWinBtn` : `cTSWin` (vtable `0x99df00`, 105 slots)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x130 | int | checked state for toggle types | mouse-up toggle logic `0x6ea35b` |
| 0x138 | int | image frame: 0 normal, 1 pressed/checked, 3 disabled | every disabled button reads 3 |
| 0x148 | int | button type: 0 push, 1 toggle/mode, 2 check/radio | toolbar and options panel |
| 0x1b8 | u8 | enabled (greyed-out buttons are 0; mouse handlers skip when 0) | slot 64/65 `cmp [esi+0x1b8],0` |
| 0x1b9 | u8 | pressed visual (slot 99 sets it) | |
| 0x1bc | u8 | armed by mouse-down; mouse-up only notifies when set | `0x6ea2d3` |
| 0x1be | u8 | notify synchronously instead of posting | `0x6ea2e8` |

| 0x144 | int | when the click notifies: 0 on release (while hovered), 1 on press, 2 both | slots 55 / 59 |

**Click notification**: `cTSWinBtn::Notify(code)` (`0x6e9710`) sends `{3, code, this, 0}` to the
owner (`+0x4c`), posted through `cTSWinMgr::PostMsg` `0x6dac50` (or delivered directly when
`+0x1be`). A click is code **1**, sent on release by slot 59 only while the hovered flag (set by the
mouse-enter message) is on, and it stores the button in the global `[0xa7f4ec]`. Code 0x14 is sent
by slot 65 (mouse leave) when armed; some dialogs react to it, the neighborhood does not. Pressing
through the message sequence (above) covers both.

## `cWinLotBtn` : `cTSWinBtn` (a house on the neighborhood map)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x1e0 | lot record | passed to the popup fill `0x492820` | `0x490d09` |
| 0x1ec | int | lot value | varies per lot |
| 0x1f8 | `std::string` | resident family name ("Hernández", "Flores"; empty when vacant) | read live |
| 0x220 | int | house number (`%d` in the popup; key of the neighborhood's lot map) | `0x492820`, `0x4a4fe0` |

Popup text: `cWinLotPopup::Fill(&btn[0x1e0], &btn[0x274], force)` at `0x492820` draws the whole card
through `cTSFont` slot 25 (`0x723a70`, `(const char* text, int len, ...)`, called from `0x493a3e`),
e.g. `"La Autovía Gunther Goth,76\n\nLa familia Flores\r\nValor neto: $14.931\r\nAmigos: 0"`.
It returns early when the same lot is already shown and `force` is 0.

## Family picker (`cWinPickFamily`)

`cPickFamilyList` (tooltip "Haz clic para crear una nueva familia") holds one `cPickFamilyItem`
(a `cTSWinBtn`) per family; buttons "De vuelta al barrio", "Crear nueva familia", "Borrar familia",
"Trasladar familia" (the last two enabled once a row is selected by pressing it).

| Class | Off | Type | Meaning | Evidence |
|---|---|---|---|---|
| `cPickFamilyItem` | 0x1e0 | `cPickFamilyItemMember*[8]` | portraits; hidden ones are empty slots | read live (4 / 2 / 1 visible) |
| | 0x200 | int | not the member count (includes empty slots) | Compañeras read 3 with 2 members |
| | 0x210 | `std::string` | family name | "Gentil" |
| | children | `cTSWinText` x3 | name, net worth ("$20.000"), friends ("0"), left to right | |
| `cPickFamilyItemMember` | 0x1e0 | int | neighbor ID of the Sim | 273 for Federico |

Hovering a portrait (slot 46, `0x43eef0`) fills the shared `cPickFamilyPersonInfo` card
(`[0xa634e0]`): `+0x1a8` char[] "Personalidad de Federico", `+0x190` `std::string` biography,
children `cWinPersonality` (caption = trait name; values not mapped yet).

## Neighbors

`0x43aac0(cXPerson*)` (cdecl) returns the neighbor ID of a Sim on the lot (-1 if none);
`0x43af20(int id, std::string* out)` (cdecl) writes a Sim's name; `0x43a4e0(int id)` (cdecl)
returns `std::string*` of the Sim's biography (the neighbor table at `[0xa63464]`, 0x48-byte
records, `+8` a lazily created object whose `+0x18` is the biography). Unknown IDs yield the
text "No Selector!".

## Live mode panel (`cWinCPanel` > `cWinPeople`)

Hidden when a lot loads; shown after F1 or a mode switch. `cWinPeople` children: family member
buttons (tooltip = first name, checked = selected Sim), subpanel toggles ("Estado de ánimo",
"Relaciones", "Empleo", "Casa", "Personalidad", "Interés", "Inventario"), two "Grado de humor"
meters, and a container titled "Necesidades" (`cTSWinText`) with eight `cWinMotive`.

| Class | Off | Type | Meaning | Evidence |
|---|---|---|---|---|
| `cWinPeople` | 0x148 | `std::vector<cTSWinBtn*>` | family portrait buttons (8 slots; unused ones hidden and keep old names), tooltip = first name, checked = selected Sim; pressing selects the Sim | read live: Cris, Marisa visible; Bobo, Duke, Leroy hidden |
| | 0x158 | `cTSWin*` | the "Necesidades" container | |
| | 0x1b0 | `cWinLivePopup*` | the panel's hover popup | read live |
| | 0x1b4..0x1cc | `cTSWinBtn*` x7 | subpanel toggles in this order: Estado de ánimo, Relaciones, Empleo, Casa, Personalidad, Interés, Inventario | read live |
| mood meter | | two plain `cTSWin` children of `cWinPeople`, tooltip "Grado de humor" (upper and lower half) | the value is not in them: see "Sims' data" below | |
| `cWinMotive` (a `cTSWinBtn`) | 0x50 | `std::string` | need name ("Hambre") | read live |
| | 0x1e0 | int | value -100..100 (bar centre = 0; red when negative) | Hambre 92, Habitación -21 matched the bars |
| | 0x1e8 | int | need index (hypothesis) | 7, 5, 6, 15... |
| | 0x1f4 | `std::string` | name again | |
| | 0x20c | `std::string` | the game's explanation of the need | "Lo primero que necesita un sim hambriento es una nevera..." |
| `cActionIcon` | 0xbc | `std::string` | queued action name ("Llenar bol"); pressing it cancels the action | pressed once by accident: the action was cancelled |

## Lot view control panel (`cWinViewControl`, child of `cDDDSimsView`)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x138..0x164 | `cTSWinBtn*` | rotate, zoom, wall modes, the five mode buttons (Vivir F1 ... Cámara F4) | tooltips read live |
| 0x168 | `cTSWinBtn*` | funds: caption "$9.500", tooltip "Fondos"; pressing opens the budget | read live |
| 0x16c..0x174 | `cTSWinBtn*` | first floor, second floor, roof | |
| 0x178 | `cTSWinBtn*` | pause toggle ("Pausa - P"), checked while paused | toggled by `p` |
| 0x17c..0x184 | `cTSWinBtn*` x3 | speed normal / high / ultra; one stays checked while paused | |
| 0x188 | `cTSWinBtn*` | help ("Ayuda") | |
| 0x1cc | `std::string` | the clock exactly as drawn ("11:46"); slot 87 (`0x4f8ad0`) draws only this string | changed 11:43 -> 11:46 while running |
| 0x1fc | `std::string` | funds text ("$9.500") | |
| 0x228 / 0x240 | `std::string` | "AM" / "PM" (not shown in Spanish) | |
| 0x25c | `cTSWinBtn*` | friend count: caption "0", tooltip "Recuento de amigos de la familia" | slot 93 (`0x4f7910`) updates its caption |
| 0x260 | `std::string` | family name ("Compañeras") | |

## Family panel subpanels

| Class | Off | Type | Meaning | Evidence |
|---|---|---|---|---|
| `cWinRelationship` (card, tooltip "Marisa Compañeras") | 0x1e0 | int | neighbor ID of the Sim on the card | 265 |
| | 0x1e4 | int | neighbor ID of the selected Sim | 269 = the ID the personality widgets carry |
| | 0x1e8 | int | lifetime relationship -100..100 (lower number and bar) | stayed 20 after a chat |
| | 0x1ec | int | daily relationship -100..100 (upper number and bar) | 20 -> 23 after one chat; draw `0x4cd4c0` prints it first |
| `cWinSubpanelJob` | 0x130 | `cFlashyBtn**` [6] | skills in the order of the table at `0x98aca0` (person data 10, 12, 11, 17, 18, 15 = cooking, mechanical, charisma, body, logic, creativity); captions are the names | draw `0x4df680` |
| | 0x13c | `cFlashyBtn**` [4] | job, pay, performance, family friends needed for the next promotion; the draw sets their captions every frame ("Decano de universidad", "$700", "Excelente", "Necesarios 9"; unemployed: "Desempleado", "no disponible") | Gunther's panel; empty while the game is minimized (no drawing) |
| | 0x198 / 0x1b0 / 0x1c8 | `std::string` | the unemployed texts | read live |
| `cWinSubpanelHouse` | 0x130 | `cTSWinBtn**` [9] | 0-4 ratings (Tamaño, Muebles, Patio, Estado, Diseño; values drawn as bars only), 5-8 text lines set by the draw | draw `0x4dc390` |
| | 0x184 / 0x19c / 0x1b4 / 0x1cc | `std::string` | formats "Área: %d m2", "Dormitorios: %d", "Baños: %d", "Solar: %s" | read live |
| | 0x15c | `std::string*` [3] | lot sizes "Pequeña", "Mediana", "Grande" | read live |
| `cWinPersonality` (caption = trait) | 0x1e0 | int | person data index of the trait (2 nice, 3 active, 5 playful, 6 outgoing, 7 neat) | draw `0x4bdd80` |
| | 0x1e4 | int | neighbor ID (-1 none) | |
| `cWinInterest` (tooltip = interest) | 0x1e0 | int | interest number | |
| | 0x1e4 | int | points 0..10 (bar = 4 px per point) | draw `0x48df00` |

Values the panel only draws are read through the game: see "Sims' data" and "House ratings".

## Sims' data (by neighbor ID, all cdecl)

| Function | Returns | Evidence |
|---|---|---|
| `0x43aec0(id)` | Sim state; `+0xc` float mood -100..100. The meter shows `round((mood + 100) * 11 / 201 - 5)` bars (-5 red .. +5 green) | family panel draw `0x4bb980` |
| `0x43aec0(id)` (same record) | every motive as a float -100..100 at `+ index * 4`, indices as `Behavior.iff` STR# 134: 3 mood, 5 energy, 6 comfort, 7 hunger, 8 hygiene, 9 bladder, 13 room, 14 social, 15 fun | `cWinMotive`'s getter `0x49f590` reads `[state + [motive + 0x1e8] * 4]`; the narrator's "necesidad baja" lines matched the bars and the Sims' behavior (2026-09-23) |
| `0x43a480(id)` | `short*` person data; personality traits at their index, 0..1000 (widget shows / 100) | personality draw `0x4bdd80`; the five traits of a Sim summed to 25 |
| `0x43bc30(id, index)` | skill points 0..1000 for person data indices 10..18 (one bar per 100) | job panel draw |
| `0x43bba0()` | neighbor ID of the selected Sim (-1 none) | used by the panel draws |
| `0x43b230(id, vector* out)` | appends the Sim's action queue to a `std::vector` of 0x28-byte entries (current action first; `+0x0c` interaction record, `+0x10` `std::string` action name). The queue icon builder `0x40f6c0` keeps a static vector at `0xa61ec0` ({first, last, end}), empties it with the entry destructor `0x438700` (thiscall) and refills it for the selected Sim; between updates it is empty, so the mod borrows it for any Sim | "Hecuba, Comprobar jaula", "Menelaus, Perseguir el rabo"; the selected Sim's icons unaffected |

### Action queue (on the `cXPerson`)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x824 | 8 x 0x50-byte records | queued interactions, a ring: entry i is `+0x824 + ((head + i) & 7) * 0x50` (`0x5f2350`) | `0x43b230`, the "Action Queues" debug dump `0x52da90` |
| 0xaa4 / 0xaa8 | int | ring head / tail counters (count = tail - head) | same |
| 0xaac | 0x50-byte record | the running interaction (object at `+0x0c` is 0 when none) | `0x43b230` puts it first, flagged "running" |

Record: `+0x04` the Sim, `+0x08` target object, `+0x0c` icon object, `+0x10` entry index, `+0x1c`
priority, `+0x20` u16 action tree. Priority seen live (2026-09-23): **50** for an interaction the
player chose from a menu, **2** for free will. The 0x28-byte entries `0x43b230` produces point at
the record from `+0x0c` and carry `+0x09` = 1 for the running one.

## Camera

- `[0xa67548]` `cXPerson*`: the Sim the camera follows. The house viewer's frame function `0x567d30` calls its vtable slot 33 (`cXPerson` `0x5f5f90`) each frame while `viewer + 0x1c0` is 0; `0x57f480(person)` (cdecl) sets it, `cWinPeople::0x4ba880()` clears it with the panel's marker at `cWinPeople + 0x4a4` (the "following" icon on the portrait).
- `cXObject::0x53a210(char immediate)` (thiscall) scrolls the view to the object: its grid position through `0x57a850`, rotated for the camera with `0x586ae0`, then `0x5e0130(x, y, level, 1, !immediate)` (-> `0x565d70`), `0x5e0180(0)`. Works on Sims, objects and the held buy-mode object.
- The lot view's key handler `0x40e610` (`cDDDSimsView` slot 53): Enter in live mode follows the selected Sim (`cWinPeople::0x4bc220`), Tab/PageUp/PageDown change floors (`0x408000`), Home/End cycle the wall modes (`0x408630`), Escape cancels the tutorial (`0x548720`), F1..F5 the modes (`0x408050(mode)`: 0 build, 1 buy, 2 live, 3 options, 4 camera; F1 in live mode toggles the panel with `0x407d90`). Plain arrows do nothing here; scrolling is polled elsewhere.
- `cWinPeople + 0x140`: index of the selected portrait (-1 none); `+0x148` the portrait vector (`people::kMemberButtons`), whose index is also the family member index for `0x43b3b0`.
- `CPState` (`[0xa63570] + 0xc4`): `+0x50` camera rotation 0..3 (`0x407cf0` / `0x407d50` rotate, calling `0x5662c0`), `+0x240/+0x244` the camera-mode frame position (`0x407500`).

## Rooms

The world `[0xa675b4]` (`cRotatableWorld`) keeps one room map per level at `+0x28 + level * 4`
(level 1 = ground): the map's `+0x0c` is `u16** rows`, indexed by tile y, each row indexed by tile
x, holding the room id (0xfffb = none). The debug tile window (`cWinTileInfo`, `0x4e8f30`) reads it
through `0x4e8c70(world, packed tile)`, which first undoes the camera rotation (`world + 0x84`);
object placements (`+0xfc`) are already grid coordinates. Grid bounds at `world + 0x70..0x7c`.

Room ids are per floor: 0..n on the ground floor, `1024 + n` on the second (level << 10). Each
floor's outdoors is a room (id 0 and 1024 on the Goth lot, "floored area" 0).

The records live in a `std::map<u16, Room*>` in the room manager `[[0xa67044]] + 0x24`; lookup
`0x627c70(manager, u16 id)` (thiscall). The debug window prints their fields:

| Off | Meaning |
|---|---|
| 0x00 | u16 id |
| 0x38 | area in half tiles (`GetArea` `0x6278a0` halves it) |
| 0x3c | floored area |
| 0x40 | object count |
| 0x44 / 0x48 | window light / object light |
| 0x4c | dirt total |
| 0x50 | room impact |
| 0x54 | good object count |
| 0x58 | bed count (the house panel counts rooms with beds as bedrooms) |
| 0x5c | bathroom object count (toilet, sink, shower...) |
| 0x280 / 0x284 | lamp count / people count |

`0x6280a0` (thiscall on a Room) = IsOutside. Verified on the Goth house: toilet and bathroom sink
in room 9 (4 bath objects), fridge, stove and counters in room 6 (74 tiles), the double bed in
1025, the shower in 1030.

## House ratings and rooms

The house subpanel builds a `HouseStats` (11 dwords) on its stack: `0x56a990` (thiscall, zeroes
it), `[[0xa67044]]->0x56b460(&stats)` (fills it), then thiscall getters `0x56b7e0` size,
`0x56b400` furnishings, `0x56b940` yard, `0x56b850` upkeep, `0x56b620` layout (each 0..100, drawn
as value / 10 bars) and `0x56b830` area, `0x56b650` bedrooms, `0x56b640` bathrooms, `0x56b630` lot
size. The fill walks a `std::map` of **room records** at `[[0xa67044] + 0x24] + 4`: each record
(node value `+0x14`) has `+0x34` (non-zero for real rooms), a tile count (`0x6278a0`, area = tiles
x 9 m2), `+0x58` > 0 for a bedroom and `+0x5c` > 0 for a bathroom; `0x6280a0` is true for outdoor
"rooms". This is the lead for room navigation in buy mode (see `ROADMAP.md`).

## Budget dialog (`cWinBudgetDlg` > `cWinBudgetRow`)

Each row is a `cWinBudgetRow` (caption `+0x50` = row label or section title). The header row
holds the column names in the value fields; the last rows are sentences with a value.

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x148 | `std::string` | first period column ("$0"), or its name ("3 días") on the header row | read live |
| 0x160 | `std::string` | second period column ("Hoy" on the header row) | read live |
| 0x17c | `std::string` | summary format ("Total contabilidad doméstica: %s") | read live |
| 0x194 | `std::string` | summary value ("$9.500") | read live |

## The lot (`ObjectModule`, `[[0xa67044]] + 0x14`)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x18 | `std::vector<cXObject*>` | every object on the lot, index = object id - 1 (null gaps) | ids at `+0xf0` match the index; `re/lot_objects.txt` |
| 0x80 | `std::vector<cXPerson*>` | the Sims on the lot (family and visitors) | 3 entries with Cris, Marisa and a visitor |
| 0xa4 | `cXObject*` | the help system's waiting "situation" (a tip): set while the tip icon blinks, 0 once shown or expired; the icon click runs "show situation info" on it (`0x532060`) | read live: set while the icon blinked, 0 after Ctrl+H |

### `cXObject` (and `cXPerson`)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0xf0 | short | object id (the pie menu opener takes it) | matches the vector index |
| 0xf4 / 0xf8 | int | x / y in 1/16 tile | Sims walking changed them smoothly |
| 0xfc | u32 | placement: byte 0 tile **y**, byte 1 tile **x**, byte 2 level (floor); low word 0xffff = not placed (inventory, templates) | a Sim at `+0xf4 >> 4` = 36, `+0xf8 >> 4` = 23 had bytes 23, 36 |
| 0x114 | u32 | flags; for people bit 0x2 = the selected Sim | toggled when switching Sims |
| 0x11c | `u16*` | the object's raw OBJD resource (fields below) | fields matched FreeSO's OBJD layout |
| 0x120 | type record | `+0x00` `ObjResFile*` (the object's IFF), `+0x18` `char*` English internal name ("Trash Outside", "Trees - Pine", "Portal - Door") | read live |

Multi-tile objects are one `cXObject` per tile sharing their file and OBJD MasterID; SubIndex is
the tile's offset `(y << 8) | x`, not a lead flag. The catalog strings sit on one part (the wet
bar's "Main Piece", SubIndex 0x1) or on none (fireplace halves, bird cage pieces); the mod keeps
the part with strings, else the part at offset 0, and names nameless parts through the buy
catalog by their file (`Product` type record `+0x40` -> file `+0`).

Helpers that are placed but invisible (`"Portal - ..."`, `"Phone Line"`, `"PlumbBob"`, `"Location - ..."` markers such as pet-mess spots) have to be
excluded by internal name: a guessed "hidden" bit (`+0x9e` 0x100) also marked every Sim and was
wrong. 

### OBJD fields (u16 each; index into `+0x11c`)

| Field | Meaning | Values seen |
|---|---|---|
| 9 | object type | 2 person, 4 normal, 7 system, 12 inventory item, 34 food (a pizza box, a slice; FreeSO's OBJDType) |
| 10 | MasterID (multi-tile group; 0 single tile) | |
| 11 | SubIndex: the part's tile offset `(y << 8) | x` (not a lead flag) | 0x0, 0x1, 0x100, 0x200 |
| 39 | room flags (suggested rooms) | 0x1 kitchen, 0x2 bedroom, 0x4 bathroom, 0x8 living, 0x10 outside, 0x20 dining |
| 40 | function flags = buy mode categories | 0x01 seating, 0x02 surfaces, 0x04 appliances, 0x08 electronics, 0x10 plumbing, 0x20 decorative, 0x40 misc, 0x80 lighting (checked against the catalog's category pages) |
| 41 | CatalogStringsID: the object's `CTSS` resource (string 1 = localized name) | |

### Localized object names

The game's string list class ("StringSet") loads a `CTSS` resource from the object's IFF in the
game language: construct (`0x58c2f0`, 0x100 bytes of storage are enough), `SetSource(resFile, id,
'CTSS', 0, -1)` (`0x58ef00`), `Load(1, 0)` (`0x58d4c0`, 0 = success), `Get(1, -1)` (`0x58cd60`,
1-based index; null past the last string), destroy (`0x58c4b0`). Returns the Spanish catalog
names. With `'STR#'` and 301 on the tutorial object's file it returns the tutorial's step texts
(`game::TutorialTexts`). The default lot objects
`MailBox.iff`, `TrashOutside.iff` and the trees have **no `CTSS` chunk at all**, so the mod
carries its own names for them (`nav/world.cpp`, `KnownObject`).

The buy catalog (`[0xa63570] + 0xc4 -> + 0x70`) keeps a vector of `Product` at `+4/+8` (name
`std::string` at `+4`, type record at `+0x40`): the same names, for objects that can be bought.

## SimAntics and balloons

Evidence: the decompiled dispatcher `0x615cb0`, the balloon primitive `0x61bc40`, its debugger
describer `0x65f8b0`, the frame runner `0x62d6e0`; `Behavior.iff` STR# 129, 161, 162, 500-510;
the sprite labels in `GameData\Sprites.iff`; balloons logged by the mod in the running game
(Lápida lot, 2026-09-22), each matching what the game had just done (a route failure right
before the route-failure help tip, conversations, the skill notice after the progress bar).

**Running primitives.** `cXObject` vtable slot 0 (`0x615cb0`, thiscall `(Frame*, Instruction*)`,
`ret 8`) runs one primitive; `cXPerson` slot 0 (`0x5f6880`) handles opcodes 3, 6, 0x11, 0x16,
0x1b, 0x1d, 0x1e, 0x25, 0x2c, 0x2d, 0x2f itself and passes the others to it. The tick
`0x62e9e0` runs the top frame through `0x62d6e0`. An instruction is u16 opcode (low 15 bits;
0x100 and up call a tree), u8 true target, u8 false target (0xfd error, 0xfe true, 0xff false),
8 operand bytes. Errors are C++ exceptions (`__CxxThrowException`) caught by the tick, so they
pass through any hook on these functions.

| Frame offset | Type | Meaning |
|---|---|---|
| 0x00 | u16 | tree id (0x100.. global, 0x1000.. the code owner's private, 0x2000.. semi-global) |
| 0x02 | u16 | instruction index |
| 0x04 | short | stack object id |
| 0x06 / 0x07 | u8 | local count / argument count |
| 0x0c | ptr | the tree's instructions (heap data; no path from here to the tree's name was found) |
| 0x10 | short[] | arguments, then locals (the accessor `0x62db70` is `frame + 0x10 + 2 * args`) |

Variables (the accessor `0x610d90`): an object's attributes are `short*` at `cXObject + 0x2c`,
count at `+0x30`; globals are shorts at `[[0xa67044] + 0x1c]` (`cSimulator`) `+ 0x10`: 0 hour,
1 day of month, 2 zoom, 3 selected Sim's object id, 4 time of day, 5 minute, 6 second, 7 month,
8 year (names: `Behavior.iff` STR# 129; read live at 2:13 AM: 2, 17, 1, 21, ...).

**Remove Object Instance** (opcode 0x12) is inline in the dispatcher: operand short 0 = 0 the
running object, 1 the stack object, 2 none; it calls `ObjectModule::0x52bfc0(id, flag)`.

**Set Balloon/Headline** (opcode 0x29 = 41, `0x61bc40`, thiscall `(Frame*, const u8 op[8])`,
`ret 8`; the dispatcher is its only caller, and it is the only code that sets a balloon's icon):

| Operand | Meaning |
|---|---|
| short 0 | bit 0: shown over the stack object instead of the running object; `>> 1`: the local holding an object id (group 7, icon 2) |
| u8 2 | icon index in the group |
| u8 3 | group (below) |
| short 4 | duration: -1 forever, 0 clears the balloon, else ticks (or loops, flag 8) |
| u8 6 | balloon: 0 thought, 1 scream, 2 speak, 3 none (`Behavior.iff` STR# 162) |
| u8 7 | flags: 1 show when inactive, 2 crossed out, 4 animation reversed, 8 duration in loops, 0x10 icon index + the running object's short at `+0x3a` (a temp; the idle motive thoughts use it) |

The balloon is kept in the first 0x3c-byte entry of the vector at `cXObject + 0x14c` (above):

| Off | Type | Meaning |
|---|---|---|
| 0x10 | int | ticks left: -1 forever, 0 nothing shown (decremented by `0x62cdb0` from the object's slot 5) |
| 0x14 | int | sprite id (group base + index), or -1 for a picture |
| 0x18 | ptr | the sprite set, or the picture (an object's cached icon, `cXObject + 0x8c`, `0x5964a0`) |
| 0x1c / 0x20 / 0x24 / 0x28 | int | frame count, frame, direction, tick counter |
| 0x2c | int | balloon sprite: 100 thought, 101 scream, 102 speak, 0 none |
| 0x30 | int | 103 = crossed out ("not" overlay), 0 |
| 0x34 | int | priority: 2 for the relationship group, else 1. A new balloon of lower priority does not replace one still shown |
| 0x38 | u8 | show when inactive |

Groups (table at `0xa51bc0`: `{int group, u16 sprite base, u16 STR# of the icon names}`; group
names `Behavior.iff` STR# 500; sprites in `Sprites.iff`, which labels them as listed):

| Group | Base | Names (STR#) | Icons |
|---|---|---|---|
| 0 old style | - | 161 | mapped by the table at `0x998d88` to sprites 200-213 (motives) and 400-404 (topics) |
| 1 balloon | 100 | 505 | the balloon frames themselves |
| 2 conversation | 400 | 501 | three per interest in the order of `Live.iff` STR# 140 (Travel, Money, Politics, The 60's, Weather, Sports, Music, Outdoors, Toys, Aliens, Pets, School, Exercise, Food, Parties, Style, Hollywood, Technology, Romance; the sprite labels say "Violence" where the interest is Money), then 57-72 careers, 73 bored, 74-75 bad mood |
| 3 motive | 200 | 502 | 0 hunger, 1 bladder, 3 hygiene, 4 energy, 5 comfort, 6 fun, 10 social, 13 room (others have no sprite) |
| 4 relationship | 300 | 503 | 0 romance, 1 friends |
| 5 headline | 500 | 504 | hate, stress, surprise, idea, love, drunk, hurt, smell, flies |
| 6 social / debug | 600 | 506 | 1-11 hug, gift, back rub, cheer up, entertain, tickle, brag, slap, scare, tease, attack; 12 need chair, 13 in use, 14 can't route, 15-18 `++ + - --` (the relationship change after a social), 19 pizza, 20-29 and 32 routing debug icons (the `route_balloons` cheat), 30 friends, 31 romance, 33 crush, 34-37 fame |
| 7 algorithmic | - | 507 | 0 the stack object's picture, 1 the face of the Sim whose neighbor id is the stack object value, 2 the picture of the object in a local |
| 8 route failure | 700 | 508 | chair not found, door not found, wall in the way, terrain not level, no pool ladder |
| 9 progress | 800 | 509 | 0-9 skill, 10-19 repair, 20-29 interest, in tenths |
| 10 conversation 2 | 900 | 510 | magic |

Trees that set balloons, seen live: global 0x18e "route failure feedback" (the router `0x5f7950`
runs it by name with the failure code and the blocking object; it shows the blocker's picture,
group 7, or a group 8 reason; "Bella can't get past RustyRedneck Barrel Chair"), global 0x137
"help display - route failure" (the help tip about it), 0x11e "show skill progress", 0x1a0
"Relationship Impact for Join" (repeats a "-" every second or so), 0x18f "wake everyone up in
stack objects room" (a shout); the beds' private "Dream Topic", "Dream Person", "Dream Object",
"Dream Motive" (0x1034-0x1037 in `Beds.iff`; other beds number them differently) and "tweak sleep
finished" (0x1015, the bladder shout on waking); semi-global 0x201a-0x201e (idle thoughts about a
need); the social trees 0x1004 (topics) and 0x1029/0x102a (`+`).

**The burglar** (`NPCController.iff`, `thief.iff`): the thief is the object "NPC Thief" ("Some
Sneaky Sim" in its catalog strings); the NPC Controller's "steal object" takes each object with
opcode 0x12. Not seen live yet. Two things that looked like him: "El Bandito" is the Unleashed
raccoon ("NPC - Unleashed - Raccoon"), created by an NPC generator object (tree 0x100b) and
removing itself within half a second when nothing tempts it; and the night music with Sims
"arriving" is the ghosts: each grave (internal name "Urn/Tombstone") creates its ghost with
Create Object Instance from its tree 0x1004 ("Bratty", "Cousin", "Auntie" on the Lápida lot). The
controller's attribute 4 holds the day of month of the thief night ("CT - Force Thief" sets it to
global 1, "Thief Day?" compares); attributes 0 and 6 took the new day of month as each day was
processed (read live). Setting attribute 4 to today brought no burglar, neither at 2 AM (day 17)
nor at 7 AM (day 18): the spawn needs more than that (**hypothesis**: the Scheduler's "find
thief's portal" and a random roll in "process burglar"). The only theft texts are the
capture dialogs (NPCController STR# 301) and the officer's "haven't been able to catch the Sim
who robbed you" (Officer.iff).

## Live mode tool and pie menus

Clicks reach the picker in **view** coordinates: slot 30 turns them into grid coordinates for the
camera rotation with `0x586ae0(&y, &x, -[world + 0x84] & 3, 0x400)` (r = 1: y' = n - x, x' = y;
r = 2: both mirrored; r = 3: x' = n - y, y' = x; n = 0x3ff). The mod converts grid positions back
before clicking. With object id 0 (the floor) the picker places its "go here" target (type 0x7c4)
and opens a one-entry menu, "Ir a este punto"; choosing it walks the selected Sim there.

`cObjPickerTool` (`[0xa67108]` in live mode). Slot 30 (`0x621960`) is what a click on an object
does: `(int worldX, int worldY, short objectId, pickInfo*)` with `pickInfo = {12, view window, 0,
short y @+0xc, short x @+0xe, ...}` (screen position of the click; the menu opens there). It builds
the object's interactions, fills the pie at `+0x148` (top level) or `+0x14c` (sub-level) and runs
it through the menu runner `0x61fe70`, which calls the pie's slot 142 (run) and 143 (DoModal). The
DoModal result is an item index, -3 (next page), -2 (leave: the picker re-hovers and **reopens**
the top level) or -1 (back; from the top level the runner returns 0 and the menu closes). The
picker loops while the runner asks for a sub-level.

**Choosing through DoModal queues nothing**: after the runner returns, slot 30 only queues an
action in tool modes 2/3 (`+0x3c`). In normal play the queueing happens in the pie's notification
to its owner `cWinPeople` (`OnNotify` slot 73, `0x4ba8d0`): code 0xe (selection changed) calls
picker `0x621260(pie)`, code 0xf (chosen) calls picker `0x621520(pie)`, which reads the selection
(pie vtable `+0x19c`), opens the sub-level for "..." entries (pie slot 148), or takes the
interaction record (entry `+0x30`), checks it (`0x5f01f0`) and queues it with
`0x40df10(record + 0x48, neighborIdOfSelectedSim, x, y)`, then closes the menu (slot 148; not while
Shift is held). The mouse path, and the mod, call pie slot 102 `SetSelection(index, choose = 1)`
(`0x6ec760`), which sends 0xe, then 0xf (0x10 for index -1), and closes the menu when `+0x1d8` is
set. Verified: "Hablar... > Sobre intereses" chosen by keyboard queued "Hablar" and Cris walked
over to Marisa.

The game warps the real cursor with `SetCursorPos` while a pie is open (edge clamping, opening a
sub-level); the mod blocks it in development builds (hook on `user32!SetCursorPos`) because the
user works on this PC during tests.

### `cTSPieMenu` : `cTSWin` (149 slots)

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x128 | int | highlighted item, -1 none (slot 102 sets it) | read after SetSelection |
| 0x1d8 | u8 | close after a choice (slot 102 calls slot 148 when set) | `0x6ec760` |
| 0x1e4 | u8 | running modally (slot 147 posts end-modal only when set) | `0x6eb930` |
| 0x1e8 | `std::vector<item>` | items, 0x34 bytes each: `+0x00` `std::string` label ("Hablar..."), `+0x1c` u32 flags (0x1 normal), `+0x30` interaction record | read live |

Slots: 102 `SetSelection(int index, bool choose)`, 142 run, 143 DoModal, 147 `EndModal(result)`,
148 close / open sub-level. The owner is the family panel `cWinPeople`, not the picker.

## Check boxes, sliders and value pickers (options panel)

- **Check box**: a `cTSWinCheck` holding the tick button (`cTSWinBtn`, type 2, checked = ticked).
  Its name is a separate label button (also type 2, never checked) just right of it.
- **Slider** (`cTSWinSlider`, sound volumes): `+0x12c` min, `+0x130` max, `+0x134` value (slots
  99, 100, 98 return them; 0..10 for volumes). Slot 101 (`0x707120`) `SetValue(int)` clamps, moves
  the thumb and sends the owner `{3, 12, slider}` synchronously, as dragging does. Its name is the
  label button left of it.
- **Value picker** ("< Alta >", graphics options): a `cTSWinText` value between two unlabeled
  arrow buttons, named by the label above; pressing an arrow changes the value text.
- The options pages (game, sound, graphics) are containers under `cWinOptions`; the selected page
  button (type 2, checked) names the page.

## Text fields (`cTSWinTextEdit2`)

While a field takes typing, the window manager's `+0x24` points at the field itself (on create a
family and create a Sim the name field has it from the start). The game handles Backspace on
key-down, before WM_CHAR.

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x128 | int | caret position | 4 after "luna", 3 after one Backspace |
| 0x138 | `std::string` | the text, cp1252 (the size can count a trailing NUL) | "luna" |

## Create a family / create a Sim

`cWinDesignFamily`: surname field, member portraits (tooltip = first name), "Agregar un nuevo sim"
(enabled once a surname is typed), delete/edit selected Sim, Hecho, Cancelar. Hecho asks "¿Has
terminado?" (`cTSWinMsgBox`).

`cWinDesignCharacter` (the create-a-Sim screen):

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x15c / 0x160 | `cTSWinTextEdit2*` | first name, biography | |
| 0x164 | int | neighbor ID of the Sim being designed (511) | argument of the head/body counts |
| 0x16c | int | personality points left (of 25) | decremented by each bar click |
| 0x174 / 0x178 | `cTSWinBtn*` | adult / child (checked = chosen) | notification handler `0x480320` |
| 0x17c / 0x180 | `cTSWinBtn*` | female / male | |
| 0x184 / 0x188 / 0x18c | `cTSWinBtn*` | dark / medium / light skin | |
| 0x190 / 0x194 | `cTSWinBtn*` | previous / next body | |
| 0x198 / 0x19c | `cTSWinBtn*` | previous / next head | |
| 0x1a0 / 0x1a4 | `cTSSystemButton*` | Hecho / Cancelar | |
| 0x1b8 / 0x1bc | int | head index / body index into the designer's lists | +1 per arrow press, wraps |
| 0x1c0 / 0x1c4 / 0x1c8 | int | sex (1 female), skin (0 light, 1 medium, 2 dark), age (1 adult) | |
| 0x1cc | `cWinVitaBtn*` | the 3D preview | |
| 0x260 | `cTSWinBtn*` | zodiac sign: tooltip "Signo del zodiaco", caption "(Cáncer)" once points are spent | |

`cWinPersonalityWidget` (one personality bar): `+0x1e0` points 0..10, `+0x1e8` `std::string` trait
name, `+0x23c` the screen. Its mouse-move handler (slot 61, `0x4beb20`) sets `+0x238` = 1 when the
pointer is right of the filled part; the mouse-down handler (slot 55, `0x4bea70`) then adds one
point (if points are left) or removes one, and updates the screen's points left.

The character designer `[0xa6347c]` holds the heads (`+0x78`) and bodies (`+0x7c`) that fit the
chosen sex, age and skin, as vectors of 0x98-byte entries: two `StackString<64>` (vtable, pointer,
u16 length, u16 capacity, 64 chars): a texture name at `+0x0c` and the mesh at `+0x58`
("HEAD-HEAD=c036malgt_dave", "BODY=b011mafitlgt_beachguy2"). Counts seen: 137 heads / 143 bodies
for an adult light-skinned man, 107 / 121 for a woman, 23 / 17 for a girl. The names encode sex
and age (ma, fa, mc, fc), body shape (fit, fat, skn; none for children), skin (lgt, med, drk) and
a word that is sometimes descriptive (tuxedobasic, clergyman, beachguy, nerd) and often a person's
name (dave, tom); many are the collection's Command & Conquer extras (gdi..., nod...).

## Dialogs

`cTSObjectDialog` (SimAntics notification): children `cTSWinText` (title, `+0x50`), `cWinTextWrap`
(message, `+0x50`), `cTSSystemButton` (caption `+0x50`, e.g. "OK"). Runs modally through
`0x6d9f50`, so `wm+0x24` points at it while open.

## `cDefaultTTWindow` (tooltip window) : `cTSWin`

vtable `0x99fae0`, 99 slots.

| Off | Type | Meaning | Evidence |
|---|---|---|---|
| 0x128 | `std::string` | current tooltip text | read after `0x702a20` returns; matched on-screen tooltip |
| 0x144 | int | style/mode passed to `0x702a20` (hypothesis) | decompiled `0x702930` |

## Class hierarchy highlights (RTTI)

- `cTSWin` (98 virtual slots, vtable `0x99d8c0`) : `cITSMessageTarget`. Base of all widgets and screens.
- `cTSWinBtn` (105), `cTSWinText` (100, plus `cITSWinText` at +296), `cTSPieMenu` (149),
  `cWinCPanel` (98), `cDefaultTTWindow` (99): all derive from `cTSWin`.
- `cXPerson` (36) : `cXObject` (34) : `TreeSim`.
- `cTSFont` (33, vtable `0x9a22a8`) : `cITSFont`.

## Buy mode

Evidence for everything below: live reads on the Compañeras and Lápida lots (2026-09-21), the
decompiled move tool (`re/decomp/0056*.c`), and purchases, moves and a sale driven by the mod.

### Money and the catalog

| What | Where | Evidence |
|---|---|---|
| Family funds | `[[0xa67044] + 0x1c]` = `cSimulator`, `+0x90` int funds; `+0x50` short 1 = unlimited (the getter `0x556e20` answers 999999) | $9.500 -> $9.420 after one $80 chair |
| Buy panel state | `CPState` at `[0xa63570] + 0xc4` (also `cWinCatalog + 0x128`). `0x408750` (thiscall `(int index, Product*)`) selects a product; `(-1, 0)` swaps in the plain hand tool. `0x407fa0(0x200)` marks the panel changed | the catalog does exactly this when nothing is chosen (`0x4744a0`) |
| `cWinCatalog + 0x474` | `std::vector<Product*>` of the shown page (26 at most; the page builder `0x4752d0` filters the category and subcategory set); `+0x480` the hover popup (`cWinCatalogPopup`, filled by `0x4775d0` on notification 0x13) | 26 dining chairs matched their 26 buttons |
| `ProductButton + 0x1e0` | `Product*` | tooltips are the prices |
| `Product` | `+0x04` std::string name, `+0x1c` std::string description (what the hover popup shows), `+0x34` int price (`0x5368e0`), `+0x38` its `ProductButton*`, `+0x40` object type record (whose `+0x60` is the lead OBJD) | popup layout `0x4761b0` reads `0x5368f0` (+4) and `0x5368c0` (+0x1c) |

Parts of multi-tile objects (the six pieces of a dining table) have no function or room flags
and often no catalog strings: the catalog's product whose file and OBJD MasterID match carries
them (one IFF file can hold several products, so the file alone named a dining table "Mesa
plegable de naipes"). `game/lot.cpp` `CatalogEntryFor`.

OBJD fields used by the catalog (u16, FreeSO numbering, checked on catalog objects): 18 price,
39 room flags, 40 function flags, 82..88 ratings (hunger, comfort, hygiene, bladder, energy, fun,
room; a chair's comfort 2 at 83), 89 skill flags, 94 function subsort (1 = dining chairs).

### The buy/move tool (`cMoveTool`, `[0xa67108]` in buy mode)

| Off | Type | Meaning |
|---|---|---|
| 0x34 | int | mode: 1 hand (pick up, move), 2 buying from the catalog, 3.. build mode items |
| 0x3c | `cXCursorObject*` | the held object's cursor, 0 when nothing is held |
| 0x44 | type record | the product being bought |
| 0x75 | u8 | the held object is a new purchase (0: moving one of the lot's) |
| 0x76 | u8 | keep buying: after a purchase the next one of the kind is in hand (chairs) |
| 0x7b | u8 | mouse held down: moves turn the object toward the drag |
| 0x97 | u8 | active: slot 22 (mouse entered the lot) sets it, slot 23 (mouse left) clears it and takes the held object out of the world; slot 7 does nothing while 0 |

Virtual slots (thiscall; mouse handlers take `(int worldX, int worldY, int objectUnderMouse,
PickInfo*)`, PickInfo = `{12, view, 0, short y, short x}` in screen coordinates, and every one
calls slot 7 first):

| Slot | VA | What |
|---|---|---|
| 5 | `0x56f2a0` | create the held object for the chosen product (via `0x633ee0`) |
| 6 | `0x56d200` | drop the held object |
| 7 | `0x56dc90` | mouse move: re-places the held object on the tile under the mouse, which it gets from `ScreenToWorld` `0x5e00e0` (cdecl `int* (int* out, u32* packedScreen, int flag)`, view coordinates in 1/16 tile; it picks through the 3D view, so only on-screen tiles work) |
| 8 | `0x56ccc0` | mouse down: with nothing held, picks up the object whose id is the third argument (asks its slot 26 first) |
| 11 | `0x56ec10` | mouse up: if the held object sits validly (`cursor + 0x1b4`) and funds suffice, buys it or puts it down |
| 18 | `0x56ea80` | key down `(int vk)`: Delete sells a moved object (refund = `0x53adc0`) or cancels a purchase; Escape cancels (slot 24) |
| 20 | `0x56e880` | characters: `,`/`<` and `.`/`>` turn the held object, `/` cycles surface slots |
| 22 / 23 | `0x56ea30` / `0x56ea50` | mouse entered / left the lot |

The mod places without the mouse: it hooks `0x5e00e0` so the tool's own handlers get the tile it
chose (slot 7, then 8 and 11). Verified: chairs, a fridge, a sofa, a table lamp on a counter
bought where the mod said; a trash can moved; a fridge sold for its $600.

### The held object

`cXCursorObject` (a `cXMTObject`, "Tile Cursor"): `+0x1b0` the real object, `+0x1b4` short placed
validly, `+0x1b8/+0x1bc/+0x1c0` last position and level, `+0x1c8/+0x1ca` surface id and slot.
Slots: 19 turn (`0x634240`), 20 take out of the world (`0x633fa0`), 21 check, 22 place
(`0x633fd0`: check `0x633290`, which searches the container slots of an object on the tile when
the floor is refused), 34 copy the object's part offsets (`0x633200`).

Objects (`cXObject`, `cXMTObject`): slot 19 turn by `turns * (+0x78)` (`0x53e690` single tile;
`0x630a30` multi-tile, which refuses turns that do not fit where the object stands), slot 20
remove, slots 21 and 24 the pure placement check `(int pos[2] {x, y}, int level, cXObject*
surface, int slot)` (24 = `0x53efc0` / `0x630ee0`: resets and sets the error `[0xa67078]`), slot 22
place, slot 34 (`0x62fcd0`, multi-tile) recompute part offsets. `0x53a6f0(part, 0)` and
`0x53a760(part)` refresh a part after its direction changes.

| Off | Type | Meaning |
|---|---|---|
| 0x4c | short | direction (below) |
| 0x4e | short | container: id of the object it sits on (a lamp's counter), 0 on the floor |
| 0x50 | short | slot number in that container |
| 0x52 | short | allowed heights: bit (h - 1) = fits a container slot of height h (a table lamp 0x18e) |
| 0x5a | short | flags (FreeSO VMEntityFlags): 4 no floor extent (windows 0x404, paintings, helpers), 8 walks itself (Sims), 16 people pass (doors 0x410, trees, flowers, stair landings) |
| 0x78 | short | direction step per turn (2 quarter turns, 0 fixed) |
| 0x134 | vector of 0x24 | container slots (SLOT type 0): entry 0 the object, then places on it; `+0x10` short occupant id, `+0x14` int height (counters 4 and 5, dining tables 3, end tables 8) |
| 0x140 | vector of 0x3c | routing slots (SLOT type 3): `+0` / `+4` float offset (1/16 tile, written for the object facing -y: -y in front, +x its right), `+0x1c` side bits 0-7 (N = its front, clockwise), `+0x20` snap target, `+0x24` / `+0x28` / `+0x2c` min / max / optimal proximity in 1/32 tile |
| 0x14c | vector of 0x3c | SLOT type 1 entries: where the balloon is drawn (floats at `+0/+4/+8`: 0, 0, 2.0 over a Sim, 4.0 over the trash can); the first entry also holds the balloon shown ("SimAntics and balloons"). Objects without one get no balloon |
| 0x190 / 0x194 | ptr | next part / lead part (multi-tile) |
| 0x1a4 / 0x1a8 / 0x1ac | int | part offset y / x / level in tiles |

The SLOT resource loader is `0x62c820` (0x48-byte records; called from object init `0x53bc69`
with OBJD field 20). Doors, windows and stairs are `cXPortal`.

**Directions** (a Sim used a fridge with direction 6 from its -y side, facing direction 2; the
engine's own "tile in front" tables at `0xa50f88` (x) and `0xa50f64` (y) agree): 0 = -x, 2 = +y,
4 = +x, 6 = -y, odd values diagonal; seen with -y as north the engine turns counter-clockwise.
A routing slot's side bit b points toward `direction - b`. The fridge's proximity 32 put the Sim
on the adjacent tile.

**Placement errors**: `[0xa67078]` holds a 1-based index into STR# 137 (PlacementErrs); the
strings are a StringSet at `[0xa6713c]` (created by `0x56e6c0`), read with `0x58cd10` (thiscall
`(int index, char lang)`). 21 = "Insufficient funds", 11 = "Can't intersect other objects".

### The cheat console

`cTSMainWindowW95 + 0x140` is the console's `cTSWinTextEdit2` (slot 105 `0x6f0b30` sets it).
The main window's key handler (slot 53, `0x6f0f70`) opens it when the message flags carry Ctrl
and Shift (`flags & 3 == 3`) and the key is C (B on one keyboard layout), through slot 107
(`0x6f0c50`, `(char show)`). Its notification handler (slot 73, `0x6f0e70`) runs the field's text
through the cheat interpreter on code 0x17 (Enter) and closes the console; code 0x18 (Escape)
closes it. Errors are message boxes ("Sorry" / "No such cheat!").

The manager is `cTSCheatCodeManager` at main window `+0x144`: `0x6e67d0` (thiscall `bool (const
char* line)`, `ret 4`) is the interpreter (splits on `;`, `!` repeats the previous line). `+0x04`
`std::map<std::string, command>` of the registered cheats (node value at `+0x28`: handler
function, `+0x2c` its argument), `+0x0c` `std::map<std::string, std::string>` of usage texts
("grid [on|off] - enables/disables terrain grid drawing"; most say only the name). This build
registers 23: autonomy, bubble_tweak, damagelist, draw_all_frames, draw_floorable, draw_routes,
genable, grid, history, interests, log_mask, map_edit, move_objects, prepare_lot, rosebud,
rotation, route_balloons, set_hour, sim_log, sim_speed, sweep, tile_info, xyzzy. The field's
`0x6ffd40` (thiscall `(const char* text, char, char)`) sets its text.

### The lot grid (`[0xa675b8]`, the same `cRotatableWorld` as `[0xa675b4]`)

`+0x10` int tiles per side (56 on a small lot; objects may use 1..size-2). Per-level maps at
`+offset + level * 4`, each with rows at `+0x0c` indexed by y and entries by x: `+0x20` u16 floor
pattern, `+0x28` u16 room id, `+0x30` u8 flags (1 floorable, 2 seeded, 4 outside, 0x20 not
editable), `+0x38` u16 object id (one per tile: a window on the wall hides the counter below it,
so the mod lists objects from the object vector instead), `+0x40` 14-byte wall entries.
`0x585a30` (thiscall on the grid, `(WallInfo* out, u8 tile[3] {y, x, level})`) fills a WallInfo:
`+0` mask 1 = edge toward y-1, 2 = toward x-1 (both owned by the tile), 4 = toward y+1, 8 = toward
x+1 (from the neighbours), 0x10/0x20 diagonals; `+8` u16 style per segment (2, 0xc-0xe fences).
Verified: the fridge the mod put "contra la pared oeste" had mask 2 and stood against that wall.
