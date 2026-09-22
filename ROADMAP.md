# Roadmap

What the mod must eventually cover so a blind player can play the whole game by keyboard. The
mouse is useless for blind players: every item below has to work without it. Status of individual
screens and dialogs lives in `SCREENS.md`; this file is the list of flows still to build and test.

## Everywhere

- Keyboard navigation of every screen, panel, dialog and popup (see `SCREENS.md` for what is not
  started yet), with each control read correctly in every language the game ships.
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

## Camera (done)

The game's sounds come from what the camera shows, so the camera is part of what a blind player
hears. Done (`nav/camera.cpp`): the camera follows the selected Sim through the game's own follow
mode; while PageUp/PageDown browse (objects, Sims, rooms, placement spots) it points at the browsed
thing and following pauses; leaving the browser (arrows, Tab, Home/End) resumes following. Ctrl+F
turns following off and on. Ctrl+1..9 select a family member by position. Open: what a player
hears while browsing far from the Sim is the object's surroundings, not the Sim; the narrator
below is what will keep them informed of the Sim meanwhile (its actions, needs, rooms, spoken as
they change, wherever the camera is).

## The tutorial (done)

House 7 ("Para aprender a jugar") runs the game's tutorial: a SimAntics object (`Tutorial.iff`,
text in its STR# 301) that shows each step in a non-modal panel with an arrow on the control to
use and waits for UI events (zoom, scroll, rotate, the live button, a portrait click, "Ir a este
punto", an interaction, the shower, a need bar, buy mode, a chair bought, placed and rotated, back
to live, personality, the second Sim, Space, socials, relationships, the newspaper job, the job
panel, options). `nav/tutorial.cpp` speaks each step (known by its index in STR# 301, so in any
language), the arrow's target and a keyboard hint;
`tools/tutorial_drive.py` replays the whole tutorial by keyboard to check it still passes. Options
> Opciones de juego > Reiniciar tutorial restarts it. Known: the "press Live" step only
completes by a mode change back to live when live mode is already on (the hint says so).

## Languages (done; checks open)

The mod speaks the game's language: all 17 translations of its strings (`mod/core/lang/`, English
in `util/strings_table.h`), the game's text decoded with each language's code page, tutorial
steps matched by index. Still to check in the real game (launch with `-l<Language>`, CLAUDE.md
"Languages"): each translation read aloud by a native speaker's synthesizer; the game's own money
format in each language (the mod's separators follow each language's custom); Thai text without
the `_` word marks; typing echo in Japanese, Chinese and Korean fields (lead bytes wait for the
second byte; untested, and whether the game's text fields take IME input at all is unknown).

## Live mode: following the Sim by ear (balloons, visitors and thefts done; the rest planned)

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
  - Balloons (done, `nav/narrator.cpp`; CLAUDE.md "The narrator"): thoughts, shouts,
    conversation topics, the `+`/`-` after a social, headlines over objects (flies), and route
    failures ("Bella no puede pasar: Silla ... bloquea el camino") come from the balloon
    primitive, hooked. Open: dreams are recognised only by repetition (a tree that keeps
    showing balloons is spoken once); knowing the tree's name ("Dream Topic") would let them be
    left out entirely, but no path from a frame to its tree's name was found yet. The listener
    answers a topic with the same topic, crossed out or not ("Bella dice: Alimentos", then
    "Mortimer dice: Alimentos, tachado"); **hypothesis**: crossed out = the listener dislikes
    the topic. Once confirmed (compare with the Sims' interests), say it in words.
  - Refusals (the Sim shakes their head when their mood is too low for an action): check
    whether they come with a balloon (crossed out) now that every balloon is logged.
  - Relationship changes after an interaction as numbers: compare the relationship values
    before and after, "+3 con Marisa" (the `+`/`-` icons are spoken already).
  - Skill gains: the game announces each point itself ("Mortimer received 1 Creativity Skill
    Point", read as a dialog); the progress bars in between are in the object browser's line.
  - Arrivals and departures (done for Sims): anyone placed on the lot from outside the family,
    "X ha llegado" / "X se ha ido". Walk-by townies and strays make it chatty at ultra speed; a
    verbosity setting (below) should be able to leave them out. Open: the carpool and the school
    bus are not Sims on the lot (their dialogs are read).
  - Thefts (done, not seen live yet): furniture removed by someone outside the family ("Some
    Sneaky Sim se ha llevado ..., valor $...") through the Remove Object Instance primitive.
    Ghosts are announced as ghosts; the burglar's arrival starts with "¡Ladrón!". "El Bandito"
    is the Unleashed raccoon, not the burglar. To
    test, a thief night can be forced by setting the NPC Controller's attribute 4 to the day of
    month early in the day (`STRUCTS.md` "SimAntics and balloons"); set at 2 AM or 7 AM it did not
    bring one in two tries (the controller also keeps attribute 6, meaning unknown).
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
(priority, and whether the current action is running or still routing).

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
