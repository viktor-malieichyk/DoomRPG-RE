#!/bin/sh
# Configure/build iPhone or simulator. Apple signing is deliberately optional.
set -eu
cd "$(dirname "$0")"
MODE=${1:-device}
case "$MODE" in
 device) SDK=iphoneos ;;
 simulator) SDK=iphonesimulator ;;
 *) echo "Usage: sh build-ios.sh [device|simulator]" >&2; exit 2 ;;
esac
mkdir -p build/ios-deps
fetch() {
 name=$1; repo=$2; revision=$3
 dest="build/ios-deps/$name"
 if [ ! -d "$dest/.git" ]; then
  git init "$dest"
  git -C "$dest" remote add origin "$repo"
 fi
 if [ "$(git -C "$dest" rev-parse HEAD 2>/dev/null || true)" != "$revision" ]; then
  git -C "$dest" fetch --depth 1 origin "$revision"
  git -C "$dest" checkout --detach FETCH_HEAD
 fi
}
fetch SDL https://github.com/libsdl-org/SDL.git 8acf7cfc0f88717e5c036f60f0314bfcafe39189
fetch SDL_mixer https://github.com/libsdl-org/SDL_mixer.git f0dd72c2c842cfe0b77b74cdfd2ef49e218d9341
fetch TinySoundFont https://github.com/schellingb/TinySoundFont.git 853a0a171759f1ddba0de1442133a75912bbeffa
OUT="build/ios-$MODE-xcode"
cmake -S . -B "$OUT" -G Xcode -DCMAKE_SYSTEM_NAME=iOS  -DCMAKE_OSX_SYSROOT="$SDK" -DCMAKE_OSX_ARCHITECTURES=arm64  -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0  -DDOOMRPG_BUNDLE_ID="${BUNDLE_ID:-org.doomrpg.port}"
if [ -n "${DEVELOPMENT_TEAM:-}" ]; then
 xcodebuild -project "$OUT/DoomRPG.xcodeproj" -scheme DoomRPG -configuration Release   -sdk "$SDK" -allowProvisioningUpdates DEVELOPMENT_TEAM="$DEVELOPMENT_TEAM" build
else
 xcodebuild -project "$OUT/DoomRPG.xcodeproj" -scheme DoomRPG -configuration Release   -sdk "$SDK" CODE_SIGNING_ALLOWED=NO build
fi
if [ "$MODE" = device ]; then
 mkdir -p dist/ios/Payload
 ditto "$OUT/Release-iphoneos/DoomRPG.app" dist/ios/Payload/DoomRPG.app
 if [ -n "${DEVELOPMENT_TEAM:-}" ]; then PACKAGE=DoomRPG-ios-development.ipa; else PACKAGE=DoomRPG-ios-unsigned.ipa; fi
 (cd dist/ios && /usr/bin/zip -qr "../$PACKAGE" Payload)
 echo "IPA: dist/$PACKAGE"
fi
