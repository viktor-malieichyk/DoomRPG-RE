# Windows port changes

Launch DoomRPG.exe directly for a console-free release build. Play.cmd starts
it and exits immediately. CMake's DOOMRPG_SHOW_CONSOLE option enables a debug
console when needed.

## Video and text

Video Options offers the original resolutions plus 960x540, 1280x720, 1600x900,
1920x1080, 2560x1440, and 3840x2160. Open the resolution submenu and select
a specific size with arrows/D-pad/left stick and Enter/A; Escape/B cancels.
The current size is preselected and marked with an asterisk. New profiles and
invalid saved modes default to 640x480; valid saved choices are retained.
Selection applies immediately, resizing
the native framebuffer and interface layout without restarting or reloading
the game. Existing saved resolution indices remain valid.

The interface scales from a 320x240 reference size while the 3D view retains
its selected native resolution. Options > Compact text reduces dialogue and
message-bar text at high resolutions. Dialogue rewraps within the same box and
fits up to eight lines instead of four. Fonts never become smaller than their
original native pixel size. The option saves in a backward-compatible extension
to Config; older profiles default to normal text.

Fullscreen uses the desktop display mode, hides the cursor, and keeps it
unconfined. Windowed mode shows the cursor. Mouse motion no longer drives game
movement; mouse buttons and wheel bindings remain available.

## Controls / Help

Open Controls/Help from the main or game menu, Input Options, or F1 while
playing or browsing menus. It shows current keyboard, mouse and gamepad bindings.
D-pad or left stick moves through the list, A selects, and B returns. Arrows,
Enter and Escape provide keyboard equivalents. Selecting an action waits for
release before capturing a new key/button. Escape, Start or Back cancels capture.
Reassignment replaces that device's mapping and clears conflicting assignments
while preserving other device mappings. Changes save immediately. Restore
Defaults includes standard gamepad bindings. Menu navigation remains available
independently of gameplay remapping.

## Build and verification

Use the upstream CMake build with SDL2, SDL2_mixer, zlib and FluidSynth
Windows development packages. C99 is required. Release builds use the GUI
subsystem by default; enable DOOMRPG_SHOW_CONSOLE for a debug console.

The optional `build.sh` helper cross-compiles with MinGW, embeds the original
appicon.ico through appicon.rc, and links the SDL2,
SDL2_mixer, FluidSynth and zlib DLLs in a separately supplied game directory.
It does not download or redistribute game assets or dependency binaries.
For a Homebrew-based cross-build:

```sh
GAME_DIR=/path/to/game ZLIB_INCLUDE_DIR=/path/to/zlib/headers sh build.sh
GAME_DIR=/path/to/game ZLIB_INCLUDE_DIR=/path/to/zlib/headers TEST_UI=1 sh build.sh
```

The zlib headers directory must contain zlib.h and zconf.h. The helper detects
Homebrew's prefix; DEPS_PREFIX, DEPS_INCLUDE_DIR and SDL2_INCLUDE_DIR override
header locations. CC selects the compiler, WINDRES selects the Windows resource compiler, and
OUTPUT_DIR selects the output
directory (default: GAME_DIR). The helper produces DoomRPG-scaled.exe or
DoomRPG-ui-test.exe. Copy the release to DoomRPG.exe in the game directory and
use Play.cmd there if desired.

`test-ui.sh` requires Wine, Python 3 and Pillow. Set GAME_DIR to the folder
containing the test executable, dependency DLLs, DoomRPG.zip and gm.sf2. An
existing Config is optional. Tests use a private copy under tests/runtime;
saves and settings in the original game folder are not modified.

```sh
GAME_DIR=/path/to/game sh test-ui.sh
```

WINE overrides the Wine executable. Set CROSSOVER_BOTTLE to an isolated test
bottle when using CrossOver; otherwise the runner uses a private Wine prefix
inside tests/runtime. UI_TEST_WINDOW=1 adds real-window checks, and
UI_TEST_MODES selects mode indices (default: 8 and 15, or 320x240 and 1280x720).
Generated images and runtime data are ignored by Git.

Checks cover rendering through 4K, native framebuffer detail, live resolution
switches with preserved game state, compact-text capacity and persistence,
menu actions, virtual-gamepad navigation/rebinding/cancellation, keyboard and
number-key bindings, defaults, scrolling, and returning from F1 help. Real-window
checks cover live resize and fullscreen cursor hiding without mouse capture.
Screenshots at 320x240 and 1280x720 are used for visual review. These checks do
not replace a full interactive gameplay session or physical-controller testing.

Menu correction: input now waits for release across menu transitions, so a held
Escape/Start/Enter cannot trigger the next screen. Main-menu labels share a
left anchor; option labels and values use separate columns with reserved spacing.
Regression checks hold Escape, Start and Enter across transitions for 60 frames;
all passed. Main and Options screenshots were reviewed at 320x240 and 1280x720.

Binary releases provide a patch executable and a ZIP containing the executable,
launcher, license and installation notes. Apply these to an existing working
Windows port folder; original game assets and dependency DLLs are not bundled.

## Text field layout

Options now offers Text size (100%, 75%, 60%, 50%) and Wide fields (on/off).
Wide fields expands dialogue boxes, their text wrapping and scrollbar to the full
game display width. Text size applies to dialogue and top-bar message text; native
font pixel size is retained at low resolutions. Both settings update immediately
and persist. Existing compact-text-on configurations migrate to 50%. Older builds
can still read the original boolean extension.
