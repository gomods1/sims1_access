# Screen inventory

Every screen, panel, dialog and popup of The Sims Legacy Collection, and how far the mod supports
it. Sources: the RTTI class list (`re/classes.tsv`, every `cTSWin` subclass), live tree dumps
(`python tools/memtool.py -f tools/wintree.py`, saved as `re/wintree_*.txt`) and play-throughs.

Status: **not started** / **announced** (named on entry) / **navigable** (every control reachable
and read correctly) / **verified** (tested with NVDA including edge cases). Focus model is tier C
everywhere (the game has no keyboard focus; the mod walks the widget tree).

## Screens and their panels

| Screen | Class | Focus model / mouse-only parts | What a sighted player sees | Status |
|---|---|---|---|---|
| Startup / loading | `cTSMainWindowW95` with no children | none | splash, loading bar | silent by design |
| Neighborhood | `cWinNeighborhoodUL` (`cWinNeighborhood`, also `cWinMagicland`) | toolbar buttons; houses are `cWinLotBtn` hovered for a card | toolbar (family, import, evict, zoning, downtown/vacation/studio/magic town views, quit), neighborhood number between arrows, 40 houses with hover cards | navigable (neighborhood number not spoken yet) |
| Move-in mode | same screen, family button checked | pick a house | cards say whether the family can afford the house | navigable, verified through purchase |
| Family picker | `cWinPickFamily` | rows are `cPickFamilyItem`; portraits hover for a person card | families with portraits, net worth, friends; back, create, delete, move in | navigable (person card details not reachable yet) |
| Create a family | `cWinDesignFamily` | text entry (family name), portraits `cWinDesignFamPeopleBtn` | surname field, members, add/delete/edit Sim | verified: surname typed with echo, Sim added, family finished and listed in the picker |
| Create a Sim | `cWinDesignCharacter` | text entry, personality bars `cWinPersonalityWidget`, head/body arrows, 3D preview | name, bio, personality, zodiac, age, sex, skin, head, body | verified: name, all 25 points by PageUp, sex/skin/age with the new head/body read, Hecho. Heads and bodies are described ("cabeza 2 de 107: pelo castaño, corto, rizado"; every shipped head and body, adults and children, `nav/appearance.cpp`); custom content says its mesh word |
| Lot view (live mode) | `cDDDSimsView` | the world itself is mouse-only (click objects, pie menu) | action queue, view controls, family panel, needs, clock, money | HUD navigable (status line with clock, funds, speed, friends; needs; family; queue; controls; pause/speed spoken); world: object browser by category or by room (engine rooms, named from contents) with distances, interaction menus by keyboard (verified: a chosen interaction is queued and performed, e.g. the fridge's quick breakfast). walking to an object's spot (Ctrl+Enter), identical objects merged ("78 iguales"), each Sim's current action ("Menelaus, Perseguir el rabo") |
| Live panel | `cWinCPanel` > `cWinPeople` | subpanels: `cWinSubpanelJob/House/Fame/ReportCard/CatSkills/DogSkills`, `cWinRelationship`, `cWinPersonality`, `cWinInterest` | Sim portraits, needs meters, mood, job, relationships... | navigable: family (select a Sim), mood (bars), needs, relationships (daily/lifetime), job (job, pay, hours, skills), house (ratings, area, rooms), personality and interests (points), inventory tabs, pet skills ("Enseñado, 0 de 10", person data 10, 12, 11), a child's report card ("Informes: B"). inventory items (`cWinGift`: "Tu varita mágica, botón", the popup's description in the details buffer; what pressing one does is not known). Not checked: fame |
| Options (F5) | `cWinOptions` | tab buttons, check boxes `cTSWinCheck`, sliders | game/sound/graphics options, save, neighborhood, quit | verified: all three pages (opens with none shown until a page tab is pressed); check boxes toggle, volume sliders and value pickers change with Left/Right (or PageUp/PageDown), Up/Down walk a page's rows |
| Camera (F4) | `cWinCamPanel` | snapshot buttons | | out of scope (user decision: visual only) |
| Buy mode (F2) | `cWinCatalog`, `cWinCatalogPopup`, tool `cMoveTool` | catalog pages; placing objects in the world; picking objects up | categories, subcategories, product icons with prices, a hover popup (name, price, ratings, description), the object following the mouse | navigable: products read name, price, affordability, ratings and the popup's description (details buffer: every rating, suggested rooms, the description by sentences); Enter starts placing with the mod's spots (floor and surfaces, rooms and the yard); buying, moving (Enter on an object of the lot browser), selling (Delete) and cancelling verified. Not checked: upper floors, the catalog's room sort (no room buttons seen in this build) |
| Build mode (F3) | `cWinArch`, `cWinRoofPanel`, tools `cNewWallTool` etc. | drawing walls/floors in the world | tools, styles with prices, the lot | navigable for walls: the panel reads (tools as tabs, styles as products); walls and rooms built by the mod's corner cursor with checks, doors placed by the spot list, floors, wallpaper (whole rooms) and pools by the tile cursor, windows and pool ladders by the spot list (all verified). Roofs, terrain, stairs, fireplaces, plants and upper floors not checked |
| Old Town map (Unleashed trolley) | `cWinNeighborhoodUL` | | | done: named "Ciudad Vieja, ¿a dónde vamos?"; lots that can be visited first, each with the filters that mark it; a filter says the lots it marks ("marcados en el mapa: El Paraíso de las Mascotas"); the bar's doubled buttons are listed once |
| Studio Town and Magic Town maps | `cWinStudiotown`, `cWinMagicland` | | | done (verified 2026-09-23): named, visitable lots first with their filter kinds, a filter says the lots it marks; filter arrays at +0x144 / +0x15c, chosen filter +0x16c / +0x17c |
| Downtown / Vacation maps | `cWinDowntown`, `cWinVacation` | | | downtown used by keyboard (Hot Date trip); vacation not checked |
| Credits | `cWinCredits` | | | done: the credits (GameData\Credits.iff STR# 163-172, the game's language) are in the details buffer, Ctrl+Up reads them line by line |
| Help | `cWinHelp` | topic list with a scroll bar | topics and their text | verified: all 47 topics reachable (also those scrolled out of view); choosing one reads its text |
| Budget | `cWinBudgetDlg`, `cWinBudgetRow` | | income/expense table, totals | navigable (rows read with column names) |
| Phone book | `CWinPhoneBook` | | whom to call | verified 2026-09-23 with the Lápidas: the title follows the choice ("Llamar a Pierre Urbino."), group "familias" (surnames; Enter chooses one and its members appear) and group "miembros" (first names), Llamar, Cancelar; with no contacts "Gunther no conoce a nadie." |
| Scrapbook (the family album of snapshots) | `cWinScrapbook` | | pages of photos with captions | out of scope (user decision: photos are visual only) |
| Magic book / spells | `CWinMagicBook`, `cPickSpellList`, `cPickSpellItem` | | page of four spells drawn as ingredient icons; the chosen one's name and description on the right | done (verified 2026-09-23 with the starter kit's book): group "conjuros" with one item per spell ("ingredientes: Cera de abejas, Cera de abejas, Cera de abejas"); Enter chooses it and reads the right page ("Talismán desconocido. Combine..."; undiscovered spells keep the game's "desconocido"); the description reads as text; anterior / Página 1 de 6 / siguiente / Cancelar. The cookbook uses the same window (group "recetas", by the constructor's cookbook flag; not seen live) |
| Gift | `cWinGift` | | | not started |
| Sim's card (Ctrl+Shift+1..9 in the lot view: the game's Ctrl+click on a portrait) | `cWinEditBio` | text entry | | done: "Bio de Cris: ..., campo de texto", OK, Cancelar (whose card: `+0x128`) |
| Downtown lot description | `cWinEditDTLotDesc` | text entry | | not started |

## Dialogs and popups

| Dialog | Class | Status |
|---|---|---|
| Object / event notices ("¡Hay una nueva compañía de taxis...") | `cTSObjectDialog` (a `cWinPictureDialog`) | verified: title, message and buttons read on open; OK by Enter. They close themselves after about 40 s. |
| Yes/No and message boxes ("¿Comprar casa?", "¿Quieres guardar la partida?") | `cTSWinMsgBox` | verified |
| Help system prompt ("Sistema de ayuda") | `cTSObjectDialog` | verified |
| Pie menu (interactions) | `cTSPieMenu` | verified: opened from the object browser, items and sub-levels by arrows, Enter queues, Escape backs out |
| Radio dialog | `cWinRadioDialogBox` | out of scope: only a debug cheat opens it (`0x459ad0`, in the cheat table at `0x980248`) |
| Picture / splash / progress dialogs | `cWinPictureDialog`, `cWinPictureSplashDialog`, `cWinProgressPictureDialog` | picture dialogs read (friend count explanation) |
| Clothing choice (wardrobe's "Cambiar de ropa") | `cWinClothingChoiceDlg` | done: the arrows and the preview say the outfit ("traje 108 de 121, atlético: hipy"; "el traje que lleva ahora" when the worn one is not in the list) |
| Pet select (a pen's "Adoptar un perro") | `cWinPetSelectDlg` | done: the arrows and the preview say the breed ("mascota 6 de 16: beagle"); the sex question, the confirmation and the name field are ordinary dialogs |
| Transform (Makin' Magic, "¿Bella o bestia?") | `cWinTransformMeDlg` | arrows say "aspecto 3 de 8" (from its click handler; not seen live) |
| Dispose (sell) popup | `cWinDisposePopup` | not needed so far: the mod sells a held object through the tool's Delete key and says the refund |
| Live popups (the live panels' hover help: a title and a description) | `cWinLivePopup`, `cWinLivePopupClient` | done: the details buffer of every need, trait (by points, a pet's texts for pets), interest, job line and skill, house rating and line, pet skill, grade and relationship card has what the popup says (needs and traits from the widget, the rest from `Live.iff` STR# 130-142, 172 through `game/iff_strings.cpp`); job performance, whose text follows the mood, is left out |
| Lot card | `cWinLotPopup` | read as the house's label |
| Person card | `cPickFamilyPersonInfo` | covered by the family picker's "miembros" group (name, personality, biography) |
| Tooltip | `cDefaultTTWindow` | tooltips are the labels of buttons |
| Tutorial arrow / help icon | `cTSWinTutorialHighlight`, `cTutorialIcon` | help icon navigable ("consejo del sistema de ayuda") and announced when it appears; the arrow's target is named by the tutorial watcher |
| Tutorial step panel (house 7) | `cTSObjectDialog` inside `cDDDSimsView` (non-modal), text in a `cWinTextWrap`, unnamed hide button | verified: every step spoken as it appears with a keyboard hint (`nav/tutorial.cpp`); the whole tutorial completed by keyboard (`tools/tutorial_drive.py`) |
| Cheat console | `cTSWinTextEdit2` of `cTSMainWindowW95` (+0x140), `cTSCheatCodeManager` (+0x144) | verified: opens with Ctrl+Shift+C; Up/Down pick from the 23 registered cheats (usage texts read); typing echoed; rosebud ran twice by Enter, Enter (+$1.000 each, said); set_hour 12 set the clock; "No such cheat!" read. The cheat help window (`cTSWinCheatHelp`) could not be opened ("help" is not registered in this build) |

## Hover-only, spatial-only and non-verbal information still to cover

- The neighborhood number (drawn between the arrow buttons).
- Mood meter values ("Grado de humor"), personality trait values, relationship values.
- Everything in the world: object positions, Sims' positions and actions, room boundaries.
  Balloons (thoughts, shouts, conversation topics, the icons over heads and objects, route
  failures) are spoken as they appear and read in the object browser (`nav/narrator.cpp`);
  visitors arriving and leaving too.
- The event log (`cEventLog`): a debug window toggled only by a cheat command (0x44ee10 calls 0x40d3a0); out of scope.
