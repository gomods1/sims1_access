# MOD_TEMPLATE.md

A playbook for an AI coding agent building a **screen reader accessibility mod for a PC game**: any genre, any engine, including old native games with no modding support at all, where the work starts with reverse engineering the executable. It is distilled from shipped mods of this kind, but it deliberately contains **no game-specific facts**. Every class, function, address, struct offset, file format, screen, keyword and key binding for the target game must be discovered by you, from the game itself. Names in the code samples (`SomeScreen`, `ItemUI`, `GetTitle`) are placeholders, not predictions.

The goal is not "the mod speaks some text". The goal is: **a blind player can start the game, play it to its natural conclusion (or indefinitely, if it has none), and use every screen in between without sighted help.**

Sections marked **[managed]** apply to games whose code runs on a managed runtime you can decompile (Unity Mono, XNA/MonoGame/.NET, Java). Sections marked **[native]** apply to games compiled to machine code (custom C/C++ engines, Unreal, IL2CPP, most games older than about 2010). Unmarked sections apply to everything. When an example uses Unity or Harmony names, it is an example of the *pattern*; translate the mechanics, keep the architecture.

---

## 0. Non-negotiable rules

1. **Understand the game before writing mod code.** Extract everything that can be extracted statically (section 4), then observe the running game (section 5). Never guess a method signature, field name, struct offset, calling convention or enum value; read it or measure it. A hook against the wrong target fails silently at best and crashes the game at worst.
2. **Traverse every screen in the game.** Build a complete screen inventory (section 6) and drive each entry to "verified". A screen you did not look at is a screen a blind player gets stuck on. Popups, confirmation dialogs, tutorials, pause menus, summaries, credits, launcher windows, installers' first-run prompts and error dialogs all count.
3. **Speak through Prism** (`C:\Users\Nitropc\code\libs\prism`), via one wrapper. No other code calls the speech library (section 8).
4. **Never interrupt speech by default.** Always queue (`interrupt = false`). Game events arrive in bursts, and interrupting cuts off information the player has no other way to get.
5. **Announcements are concise; details go to review buffers** (section 11). Focus on an object says what it is and its one or two key numbers. Everything else goes into a buffer the player reads on demand.
6. **Prefer game state over rendered output.** Read from the game's own objects and data. UI labels are a fallback (placeholders, stale values, markup); hooked draw calls are a fallback for that; OCR is the last resort and a sign that more reverse engineering is needed.
7. **Review features never change game state.** Virtual cursors (world review, map review, board review) only read; they never move the game's real selection, camera-independent state, or clock.
8. **Everything a sighted player learns by looking or hovering must be reachable by keyboard.** Tooltips, icons, meters, colors, positions on screen, things moving in the world.
9. **If the keyboard cannot reach it, build the path.** Many games are mouse-first (hover, click, drag, camera). Where the game has no keyboard navigation, the mod writes the navigation layer itself (section 10.4). "The game does not support it" is never a reason to leave something unreachable.
10. **Use the running game as an instrument.** You are allowed and expected to launch the game, attach debuggers, inspect and modify RAM, trace API calls and take screenshots (sections 1 and 5). Do not theorize for an hour about something a breakpoint answers in a minute.
11. **Prefer scriptable, text-producing tools.** A Frida script, a headless Ghidra script, a `cdb` command file or a Python memory reader can be rerun, diffed and committed to `tools/`. A finding that exists only as clicks in a GUI is lost at the end of the session.
12. **Write down what you learn.** Maintain `CLAUDE.md` (architecture plus pitfalls), `PATCH_TARGETS.md` (verified hook targets), `SCREENS.md` (the inventory) and, for native games, `STRUCTS.md` (memory layout). A future session starts with no memory of this one.
13. **Do not invent game facts in documentation or code comments.** If you did not read it in the decompiled source, the disassembly, a data file, or observe it in a log or debugger, it is a hypothesis; label it as one and verify it. Community wikis and format docs are leads, not facts, until checked against this build of the game.

---

## 1. What you may do on this machine

The user owns this machine and a legitimate copy of the game, and the mod is single-player accessibility work. Within that:

**Allowed without asking each time**

- Launch, drive and close the game. Take screenshots of it. Read its logs, config and save files.
- Install analysis tools in user scope (`scoop`, `winget`, `pip`, `dotnet tool`, `cargo install`). Say what you installed and why; record it in `CLAUDE.md`.
- Attach debuggers to the game, set breakpoints, single-step, dump memory.
- Read and write the game's process memory; inject DLLs and scripts (Frida, your own) into the game process.
- Patch **copies** of game binaries and data files for experiments.

**Ask first**

- Anything that needs administrator rights, installs a driver or kernel component, changes system-wide settings, or disables a security feature (Defender exclusions, DEP, signature enforcement).
- Installers that are known to bundle third-party offers (the Cheat Engine installer at `S:\games\software\CheatEngine751P.exe` is one): never run these silently; walk through them with the user.
- Modifying the game's install in place (replacing the original exe or DLLs rather than adding files), or touching the user's real save files. **Back up first, always**: copy saves and any file you will overwrite to a dated folder outside the install before the first experiment.
- Compatibility wrappers and shims that change how the game renders or runs (section 5.1).

**Not allowed**

- Defeating anti-cheat, or touching anything in an online/multiplayer context.
- Cracking or redistributing copy protection. If the game will not start because of legacy disc DRM (old SafeDisc/SecuROM drivers are blocked on modern Windows), stop and tell the user; how to obtain a working legitimate build is their decision.
- Shipping game code, decompiled source, extracted assets or patched game binaries. The release contains only the mod's own files (section 19). Keep extracted material gitignored or outside the repo.

**Hygiene**: never leave the game, a debugger, or an injected helper running at the end of a task. Record the PID when you launch and clean up by PID.

---

## 2. Phase 1: Reconnaissance

Answer these questions before anything else, and record the answers in `CLAUDE.md`. All of it can be done without starting the game.

| Question | How to find out |
|---|---|
| What engine / runtime? | Look at the install folder and the exe's imports, strings and resources. `<Game>_Data\Managed\Assembly-CSharp.dll` = Unity Mono. `GameAssembly.dll` + `il2cpp_data` = Unity IL2CPP. `.pck` or an embedded pck = Godot (GDRE Tools are at `S:\games\software\`). `data.win` = GameMaker. `www\` or `.rgss*` = RPG Maker. `.rpa`/`.rpyc` = Ren'Py. `.jar` = Java. `.pak` + `Engine\` = Unreal. A .NET exe (check with Detect It Easy or `ilspycmd`) = XNA/MonoGame/WinForms. None of the above = a custom native engine: plan for sections 4.3 and 5 to be most of the project. |
| Bitness | PE header of the game exe (Detect It Easy, `dumpbin /headers`, or read the `Machine` field with a few lines of Python). **Everything injected must match it.** Prism ships Windows binaries for x64 and arm64 only, so a 32-bit game needs the speech bridge (section 8.3). |
| Packed or protected? | Detect It Easy reports packers, protectors and compilers. High-entropy sections, a tiny import table, or a section named after a protector mean static analysis of the file on disk will show little; dump the unpacked image from the running process instead (section 5.3). Record the compiler and its version: it determines calling conventions, RTTI format and standard library layouts. |
| Exact build | Version resource, file size and SHA-256 of every executable and game DLL. Addresses and offsets are only valid for one build. Record them now; the mod checks them at startup (section 9.2). Note whether expansions, patches or re-releases exist that replace the exe. |
| Debug leftovers | PDB or `.map` files, a debug or editor build, assert strings with source file paths, verbose log strings, a developer console, cheat codes, command-line switches (`strings` for `-` and `/` prefixed words), ports to other platforms that kept their symbols. Any of these can save weeks. |
| Graphics API and windowing | Imports: `ddraw`, `d3d8/9/11/12`, `opengl32`, `vulkan-1`, GDI only. Does it run windowed? **Exclusive fullscreen plus a breakpoint equals a frozen desktop** (section 5.1). |
| UI framework | Engine UI (uGUI, UI Toolkit, UMG, Godot Control), an embedded browser or Flash/Scaleform layer, native Win32 controls, or a fully custom toolkit drawn by the game. Determines how focus and text extraction work. |
| Text rendering | GDI/DirectWrite calls (`TextOutW`, `DrawTextW`, `ExtTextOutW`: hookable, give you strings for free), engine text components, or bitmap fonts blitted glyph by glyph (you must hook *above* the blit, where the string still exists). |
| Input path(s) | Window messages (`WM_KEYDOWN`, `WM_MOUSEMOVE`), `GetAsyncKeyState`/`GetKeyboardState`, DirectInput, Raw Input, XInput, or an engine layer (legacy `Input`, the new Input System, Rewired). There are often **two** paths (UI navigation *and* a game-specific mapping). You must know all of them to read and suppress keys correctly (section 12). |
| Which **navigation tier** is the game? | **Tier A**: full keyboard navigation through the UI framework's focus system, so your work is mostly *reading* focus (10.1). **Tier B**: the game has its own controller/keyboard navigation system that the keyboard does not drive, or drives only partly, so you drive *that system* (10.4). **Tier C**: mouse-only, so you build the whole focus layer (10.4). Decide this per game **and re-check per screen**. |
| Is anything gated on mouse-only input? | Drag handlers, hover-dependent logic, right-click actions, camera-relative clicking in a world view, and **polled waits** ("press any mouse button to continue" loops). Each one is a place a keyboard player gets hard-stuck. |
| Data and archive formats | What file types sit in the install? Which are standard (zip, wav, bmp, xml, ini, lua) and which are proprietary containers? Magic bytes, community format documentation, existing extractors. |
| Scripting / bytecode layer | Lua, Python, Squirrel, a custom VM, behavior trees in data files. Game logic that lives in data is often easier to read than the exe, and tells you what the exe's primitives must be. |
| Localization system | Where player-facing strings live (string tables in resources, per-language files, a localization asset) and the single "look up string N" entry point. |
| Existing community knowledge | Modding tools, format specs, open-source reimplementations, fan patches, speedrun/cheat tables, prior accessibility attempts. A reimplementation project is a free, named map of the original's structures. Treat all of it as leads to verify against this build. |
| Mod loading options | [managed] BepInEx 5 (Unity Mono, matching bitness), BepInEx 6 / MelonLoader (IL2CPP), the game's built-in mod system. [native] an official plugin/DLL extension point, a proxy DLL, an ASI loader, or a launcher that injects (section 7.2). Check whether the community already standardized on one. |

---

## 3. Toolbox

Check what is already installed before installing anything (`Get-Command`, `scoop list`, `winget list`). **Verify package IDs with `scoop search` / `winget search` at install time**; the IDs below were correct when this was written, not necessarily now. `scoop` needs `scoop bucket add extras` for most of these.

| Need | Tool | Install | Notes |
|---|---|---|---|
| Identify packer/compiler/bitness | Detect It Easy | `scoop install extras/detect-it-easy` or `winget install horsicq.DIE-engine` | Has a console version (`diec`). |
| Disassemble / decompile native code | Ghidra | `scoop install extras/ghidra` (needs a JDK; read its README for the required version) | **Use headless mode** (`support\analyzeHeadless.bat <projDir> <projName> -import <exe> -postScript <script>`): scripts export function lists, decompiled C, strings, xrefs and RTTI-recovered class names as text files into a gitignored `re/` folder you can grep. Re-run with `-process` instead of `-import` after the first analysis. |
| Second opinion / quick CLI disassembly | rizin | `scoop install rizin` | Fast for "what is at this address", imports, strings, sections. |
| Decompile .NET | ilspycmd | already installed (`dotnet tool`) | Section 4.1. |
| Unity assets | AssetRipper | `S:\games\software\AssetRipper_win_x64\AssetRipper.GUI.Free.exe` | Section 4.2. |
| Godot projects | GDRE Tools | `S:\games\software\GDRE_tools-*.zip` | Recovers project, scripts and scenes. |
| User-mode debugger (GUI) | x64dbg (`x32dbg.exe` for 32-bit targets, `x64dbg.exe` for 64-bit) | `scoop install extras/x64dbg` or `winget install x64dbg.x64dbg` | Best for interactive sessions *with the user*. Supports script files and a command line, so breakpoint setups can be saved. |
| User-mode debugger (scriptable) | `cdb` / WinDbg | `winget install Microsoft.WinDbg`; `cdb.exe` ships with the Windows SDK "Debugging Tools for Windows" feature | `cdb -p <pid> -cf commands.txt` runs a command file and logs to text: breakpoints with actions, stack traces, memory dumps, all unattended. Use the x86 cdb for 32-bit targets. |
| Dynamic instrumentation | Frida | `pip install frida-tools` | The workhorse for an agent: `frida -f game.exe -l script.js` or attach by PID. Hook any function by address or export from JavaScript, log arguments and return values, read and write memory, call game functions, all without compiling anything. `frida-trace -i "TextOut*" -i "CreateFile*"` answers "which APIs does it use" in one line. Prototype every native hook here before writing C++. Verify it can attach to the target's bitness. |
| Memory scanning (GUI) | Cheat Engine | installer at `S:\games\software\CheatEngine751P.exe` (**ask first**, section 1) | Value scans, "find out what accesses this address", pointer scans, structure dissect. GUI-only and hard to use with a screen reader, so prefer the scripted route below and keep this for sessions where it is the only thing that works. |
| Memory scanning (scripted) | Python + `pymem` (or raw `ReadProcessMemory` via `ctypes`), or Frida's `Memory.scan` | `pip install pymem` | A 64-bit Python can read a 32-bit process. Write scan, narrow, watch loops as scripts in `tools/` (section 5.4). |
| File/registry/API tracing | Process Monitor, API Monitor | `winget install Microsoft.Sysinternals.ProcessMonitor` | Which files does the game open on this screen? Where are saves and config? ProcMon needs admin (ask first); Frida file-API hooks are the no-admin substitute. |
| Hex editor / binary templates | ImHex, HxD | `scoop install extras/imhex`, `extras/hxd` | ImHex pattern language doubles as format documentation. For an agent, a Python `struct` parser committed to `tools/` is usually better. |
| Graphics debugging | RenderDoc (D3D11+/GL/Vulkan), apitrace (older APIs) | `winget install BaldurKarlsson.RenderDoc` | Only when text or UI exists nowhere but in draw calls. |
| Build native mod code | A toolchain that targets the **game's** bitness | Check `gcc -dumpmachine`, `rustup target list --installed`, or MSVC Build Tools | A MinGW x64 install cannot emit 32-bit DLLs unless it is a multilib build. Rust: `i686-pc-windows-msvc` / `x86_64-pc-windows-msvc`. Decide once, record in `CLAUDE.md`. |
| Hooking library [native] | MinHook, SafetyHook, Microsoft Detours | vendored into the repo | Section 9.2. |

---

## 4. Phase 2: Static extraction

Everything extracted goes into gitignored folders (`game/` for decompiled managed code, `re/` for Ghidra output, an export directory outside the repo for assets). It is copyrighted reference material, never compiled into or shipped with the mod.

### 4.1 [managed] Code: ILSpy

```bash
ilspycmd -p --nested-directories -r "<Game>_Data\Managed" -o game "<Game>_Data\Managed\Assembly-CSharp.dll"
```

Also decompile `Assembly-CSharp-firstpass.dll` and any game-specific assemblies (ignore `UnityEngine.*`, `System.*`, third-party libs unless you need them). What matters is that the result is greppable text on disk. For Java, use a decompiler such as CFR or Vineflower the same way.

**IL2CPP**: you get type and method *signatures* (Cpp2IL / Il2CppDumper) but not bodies. Treat the game as native with an excellent symbol file: signatures from the dump, behavior from Ghidra (with the dumper's Ghidra script applied) and from runtime observation.

### 4.2 [Unity] Assets: AssetRipper

`S:\games\software\AssetRipper_win_x64\AssetRipper.GUI.Free.exe` starts a local web server and opens a browser UI. Load the game folder, then export all files to a directory outside the repo. Recent versions accept `--port <n>` and `--launch-browser false` and expose the same actions as HTTP form posts (`/LoadFolder`, `/Export/UnityProject`, each with a `path` field), which allows driving it headlessly. Verify this against the running instance; if it does not work, ask the user to click through the GUI once. It writes timestamped logs next to the exe.

Mine from the export: scene and screen prefab hierarchies (which component sits where, what serialized fields point at), localization tables, ScriptableObject/TextAsset definitions, input assets (default bindings), sprite/icon names used inline in text.

### 4.3 [native] Code: disassembly and decompilation

Work outward from things you can name toward things you cannot:

1. **Imports and exports.** The import table is the game's list of OS capabilities: which text, input, file, audio and window APIs it uses. Exports of the game's own DLLs are named entry points.
2. **Strings.** Extract all strings (ASCII and UTF-16) with addresses. Error messages, assert text with source paths, format strings, file names, UI string IDs, debug-console commands. Cross-reference each interesting string to the functions that use it: that names those functions.
3. **RTTI and vtables.** C++ games built with RTTI enabled carry their class names and inheritance hierarchy in the binary. Run an RTTI recovery script in Ghidra, export the class list and each vtable's function addresses. This is often the single biggest win: it turns "thousands of anonymous functions" into "the virtual methods of `SomeWindow`, `SomeObject`, `SomeManager`".
4. **Library identification.** Recognize statically linked libraries (the C runtime, zlib, a Lua VM, an audio middleware, MFC) via Ghidra's Function ID / signatures so you do not reverse engineer code that has public source.
5. **Name as you go.** Keep a `re/names.csv` (address, name, confidence, evidence) and a Ghidra script that applies it, so a fresh headless analysis regains all your naming. Export decompiled C for named functions to `re/decomp/` for grepping.
6. **Calling conventions.** On x86, note per function: `cdecl`, `stdcall`, `thiscall` (`this` in ECX), `fastcall`, or a compiler-specific register convention produced by link-time optimization. A hook with the wrong convention corrupts the stack a few calls later, far from the cause. x64 has one convention; enjoy it.

Static analysis answers "what does this function do". It is slow at "which function handles X". For that, switch to section 5 and come back with an address.

### 4.4 Data files

- Identify each proprietary container by magic bytes and structure; check community documentation; write a **Python extractor in `tools/`** and verify it round-trips or at least extracts files that open correctly.
- Find: string tables per language (the mod needs the same strings the game shows), object/item/character definitions, UI layout files (custom toolkits often describe screens in data: a free screen inventory), scripts/bytecode, icon and sprite sheets with names, default key bindings.
- Game logic stored as data (scripts, behavior trees, rule tables) should be dumped to readable text. It explains what the player-facing mechanics are and which exe primitives they call.

### 4.5 Build a map of the game

Locate and document in `CLAUDE.md` the following. Nearly every game has an equivalent of each; genre-specific additions are in section 13.

- The **main loop** and a per-frame function safe to hook for running mod code on the game's main thread.
- The **screen / UI manager**: the registry of screens or windows; their base class; how they open, close and stack; how the game decides which one receives input; where modal dialogs come from.
- The **UI widget model**: base widget class, how children are enumerated, where label text, enabled/visible/hovered/pressed state and screen rectangles are stored, how a click is dispatched to a handler.
- The **managers and global state**: save/profile, the simulation or match state, the player-controlled entity or entities, resources/currencies, time and game speed, the object/entity list.
- **State vs definition classes**: runtime instances versus templates/catalog entries. Find the name, description and stat accessors on each.
- The **event surface**: the functions through which things *happen* (damage, death, spawn, state change, need/meter thresholds, notifications, dialogue, item gained, turn/phase/day change).
- **Preview/simulation modes**: games that preview outcomes by running real logic with a flag set. Find that flag now; it is the source of phantom announcements later.
- The **tooltip / hover system**: how hover text is assembled. Usually the best source of explanations.
- The **selection, targeting and command flow**: how the player picks something and tells the game what to do with it, down to the function that takes (actor, action, target) with no mouse coordinates involved. That function is what the mod will call.
- The **camera and world-to-screen projection**, if there is a world view.

---

## 5. Phase 3: Dynamic analysis

### 5.1 Running the game

You launch the game yourself; you do not need the user to do it.

- **Get it windowed first.** Look for a windowed option in settings, config files or command-line switches. If the game only supports exclusive fullscreen, a wrapper that forces windowed mode (dgVoodoo2, DxWnd, cnc-ddraw, depending on the graphics API) is worth setting up before any debugging, with the user's agreement: a breakpoint hit under exclusive fullscreen leaves a frozen, unusable desktop. Old games may also need compatibility settings to start at all on modern Windows; record exactly what was needed, because players will need the same.
- **Launch and track**: `Start-Process` with the working directory set to the game folder (old games often assume it), keep the PID, wait for the main window, kill by PID when finished. Give every automated run a timeout.
- **Screenshots are your eyes.** Capture the game window (PowerShell `System.Drawing` `CopyFromScreen`, or Python `mss`/`PIL.ImageGrab`) and read the image. This is how you learn what a sighted player sees on a screen, confirm the inventory, and check that what the mod says matches what is displayed. The user may not be able to check the screen for you; do not ask them to describe it.
- **Driving input**: synthesize input with `SendInput` (scancodes, not just virtual keys; DirectInput and raw-input games ignore posted window messages). Once Frida or your mod is inside, calling the game's own functions is more reliable than synthetic input.
- **Logs**: find the game's own log and crash dump locations. Launch once with every debug/log/console command-line switch you found in the strings.
- **Saves**: copy the save folder before experiments. Build a small library of saves that land directly on hard-to-reach screens (late game, rare events, failure states) so verification does not require hours of play. Ask the user to help produce them if playing blind through unmodded parts is impossible.

### 5.2 Finding things at runtime

The core loop of native reverse engineering: **make something happen in the game, catch the code that made it happen, read backwards from there.**

| Question | Technique |
|---|---|
| Which function draws/produces this text? | Hook the text APIs the game imports (or the glyph-blit routine found statically) with Frida, log the string plus a backtrace (`Thread.backtrace`). The frames just above the draw call are the UI code that owned the string. |
| Which function runs when I click this button / press this key? | Hook the input entry point (window procedure, DirectInput `GetDeviceState`/`GetDeviceData`, the engine's dispatcher), or break on it in the debugger, then step/trace forward. Alternatively log every call to every virtual method of the widget class (addresses from RTTI) and see which fire on the click. |
| Where is this number stored (money, health, a meter, the clock)? | Memory scan: search for the value, change it in game, narrow, repeat (5.4). Then a hardware **write** breakpoint on the address to find the code that updates it, and a **read** breakpoint to find the UI code that displays it. |
| What is the layout of this object? | From a known field address, find the object base (the accessing instruction's base register), dump a few hundred bytes across several instances and several moments, and diff. The first pointer in a C++ object is its vtable: match it against the RTTI export to learn the class. Record in `STRUCTS.md`. |
| How do I find this object again next launch? | Addresses change between runs (heap, ASLR). Find a **static root**: a global pointer in the exe's data section, reached by following "what accesses this" upward or by a pointer scan. Express everything as `module base + RVA`, then pointer chain, never as absolute addresses. |
| Which files does this screen load? | Frida hooks on `CreateFileW`/`ReadFile` (or ProcMon), correlated with what you just did in game. |
| Is this function what I think it is? | Call it. From Frida (`new NativeFunction(addr, ret, args, abi)`) on the game's main thread, with arguments captured from a real call. If the game does the thing, the hypothesis is confirmed. |
| The exe is packed; Ghidra shows nothing | Let it unpack itself in memory, dump the module from the running process, and analyze the dump. |

### 5.3 Debuggers

- Attach by PID after the game reaches the screen of interest, rather than launching under the debugger, unless you need startup code. Check for anti-debug reactions once (does the game exit or misbehave when attached?); if it does, record it and rely on Frida or memory reads instead.
- **Software breakpoints** for "does this code run, with what arguments, called from where". **Hardware breakpoints** (four available) for "who reads/writes this address". Conditional and logging breakpoints (log and continue) keep the game playable while you collect data.
- For unattended use, write `cdb` command files or Frida scripts that log and continue, run the game for a bounded time, and read the log. Save them in `tools/` with a comment stating which build hash they are valid for.
- Timing-sensitive games (audio, network, watchdog timers) misbehave when paused. Prefer logging hooks over stopping breakpoints there.
- An access violation inside your hook looks like a game crash. Reproduce with the mod removed before blaming the game; enable crash dumps or run under `cdb -g -G` to get a stack.

### 5.4 RAM: scanning and watching

A scripted scan is a few dozen lines: enumerate committed, readable regions (`VirtualQueryEx`), read each, search for the encoded value, keep the hit list; after the value changes in game, re-read only the hits and filter. Things that defeat a naive scan:

- The value is stored as a different type than displayed (float or fixed-point for an integer-looking meter, scaled by 10 or 100, a 16-bit field, a percentage of a max stored elsewhere).
- The displayed value is a **copy** for the UI; the authoritative value lives in the simulation object. Writing to the copy does nothing; find both.
- The value is computed on the fly from others and stored nowhere.
- Use "changed / unchanged / increased / decreased" scans when you do not know the encoding.

**Writing memory** is for experiments (force a rare state to reach a screen, prove that an address is authoritative). The shipped mod should call game functions rather than poke values, so that the game's own validation, sounds and side effects run.

Record in `STRUCTS.md`, per structure: class name (RTTI or yours), how to reach an instance from a static root, field offsets with type, meaning, **evidence** (how you verified it) and the build hash it applies to.

---

## 6. Phase 4: Screen inventory (mandatory)

Create `SCREENS.md` and enumerate **every** screen, window, overlay and modal. Use independent sources, because each one misses things:

1. **Code**: the screen enum/registry, every subclass of the screen/window base class (decompiled source or the RTTI class list), every class named like `*Screen`, `*Dialog`, `*Popup`, `*Panel`, `*Window`, `*Overlay`, `*UI`.
2. **Assets/data**: scene lists, screen prefabs, UI layout files, dialog resources in the exe.
3. **Runtime**: a temporary logging hook on the screen manager's open/show function, then play: start a game, succeed, fail, open every menu. Log every screen that appears, and **screenshot each one**.

Beware of "screens" that are not screens: a results panel may be a component of the gameplay screen rather than a separate entry. Your state tracking must handle those explicitly. Also beware of things outside the game process: launchers, configuration tools, first-run dialogs, which need their own accessibility check (they are often standard Win32 and already readable).

One row per screen:

| Screen | Class / address | Entry function (verified) | Focus model | Info a sighted player sees | Hover-only / spatial-only info | Status |
|---|---|---|---|---|---|---|
| … | … | … | tier A / B / C, plus any mouse-only interactions on it (drag, right-click, hover, world clicks) | … | … | not started / announced / navigable / verified |

Status definitions:

- **announced**: opening the screen says its name and a one-line orientation.
- **navigable**: every interactive element can be reached by keyboard and reads correctly on focus; every piece of visible information is reachable (focus, hotkey, buffer or read-all).
- **verified**: tested in the running game with a screen reader, including the edge cases (empty lists, locked items, disabled buttons, not enough currency, interrupted actions).

Prompts to make sure nothing is forgotten (a checklist of *questions*, not the inventory; the inventory comes from the game): launcher and first-run setup, intro videos and how to skip them, main menu, profile/save/load slots, new-game setup (every choice and creator/editor screen), tutorial, the main gameplay view and **each of its modes** (live, build/edit, buy/shop, map, inventory, character/status panels, relationship/skill/quest/journal panels), context and radial menus, targeting and placement modes, queues and notifications, time and speed controls, random events and their choice dialogs, conversations, shops, crafting/upgrading, success and failure summaries, unlocks and meta progression, encyclopedia/collection, statistics, options (every tab, including key rebinding), pause menu, confirmation dialogs, error dialogs, credits, expansion/DLC/news panels, multiplayer lobbies.

**The mod is not done while any row is below "verified".**

---

## 7. Phase 5: Mod architecture and scaffold

The architecture is the same for every engine; only the bottom layer (how code gets into the process and how it reaches game objects) changes.

```
<Game>Accessibility/
├── Core/              # Speech wrapper, input, config, keywords, buffers, focus helpers
├── Core/Buffers/      # AnnouncementBuffer, BufferManager, FocusReadout, buffer sets
├── Game/              # The ONLY layer that knows addresses/offsets/reflection names: typed accessors over game objects
├── Gameplay/          # State cache, readers for the game's domains, targeting/commands, review cursors (section 13)
├── Navigation/        # Tier B/C only: item collection, focus + hover sync, activation, input repeat
├── Screens/           # Per-screen handlers or coordinators (10.4)
├── Screens/Readers/   # One text extractor per UI domain
├── Patches/ (Hooks/)  # One file per hooked target: screen transitions, game events, input suppression
├── Help/              # Context-sensitive help + screen state tracker
├── Utilities/         # Text cleanup, localization, logging
├── SpeechHost/        # Only if the game is 32-bit: the x64 process that owns Prism (8.3)
└── tools/             # Frida scripts, Ghidra scripts, extractors, memory scanners used during RE. Not shipped.
```

Startup order, whatever the engine: logging, build check (9.2), config, speech, keyword dictionary, buffers, install hooks, create handlers, register help contexts. Expose `LogInfo/LogWarning/LogError` helpers usable from everywhere, writing to a file next to the mod.

### 7.1 [managed] BepInEx / Harmony

`csproj` essentials (BepInEx 5 / Unity Mono):

```xml
<PropertyGroup>
  <TargetFramework>netstandard2.0</TargetFramework>  <!-- or net35/net46 for old Unity; match the game's Mono profile -->
  <LangVersion>8.0</LangVersion>
  <GamePath Condition="'$(GAME_PATH)' != ''">$(GAME_PATH)</GamePath>
  <GamePath Condition="'$(GamePath)' == ''">DEFAULT INSTALL PATH</GamePath>
</PropertyGroup>

<!-- Reference from the game folder; never copy these to output -->
<Reference Include="BepInEx"><HintPath>$(GamePath)\BepInEx\core\BepInEx.dll</HintPath><Private>false</Private></Reference>
<Reference Include="0Harmony"><HintPath>$(GamePath)\BepInEx\core\0Harmony.dll</HintPath><Private>false</Private></Reference>
<!-- plus the engine modules in use and the game assembly -->

<!-- Deploy on build so the test loop is: build, launch -->
<Target Name="CopyToPlugins" AfterTargets="Build">
  <Copy SourceFiles="$(OutputPath)$(AssemblyName).dll" DestinationFolder="$(GamePath)\BepInEx\plugins\" />
  <Copy SourceFiles="PATH\TO\prism.dll" DestinationFolder="$(GamePath)\BepInEx\plugins\" />
</Target>
```

Host your per-frame code on persistent (`DontDestroyOnLoad`) GameObjects. Even though you reference the game assembly, do most game access through **cached reflection** and patch by name (9.1). Game updates rename and move things. Reflection degrades to a logged warning; a hard compile-time reference degrades to a plugin that fails to load at all.

### 7.2 [native] Getting into the process

In order of preference:

1. **An official extension point** (plugin folder, script mods, a documented DLL interface). Rare in old games; check anyway.
2. **Proxy DLL.** Pick a system DLL the game imports that is *not* on the OS KnownDLLs list (commonly one of `dinput8`, `dinput`, `ddraw`, `winmm`, `version`, `dsound`; read the import table, do not guess). Your DLL takes that name, sits next to the exe, forwards every export to the real one in the system directory, and starts the mod from `DllMain` (do almost nothing *in* `DllMain`: create a thread or, better, defer to the first hooked call on the main thread). Installation for the player is "copy files into the game folder", which is what you want. A community **ASI loader** is a ready-made version of this.
3. **A launcher** that starts the game suspended and injects the mod DLL. More moving parts, more antivirus false positives; use only when no proxy candidate exists.

Rules for native mod code:

- **Match the game's bitness and do not assume its C runtime.** Never pass CRT objects (`FILE*`, `std::string`, heap blocks to free) across the boundary between your DLL and the game; allocate and free game objects with the game's own allocator functions.
- **Run on the game's main thread.** Game data structures are not thread-safe. Hook a per-frame function (main loop tick, message pump, present/flip) and do all reading, navigation and game calls from there. Background threads only for the speech pipe and file I/O.
- **Isolate every address and offset in `Game/`.** The rest of the mod sees typed accessors (`Widget::Label()`, `World::Objects()`), never raw pointer arithmetic. When an offset turns out wrong, there is one place to fix.
- **Validate pointers at the boundary.** Null-check, vtable-check (is this pointer's first word one of the vtables I expect?) and, in debug builds, wrap reads in SEH or `IsBadReadPtr`-style probes. A wrong offset should produce a log line, not a crash.
- **Crash handler**: install an unhandled-exception filter that writes the faulting address, module-relative RVA and a minidump next to the mod log. Blind players cannot read a crash dialog's details to you.
- Prototype every hook in Frida first (5.2); port to compiled code once the target, signature and convention are confirmed.

---

## 8. Speech: Prism

Prism is a C library that abstracts screen readers (NVDA, JAWS, and others) and TTS engines (SAPI, OneCore, …) behind one API and picks the best available backend. It lives at:

```
C:\Users\Nitropc\code\libs\prism\
├── prism-windows-x64\include\prism.h                ← the API. Read it; it is the ground truth.
├── prism-windows-x64\dynamic\release\bin\prism.dll  ← ship this with the mod
└── prism-sdk-v0.18.2\doc\index.html                 ← full manual (conventions, thread safety, backend notes)
```

Two packages are present (`prism-windows-x64` and `prism-sdk-v0.18.2`). Take the header and the DLL **from the same package**. The `tolk.dll` beside it is a Tolk-compatibility shim and is not needed. Windows binaries exist for **x64 and arm64 only** (see 8.3 for 32-bit games). Facts from the header and manual that matter:

- Calling convention is **cdecl**. Strings are **null-terminated UTF-8**; the caller owns them and they only need to outlive the call.
- C `bool` is **1 byte**.
- `prism_init(NULL)` is valid and uses the global backend registry.
- `prism_registry_create_best(ctx)` returns the highest-priority backend that initializes, **already initialized**; do not call `prism_backend_initialize` on it. The manual recommends `create_best` over `acquire_best` unless you need to share backend state.
- Pointers returned by the library (backend name, error strings) are library-owned: **never free them**. The context and the backend are the two things you *do* release (`prism_backend_free`, then `prism_shutdown`).
- **A backend instance is not thread-safe.** Call it from one thread only.
- `prism_backend_output` = speech + braille where the backend supports both. Check `prism_backend_get_features` for `PRISM_BACKEND_SUPPORTS_OUTPUT` (bit 5) / `SPEAK` (bit 2) / `BRAILLE` (bit 4) / `STOP` (bit 7).
- Most functions return `PrismError`; `0` is `PRISM_OK`. `prism_error_string` gives readable text for the log.

### 8.1 The wrapper (`ScreenReaderOutput`), any engine

- **Initialize**: load `prism.dll` by full path (mod folders are not on the DLL search path), `prism_init`, `prism_registry_create_best`. Log the backend name. Announce "`<Mod>` loaded" so the player knows speech works. Handle "DLL missing" and "wrong bitness" with a clear log message.
- **Speak(text, interrupt = false)**: bail on empty text or missing backend; run the text through the cleanup pipeline (section 14); call `prism_backend_output` if supported, otherwise `prism_backend_speak` (+ `prism_backend_braille` if enabled in config). On a non-OK result, log `prism_error_string`, and (rate-limited) free the backend and `create_best` again. That is how the mod recovers when the player starts or restarts their screen reader after the game.
- **One thread**: if any hook can fire off the main thread, enqueue and drain from the per-frame function.
- **Log every spoken line verbatim** ("Spoke: …") behind a debug category. A report of "it read this wrong" is undiagnosable if the log has nothing to match it against.
- **Two entry points**: `Say` for focus echoes and on-demand readouts, `SayEvent`/`LogEvent` for unsolicited announcements (game events, story, popups, screen changes). Only the second is recorded in the Events buffer; recording focus echoes would bury the events under navigation chatter.
- **LogEvent(text)**: append to the Events buffer and to a plain-text event log file next to the mod (overwritten each launch). The file is invaluable for bug reports from blind players.
- **Shutdown**: stop, `prism_backend_free`, `prism_shutdown`.
- Some mod loaders try to load *every* DLL in a mod folder as a managed assembly and choke on a native one. In that case ship `prism.dll` elsewhere (the game root, for example) and load it by full path.

### 8.2 [managed] P/Invoke layer

There is no C# binding. Marshal C `bool` as `UnmanagedType.I1` (the default P/Invoke `bool` is a 4-byte Win32 `BOOL` and will misbehave).

```csharp
internal static class PrismNative
{
    private const string Dll = "prism";

    [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
    internal static extern IntPtr LoadLibrary(string path);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern IntPtr prism_init(IntPtr cfg);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern void prism_shutdown(IntPtr ctx);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern IntPtr prism_registry_create_best(IntPtr ctx);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern void prism_backend_free(IntPtr backend);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern IntPtr prism_backend_name(IntPtr backend);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern ulong prism_backend_get_features(IntPtr backend);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern int prism_backend_speak(IntPtr backend, byte[] utf8, [MarshalAs(UnmanagedType.I1)] bool interrupt);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern int prism_backend_output(IntPtr backend, byte[] utf8, [MarshalAs(UnmanagedType.I1)] bool interrupt);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern int prism_backend_braille(IntPtr backend, byte[] utf8);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern int prism_backend_stop(IntPtr backend);
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)] internal static extern IntPtr prism_error_string(int error);

    // Old Unity Mono runtimes do not reliably support UnmanagedType.LPUTF8Str; encode by hand.
    internal static byte[] Utf8Z(string s)
    {
        var bytes = new byte[Encoding.UTF8.GetByteCount(s) + 1];
        Encoding.UTF8.GetBytes(s, 0, s.Length, bytes, 0);
        return bytes; // last byte is already 0
    }
}
```

### 8.3 [native] Direct use, and the bridge for 32-bit games

A 64-bit native mod includes `prism.h` and links or `LoadLibrary`s `prism.dll` directly. Convert the game's strings (often a legacy code page or UTF-16, *find out which*) to UTF-8 before the call.

A **32-bit game cannot load the x64 `prism.dll`.** Keep rule 3 anyway by moving Prism out of process:

- `SpeechHost.exe`: a small **x64** console-less program that owns Prism (the 8.1 wrapper lives here), listens on a named pipe (`\\.\pipe\<Mod>Speech-<gamePid>`), and reads newline-delimited UTF-8 messages: `say <0|1 interrupt> <text>`, `braille <text>`, `stop`, `quit`.
- The in-game 32-bit DLL starts the host (full path, next to the mod), connects, and sends from a background thread fed by a lock-free or mutex-guarded queue, so a slow or dead host can never stall the game's main thread.
- The host exits when the pipe closes or the game PID disappears (pass the PID on the command line and wait on the process handle). The mod restarts it, rate-limited, if it dies.
- Everything else in 8.1 (queueing, logging every line, recovery when the screen reader restarts) applies unchanged, inside the host.

If building Prism from source for x86 turns out to be practical, that replaces the bridge; check with the user before spending time there.

---

## 9. Hooking pattern

Universal rules, then the two mechanics.

- **Every hook body is fault-isolated.** An exception escaping a hook breaks the game function it is attached to.
- **Every hook installation logs success or failure.** After each launch, grep the log for failures. A hook that silently did not apply looks exactly like a feature that does not work.
- **Record each verified target in `PATCH_TARGETS.md`**: type and method (or RVA, signature bytes and calling convention), parameters, what the hook uses it for, the build it was verified on.
- **Order of execution**: when game function A synchronously calls hooked function B, B's post-hook runs *before* A's. If you need state "before A changed it", capture it on entry.
- **Async/coroutine/state-machine functions**: hooking the function catches the *kickoff*, not the completion. Know which one you are getting.
- Hooks do the minimum: capture data, set a flag or enqueue an event. Heavy work (text building, speech) happens in the per-frame update.

### 9.1 [managed] Harmony, manual patching by name

No `[HarmonyPatch]` attributes. One static class per target with a `TryPatch(Harmony)` that can fail without taking the plugin down:

```csharp
public static class SomeScreenPatch
{
    public static void TryPatch(Harmony harmony)
    {
        try
        {
            var type = AccessTools.TypeByName("SomeScreen");            // verified in game/
            var method = type != null ? AccessTools.Method(type, "Initialize") : null;
            if (method == null)
            {
                Plugin.LogWarning("SomeScreen.Initialize not found - screen will not be announced");
                return;
            }
            harmony.Patch(method, postfix: new HarmonyMethod(typeof(SomeScreenPatch).GetMethod(nameof(Postfix))));
            Plugin.LogInfo("Patched SomeScreen.Initialize");
        }
        catch (Exception ex) { Plugin.LogError($"Failed to patch SomeScreen: {ex.Message}"); }
    }

    public static void Postfix(object __instance)
    {
        try
        {
            ScreenStateTracker.SetScreen(GameScreen.Some);
            // read orientation info from __instance / managers, then Speak(..., false)
        }
        catch (Exception ex) { Plugin.LogError($"Error in SomeScreen patch: {ex.Message}"); }
    }
}
```

Use positional args (`__0`, `__1`), `__instance`, `__result`, and `__state` to pass data from prefix to postfix. `AccessTools.Method(type, name)` throws on ambiguous overloads; pass parameter types. Cache every `MethodInfo`/`FieldInfo`; uncached reflection in a per-frame path causes visible stutter.

### 9.2 [native] Detours by verified address

- **Build gate first.** At startup, hash the exe (and any game DLL you hook) and compare against the builds you support. On an unknown build: install **no** hooks, say "`<Mod>`: unsupported game version" through speech, log the hash, and let the game run unmodified. Hooking the right RVA in the wrong build is a guaranteed crash.
- **Targets are `module base + RVA`**, never absolute addresses. For resilience across minor builds, additionally store a **byte signature** (with wildcards over relocated operands) for each target; verify the bytes at the RVA match before hooking, and fall back to a signature scan if they do not.
- One `TryHook(name, rva, signature, detour, &original)` helper that checks, installs, and logs. Same fail-soft behavior as the managed pattern: a missing target disables one feature, not the mod.
- **Get the calling convention exactly right** (4.3). For x86 `thiscall` targets, write the detour as `fastcall` with a dummy second parameter (`this` in ECX, unused EDX), or use the hooking library's explicit convention support. Test each hook by logging its arguments against values you already know from the debugger.
- Prefer hooking **virtual methods via the function address**, not by overwriting vtable slots, unless you need per-class granularity; if you do patch a vtable, remember subclasses have their own copy.
- Calling game functions from the mod uses the same verified address table and typed function pointers declared once in `Game/`.
- Mid-function hooks and byte patches are a last resort; document the original bytes and why a function-level hook was not possible.

---

## 10. Reading focus and text

### 10.1 Focus polling

If the game has a keyboard focus concept, poll it every frame and, when it changes, extract text and speak it. Polling (rather than hooking every widget) catches focus changes from every source: keys, mouse, and game code. In uGUI this is `EventSystem.current.currentSelectedGameObject`; in a custom toolkit it is whatever "focused widget" or "hovered widget" pointer the UI manager holds.

If the game tracks some selections **outside** the UI focus system (the selected unit, the active character, the selected inventory slot, the selected map node), poll that game state too. Do not rely on key detection alone: the game changes selection on its own.

### 10.2 The extraction chain

`GetTextForElement(element)` tries **domain readers in priority order**, most specific first, and falls through:

1. Special cases that would otherwise be misread (scrollbars, dialog buttons, opening screens)
2. Rich game objects (whatever this game's equivalents of items, characters, units, cards, catalog entries are)
3. Map/world nodes, settings controls (dropdown / slider / toggle, *with current value and state*), generic toggles
4. Encyclopedia / collection / stats entries
5. New-game setup elements (pickers, *locked* state and unlock requirement)
6. Tooltip-only elements, dialogue or event choices
7. `GetTextWithContext()`: generic label handling
8. A cleaned internal name: last resort, so that *something* is always spoken

To support a new UI element: add a reader and insert it at the right priority. One reader per UI domain.

`GetTextWithContext()` heuristics that generalize well: text of 1-2 characters is probably an icon glyph, so use the cleaned element name instead; 3-4 characters or empty means look up the hierarchy for a label, **skipping container names** (container, panel, holder, group, content, root, layout, wrapper, …).

**Icon-only buttons** are the norm in older and mouse-first games: the widget has an image ID and no text at all. Sources for a label, in order: the widget's tooltip string; the string the game shows in a status/help line on hover; a mapping from the handler function or image ID to a name, which you write by hand and verify with a screenshot of each one. Keep that mapping in one data file, localizable.

### 10.3 Traps in UI text extraction

- **The focused element is usually a child.** The component that knows what the thing *is* sits on an ancestor. Always search self **and parents**.
- **References point down.** A parent referencing a child button will not be found by searching up from the parent.
- **Displayed text can differ from stored text.** [Unity] If the game sets text through `TMP_Text.SetText(...)`, the rendered buffer updates but the `text` property may still hold the **prefab's design-time placeholder**. Prefer `GetParsedText()` with a `.text` fallback for focused items (it is one frame stale for labels set this frame), and rendered text with **no** fallback for whole-screen dumps. [native] The widget may hold a string ID, a format string plus arguments, or text in a legacy code page; find the function that produces the final string and call or hook that.
- **Hidden things keep their text.** Check enabled/visible state on the element *and its ancestors* before trusting it.
- **Placeholder filter**: keep an `IsPlaceholderText()` for design-time junk ("New Text", "Lorem", "12345", …) found in this game.
- **State is part of the text**: selected/unselected, on/off, locked (and why), disabled, affordable/unaffordable, "3 of 7". Sighted players get these from color, icon frames and position; you must say them.
- **Hooked draw calls have no structure.** If the fallback of hooking text rendering is ever used, you get strings with screen coordinates in draw order, redrawn every frame. Deduplicate per frame, sort by position, and treat the result as a "read screen" source only, never as focus.

### 10.4 Writing a keyboard navigation layer (tier B and C games)

Some games cannot be made accessible by *reading* focus, because there is no keyboard focus to read: the game is played with a mouse (hover, click, right-click, drag and drop, clicking in a world view). Then the mod must **own navigation**: decide what is focusable, move focus with the arrows, keep the game's own pointer state consistent with that focus, and activate things the way a mouse would. This is the largest single piece of work in such a mod, and every item below comes from a bug that shipped at least once. API names are Unity's where an example is needed; the behavior is universal.

#### Reuse the game's navigation system if one exists (tier B)

If the game supports a controller, it almost certainly has its own registry of navigable items (grouped in layers for popups, with a "current item" and a "set current item" method). **Drive that system instead of building a parallel one**:

- The game already maintains the item list, enables and disables items as panels open, and scopes them to the active layer/popup. Your `GetItems()` is "the game's available items, filtered and sorted".
- Setting the game's current item gives you its highlight visuals, scrolling-into-view and sounds for free, which also keeps the mod usable by low-vision players and testable by sighted ones.
- The game may only process its navigation in **controller mode** and clear the current item whenever it thinks the mouse is in use. Find that flag and force controller mode whenever you move focus.
- Watch the game's **active navigation layer**. A change means a popup or panel opened inside the same screen: reset focus tracking, re-announce, and let the handler react.

Only in tier C (nothing to reuse) do you build your own registry: `FocusableItem` (label provider, activate action, optional adjust action, anchor position), `FocusContext` (items and ordering for one screen or modal, stackable for popups), `VirtualFocusManager` (current context, movement, activation). In a custom toolkit, `GetItems()` is a walk of the live widget tree from the active window's root, which is why section 4.5 asks for the widget model. Draw a visible focus indicator if you can.

#### Per-screen handlers with a generic fallback

When the mod owns navigation, organise it as one **handler object per screen** with `OnEnter` / `OnUpdate` / `OnExit`, and a router that picks the active handler every frame **from game state, not from hooks**: the active screen or window; overlays that open on top without changing it (check these first, in stacking order); flags such as "paused".

- One handler may serve several closely related screens or modes.
- **Register a generic fallback handler** for every screen with no dedicated handler: it names the screen and offers default arrow navigation over whatever is focusable. A screen nobody wrote a handler for must degrade to "clumsy", never to "dead keyboard". It also tells you, through the log, which screens still need a real handler.

A `NavigableScreenHandler` base class runs the standard loop each frame:

1. **Announce the screen once the UI has settled**: `TryAnnounceScreen()` returns false to retry while content is still loading. Add a **timeout** that speaks the plain screen name anyway; a precondition that never becomes true must not leave the screen unnamed forever.
2. Detect navigation-layer changes.
3. Ignore input for a short grace period after entry (a few hundred milliseconds), so the key press that opened the screen does not also act inside it.
4. **Route input through a priority chain**: blocking cinematics/overlays, then help or tutorial popups, then inspect/detail views, then inventory-style overlays, then the handler's own `HandleInput()`. Each stage returns true when it owns the keys this frame.
5. Pump any pending refocus request (below).
6. Announce the focused item if it changed.

Virtual hooks keep handlers small: `GetItems()`, `Navigate(dir)`, `Confirm()`, `DefaultFocusItem()`, `GetItemDescription(item)`, `ShouldAnnounceFocus(item)`, `SuppressFocusAnnouncements`, `OnNavigationLayerChanged(layer)`.

Use **real (unscaled) time** for every timer in this layer. Pause menus and game-speed controls stop or stretch game time, and timers built on it simply freeze.

#### Collecting and ordering items

- **Collect**: controls with a real activation or value-change handler (buttons, sliders, toggles, dropdowns, tabs), plus game objects that are meaningful to inspect.
- **Skip**: inactive objects, objects whose parent panel is hidden or non-interactable, decorative elements, and internal containers.
- **Order spatially**, because that is the order a sighted player perceives. Default linear model: Up/Down walks items sorted top to bottom, Left/Right walks items sorted left to right. Give grids real row and column movement, and multi-column pages an explicit column-major or row-major order. Make the sort stable (tie-break on an insertion index) for items that share a position.
- **Never answer a key press with silence.** Nothing focusable: say so ("nothing to navigate here"). At the end of a list: say "first item" / "last item" or repeat the item. A silent arrow key is indistinguishable from a broken mod.
- **Hold-to-repeat**: fire on the first press, then after an initial delay, then at a repeat rate. While Ctrl is held the arrows belong to the review buffers and the real focus must not move.
- **Text-field guard**: while a text input has focus (naming something, a console), all letter hotkeys are inactive.

#### Focus is hover is the armed target

The most dangerous class of bug in a navigation layer: **the item the player heard is not the item the game will act on.** Mouse-driven games act on whatever is *hovered* when the select input fires. If focus moves but the game's hover does not follow, Enter activates something the player never heard: a button behind the open menu, the previously focused object, a leftover tab.

- After every focus move, push the game's hover state onto the focused item. Make this idempotent and call it freely.
- **Unhover the old object first, then hover the new one.** If the game then refuses the new hover (not a legal target), the previous target must not stay armed. Clearing first makes a refused hover **fail closed**.
- If the focused item has nothing to hover, **clear** the stale hover.
- **Clear the UI framework's own "selected/pressed" state** after any simulated pointer-down. [Unity] uGUI marks the pressed `Button` as selected, and Submit then re-clicks that old button on every later Enter, invisibly.
- **A real mouse hovers everything under the cursor**: the object, the slot or tile beneath it *and* the container behind it. If the game's logic reads a hovered slot or container, mirror the focus onto those as well, through the game's own hover methods so its legality gates still apply.
- The game also moves focus on its own. Re-sync the hover when you *describe* the focused item, not only when you move it.
- If the only way to hover is to **move the real mouse cursor** (the game polls cursor position), do it deliberately: warp the cursor to the item's screen rectangle center, and treat "window lost focus" and "user moved the mouse" as events that invalidate the armed target.

#### Activation ladder

Try these in order and use the first that works for that kind of object:

1. **The game's own handler** for the action (the function a click ultimately reaches). Best option: sounds, validation and side effects all run.
2. **A simulated pointer sequence** delivered to the widget: pointer-down now, pointer-up and click **on the next frame**. Many buttons need the full sequence, and same-frame down+up is often ignored.
3. **The object's press/release path**, for objects that only track hover and rely on a global "select" input while hovered. Mirror what the game's own update loop does: set the hovered entity, press now, release next frame, and check on release that the game's own input polling has not already released it, so the action never fires twice.
4. **Synthetic OS input at a screen position** (warp cursor, `SendInput` click). Last resort: it breaks when the window moves, loses focus or scales, and cannot work while the player is alt-tabbed.

**Ask before pressing.** If the game can refuse an action (a tutorial gate, an illegal choice, not enough money), call its validation first and **speak the refusal**, including the game's prompt text when there is one. The game's own refusal feedback is usually a visual shake or a sound with no words.

#### Drag and drop becomes pick up and place

Re-express dragging as a two-step keyboard action built on the game's own drag state:

- **Enter picks up** the focused object (put the game into its real "dragging this entity" state).
- Arrows move across the possible destinations. While something is held, the focus description switches to a **target-aware** form: position, the occupant, and an explicit **"not a valid target"** whenever the game would refuse the drop. Ask the game's own legality check (the same call its release code makes); do not reimplement the rules.
- Focus may still rest on illegal destinations, because browsing them is how the player learns the layout. They must be *labelled*, or the object ends up somewhere the player never chose.
- **Enter places** (run the game's release path against the mirrored hover). **Escape cancels** and returns the object.
- Actions that need no target should execute from anywhere without demanding a destination.

Right-click and hover-to-inspect map to a dedicated **inspect key** that opens the game's own inspect view when it has one (then route input to that view while it is open) or reads full details when it does not. **Context menus and radial menus** that appear at the cursor become plain vertical lists: read the options from the menu object, arrows move, Enter chooses, Escape dismisses; nested levels push and pop.

#### Polled waits and blocking overlays

- **Polled input waits**: cinematics and reveal sequences that spin on "is Select pressed?" where that action is bound only to mouse or gamepad. A keyboard player is hard-stuck there. Fix it with a small simulator: hook the game's single input-poll function so that, for a **two-frame armed window**, it reports the action as pressed. Arm it only from code that knows the game is waiting, **clear all hover state first** (a true "select" also clicks whatever is hovered), and respect the game's own input-disabled flag.
- **Blocking overlays that are not screens** (full-screen sequences layered over the active screen): a watcher detects them cheaply and **owns the keyboard while they run**. Enter answers the prompt; arrows are swallowed so they cannot drive the invisible screen underneath.
- **Videos**: announce "video playing, press X to skip", and make sure X works.

#### Virtual rows

Pages often contain content with **no navigable item at all**: statistics, logs, lists, lore entries. Build virtual rows (spoken text, an optional activate action, an anchor position) and merge them into the navigation order by position, skipping any entry that already has a real item so nothing is read twice.

- While focus sits on a virtual row, **clear the game's hover** so a game-side select cannot click a leftover object.
- Games render tables as a few large text blocks (a names column and a parallel values column). Split the blocks and re-pair the lines into one row per entry.

#### Focus recovery and quiet periods

- Overlays that close often leave the game with **no focus at all**. Provide `RequestRefocus()`: for about a second, wait until the default item is available again (the close animation may still own the layer), then focus it and force a fresh announcement.
- The game moves focus by itself while it resolves actions. Track those changes **silently** (`SuppressFocusAnnouncements`) so focus chatter does not bury event narration.
- Offer `SuppressFocusFor(seconds)` so an important announcement is not immediately talked over by the focus echo that follows it.
- Some items are destinations rather than places to browse; `ShouldAnnounceFocus(item)` lets a handler keep them silent.

#### Controls announce role, value and how to change it

| Control | Announcement | Keys |
|---|---|---|
| Button | label | Enter |
| Slider | label, "slider", value | Left/Right adjust by the game's own step (or 5-10% of the range); speak the new value |
| Toggle | label, "checkbox", checked / unchecked | Enter flips; speak the new state |
| Dropdown / selector | label, current choice | Left/Right cycle; speak the new choice |
| Tabs | label, "tab", selected state | Enter opens the tab's content; a back key returns to the tab list |
| Text field | label, "edit", current text | Enter starts editing; typed characters echo; Enter/Escape finish |

Label resolution order for unlabeled controls: the game's localization component or string ID, then a parent's title or label, then tooltip text, then the hand-written name map (10.2), then the cleaned internal name.

#### Rewrite the game's mouse wording

Tutorials and help popups teach mouse controls ("right click the item", "drag it onto the slot"). Read verbatim, they teach a blind player controls they do not have. Add a text pass that rewrites them into the mod's model: "right click" becomes the inspect key, "click" becomes Enter, "drag X onto Y" becomes "select X and place it on Y" (keep the verb form), and append a one-line keyboard how-to whenever a drag sentence was rewritten. Bound each pattern by sentence punctuation so a stray "to" in a later sentence cannot swallow text, and do it **per supported language**.

---

## 11. Concise focus + review buffers

### 11.1 FocusReadout

Readers for rich objects return three views of the same thing:

```csharp
public class FocusReadout
{
    public string Summary;                            // spoken on focus: name plus the one or two numbers that matter
    public List<string> Details = new List<string>(); // one buffer item each: type, stats, description, each keyword, modifiers, lore
    public string FullText;                           // everything in one string, for "re-read" and non-focus callers
}
```

Decide per object type what belongs in the summary by asking what a sighted player takes in at a glance: name, price and affordability for shop goods; name and current action for a character; name, state and whether it can be used for a world object. Read *current* values from the runtime instance, not the definition. Where an action is unavailable, say why.

**Keyword and mechanic explanations live only in buffer content, never in live announcements.** Do not build "explain it the first time only" tracking; it is unpredictable for the player. Give description builders a `bool includeKeywords` parameter: buffers pass true, hotkey reads and event announcements pass false.

### 11.2 AnnouncementBuffer

A named list of strings with a cursor:

- Items are stored in **review order**: index 0 is the current/top item (the newest event, or the first detail line).
- The cursor starts at **-1** ("before the top"), so the first *next* press reads the top item.
- `MoveDeeper()` (Ctrl+Up) goes to older events / further detail lines; `MoveTowardTop()` (Ctrl+Down) goes back. **No wrap**; at either end, repeat the item or say "top"/"end".
- `Add(item)` inserts at 0, shifts the cursor so a player mid-review stays on the same item, and trims past `maxItems`.
- Optional `Refresher` rebuilds content from game state on demand; returning nothing marks the buffer unavailable. **Reset the cursor only when the content actually changed**; otherwise re-reading the same focus resets the player's position.
- `FollowLatest` (Events buffer): focusing the buffer returns the cursor to the newest item.

### 11.3 BufferManager

Owns the ordered buffer list. Ctrl+Up/Down move within the current buffer; Ctrl+Left/Right switch buffers, **skipping empty ones**, announcing "Name, N items". A good default shape:

- Focus-fed: **UI** (details of whatever generic thing has focus), **Events** (every game event, capped at around 200), one buffer per major object type the game has (**Item**, **Character/Unit**, **Reward**, …), **Story** (dialogue and narrative text).
- Gameplay-only, refresher-driven: the standing state a sighted player reads off the HUD (**Status/Needs**, **Queue**, **Resources**, **Nearby**, or **Hand/Board** for a card game).

**Clear focus-fed content on screen change and on leaving gameplay**, or stale details will be read on the wrong screen.

---

## 12. Input

### 12.1 Reading keys

[managed/engine] One `InputInterceptor` reads keys in the per-frame update. [native] Read keys where the game does: subclass or hook the window procedure for `WM_KEYDOWN`/`WM_KEYUP`/`WM_CHAR` (gives you key repeat and text input for free), or hook the DirectInput/raw-input read and inspect the buffer there. `GetAsyncKeyState` polling from your per-frame hook is the simple fallback, but gate it on the game window being in the foreground or the mod will react to typing in other applications.

Route **by mode, in strict priority order**: help browser open, then Ctrl combos, then targeting/placement mode, then review cursor, then plain hotkeys. Every modal mode your mod adds must claim its keys *before* lower layers see them.

Baseline key set (adapt letters to the game; all rebindable through config):

| Key | Action |
|---|---|
| F1 | Context help for the current screen, as a browsable list |
| one letter | Re-read the focused item (full text, not the concise summary) |
| one letter | Read all text on screen (a structured summary where a raw dump would be noise) |
| one letter each (gameplay) | The standing HUD information: status/meters, resources, current action/queue, time, whatever this game shows permanently |
| Ctrl+letter | Information that must **work on every screen** (money, date/time), so read it from global state, not a per-mode cache |
| Ctrl+arrows | Review buffers (or a review cursor where one exists) |
| one letter | Cycle verbosity |

**Before choosing keys, extract the game's default bindings and list them in `CLAUDE.md`.** Sharing a key with a game action that is inert on most screens is acceptable; never share with one that is disruptive. Do not take keys screen readers need (Insert/CapsLock combos, NumPad when used as the review cursor).

### 12.2 Suppression

When your mod claims a key, the game must not *also* act on it, or the player gets double actions and double announcements. The game's input code usually ignores modifiers, so Ctrl+Up still moves the game's selection.

- Find **every** input path (section 2). Hook each; when a claim is active, return the neutral value and skip the original. [native] In a window-procedure hook, swallow the message; in a DirectInput hook, zero the claimed keys in the returned buffer; for `GetAsyncKeyState`-style polling, hook the import (IAT) for the game module only.
- Suppress only what you claim: arrows while Ctrl is held; arrows + confirm + cancel while a mod modal (help list, review cursor) is open; letters for your Ctrl+letter combos while Ctrl is held. Leave gamepad and non-claimed keys alone.
- **Key off physical key state, not your mod's state flags**, for the press that *opens* a mode. Execution order between the game's input handling and yours is not guaranteed.
- In tier B/C games the conflict runs the other way too: keys you use for navigation may be bound to game actions that fire *alongside* your handling. Check the game's bindings for each one, and either suppress the game's handling or make your own action idempotent with it.
- When a mod modal closes on Escape/Enter, latch a `ClosedThisFrame` flag so the same key press does not also reach the game.
- **Mouse-driven camera/edge scrolling**: if the mod warps the cursor (10.4), the game may scroll the view when the cursor touches a window edge. Clamp warps away from edges or suppress edge scrolling.

### 12.3 "Is this screen really in front?"

Mode gates like "plain arrows move the world cursor during gameplay" need a reliable answer to *is the gameplay view frontmost and interactable right now*. Use **the game's own notion of interactability** (whatever its UI manager uses to decide who receives input). Two designs that fail in practice: a **denylist of overlay names** (you will miss one), and **"top of the active-screen stack"** (hidden-but-still-registered screens can sit on top forever). Your own `ScreenStateTracker` is fed by screen *open* hooks and does not see screens close, so treat it as a fallback and hook close/hide where a stale value would hurt.

---

## 13. Gameplay

Menus are the easy part. This section is about the part of the game where the playing happens. Identify which patterns the target game uses (usually more than one) and build the matching pieces.

### 13.1 Common to every genre

- **StateCache**: locate the game's managers and roots once per session/level, cache handles, and invalidate on exit. Give it an `IsInGameplay` flag driven by verified start/stop hooks, and **confirm the stop hook actually fires**; otherwise gameplay keys stay live on the next menu.
- **Readers** per domain, each returning a `FocusReadout`.
- **Event announcements** for everything a sighted player would notice happening without asking: state changes of the things they control, arrivals and departures, successes and failures, notifications and their text, time milestones, warnings. Every one goes through `LogEvent`.
- **Phantom events**: if the game previews outcomes by running real logic, every event hook checks the preview flag and stays silent. Centralize this in one `PreviewModeDetector`.
- **Floods**: coalesce bursts ("3 enemies take 5 damage"; "5 items added"), rate-limit repeating conditions (a meter that stays critical should be announced on crossing the threshold and then at a slow interval, not every tick), and keep raw per-event lines in the buffer.
- **Priorities**: distinguish *urgent* (danger, failure imminent, a prompt that pauses the game), *normal* and *ambient*. Verbosity settings drop ambient first. Urgent is the only category that may ever be allowed to interrupt, and only by explicit user configuration.
- **Internal names mislead.** Enums, indices and team/side names are frequently the reverse of what the player sees. Verify by observation, then document the mapping once.

### 13.2 Discrete and turn-based play (cards, tactics, board, puzzle)

- Readers for each zone (hand, board rows/lanes/tiles, piles, enemy intents), in a *consistent, documented* order.
- **Targeting mode**: when an action needs a target, arrows or number keys move between valid targets, announcing each (name, stats, position, previewed outcome if the game computes one), Enter confirms, Escape cancels. Drive the game's *own* selection/confirm methods. Poll game state while targeting, because the game changes the selected target by itself.
- **Board review cursor**: a virtual, read-only cursor over the play area. Re-read game state on every key press so it never goes stale. Auto-close when targeting starts, when the screen changes, and when the encounter ends.
- Turn/phase announcements, and an end-of-turn summary where event volume is high.

### 13.3 Real-time worlds with a camera (simulation, management, strategy, life sim, city builder, RPG overworld)

Here there is no focus, no list, and no turn: a sighted player watches a scene and clicks into it. The mod supplies the missing structure.

- **Pause is the player's friend.** Expose pause and every game speed on keys, announce speed changes, and make sure every review and command feature works while paused. If the game forbids commands while paused, say so when the player tries.
- **The command function, not the click.** Find the function that takes (actor, interaction, target object/location) (section 4.5) and call it. Do not drive gameplay by projecting world positions to screen coordinates and clicking: it depends on camera, occlusion, floor/level visibility and window state. World-to-screen clicking is the fallback when no clean entry point can be found.
- **Object browser**: the entity list, filtered to what is relevant and perceivable (same room/area/level, discovered, not hidden), grouped by category (characters, usable objects, exits, hazards, resources), sorted by distance from the controlled character or cursor. Each entry: name, state, distance and direction ("fridge, 4 tiles north-east"). Enter opens that object's interaction list; choosing one issues the command through the game's function. This one feature replaces most of what the mouse does.
- **Interaction lists**: get the available actions from the game's own "what can this actor do with this object right now" query, including the *unavailable* ones with the reason, when the game shows those.
- **World cursor**: a virtual tile/grid/position cursor moved with arrows (with a larger-step modifier), announcing what is at the position: terrain/floor, objects, characters, room/zone, walkability. Read-only by default (rule 7); a separate explicit key issues "go here" or opens the interaction list for the thing under it. Announce coordinates relative to something meaningful (the lot/map origin, or the controlled character). Support multiple floors/levels/layers with their own keys.
- **Orientation helpers**: "where am I" (room/zone, floor, what is adjacent), "where is my character relative to the cursor", jump cursor to character, jump to next object of the same category, jump to the source of the last event.
- **Camera**: a blind player does not need it, but the *game* may: some games only simulate, stream or allow interaction with what is on screen or on the visible level. Find out, and if so make the camera follow the cursor/active character automatically.
- **Controlled-entity status**: one key per HUD cluster (needs/meters as words and numbers, mood/health, current action and the queued ones, money, time and date, active goals). Meter thresholds produce events (13.1). The action queue needs review *and* cancel.
- **Multiple controllable entities**: keys to cycle the active one, announce who is active on change, and a roster overview (who, where, doing what, most urgent need).
- **Build/edit/placement modes**: a grid cursor as above, plus: the catalog as a navigable list with price and description; pick up, rotate (announce facing), place, delete; **legality and cost before confirming**, with the game's reason when illegal; area tools (walls, floors, zones) as "mark start, move, mark end" with the dimensions and total cost announced; an audit key ("what is in this room", "is this room enclosed", "what is missing") because a sighted builder verifies by looking. Undo, if the game has it, on a key.
- **Pathing and reachability**: when a command fails because something is unreachable or blocked, the game usually shows a thought bubble, icon or animation. Find that failure signal and speak it.
- **Notifications and timed prompts**: read them when they appear, keep them in a buffer, and provide a key to focus the newest prompt's buttons. If prompts time out, say how long remains or (better) stop the timer while the player is reading, if that can be done without changing outcomes.
- **Non-verbal channels**: games communicate through icons over heads, animations, color, music changes and gibberish speech. Inventory these in `SCREENS.md` under "spatial-only info", find the state behind each, and turn them into words or distinct sounds.
- **Audio cues** (optional, config-gated): short positional sounds for cursor movement over walls/objects/empty space and for proximity are faster than speech for spatial tasks. Use the game's own audio system or a tiny bundled one; never block the main thread.

### 13.4 Real-time action

If success depends on reaction to visual events, speech is too slow on its own. Convert the telegraph to an **early, distinct sound**, keep speech for state, and offer assist options (slow-down, auto-aim, generous timing) behind config, stating honestly in the README what the mod changes. Discuss with the user before building assists: they are a design decision, not a technical one.

### 13.5 Map / progression review

A virtual cursor over the map's nodes or regions without moving the real selection. Build the list from the **live map UI first** (it reflects exactly what a sighted player sees) and fall back to state data when the UI is not loaded. Say what each node is, whether it is reachable, and what leads to it.

---

## 14. Text pipeline, keywords, localization

All speech passes through one cleanup function:

1. Decode correctly: know the game's string encoding (UTF-16, UTF-8, a legacy code page per language) and convert once, at the boundary.
2. Convert inline icon tags and control codes to words, using the icon list from the asset export.
3. Strip markup (rich text tags, color codes, the game's own escape sequences).
4. Resolve leaked localization keys and unexpanded format placeholders (detect the game's pattern; localize or fill them; never speak `%s` or `{0}`).
5. In tier B/C games, rewrite mouse wording into the mod's keyboard model (end of 10.4).
6. Collapse whitespace; drop empty results.

A `SplitIntoSpeechItems(text)` helper (sentence and line boundaries) turns any long text into buffer items and help entries.

**Localization**: find the game's single "get string" entry point and wrap it in `TryLocalize(keyOrId)`. Prefer asking the game for the final display string of an object (`GetName()`-style accessors) over assembling it yourself. Your mod's *own* strings are centralized in one table from day one, keyed, with English as the fallback, so they can be translated to every language the game ships in.

**KeywordManager**: build the glossary of mechanics **from the game's own strings at runtime** (tooltips, encyclopedia, status descriptions) so it stays correct across languages and patches. Hardcode a small fallback dictionary only for mechanics the game never explains in text, which in older games can be most of them: then the glossary is original writing, and it goes through the same localization table.

---

## 15. Help system

- `IHelpContext { ContextId, ContextName, Priority, IsActive(), GetHelpText() }`, one per screen or mode. `HelpSystem` picks the highest-priority active context; modes outrank screens (targeting > gameplay; dialog > everything).
- `ScreenStateTracker.CurrentScreen` (set by every screen hook) is the default `IsActive()` input; mod modes check their own state.
- F1 opens a **browsable list**: help text split into entries, Up/Down read one at a time, F1/Enter/Escape close. While open, all input routes to it and suppression keeps those keys from the game.
- Every screen in `SCREENS.md` needs a help context. Help text lists the *game's* keys for that screen as well as the mod's.
- Each screen's open announcement ends with a one-line hint for first-time users; make hints suppressible through verbosity.

---

## 16. Configuration

One plain-text config file (the mod loader's, or an `.ini`/`.toml` next to a native mod): verbosity level, every hotkey, braille on/off, per-category announcement toggles, audio cues, tutorial hints. Blind players edit this in a text editor; write clear descriptions in comments, tolerate bad values with a logged warning and a default, and document the file in the README. Offer an in-game reload key.

---

## 17. Test and debug loop

There are rarely automated tests for the game-facing part; the loop is build, deploy, launch, read the log, look at the screenshot, listen. Unit-test what does not need the game (text pipeline, buffers, sort order, config parsing, data-file parsers).

1. Build (auto-deploy into the game folder). For native mods, build Debug with symbols and keep the PDB; your crash handler's RVAs are useless without it.
2. **Launch the game yourself** (5.1), load a test save for the screen under test, and drive it. Check the mod log for hook failures and exceptions first.
3. Compare three things for every screen: a **screenshot** (what a sighted player sees), the **spoken-line log** (what a blind player heard), and the game state you read. Anything in the first that is not reachable through the second is a bug.
4. For an unreadable element, dump diagnostics on focus: the element's path in the UI tree, its class (RTTI name or managed type), its ancestors, and every field of the interesting object (reflection in managed code; a hex dump plus the `STRUCTS.md` overlay in native). Then go back to the extracted code with those names. Remove or gate the logging afterwards.
5. When behavior contradicts your model of the game, stop patching around it and **go measure** (5.2): a logging hook or a breakpoint settles it.
6. You cannot hear the output. The event log and mod log are your ears; ask the user to confirm what was actually spoken whenever timing, ordering, interruption or screen reader behavior matters. With no screen reader running, Prism falls back to a TTS backend, which is fine for smoke tests.
7. After any game crash during testing, determine whether the mod caused it (crash handler output, repro with the mod removed) before continuing. Record the cause in `CLAUDE.md` pitfalls.

---

## 18. Pitfalls checklist

**Any engine**

- Hook target guessed instead of read: silent no-op or crash.
- Post-hook order surprises when hooked functions call each other; capture earlier state on entry.
- Preview/simulation firing event announcements.
- Game changes selection without a key press; poll state.
- Focus is on a child; the component is on a parent. A reference is on a parent; the target is a child.
- Displayed text differs from stored text (placeholders, format strings, one-frame staleness).
- Hidden or disabled elements still holding text.
- "Screens" that are really panels inside another screen; screens that stay registered after they hide.
- State tracker that only sees opens, never closes.
- Modifier keys ignored by the game's input layer; keys processed twice.
- The key that opens a mod mode racing the game's own handling of the same key.
- Internal naming reversed relative to what the player sees.
- Per-frame reflection or tree walks without caching.
- `interrupt = true` anywhere.
- Keyword explanations leaking into live announcements.
- Stale buffer content after a screen change.
- Threshold conditions announced every tick instead of on crossing.
- Timers on game time frozen by pause or stretched by game speed.
- Navigation layer: focus moved but the game's hover did not (Enter hits something the player never heard); old hover left armed after a refused hover; the framework's pressed/selected state left armed after a simulated pointer-down; container under a focused object never hovered; silent arrow keys on empty screens; focus left null after an overlay closes; a polled "press Select" wait with no keyboard binding; tutorial text teaching mouse controls; cursor warps triggering edge scrolling.
- Driving gameplay through screen-coordinate clicks when a command function exists.
- Prism called from two threads; `bool` marshalled as 4 bytes; wrong-bitness `prism.dll`; freeing library-owned strings.

**Native / reverse engineering**

- Absolute addresses instead of `module base + RVA`; offsets from one build used on another; no build gate.
- Wrong calling convention on a detour or a game-function pointer: stack corruption that crashes later, elsewhere.
- Reading game structures from a non-main thread.
- Mixing CRTs: freeing the game's memory with your `free`, passing `std::string` across the boundary.
- Trusting a community struct layout or format doc without verifying it against this build.
- Finding the UI's *copy* of a value and taking it for the authoritative one.
- Assuming strings are UTF-8 or UTF-16 when the game uses a legacy code page (breaks on the first accented character or non-English install).
- Work done in `DllMain` under the loader lock: deadlocks at startup.
- A proxy DLL that forwards some exports but not all: the game fails to start on a code path you did not test.
- Breakpoint hit in exclusive fullscreen: frozen desktop.
- Analyzing a packed exe on disk instead of the unpacked image in memory.
- Debugger or Frida left attached, or the game left running, after the task.
- Findings that live only in a GUI tool's session instead of in `tools/`, `re/names.csv`, `STRUCTS.md` and `PATCH_TARGETS.md`.
- The 32-bit speech bridge blocking the game's main thread on pipe I/O; orphaned `SpeechHost.exe` after the game exits.
- Antivirus quarantining the proxy DLL or injector; test the release bundle on a clean Defender setup and document exclusions honestly if they are unavoidable.

---

## 19. Definition of done

- [ ] Every row in `SCREENS.md` is **verified**.
- [ ] The game is playable from launch to its natural conclusion (or a long, representative session of an open-ended game, covering success *and* failure states) with the monitor off.
- [ ] Every hover-only, icon-only, color-only, animation-only or position-only piece of information has a keyboard path or an announcement.
- [ ] Every interactive element announces name, role-relevant state and value.
- [ ] No double actions: claimed keys never reach the game.
- [ ] No mouse needed anywhere: every click, right-click, hover, drag, context menu and world interaction has a keyboard path, and no cinematic or prompt waits on an input the keyboard cannot produce.
- [ ] What Enter acts on is always the item that was last announced; illegal targets and unavailable actions are labelled as such, with the reason when the game has one.
- [ ] No arrow key is ever answered with silence, and screens without a dedicated handler still get the generic fallback.
- [ ] No phantom announcements during previews, targeting or placement.
- [ ] Buffers: Events fills during play; object buffers fill on focus; all clear on screen change.
- [ ] F1 help exists and is accurate for every screen and mode.
- [ ] Works with at least NVDA and with no screen reader (TTS fallback); recovers if the screen reader starts after the game.
- [ ] Log is free of hook failures and recurring exceptions; an unsupported game build is detected, announced and left unhooked.
- [ ] `CLAUDE.md`, `PATCH_TARGETS.md`, `SCREENS.md`, `STRUCTS.md` (native) and a player-facing README (supported game version(s), any compatibility setup the game itself needs, install steps, full key list, config) are current. RE scripts are in `tools/` and rerunnable.
- [ ] Release bundle: mod loader or proxy DLL + mod binaries + `prism.dll` (+ `SpeechHost.exe` for 32-bit games) with Prism's `LICENSES` and `NOTICE`; **no game code, assets, decompiled source or patched game executables**. Install is "copy these files into the game folder"; uninstall is "delete them".
