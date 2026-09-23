# The Sims Legacy Collection: screen reader accessibility mod

Playbook: `template_generic.md` (read it first; its rules apply). This file holds what has been
learned about *this* game. Anything marked **hypothesis** has not been verified yet.

Status (2026-09-21): the native mod runs (`mod/`, see "The mod" below). Speech through NVDA works
(Prism in `SpeechHost.exe`). Keyboard navigation covers the neighborhood (toolbar, houses with
their cards, the neighborhood number), the family picker (members with personality and bio),
create-a-family and create-a-Sim (typing with echo, personality points, appearance), every
dialog, the options pages (check boxes, sliders, value pickers), and the lot view: a HUD (status
line, mood, needs, family, action queue, controls, every Sim panel with real values), an object
browser by category or by engine room with distances, interaction menus whose choices are queued,
walking to an object's spot, what each Sim is doing, and a narrator for what the game only shows
(balloons over Sims and objects, route failures, visitors arriving and leaving, furniture a
burglar or the repo man takes). Buy mode: products read like their
hover popup, and placing, moving and selling go through a list of spots the mod computes from
the engine (placement check, routing slots, walls, doors) so every object stays usable. The mod
speaks every language the game ships (see "Languages"). Not done yet: build mode, expansions
and travel, review buffers, help. Planned work: `ROADMAP.md`. Screen
inventory: `SCREENS.md`. Verified targets: `PATCH_TARGETS.md`; layouts: `STRUCTS.md`.

## Working on this machine (read first)

- **The user uses this PC while sessions run** (e.g. audio in Chrome in front of the game). Never
  move the real mouse, steal focus, or grab the screen. Use `tools/game.py shot` (PrintWindow,
  works while the game is covered) and `tools/frida/virtual_mouse.js` (fakes `GetCursorPos` for
  Sims.exe and posts mouse messages to the game window).
- **The game runs in Spanish** (`HKCU\Software\Electronic Arts\The Sims 25\SIMS_LANGUAGE = Spanish`,
  STR# language code 6). Test against Spanish text; every mod string needs all the game's
  languages (see "Languages").
- **Save files are disposable**: the user wipes them before really playing. Tests may answer yes
  to save / buy / move-in prompts. (An old backup of the stock saves exists in
  `C:\Users\Nitropc\code\mods\sims-backups\2026-09-21-pre-first-launch`.)
- NVDA runs on this PC; audible speech tests are allowed any time (keep them short).

## The mod (`mod/`)

| Part | Output | Notes |
|---|---|---|
| `mod/loader/` | `version.dll` (x86) next to `Sims.exe` | Proxy for the system DLL (all 17 exports). Checks the exe hash, waits for the window, subclasses it (ANSI), loads a **copy** of the core, runs a 30 ms timer. Catches crashes in the core and disables it. Owns the log file. |
| `mod/core/` | `SimsAccess\SimsAccess.dll` (x86) | The mod. `main.cpp` wires `util/` (log, text, language, strings), `lang/` (translations), `speech/`, `game/` (all addresses in `game/addresses.h`), `nav/` (collect, describe, screens, navigator), `input/` (keys). |
| `mod/speechhost/` | `SimsAccess\SpeechHost.exe` (x64) + `prism.dll` | Owns Prism; reads `say 0 <utf8>` lines from `\\.\pipe\SimsAccessSpeech-<pid>`; logs every line to `SpeechHost.log`; `--mute` logs without speaking. |
| `mod/third_party/minhook/` | | Vendored MinHook (BSD). |

- Build: `mod\build.bat` (MSVC 2022, x64 + x86); `mod\build.bat deploy` copies into the game folder.
- **Dev loop**: with `SimsAccess\dev.txt` present, the loader hot-reloads `SimsAccess.dll` within a
  second of a rebuild (no game restart; Ctrl+Alt+F12 forces it) and verbose logging is on. With
  `SimsAccess\force_active.txt` the core keeps the game's app-active flag set, so the game takes
  input while other windows are in front (development only). `version.dll` and `SpeechHost.exe`
  only update after the game exits.
- **Launching**: the game restarts itself through Steam (SteamStub) with its arguments, and Steam
  then waits for the user to confirm "launch with these arguments" (2026-09-23: `launch.py` and
  `game.py launch` both hung there; every later launch queues behind it). Ask the user to accept
  it, or launch with no arguments (`steam://run/3314060`). `launch.py` writes the user's
  foreground window to `SimsAccess
eturn_focus.txt`; a dev core hands the foreground back to it
  when the game window takes it at startup.
- `python tools/nav_to.py <key> <text>` presses a key until the mod speaks a line containing the
  text (stops at a group's end); never chain an `enter` after it with `;`, only with `&&`.
- Drive the game by keyboard without focusing it: `python tools/keys.py tab down enter` (posts key
  messages; TranslateMessage in the game makes the WM_CHAR; the key-up is posted 60 ms later so the
  character lands before it, as with a real keyboard: the game counts key-downs since the last
  key-up and Space / N select the next Sim only on a first press). Posted keys carry lParam bit 25 so
  the mod tracks their Shift from the messages; real keys use the physical keyboard state
  (`GetAsyncKeyState`), because a system shortcut (Ctrl+Win+Alt) once left the tracked Ctrl
  "held" and every key went to the game. Read results in
  `<game>\SimsAccess\SimsAccess.log` (every spoken line is logged as `Spoke:`).
- Mod keys (none is a game hotkey; Ctrl/Alt combinations are free for later features):
  - Ctrl+F1 anywhere: the mod's keys for the current screen, spoken.
  - Ctrl+E (lot view): the selected Sim's state in one line ("Cris, en exterior, Tomar un
    tentempié; necesidades urgentes: comodidad, higiene; humor, 3 de 5 barras en rojo").
  - Ctrl+L (lot view): the room's layout (the build cursor's, else the selected Sim's): size,
    bounds, doors and windows by wall and where they lead; what stands along each wall in the
    details buffer. Every build tool has a cursor now (walls, floors, wallpaper, pools, terrain).
  - Ctrl+N: the narrator's verbosity, everything / warnings only / off (saved in
    `SimsAccess\SimsAccess.ini`, `[mod] narrator=2/1/0`); unspoken lines still go to eventos.
  - Review buffers (`nav/buffers.cpp`, playbook section 11): Ctrl+Up reads the next line deeper,
    Ctrl+Down goes back toward the top (no wrap); Ctrl+Left/Right switch buffers, skipping empty
    ones ("detalles, 5 elementos"). **detalles**: the details of what focus last spoke (a
    product's ratings, rooms and description by sentences; a placement spot's parts, tile and the
    area's refusals; a lot object's actions, room, distance, categories and sale value; any
    control's tooltip and place in its group), replaced on each focus change, the position kept
    when the same thing is spoken again. **eventos**: purchases, sales, speed changes, dialog
    texts, queued actions, cheat results, newest first (200 kept).
  - Cheat console: Ctrl+Shift+C (the mod calls the game's own opener, main window slot 107, so
    posted keys work too). It reads "consola de trucos <text>, campo de texto" and a hint once.
    Up/Down/Home/End pick one of the cheats this build registers (read from the game's cheat
    manager: 23, e.g. rosebud, move_objects, set_hour, grid, with the game's usage text when it
    has one) and put its name in the field; typing is echoed and adds to it ("set_hour 12");
    Enter runs the line through the game's interpreter and keeps the console open, so Enter
    again repeats it ("truco: rosebud. fondos $4.232, +$1.000"); the game's errors come as a
    message box ("Sorry. No such cheat!"); Escape closes.
  - Screens and dialogs: arrows (move in group), Home/End, Tab/Shift+Tab (groups), Enter/Space
    (press; texts the press changes are read, e.g. a help topic), R (repeat), Escape (cancel an
    open interaction menu). In the lot view Space is left to the game: it selects the next Sim of
    the family (the tutorial teaches it), and the change is announced ("Isabel, seleccionado,
    Comer"); Enter presses there. Enter on the mode tab that is already selected only says
    "seleccionada": pressing it would hide the whole panel (the game's F1 toggle).
  - Text fields (family and Sim names, biography): typing goes to the game's field; the mod
    echoes each character and the one Backspace deletes; the field reads "Introduce apellido:
    luna, campo de texto". While a field is on screen, letters, Space and R are typing.
  - Roles: controls say what they are after their name, as a screen reader does: "Guardar,
    botón"; "Opciones de sonido, pestaña, seleccionada"; "Autocentrado, casilla de verificación,
    marcado"; "Efectos, control deslizante, 8 de 10"; "Detalle del terreno, selector, Alta".
    Tabs (`Role::Tab`, `IsTab` in `nav/collect.cpp`) are the buttons that show a page of their
    panel: the five mode buttons, the Sim panels, options pages, catalog categories, build tools,
    the inventory and relationship filters. List entries say "botón" right after their name,
    before their details: houses ("Calle Sim,1, botón. La familia ..."), families, catalog
    products ("Silla ..., botón, $80, ..."), interaction menu entries. Read-only items say no role.
  - Values: Left/Right (or PageUp/PageDown) change the focused slider ("6 de 10"), value picker
    ("Media") or personality bar; Up/Down move; Enter only repeats them (a press would click the
    slider's middle and set it to half). Check boxes toggle with Enter.
  - Create a Sim: Left/Right on a personality bar add/remove a point ("4 de 10, quedan 6
    puntos"); age, sex and skin choices and the head/body arrows say the new head and body
    ("cabeza 3 de 137: waylon").
  - Lot view (the HUD): Up/Down switch groups (status line: time, funds, speed, friends; needs;
    family, where Enter selects a Sim; action queue, where Enter cancels; controls; Sim panels),
    Left/Right move inside a group. On a settings page (an options page, or any group with a
    slider) Up/Down walk its rows first and leave it at either end; coming up into one lands on
    its last row. The options panel opens with no page shown until a page tab is pressed (the
    game's behavior). Pausing (the game's P) says "Pausa. 11:46, $9.500" and puts
    focus on the status line; unpausing and speed changes are spoken.
  - Lot (live mode) object browser: PageDown/PageUp (next/previous object in the category or
    room), Shift+PageDown/PageUp (next/previous category: Sims, then the buy mode categories,
    then "other"), Ctrl+PageDown/PageUp (next/previous room, the selected Sim's first: "cocina,
    piso 1, 22 objetos, 74 casillas"), Enter/Space on the object (open its interaction menu),
    Ctrl+Enter (walk the selected Sim to the object's spot, e.g. into that room), R
    (repeat with distance). Arrows/Tab return to the panels. Objects read as "name, room, a N
    casillas al noreste" from the selected Sim. Rooms are the engine's; their names are the
    mod's guess from their contents (`nav/room_names.cpp`).
  - Camera and Sims (`nav/camera.cpp`, `game/view.cpp`): Ctrl+1..Ctrl+9 select the n-th Sim of the
    family the way a click on its portrait does ("Isabel, seleccionado, Comer"), in any mode. The
    camera follows the selected Sim (the game's own follow mode, what Enter does in live mode)
    because the game's sounds come from what is on screen; while PageUp/PageDown browse objects,
    Sims or placement spots the camera points at the browsed thing instead (following pauses) and
    follows the Sim again once the browser is left (arrows, Tab, Home/End). Ctrl+F turns following
    off ("cámara libre") and on ("siguiendo a Isabel"). Ctrl+n presses the portrait as a click
    does (the click is what the tutorial detects); the game opens the Sim's card instead when it
    sees Ctrl held at that moment, so `GetAsyncKeyState` is hooked to report Ctrl up for 400 ms.
  - The tutorial (house 7, "Para aprender a jugar", `nav/tutorial.cpp`): its steps are non-modal
    `cTSObjectDialog` panels inside the lot view plus a red arrow (`cTSWinTutorialHighlight`)
    on the control to use. The watcher speaks every new step as a **keyboard version of the
    game's text** ("Tutorial: ... Pulsa el botón Ver de cerca: Tab hasta controles, flechas hasta
    Ver de cerca, Intro"; `Str::Step*`). A step is known by its index in the tutorial's STR# 301
    (`game::TutorialTexts` loads it in the game's language through the game's StringSet; the
    shown text equals one entry after `CleanForSpeech`), so this works in every language; steps
    without a keyboard version are read as the game wrote them, 48 with a hint), then the
    control the arrow is nearest to
    ("la flecha señala a Ver de cerca +, en controles"); the
    panel's group is "tutorial" (its unnamed button hides the text) and never takes focus. The
    help system's tip icon is announced when it appears. Options > Opciones de juego >
    Reiniciar tutorial restarts it (the family is evicted, the house restored). Every step was
    completed by keyboard (`tools/tutorial_drive.py` replays them): browsing with PageDown counts
    as scrolling, Ctrl+Enter as "Ir a este punto", the mod's placing and turning as buying and
    rotating. The game's Escape cancels the tutorial silently, so while a step is shown the first
    Escape only says "Escape otra vez para salir del tutorial" and the second within 4 s goes
    through; "el tutorial ha terminado" is said when the tutorial object leaves the lot.
  - Help tips: the help system leaves a blinking icon (`cTutorialIcon`, top left) while the lot
    module holds a "situation" object (`ObjectModule + 0xa4`); the icon widget lingers hidden
    after the tip is gone, so the pointer is the truth. "hay un consejo del sistema de ayuda:
    Control+H lo lee" is said when it appears; the icon is listed (group "consejo") while it is
    pending; Ctrl+H does what a click does (`ObjectModule::0x532060`: the help system runs "show
    situation info": a "¿Sabías que...?" dialog, read like any dialog, or the red arrow on the thing the tip is about), and the
    watcher names what the arrow points at: a control, or the lot object or spot under the
    arrow's tip through the game's own screen-to-world ("la flecha señala a Cubo de basura,
    exterior, a 5 casillas al sur"). Ctrl+H with nothing pending says so.
  - Pizza (verified by keyboard): phone > Servicios... > Pizza ("Son $40 pavos"); about a game
    hour later the delivery Sim ("Dandi de la Pizza Dale") stands outside and **must be greeted**:
    Sims category, PageDown to him, Enter, Saludar ("Recibir Pizza, en cola"); if nobody greets
    him he leaves with the pizza and the money comes back. The Sim who receives it puts the box
    on a counter and eats; the box ("Caja de pizza", otros objetos, OBJD type 34 = food, listed
    since this session) offers "Coger una porción" and "Limpiar" to the others. His arrival is
    announced ("Dandi de la Pizza Dale ha llegado", the narrator below).
  - The narrator (`nav/narrator.cpp`, `game/simantics.cpp`; layouts in `STRUCTS.md` "SimAntics and
    balloons"): every balloon the game shows is heard as it appears and kept in the eventos
    buffer: "Bella piensa: hambre", "Mortimer grita: vejiga", "Bella dice: Alienígenas" (topics
    by the game's interest names), "Cassandra: la relación empeora" (the `-` after a social),
    "Cubo de basura: moscas", and route failures as "Bella no puede pasar: Silla ... bloquea el
    camino" (the global tree 0x18e, what the help tip about blocked Sims refers to). The hook is
    on the primitive itself (`0x61bc40`), so nothing is missed. Spoken: the family's and objects'
    balloons, a visitor's only when it concerns the family or is a shout; skill progress bars
    are not spoken (the game announces each skill point itself). A tree that keeps showing
    balloons over a Sim (a sleeping Sim's dreams cycle an icon every second, "Relationship
    Impact for Join" repeats its `-`) is spoken once and then quiet until it stops for 30 s; the
    same line over the same Sim once per 15 s (60 s over objects). The object browser adds the
    balloon still shown to a Sim's or object's line ("Bella, Comer, piensa: hambre"). Visitors:
    Sims placed on the lot from outside the family, "X ha llegado" / "X se ha ido", polled every
    100 ms (walk-by townies and strays included; the game keeps unplaced NPCs such as "Grandma"
    in its list, left out); announced once they have stayed a second (the Unleashed raccoon
    "El Bandito" is created and removes itself within half a second). Ghosts, recognised by the
    grave that creates them: "ha aparecido el fantasma de Bratty" / "el fantasma de Bratty se ha
    ido" (the night music the user took for the burglar's). The burglar's arrival starts with
    the game's own cry from his file ("¡Ladrón! ...", `thief.iff` STR# 305; not seen live yet). Taken objects: Remove Object Instance (opcode 0x12) of furniture (OBJD function
    flags) by anyone outside the family: "X se ha llevado Y, valor $Z" (not yet seen live: no
    burglar came during the tests; the only game texts about thefts are the capture dialogs).
  - The family watch (`nav/sim_watch.cpp`, part of the narrator): the selected Sim's running
    action ("Cris: Sentarte" when the player chose it, "Cris, por su cuenta: ..." for free will,
    by the queue record's priority 50 / 2), the end of what the player chose ("Cris terminó:
    Sentarte"), room changes once the Sim stays ("Cris, ahora en cocina"), needs falling low
    (selected Sim) or urgent (anyone), the mood turning red, the selected Sim's daily
    relationship changes ("Marisa, con Cris: +8, ahora 28"; the game's `+`/`-` balloon is then
    only kept in the buffer), money paid or received outside buy mode ("-$40, quedan $8.415").
    An empty interaction menu says why ("sin interacciones. solo para niños: Jugar"). Sounds are **not** narrated (the
    player hears them), except which car honks (carpool, school bus, police) through the sound
    event hook `0x527250`.
  - In buy mode identical objects are listed one by one (one of them is to be moved) and the
    last purchase reads "recién comprado".
  - Interaction menu: Up/Down/Home/End (items; the game highlights them too), Enter (choose;
    entries ending in "..." open a sub-level), Escape (back one level / close).
  - Buy mode (F2): the catalog's buttons by arrows; a product reads name, price, ratings and its
    description (the details buffer: all ratings, rooms it is meant for). Enter on a product starts placing
    it: PageDown/PageUp the spots (best first), Ctrl+PageDown/PageUp rooms, Shift+PageDown/PageUp
    other directions on the tile, Enter buys, Escape cancels, R repeats; the details buffer has
    the spot's tile and the area's refused spots with the game's reason. Enter on an object of the lot browser picks it up to move it (same spot list;
    Escape puts it back, Delete sells it and says the refund). Moving past a page's last product
    (or back from its first) turns the catalog's page by itself ("página siguiente").
- Build mode (F3): the panel's tools are tabs, their styles products. Enter on a wall or fence
  style starts the build cursor (`nav/build_mode.cpp`, keys in its header and Ctrl+F1): arrows
  move a corner (Shift five), Space marks the start, Enter builds a wall, Shift+Enter a room,
  Ctrl+Enter/Delete removes, PageDown/PageUp + Insert ready-made rooms, Home back to the Sim, R
  everything (free space, room), Escape leaves. The mod drags the wall tool itself (down, drag,
  up; `STRUCTS.md` "Build mode"), checks the walls first from the lot's entrance
  (`placement::CheckNewWalls`) and asks for a second Enter when they would shut in a Sim, cut an
  object off, close a door or isolate tiles. Some expansion "fences" are objects: their press
  brings the buy tool and no cursor. Doors and windows (build items, move tool mode 3) go through
  buy mode's spot list: "en la pared oeste de habitación vacía, 34 casillas, da a exterior, la
  primera puerta de esa habitación, casilla 29, 31". The floor tool (`cNewFloorTool`, same slots)
  puts the cursor on tiles: Enter a rectangle, Shift+Enter the room, Ctrl removes.
- Buy mode placement internals: the mod hooks `ScreenToWorld` (0x5e00e0) and drives the buy tool's
  own mouse handlers on the tile it chose. The view calls the tool's "mouse left" (0x56ea50) every
  frame the real mouse is not over the lot, which takes the held object out of the world: the mod
  hooks it and skips it while it shows a spot, so the object stays visible and findable (the
  tutorial cancelled itself when its chair vanished). Spots come from `nav/placement.cpp`; object directions are the engine's
  (0 = -x, 2 = +y, 4 = +x, 6 = -y; the mod calls -y north), see `STRUCTS.md` "Buy mode".
- When the game window has keyboard focus, everything typed goes to it: Space and R are mod keys
  there (Space presses the focused item, e.g. cancels a queued action) and letters are game
  hotkeys (T shows the FPS counter). Happened once when the user typed elsewhere-bound text.
- Game behaviors that affect testing: posted keys arriving during a screen transition are
  discarded by the game's input flush (`0x6da260`); SimAntics notice dialogs close themselves after
  about 40 s; the live panel is hidden when a lot loads until F1 (or a mode switch). A pause key
  sent while a notice is up goes to the notice, and the game runs on when it closes itself.
- Since the narrator speaks on its own, the last spoken line is often a balloon: `tools/nav_to.py`
  can miss the item it looks for. Check every line spoken after the key instead, and never send
  an Enter unless the item was found (an Enter once pressed OK on a notice instead).
- memtool pointer trap: `G(0xa67044)` is already `[0xa67044]` (the app root): the simulator
  (funds) is `u32(G(0xa67044) + 0x1c)`, the ObjectModule `u32(u32(G(0xa67044)) + 0x14)`. A write
  through one deref too many hit a heap value once (restored at once, 2026-09-23).
- **Frida Interceptor on SimAntics code can crash the game when it detaches**: SimAntics throws
  C++ exceptions through its primitives, which breaks Frida's per-call state (fail-fast
  0xC0000409 in frida-agent.dll after `tools/frida/prim_trace.js` watched `0x615cb0` for 15
  minutes, 2026-09-22). `tools/frida/balloon_trace.js` (`0x61bc40`, rarely throws) detached fine.
  Use the mod's own MinHook hooks (no per-call state) for anything on that path.

## Languages

- **How the game picks its language** (verified in `0x457600`, the app's startup): the registry
  value `SIMS_OTHERLANGUAGE`, else `SIMS_LANGUAGE`; then Steam's language for the game
  (`ISteamApps::GetCurrentGameLanguage`, "spanish", "brazilian", "koreana"...); then a
  command-line switch `-l<Language>`. Each source overrides the one before. There is no in-game
  language option. The result is a language number in `[[0xa67044]] + 0x68` (the STR# language
  byte): 1 US English, 2 UK English, 3 French, 4 German, 5 Italian, 6 Spanish, 7 Dutch, 8 Danish,
  9 Swedish, 10 Norwegian, 11 Finnish, 12 Hebrew and 13 Russian (slots holding English text), 14
  Portuguese (Brazilian text), 15 Japanese, 16 Polish, 17 Simplified Chinese, 18 Traditional
  Chinese, 19 Thai, 20 Korean.
- **To change it**: Steam > Library > The Sims Legacy Collection > Properties > General > Language.
  Steam runs `3314060_install.vdf`, which writes `SIMS_LANGUAGE`/`SIMS_OTHERLANGUAGE` (and
  `SIMS_SKU`); every language is in the one installed depot. Steam's "russian" writes "Russian",
  which the game ignores (English). **Without touching Steam** (tests): `python tools/launch.py fr`
  (en, en-gb, fr, de, it, es, nl, da, sv, no, fi, pt, ja, pl, zh-hans, zh-hant, th, ko; `--restart`
  closes a running game first) starts `Sims.exe` directly with the switch, which works while Steam
  runs and avoids the confirmation Steam shows for `steam://run` arguments; the names are `USEnglish`,
  `UKEnglish`, `French`, `German`, `Italian`, `Spanish`, `Dutch`, `Danish`, `Swedish`, `Norwegian`,
  `Finnish`, `Portuguese`, `Japanese`, `Polish`, `SimplifiedChinese`, `TraditionalChinese`, `Thai`,
  `Korean` (case-sensitive, after a dash: the switch parser `0x6e1550`). The registry names are
  the same.
- **Text encoding** per language (verified on `UIText.iff`): Windows-1252 for the Western
  languages, 1250 Polish, 932 Japanese, 936 Simplified Chinese, 950 Traditional Chinese, 874 Thai,
  949 Korean. Thai text joins words with `_` (line-break marks; **hypothesis** that the game never
  draws them: its font code special-cases `_` at `0x725413`/`0x725db8`), so the mod drops them.
  `util/language.cpp` holds the table; `util::GameTextToUtf8` converts all game text.
- **The mod's strings**: English is the source, `mod/core/util/strings_table.h` (`X(Id, "English")`).
  Each translation is `mod/core/lang/<language>.cpp` (16 files; UK English uses English), written
  by `python tools/mod_strings.py write <language> <json>` from a `{Id: text}` JSON so they all
  look the same; `python tools/mod_strings.py check` compares every file with the English table
  (missing ids, unknown ids, printf placeholders); `export <language>` gives the JSON back for
  editing. A missing translation is spoken in English. Translations may reorder placeholders
  (`%2$s`): `util::Format` uses MSVC's positional printf. **Adding a string**: add it to
  `strings_table.h`, add it to every language (export, edit, write), run `check`. Names of game
  controls, Sims, interactions in a string must be the game's own label in each language:
  `python tools/game_terms.py [--objects] "<English label>"` prints it in all languages. Two
  traps found while translating: the Sims' names are what the game shows, from
  `UserData\Tutorial.FAM` (CTSS 48 Bob, 47 Betty), which has no Danish, Norwegian or Finnish
  names, so those games show Bob and Betty although their tutorial texts use local names; and
  need names (`Rating*`) follow the Needs panel (`Live.iff` STR# 130), since the catalog popup's
  words differ in some languages and a few game labels mean something else (German and Dutch
  "+Study" name the room: the mod says the skill).
- The mod reads the game's language number after `game::InitMemory` (before that, from the
  registry, so even an unsupported build speaks the player's language) and logs
  `language: <name> (<source>), code page <n>`. Money the mod writes groups digits by the
  language's custom (`$1,200`, `$1.200`; none where spaces group digits); **not verified** against
  the game's own format outside Spanish.

## RE workflow (all scripts in `tools/`)

1. `python tools/game.py launch [switches]` (through Steam, prints PID), `game.py shot <name>`,
   `game.py kill`. Borderless 2560x1600 window, class `Gonzo`; the 1024x768 (4:3) game image is
   scaled into it (hypothesis for the exact internal resolution; measured scale about 2.08).
2. `python tools/dump_exe.py`: writes `dumps/Sims_1.10.7_b7c8765e_dump.exe` (decrypted `.text` from
   the live process, relocations undone to base 0x400000, everything else from disk).
3. Ghidra headless (`re/run_ghidra.bat`, about 2 min) imports the dump into `re/ghidra/SimsRE` and
   runs `tools/ghidra/ExportRE.java` -> `re/functions.tsv`, `re/string_xrefs.tsv`,
   `re/import_xrefs.tsv`. Ghidra 12's RTTI analyzer does not run on this dump, so:
4. `python tools/rtti.py` -> `re/classes.tsv` (596 vtables, bases) and `re/vtables.tsv` (14,916 slots).
5. `python tools/decompile.py <va> ...` -> `re/decomp/<entry>_<name>.c`.
6. `python tools/memtool.py "<code>"` / `-f tools/wintree.py` inspects live memory with RTTI class
   names (e.g. dump the current screen's widget tree); `python tools/disasm.py <va> [n|--func]`
   disassembles the dump quickly (capstone).
7. `python tools/frida_run.py a.js[,b.js] [seconds] [--drive x.py] [--speech|--speech-mute]` runs Frida scripts
   (with `tools/frida/common.js` helpers: `va()`, `where()`); `python tools/trace_class.py
   <Class>[,...] --with frida/virtual_mouse.js --drive drive_*.py` hooks every virtual method of a
   class and reports calls with sampled arguments.

Tools installed for this: Frida 17.18 + pymem + mss (pip --user), Ghidra 12.1.3 and Temurin
JDK 21 (scoop, buckets `extras` and `java`). Ghidra needs `JAVA_HOME` set explicitly in the
shell (the scripts do it).

## Target build

| Item | Value |
|---|---|
| Install | `S:\steam\steamapps\common\The Sims Legacy Collection` (Steam app 3314060) |
| Game | The Sims 1 + all 7 expansions, 2025 re-release ("25th year Release") |
| `Sims.exe` | FileVersion 1.10.7, 7,125,288 bytes, SHA-256 `b7c8765ed2383cbda8c047694d54a21d97895c8dae57dcc87716ca6e78b342e3` |
| `changelist.txt` | 936127 (`eadpsdk.json` says applicationVersion 1.10.0) |
| Bitness | **x86 (32-bit)** PE, MSVC linker 14.40, ASLR on (DllCharacteristics 0x8140), preferred base 0x400000 |
| PDB | `C:\dev\sims1remaster\build\Sims\Release\Sims.pdb` (path only; not shipped) |
| Other binaries | `steam_api.dll` (Steamworks), `gimex.dll` (EA image lib), `ijl10.dll` (Intel JPEG), `crashpad_handler.exe` (uploads to backtrace.io) |

## Reconnaissance findings

- **Engine**: original Maxis Sims 1 C++ engine, recompiled. No managed runtime, no scripting
  language in the exe; object behaviour is SimAntics bytecode (BHAV chunks) inside IFF files.
- **Protection**: `.text` entropy 8.00 plus a `.bind` section = SteamStub DRM wrapper; code is
  encrypted on disk. Static analysis must use a **dump of the running process**, not the file.
  `.rdata`/`.data` are plaintext (strings and RTTI readable). Do not strip the DRM (template 1).
  FaithBeam's hack tool AOB-scans the live process successfully, which agrees with this.
- **RTTI**: 562 class names in `.rdata`. Highlights:
  - UI toolkit: `cTSWin` base, `cTSWinBtn`, `cTSWinCheck`, `cTSWinSlider`, `cTSWinScrollbar`,
    `cTSWinText`, `cTSWinTextEdit2`, `cTSWinTextList`, `cTSWinDlg`, `cTSWinGenDlg`,
    `cTSWinMsgBox`, `cTSWinMgrW95` (window manager), `cTSWinCtrlMgr`, `cTSPieMenu`,
    `cTSWinTutorialHighlight`, `cTSCursorManager`, `cTSKeyboard`.
  - Screens/panels: `cWinNeighborhood`, `cWinPickFamily`, `cWinDesignFamily`,
    `cWinDesignCharacter`, `cWinCPanel` (control panel), `cWinMotive`, `cWinRelationship`,
    `cWinPersonality`, `cWinInterest`, `cWinSubpanelJob/House/Fame/ReportCard/CatSkills/DogSkills`,
    `cWinCatalog`, `cWinBudgetDlg`, `cWinOptions`, `cWinLivePopup`, `cWinLotPopup`,
    `cWinPhoneBook`, `cWinScrapbook`, `cWinCredits`, `cWinDowntown`, `cWinVacation`,
    `cWinStudiotown`, `cWinMagicland`, `cWinHelp`, `cWinGift`, ... (full list: re-run the RTTI grep).
  - Simulation: `cSimulator`, `cXObject`, `cXPerson`, `cXPortal`, `House`, `Family`,
    `Neighborhood`, `Behavior`, `TreeSim`, `ObjectModule`.
  - Build tools: `cNewWallTool`, `cNewFloorTool`, `cObjPickerTool`, `cMoveTool`, `cPoolTool`, ...
  - Debug leftovers: `cDebugPanel`, `cWinMemView`, `cCheatCommandLine`, `cTSCheatCodeManager`.
- **Rendering**: Vulkan (`vulkan-1.dll` loaded at runtime; not in the import table). Runs as a
  borderless popup window, not exclusive fullscreen: debugger breakpoints will not freeze the desktop.
- **Text**: the game draws text with its own bitmap-font code (`cTSFont`), **not GDI**: a Frida trace
  of `DrawText*`, `TextOut*`, `ExtTextOut*`, `GetTextExtentPoint32*` saw zero calls while tooltips
  rendered. Hook the game's own text functions instead (see `PATCH_TARGETS.md`).
- **Input**: window messages (`PeekMessageA` at `0x712574`, wndproc `0x6f0e40`); the mouse position
  is **polled** with `GetCursorPos` + `ScreenToClient` (`0x6da408`), about 13 Hz while the game is in
  the background. No DirectInput/XInput strings or imports found.
- **Strings**: the legacy code page of the game's language (see "Languages"); Windows-1252
  verified live for Spanish (UI text and the live tooltip `std::string` both decode correctly).
  The exe calls `GetACP` and warns when the Windows user name does not match the locale.
- **Command-line switches** (strings next to each other at VA 0x980b90): `memlog`, `memdiffNT`,
  `enable_sound_trycatch`, `skip_intro`, `software_intro`, `no_thread_init`, `dynamic_draw`,
  `debug_objects`, `rebuild_catalog`, `non_us_locale`, `show_init_progress`, `soundguys`,
  `dumpassets`, `reportassets`. Prefix syntax (`-x`?) not yet verified.
- **Saves**: inside the install folder: `UserData` .. `UserData8` (Houses, Characters, Neighborhood.iff,
  Options.iff). Current neighborhood is in the registry (`SIMS_CURRENT_NEIGHBORHOOD_PATH`).
- **Startup**: no launcher, no EA account prompt; through Steam the game reaches the neighborhood
  screen in about 10 seconds on this machine (first launch, 2026-09-21).
- **Custom content**: `Downloads\<name>\*.far|*.iff` and `GameData\UserObjects`.

## Mod loading

- **Proxy DLL: `version.dll`**. Imported by `Sims.exe` (`GetFileVersionInfoSizeA`,
  `GetFileVersionInfoA`, `VerQueryValueA`), not in KnownDLLs on this machine. Alternatives: `winmm.dll`,
  `dwmapi.dll`, `msvfw32.dll`, `wininet.dll` (all not KnownDLLs). `imm32.dll` is a KnownDLL.
- The mod DLL must be **x86**. Prism ships x64 only, so speech needs the out-of-process
  `SpeechHost.exe` bridge (template 8.3).
- Hooks must wait until SteamStub has decrypted `.text`: the loader waits for the game window
  (verified: signatures match and hooks install once the window exists).
- UI model (verified, details in `STRUCTS.md`): every widget is a `cTSWin` with a child list,
  rectangle, caption, tooltip and visible flag. The window manager's `+0x24` is the window that owns
  input (screen or modal dialog). Pressing = handing the widget the same messages a real click
  delivers (mouse enter, left down, left up) through its own handler (`game::Press`).

## Data formats (verified parsers in `tools/simsfmt.py`)

- **FAR** v1 archives (`FAR!byAZ`), uncompressed entries in all files checked so far.
- **IFF** 2.5 (`IFF FILE 2.5:TYPE FOLLOWED BY SIZE`, header says "JAMIE DOORNBOS & MAXIS 1").
  Big-endian chunk headers, 76 bytes.
- **`GameData\UIText.iff`**: all UI text for all languages as `STR#` chunks (format -3: each entry
  has a language byte), one chunk per screen/panel (e.g. #128 PickFamilyStrs, #129
  DesignFamilyStrs, #130 DesignCharStrs, #138 VCtlTips, #145 optionstrs, #146 budgetstrs, #148
  Pause, #151 NghbBtnTips, #161 LocalKeys, #166 Help). Entries carry a comment naming the widget
  (`"2 Control Panel: Button07: Tooltip"`) and many screens store widget positions as `"(x;y)"`
  strings. Language codes: the game's language numbers ("Languages"); every language except 12
  and 13 has the full text, and tables line up entry by entry across languages (#166 Help does
  not). The live panels keep their own tables in `Live.iff` (#134 ModeTips: the Sim panel tabs;
  #130 Motives; #136 job labels), build mode in `Build.iff`.
  Dumps: `extracted/uitext_en.txt` (`... strings GameData\UIText.iff 1`), `extracted/uitext_es.txt` (`... 6`).
  The Spanish text has translation slips (the "Import Family" tooltip reads "Barrio actual").
- `UIGraphics\*.RT` + `*.h`: resource scripts mapping resource IDs to bitmaps (parsed by the
  game's `cGZResMgrRTParser`).

## Game key bindings (from `UIText.iff` STR# 161 LocalKeys and 138 VCtlTips)

| Key | Action |
|---|---|
| F1 / F2 / F3 / F4 / F5 | Live / Buy / Build / Camera / Options mode |
| P, `~`, `` ` `` | Pause |
| 1 / 2 / 3 | Normal / High / Ultra speed |
| `,` `.` (`<` `>`) | Rotate left / right |
| `+` `=` / `-` `_` | Zoom in / out |
| x / z | Next / previous page |
| a, s, t, v | Anti-alias, shadows, framerate, version |
| y / n | Yes / No in dialogs |
| q | Quit |
| Ctrl+S, Ctrl+Q | Save, quit game |
| Ctrl+X/C/V/Z/Y/A | Text editing |
| Ctrl+Shift+C | Cheat console (verified: main window key handler `0x6f0f70` checks Ctrl+Shift in the message flags, then slot 107 `0x6f0c50` shows the field at `+0x140`; cheat names are the table near `0x981ddc`: rosebud, move_objects, set_hour, ...; no klapaucius or motherlode, and "help" is not registered) |

## Tooling on this machine

- Present: Python 3.14 (+ `pefile`, `pillow`, `frida-tools`, `pymem`, `mss`), Ghidra 12.1.3,
  Temurin JDK 21, MSVC x86 compilers (VS 2022 14.44, VS 2026 14.51), MinGW-w64 x64 (winlibs),
  Rust (x64 targets only), .NET SDK, git, scoop (buckets main, extras, java), winget.
- Not installed: x32dbg, cdb, Detect It Easy (not needed so far).

## Community leads (verify before trusting)

- **FreeSO / Simitone** (github.com/riperiperi): open-source C# reimplementation of TS1/TSO. Best
  reference for IFF chunk formats, SimAntics, and the game's object model concepts; says nothing
  about this exe's memory layout.
- **FaithBeam/Sims1LegacyCollectionHacks**: external `WriteProcessMemory` patcher. Debug-cheat
  patch = find `80 7B 4C 00 75 43` in memory and write `EB` at +4 (turns a `jnz` into `jmp`).
- No existing Sims 1 screen reader mod was found (Sims 4 has one by tyjokr).

## Layout

- `mod/`: the mod's source and build script (`mod/out/` is build output).
- `tools/`: RE and data scripts (not shipped).
- `extracted/`: dumped game text and data (gitignored; copyrighted, never shipped).
- `dumps/`: decrypted exe dump (gitignored; never shipped).
- `re/`: Ghidra project and exports, decompiled C (gitignored).
- `shots/`: screenshots from `game.py shot`.
