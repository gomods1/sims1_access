# Patch targets

Verified hook/call targets. VAs are at the preferred base 0x400000 (runtime address =
module base + (VA - 0x400000)). Build: `Sims.exe` 1.10.7, SHA-256 `b7c8765e...` (see CLAUDE.md).

| VA | Owner | Convention | Signature (as understood) | Use | Evidence |
|---|---|---|---|---|---|
| `0x702a20` | `cDefaultTTWindow` (non-virtual) | thiscall | `void SetTextAndRender(std::string* text, int style)`; stores text at `this+0x128` | Tooltip text source: after the call, read `this+0x128` | Frida `tools/frida/tooltip_watch.js`: printed every neighborhood-button tooltip in Spanish while hovered (2026-09-21); spoken through NVDA by `frida/speech.js` + SpeechHost |
| `0x724210` | `cTSFont` vtable slot 26 | thiscall | `int MeasureString(const char* text, int len)` = sum of glyph widths + `this[0x21]` | Width of a string in a font | Decompiled; called with len 24 for "Elige o crea una familia" |
| `0x723a70` | `cTSFont` vtable slot 25 | thiscall, `ret 0xc` | `int CountLines(const char* text, int len, int width)`: wraps a text block | **Hooked by the mod** (MinHook): captures the text a popup lays out (lot card from `0x493a3e`) | `tools/frida/font_text.js`; the mod's `game/hooks.cpp` |
| `0x723ad0` | `cTSFont` vtable slot 24 | thiscall | draws one wrapped line (called by slot 25) | | same |
| `0x492820` | `cWinLotPopup` | thiscall, `ret 0xc` | `void Fill(lotRecord* = btn+0x1e0, extra* = btn+0x274, bool force)` | Produce a house's card text (captured through `0x723a70`); the mod calls it with force = 1 | `game::LotCardText`: every house read correctly in Spanish |
| `0x43af20` | neighbors | cdecl | `void Name(int neighborId, std::string* out)` | Sim names (family picker) | `game::NeighborName`: "Federico, Diana, Daniel, Virginia" |
| `0x43a4e0` | neighbors | cdecl | `std::string* Bio(int neighborId)` | Sim biography | returned the bios shown on the person card |
| `0x43aac0` | neighbors | cdecl | `int NeighborId(cXPerson*)` | Name a Sim on the lot through `0x43af20` | `game::ObjectName`: "Cris", "Marisa", visitors |
| `0x58c2f0` / `0x58ef00` / `0x58d4c0` / `0x58cd60` / `0x58c4b0` | StringSet | thiscall | ctor; `SetSource(resFile*, short id, u32 type, bool, char lang)`; `int Load(char, char)` (0 = ok); `const char* Get(int index, char lang)`; dtor | Localized object names from the object's `CTSS` (index 1) | `game::CatalogName`: catalog objects read in Spanish |
| `0x621960` | `cObjPickerTool` vtable slot 30 | thiscall | `void OpenMenu(int viewX, int viewY, short objectId, pickInfo*)`: a live mode click; builds and runs the object's pie menu modally (returns when it closes); object id 0 = the floor ("Ir a este punto") | Open an object's interactions, walk to a spot | `game::OpenInteractionMenu`, `game::WalkTo`: a Sim walked from 45 to 9 tiles of the mailbox |
| `0x586ae0` | | cdecl | `void ViewToGrid(int* y, int* x, int r, int span)` | Inverse applied by `GridToView` in `game/lot.cpp` | the walk went the wrong way before the conversion |
| `0x61fe70` | menu runner | | runs the pie (slots 142/143) and interprets the result (index, -1, -2, -3) | | decompiled; Frida saw it return the chosen index |
| `0x6ec760` | `cTSPieMenu` vtable slot 102 | thiscall | `void SetSelection(int index, bool choose)`: highlight; with choose, notify the owner 0xe + 0xf (queues the interaction or opens the sub-level) and close | Move the highlight (choose 0) and choose (choose 1) by keyboard | `game::PieHighlight` / `game::PieChoose`: "Sobre intereses" queued and performed |
| `0x6eb930` | `cTSPieMenu` vtable slot 147 | thiscall | `void EndModal(int result)` (only while modal) | Cancel: -1 closes the top level, -2 goes back from a sub-level. Ending with an item index queues **nothing** | `game::PieCancel` |
| `0x4ba8d0` | `cWinPeople` vtable slot 73 | thiscall | `OnNotify(code, sender)`: 0xe / 0xf from the pie -> picker `0x621260` / `0x621520` | Explains where interactions are queued | decompiled |
| `0x621520` | `cObjPickerTool` | thiscall `(pie*)` | chosen entry: sub-level (pie slot 148), or queue via `0x40df10(record + 0x48, neighborId, x, y)` and close | | decompiled |
| `0x43aec0` | Sims' data | cdecl | `SimState* (int neighborId)`; mood float at `+0xc` | Mood meter value | `game::SimMood`: "2 de 5 barras en verde" matched the meter |
| `0x43a480` | Sims' data | cdecl | `short* PersonData(int neighborId)` | Personality traits (data[index] / 100) | `game::TraitPoints`: five traits summed to 25 |
| `0x43bc30` | Sims' data | cdecl | `int Skill(int neighborId, int personDataIndex)` 0..1000 | Skills of the job panel | `game::SkillPoints` (0 for a new Sim; verify with a skilled Sim) |
| `0x56a990` / `0x56b460` / `0x56b7e0` `0x56b400` `0x56b940` `0x56b850` `0x56b620` | HouseStats | thiscall | init; fill (on the sim root); size, furnishings, yard, upkeep, layout ratings 0..100 | House ratings | `game::HouseRatings`: empty lot read 0, 0, 0, 10, 1 |
| `0x627c70` | room manager (`[[0xa67044]] + 0x24`) | thiscall | `Room* Find(u16 id)` | Room records for the room browser | `game::GetRoom`: sizes and bath/bed counts matched the Goth house |
| `0x6280a0` | Room | thiscall | `bool IsOutside()` | Name the outdoors | `game::GetRoom` |
| `0x707120` | `cTSWinSlider` vtable slot 101 | thiscall | `void SetValue(int)`; clamps, notifies the owner (code 12) | Volume sliders by PageUp/PageDown | `game::SetSlider`: 8 -> 7 -> 8 |
| `0x43b230` / `0x438700` | Sims' data | cdecl / thiscall | `void Queue(int neighborId, vector<Entry>* out)`; entry destructor | What each Sim is doing (Sims category of the object browser) | `game::SimActions` |
| `0x6dd7a0` | `cTSWin` vtable slot 0 | thiscall `(msg*)` | message dispatcher (types in `STRUCTS.md`) | `game::Press` hands widgets click messages | presses verified on many widget kinds |
| `0x6e9710` | `cTSWinBtn` | thiscall `(int code)` | `Notify(code)` to the owner | explains clicks (code 1) | disassembly |
| `0x6da260` | `cTSWinMgr` | thiscall | flushes queued key/mouse messages (PeekMessage PM_REMOVE over 0x100-0x109 and 0x200-0x20e, plus its own queue) | Why keys typed during a screen change are lost | disassembly; 30 call sites at transitions |
| `0x6dac50` | `cTSWinMgr` | thiscall, `ret 0x14` | `void PostMsg(cITSMessageTarget* target, int type, int code, void* sender, int extra)` | Press a button without the mouse: `PostMsg(btn->owner, 3, 0x14, btn, 0)` from the main thread | dialog OK closed (`tools/frida/mainthread.js`) |
| `0x712560` | message pump helper | cdecl | `bool PeekOne(MSG*)`: `PeekMessageA(msg, 0, 0, 0, PM_REMOVE)` | Runs every iteration of the main loop and every modal loop: main-thread hook site for Frida jobs | `frida/mainthread.js` |
| `0x6d9f50` | `cTSWinMgr` | thiscall | `DoModal(parent, dialog)`: modal message loop; sets `wm+0x24/+0x28` to the dialog | Modal dialogs | disassembly |
| `0x6f0e40` | main window procedure (class `Gonzo`) | stdcall WNDPROC | forwards to `0x6f14f0` (thiscall on the framework window) | Key/mouse message entry; the mod subclasses the window instead of hooking this | `GetWindowLongA(hwnd, GWL_WNDPROC)` from inside the process |
| `0x6dae40` | input dispatcher | thiscall `(msg*)` | returns false immediately while `[0xa52074] == 0` (app inactive) | | disassembly |
| `0x6da408` | mouse polling (return address of `GetCursorPos`, then `ScreenToClient` at `0x6da418`) | | | The game reads the mouse by polling; faking `GetCursorPos` drives hover | `tools/frida/virtual_mouse.js` hovered buttons with the real cursor untouched |
| `0x5e00e0` | ScreenToWorld | cdecl | `int* (int* out, u32* packedScreen, int flag)`: view coordinates (1/16 tile) under a screen point, through the 3D view | **Hooked by the mod** (`game/hooks.cpp`): while an override is set it answers with the tile the mod chose, so the buy tool's own handlers place there | chairs, fridge, sofa, lamp placed on the announced tiles |
| `0x56dc90` / `0x56ccc0` / `0x56ec10` | `cMoveTool` slots 7 / 8 / 11 | thiscall `(int, int, int objectUnderMouse, PickInfo*)` | mouse move / down / up | `game::PlaceHeldAt` (move, down, up = buy or put down), `game::PickUp` (down on an object id) | purchases and a move in `SimsAccess.log` |
| `0x56ea30` | `cMoveTool` slot 22 | thiscall `()` | mouse entered the lot: creates the held object for the chosen product, sets `+0x97` | `game::BeginPlacing` when the real mouse is elsewhere | cursor created, spots announced |
| `0x56ea80` | `cMoveTool` slot 18 | thiscall `int (int vk)` | key down: Delete sells the moved object, Escape puts it back | `game::SellHeld`, `game::PutBackHeld` | "vendido Nevera Llamark por $600" and the funds went back up |
| `0x633fa0` / `0x633200` | `cXCursorObject` slots 20 / 34 | thiscall `(int)` / `()` | take out of the world / copy part offsets | `game::LiftHeld`, `game::TurnHeld` | |
| `0x53efc0` / `0x630ee0` | `cXObject` / `cXMTObject` slot 24 | thiscall `bool (int pos[2], int level, cXObject* surface, int slot)`, `ret 0x10` | the pure placement check; sets `[0xa67078]` | `game::CheckHeldAt`, `game::CheckHeldOn` | spots the engine refuses carry its STR# 137 reason |
| `0x53e690` | `cXObject` slot 19 | thiscall `(int turns)` | turn a single-tile object | `game::TurnHeld` | |
| `0x62fcd0`, `0x53a6f0`, `0x53a760` | `cXMTObject` slot 34; part refresh / redraw | thiscall | what the multi-tile turn `0x630a30` does per part, without its fit test | `game::TurnHeld` for multi-tile objects | sofa turned on the lot (screenshot) |
| `0x585a30` | the lot grid (`[0xa675b8]`) | thiscall `WallInfo* (WallInfo* out, u8 tile[3])`, `ret 8` | walls on a tile's edges | `game::WallsAt` | fridge against the wall it named |
| `0x408750` / `0x407fa0` | `CPState` | thiscall `(int index, Product*)` / `(u32)` | select a product (-1, 0 = none) / mark the panel changed | `game::CancelPlacing` | tool back to mode 1 |
| `0x6e9b30` | `cTSWinBtn` | thiscall `(int checked)` | set checked and redraw | uncheck the product button after a cancel | |
| `0x58cd10`, `0x56e6c0` | StringSet, loader | thiscall `(int index1, char lang)`; cdecl `()` | placement error text (STR# 137 at `[0xa6713c]`) | `game::PlacementErrorText` | "¡Eso se sale del terreno!" |
| `0x53adc0` | objects | fastcall `int (cXObject*)` | current sale value | spoken when an object is picked up | "se vende por $600" for a new fridge |
| `0x590080` | | cdecl `int ()` | the floor buy/build mode work on | `game::HeldLevel` | |
| `0x6f0c50` | `cTSMainWindowW95` slot 107 | thiscall `(char show)` | show the cheat console (what Ctrl+Shift+C does) | `game::OpenCheatConsole` | console opened from posted keys; rosebud ran |
| `0x6e67d0` | `cTSCheatCodeManager` | thiscall `bool (const char* line)`, `ret 4` | the cheat interpreter | `game::RunCheat` (Enter in the console; the console stays open) | rosebud +$1.000 twice; set_hour 12 |
| `0x6ffd40` | `cTSWinTextEdit2` | thiscall `(const char* text, char, char)` | set the field's text | `game::SetCheatText` (Up/Down in the console) | the picked cheat's name was run |

## Hypotheses (not verified)

- `0x6ddf30` (`cTSWin` slot 46): "show tooltip for this widget"; calls `0x6da4d0` (get/create the
  tooltip window) then `0x702a20(this+0xbc)`.
- `cWinNeighborhoodUL` slot 98 (`0x4a4fe0`): select/highlight lot by house number (walks the lot map at `+0x1a4`).
- `cTSFont` slots 23/15 (`0x724420`, `0x7242b0`): per-glyph draw/measure, called per character.
