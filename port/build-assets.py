#!/usr/bin/env python3
"""Build prototype sprite previews and SNES 4bpp data without touching mvp/."""

import argparse
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--gfx4snes",
        default=os.path.join(
            os.environ.get("PVSNESLIB_HOME", os.path.expanduser("~/snesdev/pvsneslib")),
            "devkitsnes/tools/gfx4snes",
        ),
        help="path to gfx4snes (defaults to PVSNESLIB_HOME)",
    )
    args = parser.parse_args()
    converter = args.gfx4snes if os.path.isfile(args.gfx4snes) else "gfx4snes"

    manifests = [
        ("port/characters.json", 24, "port/generated/characters"),
        ("port/enemies.json", 24, "port/generated/enemies"),
        ("port/chests.json", 32, "port/generated/chests"),
        ("port/ui.json", 32, "port/generated/ui"),
        ("port/weapons.json", 32, "port/generated/weapons"),
        ("port/weapon-icons.json", 16, "port/generated/weapon-icons"),
    ]
    for manifest, size, output_dir in manifests:
        subprocess.run(
            [sys.executable, "scripts/build-sprites.py", "--manifest", manifest,
             "--preview-size", str(size), "--preview-dir", output_dir],
            cwd=ROOT,
            check=True,
        )

    subprocess.run([sys.executable, "port/build-menu-buttons.py"], cwd=ROOT, check=True)
    subprocess.run([sys.executable, "port/build-weapon-art.py"], cwd=ROOT, check=True)

    for png in sorted((ROOT / "port/generated").glob("*/*.png")):
        subprocess.run(
            [converter, "-s", "8", "-o", "16", "-p", "-e", "0", "-i", png.name],
            cwd=png.parent,
            check=True,
        )
        if png.stem == "WhipSlash64x16":
            # SNES 16x16 OBJ tiles use a 16-tile stride between rows.
            # The 64x16 source has 8 tiles per row; reserve indices 72-79
            # so the four adjacent 16x16 OBJ pieces don't read item tiles.
            pic = png.with_suffix(".pic")
            raw = pic.read_bytes()
            if len(raw) != 16 * 32:
                raise SystemExit(f"Unexpected whip tile data size: {len(raw)}")
            blank_row_gap = bytes(8 * 32)
            pic.write_bytes(raw[:8 * 32] + blank_row_gap + raw[8 * 32:])
    print("Port prototype assets built in port/generated/ (SNES 4bpp).")


if __name__ == "__main__":
    main()
