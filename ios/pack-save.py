#!/usr/bin/env python3
"""Pack the Windows port's four save files for iOS Files import."""
import argparse
import plistlib
from pathlib import Path
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("save_directory", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
files = {n: (args.save_directory / n).read_bytes() for n in ("Config", "Player", "Player2", "World")}
if any(len(data) < 4 or len(data) > 2 * 1024 * 1024 for data in files.values()):
    parser.error("Save files must be complete and no larger than 2 MiB each")
if files["Config"][:4] != b"\x17\0\0\0":
    parser.error("Config must use the Windows port's version 23 format")
args.output.write_bytes(plistlib.dumps({"format": 1, "files": files}, fmt=plistlib.FMT_BINARY))
print(args.output)
