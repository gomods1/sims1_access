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
