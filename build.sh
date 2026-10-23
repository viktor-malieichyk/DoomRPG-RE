#!/bin/sh
# Optional MinGW cross-build helper. Native Windows builds can use CMake.
set -eu
cd "$(dirname "$0")"
compiler=${CC:-i686-w64-mingw32-gcc}
game_dir=${GAME_DIR:-..}
output_dir=${OUTPUT_DIR:-$game_dir}
if [ -n "${DEPS_PREFIX:-}" ]; then
  deps_prefix=$DEPS_PREFIX
elif command -v brew >/dev/null 2>&1; then
  deps_prefix=$(brew --prefix)
else
  echo 'Set DEPS_PREFIX to the dependency headers prefix, or use CMake.' >&2
  exit 2
fi
sdl_include=${SDL2_INCLUDE_DIR:-$deps_prefix/include/SDL2}
deps_include=${DEPS_INCLUDE_DIR:-$deps_prefix/include}
zlib_include=${ZLIB_INCLUDE_DIR:-$deps_prefix/opt/zlib/include}
if [ ! -f "$zlib_include/zlib.h" ]; then
  echo 'Set ZLIB_INCLUDE_DIR to a directory containing zlib.h and zconf.h.' >&2
  exit 2
fi
set -- -std=gnu99 -O2 -Isrc "-I$sdl_include" "-I$deps_include" "-I$zlib_include"
if [ "${TEST_UI:-0}" = 1 ]; then
  for unit in src/*.c; do
    if [ "$unit" != src/Main.c ]; then set -- "$@" "$unit"; fi
  done
  set -- "$@" tests/ui_scaling.c tests/controls_tests.c tests/video_tests.c
  output=$output_dir/DoomRPG-ui-test.exe
else
  set -- "$@" -mwindows src/*.c
  output=$output_dir/DoomRPG-scaled.exe
fi
mkdir -p "$output_dir"
"$compiler" "$@" "$game_dir/SDL2.dll" "$game_dir/SDL2_mixer.dll" "$game_dir/libfluidsynth-3.dll" "$game_dir/zlib.dll" -lm -static-libgcc -o "$output"
