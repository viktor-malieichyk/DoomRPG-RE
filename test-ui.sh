#!/bin/sh
# Requires Wine (or CrossOver), Python 3/Pillow, and separately supplied game data.
set -eu
cd "$(dirname "$0")"
GAME_DIR=${GAME_DIR:-..}
export GAME_DIR
python=${PYTHON:-python3}
wine=${WINE:-wine}
"$python" - <<'PY'
from pathlib import Path
import os
import shutil
source=Path(os.environ['GAME_DIR']).resolve()
fixture=Path('tests/runtime').resolve()
fixture.mkdir(parents=True, exist_ok=True)
(fixture/'source/tests').mkdir(parents=True, exist_ok=True)
for name in ['DoomRPG-ui-test.exe', 'DoomRPG.zip', 'gm.sf2']:
    shutil.copy2(source/name,fixture/name)
if (source/'Config').exists():
    shutil.copy2(source/'Config',fixture/'Config')
for path in source.glob('*.dll'):shutil.copy2(path,fixture/path.name)
PY
cd tests/runtime
run_test() {
  if [ -n "${CROSSOVER_BOTTLE:-}" ]; then
    "$wine" --bottle "$CROSSOVER_BOTTLE" './DoomRPG-ui-test.exe' "$@"
  else
    WINEPREFIX="$PWD/.wine" "$wine" './DoomRPG-ui-test.exe' "$@"
  fi
}
if [ "${UI_TEST_WINDOW:-0}" = 1 ]; then run_test --window; fi
for mode in ${UI_TEST_MODES:-8 15}; do run_test "$mode"; done
"$python" - <<'PY'
from pathlib import Path
from PIL import Image
for path in Path('source/tests').glob('*.bmp'):
    Image.open(path).save(Path('..')/path.with_suffix('.png').name)
PY
