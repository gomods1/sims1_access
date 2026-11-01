---
title: Sims Accessibility manual
lang: en
toc-title: Contents
---

Sims Accessibility makes The Sims Legacy Collection playable with a screen reader and the
keyboard alone. It speaks every screen, dialog and panel through your screen reader (NVDA, JAWS,
Narrator and others), lets you move through them with the same keys everywhere, and adds tools a
blind player needs: a list of the lot's things, a map of the house, a narrator for what the game
only shows, spots where furniture fits, and a cursor for building walls and rooms.

This manual is opened by **Control+Shift+F1** in the game, in the game's language. **Control+F1**
says the keys of the screen you are on.

## Getting started

1. Run the installer, or copy `version.dll` and the `SimsAccess` folder into the game's folder
   (the one with `Sims.exe`).
2. Start your screen reader, then start The Sims Legacy Collection from Steam.
3. When the game window opens you hear "Sims accessibility loaded". The game starts on the
   neighborhood screen.

The mod speaks in the game's language. To change it, open Steam, then the game's Properties,
General, Language. The mod speaks English, French, German, Italian, Spanish, Dutch, Danish,
Swedish, Norwegian, Finnish, Brazilian Portuguese, Japanese, Polish, Simplified and Traditional
Chinese, Thai and Korean.

The mod works with the current Steam version of the game. If the game updates and the mod says
"unsupported game version", install the mod's next version.

## Moving around: one set of keys everywhere

Every screen, dialog, panel and mode works the same way. What you move through is a ring of
**groups** (a dialog's buttons, the needs, the family, the catalog...). A group holds **items**,
and some items open a deeper **level**.

| Key | What it does |
|---|---|
| Tab, Shift+Tab | Next or previous group. The group's name and number of items are said first. |
| Up, Down (or Left, Right) | Previous or next item in the group. |
| Home, End | First or last item in the group. |
| Enter | Press the item. On a level (a kind or a room of the lot's things) open it. |
| Space | Press, except in the lot view, where Space is the game's "next Sim" key. |
| Backspace | Back up a level. |
| R | Say the item again, with its group. |
| Escape | Close the interaction menu; otherwise the game's own Escape. |
| Alt+letter | The next group whose name starts with that letter. |
| Control+F1 | The keys of the screen you are on. |
| Control+Shift+F1 | This manual. |

Controls say what they are after their name, as a screen reader does: "Save, button", "Sound
options, tab, selected", "Auto-center, check box, checked", "Effects, slider, 8 of 10",
"Terrain detail, value picker, High".

- **Sliders, value pickers and personality bars**: Left and Right (or Page Up and Page Down)
  change the value. Enter only says it again.
- **Check boxes**: Enter checks and unchecks them.
- **Text fields** (family and Sim names, biographies): arrow onto the field and type. Each letter
  is echoed, and Backspace says what it deletes. Left, Right, Home, End and Delete move in the
  field. While a text field is on screen, letters, Space and R type.
- **Disabled controls** say "unavailable" and are not pressed.

The last group of every screen is **mod help**: its buttons say the keys of that screen (as
Control+F1 does) and open this manual (as Control+Shift+F1 does). When the game starts, the mod
says both keys after "Sims accessibility loaded".

## Reading more: the review buffers

Not everything fits in one spoken line. The mod keeps two buffers you can read line by line:

- **details**: everything about what was just spoken. A product's ratings, rooms and
  description; a lot thing's actions, room, distance and sale value; a need's or skill's
  explanation; a control's tooltip and its place in its group ("3 of 12, controls").
- **events**: what happened, newest first: purchases, sales, speed changes, dialog texts, queued
  actions, everything the narrator said (the last 200).

| Key | What it does |
|---|---|
| Control+Up | The next line, deeper into the buffer. |
| Control+Down | Back toward the top. |
| Control+Left, Control+Right | Switch between the buffers. |

## The neighborhood

The neighborhood screen's main group is **houses**: each says its address and who lives there.
Enter on a house presses it, as a click does, and the game's own card or questions follow. On a
house, Control+Up reads its details: what build mode's house overview would say,
from the house's last save (rooms, what is missing, rooms with no way in). Control+Shift+L speaks
it all at once.

The other groups hold the game's buttons: create a family, the family bin, the neighborhood
number, options, quit.

A house with no family opens in build mode only, with unlimited funds; the mod says "no family,
unlimited funds" instead of a sum of money.

## Creating a family and Sims

Everything on these screens is a group or a field:

- Type the family name and the Sim's first name in their text fields.
- **personality**: Left and Right add or take a point from each trait ("4 of 10, 6 points
  left"). The zodiac sign follows from the points.
- **appearance**: age, sex and skin, and the head and body arrows. Each head and body is
  described ("head 3 of 137: blond hair, bob").
- The biography is a text field too.
- Control+Shift and a number (1 to 9) on the lot open a Sim's card later, where the biography
  can be edited.

## The lot: live mode

On a lot the HUD is a ring of groups like any screen. Tab goes through them in this order:

1. **status**: time, funds, game speed, friends.
2. **needs** of the selected Sim, each as a percentage; Enter says what the need means.
3. **family**: the Sims of the family; Enter selects one.
4. **action queue**: what the selected Sim will do; Enter cancels an action.
5. **things by kind** and **things by room**: the lot's things (see below).
6. **controls**, the **Sim panels** (mood, relationships, job and skills, personality,
   interests, house), and any panel you open.
7. **mod help**.

On a live panel item (a need, skill, job, house rating), Enter speaks the game's popup for it.

### Selecting Sims and the camera

| Key | What it does |
|---|---|
| Space | The game's "next Sim": the next family member is selected and announced ("Isabel, selected, Eat"). |
| Control+1 to Control+9 | Select the first to ninth Sim of the family. |
| Control+Shift+1 to Control+Shift+9 | Open that Sim's card (biography). |
| Control+F | The camera follows the selected Sim, or stays free. |
| Control+E | The selected Sim in one line: room, what it is doing, urgent needs, mood. |
| P | Pause (the game's key); the mod says the time and funds. 1, 2, 3 set the speed. |

The camera follows the selected Sim, because the game's sounds come from what is on screen.
While you browse the lot's things, the camera goes to each thing and comes back to the Sim
afterwards.

### The lot's things

Everything on the lot is in two groups: **things by kind** (Sims, then the buy mode categories:
seating, surfaces, appliances..., then other) and **things by room** (the selected Sim's room
first, then the others, each with its floor and size). Enter on a kind or a room lists its
things, nearest first, as "Fridge, kitchen, 6 tiles north-east" from the selected Sim. Backspace
goes back to the kinds or rooms. Room names are the mod's guess from what is in them.

You do not need to Tab there: the Page keys browse the things from anywhere on the lot, in live,
buy and build mode.

| Key | What it does |
|---|---|
| Page Down, Page Up | Next or previous thing of the kind or room chosen last. |
| Shift+Page Down, Shift+Page Up | Next or previous kind of things. |
| Control+Page Down, Control+Page Up | Next or previous room (the first press: the selected Sim's room). |
| Enter | The thing's interactions (in buy mode: pick it up to move it). |
| Control+Enter | Walk the selected Sim to the thing. |

A Sim or an object still showing a balloon says it too ("Bella, Eat, thinks: hunger").

### Interaction menus

Enter on a thing (or on a Sim) opens the game's interaction menu. Up and Down choose, Enter does
it, Escape goes back a level or closes. Entries ending in "..." open more choices. The chosen
action is said as queued ("Talk, queued"). When the menu is empty the mod says why ("no
interactions; for children only: Play").

### Rooms and the house

| Key | What it does |
|---|---|
| Control+L | The room's layout: size, doors and windows by wall and where they lead; what stands along each wall is in the details buffer. |
| Control+Shift+L | The house overview: floors, rooms, ratings, what is missing (toilet, shower, beds, fridge, stove, phone, smoke alarm), rooms with no way in, things nobody can reach, dark or disliked rooms. |

### The house map

**Control+M** turns on the map: a cursor that walks the house tile by tile, starting at the
selected Sim. Each step plays a sound for the floor and for each wall, fence, door or window
around it.

| Key | What it does |
|---|---|
| Arrows | One tile; with Shift, five. Says what stands there and which way it faces, the room and floor when they change, the walls, fences, railings, doors and windows by side. |
| Home | Back to the selected Sim. |
| Control+Shift+Page Up, Page Down | The floor above or below. |
| Enter | The interaction menu of what stands there. |
| Control+Enter | Walk the selected Sim there. |
| Page Down, Page Up (and Shift, Control) | The lot's things as always; the map goes to each one. |
| R | Everything about the tile. Control+L the room's layout. |
| Escape or Control+M | Leave the map. |

### The narrator

The game shows many things only as pictures. The mod says them as they happen and keeps them in
the events buffer:

- **Balloons** over Sims and objects: "Bella thinks: hunger", "Mortimer shouts: bladder",
  "Bella says: Aliens", a topic the other Sim dislikes ("..., not interested"), "the
  relationship worsens", and "Trash can: flies".
- **Blocked routes**: "Bella cannot get through: Chair blocks the way".
- **Visitors** arriving and leaving, ghosts, the burglar, the Grim Reaper, the pizza delivery,
  the carpool and school bus (which car honks).
- **Things taken** by the repo man or a burglar, with their value; **bills** arriving.
- **The family watch**: what the selected Sim starts doing, by your order or on its own
  ("Cris, on its own: Sit"), when it finishes, room changes, needs falling low, the mood turning
  red, the day's relationship changes ("Marisa, with Cris: +8, now 28"), and money paid or
  received outside buy mode.
- **Trips**: where you arrived, and "back home".

**Control+N** changes how much is spoken: everything, warnings only, or off. Lines not spoken
still go to the events buffer.

Sounds you can hear (the phone, the doorbell, alarms) are not announced.

### Help tips and the tutorial

When the game's help system has a tip waiting, the mod says "there is a help tip: Control+H reads
it". **Control+H** opens it: the tip is read, or the mod names what the game's red arrow points
at.

House 7 of the first neighborhood runs the game's tutorial. Each step is read
as a keyboard version of the game's text ("Press the Zoom in button: Tab to controls, arrows to
Zoom in, Enter"), followed by what the arrow points at. Every step can be completed by keyboard.
The first Escape during the tutorial warns you; a second Escape within four seconds leaves it.

### Pizza and other services

Use the phone's interactions to call services. The pizza delivery person must be greeted: when
he arrives, go to things by kind, Sims, Down to him, Enter, Greet. If nobody greets him, he
leaves with the pizza and the money comes back.

## Buy mode

F2 opens buy mode. The catalog's categories are tabs and its products are buttons: each product
says its name, price, main ratings and description, and the details buffer holds all its ratings
and the rooms it is meant for. Moving past a page's last product turns the page.

| Key | What it does |
|---|---|
| Control+R | Keep the catalog to products for one kind of room (kitchen, bathroom, bedroom, living room, dining room, study, outside) or all. |
| Backspace | The catalog's "Back". |
| Enter on a product | Start placing it. |

### Placing an object

While you hold an object, the mod has already found the spots where it fits **and can be used**
(the game's placement check, the room's walls and doors, the space Sims need to use it). The
spot says its room, what it is next to and which way it faces.

| Key | What it does |
|---|---|
| Page Down, Page Up (or Tab) | The spots, best first. |
| Control+Page Down, Control+Page Up | The rooms that have spots. |
| Shift+Page Down, Shift+Page Up (or comma, period) | The other directions on the same spot. |
| Control+Shift+Page Down, Page Up | Beside the lot's things, nearest first. |
| Arrows | Move it one tile instead (Shift: five), saying whether it fits and what it runs into. |
| Enter | Buy it there (or put it down, when moving). |
| Escape | Cancel (a moved object goes back). |
| Delete | Sell the held object; the refund is said. |
| R | Say the spot again. |

The details buffer holds the spot's tile and the spots the game refused, with its reason.

### Moving and selling

In buy mode, Page Down to a thing of the lot and press Enter: it is picked up and the same spot list
appears. Identical objects are listed one by one, and the last purchase says "just bought".

## Build mode

F3 opens build mode. Its tools are tabs, their styles are products. The house overview is spoken
when you enter build mode (Control+Shift+L repeats it).

### The build cursor

Enter on a wall or fence style starts the build cursor at the selected Sim. It moves on the
corners between tiles.

| Key | What it does |
|---|---|
| Arrows | One corner; with Shift, five. Says where it is, the walls that meet there and what stands there. |
| Space | Mark the start. The cursor then says the distance from the start and what Enter would build and cost. |
| Enter | Build a wall from the start to the cursor. |
| Shift+Enter | Build the four walls of a room between the start and the cursor. |
| Control+Enter or Delete | Remove walls. |
| comma, period | Choose a ready-made room size. |
| Insert | Build the ready-made room with its north-west corner at the cursor. |
| Tab, Shift+Tab (or Control+Page Down, Page Up) | Jump to the next or previous room. |
| Page Up, Page Down | The floor above or below. |
| Home | Back to the selected Sim. |
| R | Everything: the free space each way, the room. |
| Escape | Leave the cursor. |

Before building, the mod checks the new walls. If they would shut a Sim in, cut an object off,
close a door or shut off part of the lot, it says so, and a second Enter builds anyway.

### Floors, wallpaper, pools and terrain

Each of these tools has the same cursor on tiles (arrows, Shift for five, Home, R, Escape;
Control+F1 for the details):

- **Floors**: Enter floors the tile, or the rectangle from a start marked with Space; Shift+Enter
  the whole room; Control+Enter removes.
- **Wallpaper**: Enter papers every wall of the room under the cursor; Control+Enter takes it
  off.
- **Pools**: Space marks a corner, Enter digs; the mod warns before water cuts anything off.
  Control+Enter fills tiles back in. Sims need a pool ladder (in the tool's list).
- **Terrain**: level from a marked tile to the cursor, or raise and lower the ground.

### Doors, windows and stairs

Doors and windows are placed like furniture, from a list of spots: "in the west wall of the
kitchen, opening onto the outside, the first door of that room".

## Sound cues

The build cursor, placing and the house map play a short sound on each step: the floor, each
side's wall, fence, door or window (each side sounds different), objects, a room change, the lot's edge, slopes, and whether an object fits.
**Control+Shift+S** turns them off and on.

## Other keys

| Key | What it does |
|---|---|
| Control+Shift+C | The game's cheat console. Up and Down pick a cheat, typing adds to it, Enter runs it (again: repeats), Escape closes. |
| Control+Shift+V | Light the focused control on screen, for a sighted helper. |

## The game's own keys

These are the game's, and still work:

| Key | What it does |
|---|---|
| F1, F2, F3, F4, F5 | Live, buy, build, camera and options mode. |
| P | Pause. |
| 1, 2, 3 | Normal, fast, fastest speed. |
| comma, period | Rotate the view. |
| plus, minus | Zoom in and out. |
| Y, N | Yes and no in dialogs. |
| Control+S | Save. |

## Settings

The mod's settings are in `SimsAccess\SimsAccess.ini` in the game's folder, section `[mod]`. The
keys that change them save them at once.

| Setting | Values |
|---|---|
| `narrator` | 2 everything, 1 warnings only, 0 off (Control+N). |
| `cues` | 1 sound cues on, 0 off (Control+Shift+S). |
| `cue_volume` | Volume of the sound cues, 0 to 100 (default 80). |
| `show_focus` | 1 lights the focused control on screen (Control+Shift+V). |

## When something goes wrong

- **No speech at all**: start your screen reader before the game. The mod's log is
  `SimsAccess\SimsAccess.log`; send it with your report.
- **"Unsupported game version"**: the game has updated; install the mod's next version.
- **A key does nothing**: Control+F1 says which keys work where you are.

## Not done yet

Vacation, Superstar and Makin' Magic's own trips and screens are not fully covered yet. Photo
and camera mode and the family album are visual only and are left out.
