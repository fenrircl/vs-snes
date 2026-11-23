#!/usr/bin/env python3
"""Build the 7/8 zoom background into the isolated port runtime."""

import argparse
import os
import subprocess
from pathlib import Path

from PIL import Image


PORT_ROOT = Path(__file__).resolve().parents[1]
GAME = PORT_ROOT / "runtime"
SOURCE = GAME / "gfx/fondos/Mad_Forest_crop_64x64_indexed.png"
OUTPUT = GAME / "gfx/fondos/Mad_Forest_zoom_7_8.png"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", default=None)
    args = parser.parse_args()
    image = Image.open(SOURCE)
    if image.mode != "P" or image.size != (64, 64):
        raise SystemExit(f"Expected indexed 64x64 background: {SOURCE}")
    image.resize((56, 56), Image.Resampling.NEAREST).save(OUTPUT)
    converter = args.converter or os.path.join(
        os.environ.get("PVSNESLIB_HOME", "/home/fenrir/snesdev/pvsneslib"),
        "devkitsnes/tools/gfx4snes",
    )
    subprocess.run(
        [converter, "-s", "8", "-o", "16", "-p", "-m", "-e", "0",
         "-i", "gfx/fondos/Mad_Forest_zoom_7_8.png"],
        cwd=GAME,
        check=True,
    )
    print(f"Built isolated 56x56 indexed pattern: {OUTPUT}")


if __name__ == "__main__":
    main()
