#!/usr/bin/env python3
"""Create the 7/8 zoom background pattern and its SNES graphics data."""

import argparse
import os
import subprocess
from pathlib import Path

from PIL import Image


def main():
    repo = Path(__file__).resolve().parents[1]
    game = repo / "mvp"
    source = game / "gfx/fondos/Mad_Forest_crop_64x64_indexed.png"
    output = game / "gfx/fondos/Mad_Forest_zoom_7_8.png"
    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", default=None)
    args = parser.parse_args()

    image = Image.open(source)
    if image.mode != "P" or image.size != (64, 64):
        raise SystemExit(f"Expected indexed 64x64 background: {source}")
    image.resize((56, 56), Image.Resampling.NEAREST).save(output)

    converter = args.converter or os.path.join(
        os.environ.get("PVSNESLIB_HOME", os.path.expanduser("~/snesdev/pvsneslib")),
        "devkitsnes/tools/gfx4snes",
    )
    subprocess.run(
        [converter, "-s", "8", "-o", "16", "-p", "-m", "-e", "0",
         "-i", "gfx/fondos/Mad_Forest_zoom_7_8.png"],
        cwd=game,
        check=True,
    )
    print(f"Built 56x56 indexed background pattern: {output}")


if __name__ == "__main__":
    main()
