# iPhone and iPad

The iOS port runs in landscape with touch controls and SDL gamepad support.
It imports your own `DoomRPG.zip` and `gm.sf2` from Files, including iCloud Drive,
and stores local copies so the game works offline. Game data is not included.
The original repository icon is used for the app. Options includes four text sizes
(100%, 75%, 60%, 50%) and a Wide fields toggle for full-width dialogue.

## Build and install with a free Apple ID

Requirements: macOS, full Xcode with the iOS platform installed, CMake 3.25+,
and Git. Dependencies are downloaded at pinned revisions by the build script.

```sh
sh build-ios.sh device
```

The unsigned device build is `dist/DoomRPG-ios-unsigned.ipa`. It needs signing
before installation; copying an unsigned IPA to an iPhone will not install it.

To sign and run directly from Xcode:

1. Open `build/ios-device-xcode/DoomRPG.xcodeproj`.
2. Add your Apple ID in Xcode Settings > Accounts.
3. Select the **DoomRPG** target > Signing & Capabilities, enable automatic signing,
   and choose your **Personal Team**. Set a unique bundle identifier if requested.
4. Connect your iPhone, enable Developer Mode if prompted, select it as the
   destination, and run the **DoomRPG** scheme.

You can also set `DEVELOPMENT_TEAM=YOUR_TEAM_ID` and `BUNDLE_ID=your.unique.id`
when invoking the script after configuring Xcode signing. A development-signed
IPA is then written as `dist/DoomRPG-ios-development.ipa`.
A free Personal Team requires rebuilding/reinstalling approximately every 7 days.
Keep exported save backups before deleting/reinstalling the app.
[Apple’s Personal Team limits](https://developer.apple.com/help/account/basics/about-your-developer-account).

For an Apple Silicon simulator:

```sh
sh build-ios.sh simulator
xcrun simctl install booted build/ios-simulator-xcode/Release-iphonesimulator/DoomRPG.app
xcrun simctl launch booted org.doomrpg.port
```

## Import game data from iCloud Drive

1. Put your `DoomRPG.zip` and `gm.sf2` in iCloud Drive using Files or Finder.
2. Launch the app and select **Import game data from Files / iCloud**.
3. In the system Files picker, choose Browse > iCloud Drive and select both files.
   You can import one at a time as well. Wait for the import confirmation.
4. Choose **Play**. Imported data stays in the app’s local `Game` folder.

`DoomRPG.zip` must be the prepared data archive used by the Windows port,
with the game resources at its root, rather than a ZIP of the whole Windows folder.
The SF2 soundfont must be named `gm.sf2`.

## Put saves on iCloud Drive

This uses manual Files backup/import, which works without the app’s paid iCloud
capability. It is not automatic cross-device cloud synchronization.

- Save in the game first. Tap **Files** > **Export save to Files / iCloud**, then
  choose a location in iCloud Drive. The single `.doomsave` file contains
  `Config`, `Player`, `Player2`, and `World` from the latest in-game save.
- To restore, fully close and reopen the app. Before choosing Play, select
  **Import save backup**, choose the `.doomsave` file in iCloud Drive, and confirm
  replacement. Choose Play, then Continue in the game.
- Restore validates the backup and keeps the previous local files in a
  `Previous Saves …` folder visible in Files > On My iPhone/iPad > Doom RPG.
  Save import is restricted to startup to avoid overwriting an active game.
- Windows save bytes retain their original format. To transfer existing Windows
  saves, create a `.doomsave` with `python3 ios/pack-save.py /path/to/saves backup.doomsave`,
  or copy all four files into the local `Game` folder through Files before launch.

The system document picker supplies access to iCloud Drive; no iCloud account
credentials or iCloud app entitlements are used by this app.
[Apple document picker documentation](https://developer.apple.com/documentation/uikit/uidocumentpickerviewcontroller).

## Implementation and checks

SDL2 and SDL2_mixer are linked statically. WAV effects use SDL2_mixer; MIDI uses
TinySoundFont/TinyMidiLoader with the imported soundfont, mixed through the same
audio device. The adapter implements only the FluidSynth APIs this game calls;
desktop builds continue using FluidSynth. The build applies a small, marked SDL2
window patch to attach it to a UIKit scene for current SDK compatibility.
The save format uses fixed-width 32-bit values on both Windows and ARM64 iOS.

```sh
xcrun --sdk macosx clang -fobjc-arc -Iios -framework Foundation \
  ios/SaveArchive.m tests/ios_save_tests.m -o build/ios-save-tests
build/ios-save-tests
```

Save tests cover round-trip export/import, malformed and incomplete archives,
replacement, previous-save preservation, and unchanged game assets.
A device build can compile unsigned on a Mac without signing identities. Actual
iCloud transfer and device installation require testing on your signed-in device.

Audio uses the ambient mixing session and does not mute or lower other apps.
Switching away suspends game audio; returning restores the scene layout and audio context.

Options > HUD panels hides or shows both status panels and the softkey strip.
Desktop builds also support F2 to toggle them; iOS has no extra touch button.
\nPassword terminals show an on-screen numeric keypad while requesting a PIN.\n