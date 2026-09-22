# Roadmap

What the mod must eventually cover so a blind player can play the whole game by keyboard. The
mouse is useless for blind players: every item below has to work without it. Status of individual
screens and dialogs lives in `SCREENS.md`; this file is the list of flows still to build and test.

## Everywhere

- Keyboard navigation of every screen, panel, dialog and popup (see `SCREENS.md` for what is not
  started yet), with each control read correctly in Spanish and English.
- Objects work: every object type's interaction menu opens by keyboard, its choices are queued and
  performed, and the result is announced (what the Sim is doing, needs changing, messages).
- Game state by ear: pause and speed changes, clock, day, money, the selected Sim's current action,
  need warnings, live popups and the event log.

## Base game

- Neighborhood: choose, buy, move into and evict houses (done: choose, buy, move in).
- Create a family and create Sims (`cWinDesignFamily`, `cWinDesignCharacter`): names by typing,
  gender, age, skin, personality points, zodiac, biography. Heads, bodies, hair and outfits need
  spoken **descriptions of what each one looks like**; the game has no text for them, so the mod
  needs its own descriptions (written by us, never game assets), keyed by the skin/mesh names.
  The mod reads the mesh names already (`game::CurrentLook`: "c036malgt_dave"); a description
  table per mesh (hair, face, clothing, style) is the next step, e.g. written from rendered
  previews. Until then the mod says the list position, body shape and the mesh's word.
- Live mode: browse objects by category (done), rooms, walk to a spot, what each Sim is doing,
  relationships, mood, job, skills, personality, interests, inventory, phone, calls and visitors.
- Options, saving, quitting, the help system (done). Out of scope: photo/camera mode and the
  family album (visual only; the players are blind).

## Live mode: following the Sim by ear (planned, not started)

A sighted player watches the selected Sim walk, act, and show thought balloons. A blind player
hears none of that today unless they ask (the Sims category of the object browser, the action
queue group). The mod should announce what changes, briefly, as it happens, and put each line in
the eventos buffer.

What to announce (the selected Sim; the others only when it needs the player):

- **Rooms**: "Cris entra en la cocina", "sale al jardín", "sube al piso 2". The Sim's room is
  already known (`game::LotObjects`: `room`, `level`; names from `nav/room_names.cpp`). Speak
  only after the Sim has been in the room about a second, so a walk through a hallway or a
  doorway does not produce three lines. Rooms crossed on the way to a target could be skipped
  entirely: only the room where the Sim stops matters.
- **Actions starting and ending**: "Cris: Tomar un tentempié", "terminó de comer". Compare
  `game::SimActions` (current action first) every few hundred ms: a new first entry means an
  action started, and a first entry that disappears means it ended. Queuing by the player is
  already spoken. Open questions: whether an action is shown while the Sim walks to the object
  or only once it starts using it; telling a finished action from a cancelled one; telling the
  player's actions from free will ("por su cuenta"). The debug dump `0x52da90` ("Action
  Queues:") prints each entry's "Priority" and "Entry Index", which should give the layout
  (user-directed actions have a higher priority than autonomous ones: **hypothesis**).
- **Needs and mood**: speak when a need falls past a threshold ("Cris tiene hambre" around -30,
  "urgente: vejiga" around -70), once per crossing, with some margin so a value hovering near a
  threshold does not repeat the line. Speak when the mood meter turns red. Recovery goes into the
  buffer and is not spoken. The values must be read without the panel, which is hidden until F1
  and drawn only for the selected Sim: find where `cWinMotive` gets its value (`+0x1e8` looks
  like the Sim's motive index, 7, 5, 6, 15 seen: **hypothesis**) and read every Sim that way.
- **What the game only shows**:
  - Thought balloons: the Sim thinks about food, the toilet, a bed, or someone. Leads: the
    SimAntics error texts near `0x9959a0` ("Balloon Type Is Invalid. Values are 0:thought,
    1:scream, 2:speak"; "Search type ... 0: all motive, 1:mental motives, 2:physical motives",
    the "find worst motives" primitive). `FUN_0063e760` only maps error codes to texts; the
    primitive handlers still have to be found (FreeSO's `VMSetBalloonHeadline` documents the
    operands).
  - Route failures: "Cris no puede llegar a Nevera Llamark". The routing code `0x5f7950` has
    "Route failure feedback" and "route waiting animation". Without this the player cannot tell
    why a chosen action never happened.
  - Refusals (the Sim shakes their head when their mood is too low for an action).
  - Conversation topics (interest icons in green or red): "hablan de deportes, a Marisa le
    gusta". Relationship changes after an interaction (the plus and minus signs): compare the
    relationship values before and after, "+3 con Marisa".
  - Skill gains: "Cris: cocina 3" (compare `game::SkillPoints` per 100).
  - Arrivals and departures: visitors, the carpool, the school bus, the maid (compare the Sims
    on the lot). Dialogs about these (carpool, passing out, bills) are already read.
  - Sounds the player hears but may not recognise: the phone ringing, the doorbell, the smoke
    alarm, the burglar alarm, a crying baby. The game names its sound events (`UI_queued` in
    `0x40df10`), so hooking where they are played could name them. **Hypothesis**: the same path
    plays object sounds.
- **The other family members**: only urgent needs, passing out, and events that need the player
  ("Marisa se ha quedado dormida en el suelo"), not every action (free will would flood the
  speech). What each one is doing is already available from the object browser.

Rules:

- Announcements never interrupt the player's own navigation (`speech::Say` without interrupt).
  Several changes in one poll become one line. Lines of the same kind are rate-limited, since
  ultra speed changes everything quickly.
- Switching the selected Sim, leaving the lot, or changing mode resets the baselines, so no stale
  change is announced.
- A verbosity setting (all / important / off) and one mod key that says the whole state on
  demand: "Cris, en la cocina, Cocinar, hambre baja, humor 2 verde". The key is still to be
  chosen; Ctrl combinations are free.
- One module (`nav/narrator.cpp`, ticked from the navigator in live mode) that polls and
  compares snapshots. Hooks only for what cannot be polled (balloons, route failures, sounds).

Research first, in this order: need values for any Sim without the panel; the queue entry layout
(priority, and whether the current action is running or still routing); the balloon primitive;
route failure feedback.

## Buy mode

How it works (done, `nav/buy_mode.cpp`, `nav/placement.cpp`, `game/buy.cpp`, `game/lotgrid.cpp`):

- **The catalog** is the game's own (categories, subcategories, product buttons, walked with the
  arrows; moving past a page's end turns the page); each product reads what its hover popup
  shows: name, price (and "no te alcanza"), ratings, the description. The details buffer
  (Ctrl+Up/Down) has every rating, the rooms it is meant for and the description by sentences.
- **Placing** (Enter on a product): the mod lifts the held object and, for every room of the floor
  and the yard around the house, asks the engine's own placement check which tiles and directions
  it fits in (walls, floor, other objects, money), and which places on tables and counters take
  it (container slots). Then it keeps only the spots where the object is **usable**: a Sim can
  reach one of the tiles its routing slots (SLOT resource) say it is used from, on the object's
  side of any wall; no door or stair landing is blocked; every object already there stays usable;
  no floor is cut off. Nothing is known per object: footprints, use tiles, walls and flags all
  come from the engine (`STRUCTS.md` "Buy mode").
- **Order and words**: rooms the object is meant for first (OBJD room flags; when moving, the room
  it came from), then by distance. In a room the best spots come first: objects used only from
  the front get their back to a wall, seats face a table, objects go next to related ones.
  "cocina, piso 1, 19 sitios. 1 de 19: contra la pared oeste, mirando al este, junto a Cubo de
  basura, a 8 casillas al noreste"; "sobre Encimera alicatada".
- **Keys while placing**: PageDown/PageUp spots, Ctrl+ rooms, Shift+ other directions on the same
  tile, Enter buys (or puts down), Escape cancels (a moved object goes back), Delete sells a
  moved object, R repeats; the details buffer (Ctrl+Up/Down) has the spot's parts and tile and
  how many spots were refused and why (the engine's reason).
- **Moving and selling**: in buy mode, Enter on an object of the lot browser (PageUp/PageDown)
  picks it up ("moviendo Nevera Llamark, se vende por $600") and the same spot list follows.

Still to do:

- The catalog by room: this build's buy panel shows only the function categories (the room
  tooltips exist in STR# 150 but no room buttons were seen). A mod-side filter by the products'
  room flags would do.
- A tile-by-tile cursor for fine control (each tile spoken: walls, doors, objects), for placements
  the spot list leaves out.
- Upper floors (spots are searched on the floor buy mode shows; the level buttons and stairs
  reachability are untested) and multi-tile objects on surfaces. (Wall hangings work: "contra
  la pared norte, mirando al sur"; having no floor footprint they block nothing.)
- Hear a room's layout on demand (its size, doors, what is along each wall).

## Build mode

- Walls, doors, windows, floors, wallpaper, stairs, terrain, water, pools, fences, roofs, second
  floors (`cWinArch`, `cWinRoofPanel`, tools such as `cNewWallTool`, `cNewFloorTool`,
  `cPoolTool`): a keyboard cursor on the lot grid, drawing by start and end points, costs and
  errors spoken.
- Hear the lot layout: rooms, their sizes, doors between them.

## Expansions (each flow end to end)

- **Livin' Large**: its new objects and events (genie lamp, chemistry set, servo, visitors such as
  the tragic clown and the vampire state).
- **House Party**: throwing parties (party meter, guests arriving, costume trunk, buffet, DJ booth,
  campfire, dancing, celebrity visitors).
- **Hot Date**: travelling downtown, dates, downtown lots and shopping, gifts, the relationship
  changes they bring.
- **Vacation**: travelling to Vacation Island, lodging, activities, souvenirs, coming home.
- **Unleashed**: pets (adopting, training, pet skills subpanels), Old Town, community lots.
- **Superstar**: Studio Town, fame and the career, performing.
- **Makin' Magic**: travelling to Magic Town, spells and charms, magic recipes and ingredients
  (the spell book `CWinMagicBook`, `cPickSpellList`), magic duels and the other magic places.

## Travel

Every trip away from the home lot (downtown, vacation, studio, magic town, and any other magic
places) needs: choosing the destination, who goes, the loading and arrival announcements, moving
around the destination lot, and coming back home.
