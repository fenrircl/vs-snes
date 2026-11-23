#!/usr/bin/env python3
"""Extract selected TexturePacker/Phaser frames into an SNES-ready source strip.

The extractor preserves transparent padding and frame order. Palette reduction,
24px artwork fitting, OBJ-cell packing, and gfx4snes output remain the job of
build-sprites.py.
"""

import argparse
import json
from pathlib import Path

from PIL import Image


REPO_ROOT = Path(__file__).resolve().parents[1]


def read_atlas(json_path):
    data = json.loads(json_path.read_text(encoding="utf-8"))
    if isinstance(data.get("textures"), list):
        texture = data["textures"][0]
    else:
        texture = data
    entries = texture.get("frames")
    if isinstance(entries, list):
        frames = {entry["filename"]: entry for entry in entries}
    elif isinstance(entries, dict):
        frames = entries
    else:
        raise ValueError("Atlas JSON must contain TexturePacker 'frames'.")
    return texture, frames


def unpack_frame(atlas, entry):
    rect = entry["frame"]
    x, y, w, h = (int(rect[k]) for k in ("x", "y", "w", "h"))
    image = atlas.crop((x, y, x + w, y + h)).convert("RGBA")

    source = entry.get("sourceSize", {"w": image.width, "h": image.height})
    if entry.get("trimmed", False):
        trim = entry.get("spriteSourceSize", {"x": 0, "y": 0,
                                                "w": image.width, "h": image.height})
        offset = (int(trim.get("x", 0)), int(trim.get("y", 0)))
        expected = (int(trim.get("w", image.width)), int(trim.get("h", image.height)))
    else:
        # Some atlases carry stale spriteSourceSize offsets even when trimmed
        # is false. The packed rectangle already is the full source frame.
        trim = {"x": 0, "y": 0, "w": int(source["w"]), "h": int(source["h"])}
        offset = (0, 0)
        expected = (int(source["w"]), int(source["h"]))
    if entry.get("rotated"):
        candidates = [image.transpose(Image.Transpose.ROTATE_90),
                      image.transpose(Image.Transpose.ROTATE_270)]
        image = next((candidate for candidate in candidates
                      if candidate.size == expected), candidates[0])
    if image.size != expected:
        raise ValueError(
            f"Atlas frame crop is {image.size}, metadata expects {expected}; "
            "unsupported TexturePacker rotation/trim data."
        )

    canvas = Image.new("RGBA", (int(source["w"]), int(source["h"])), (0, 0, 0, 0))
    canvas.alpha_composite(image, offset)
    return canvas, {"source_size": [int(source["w"]), int(source["h"])],
                    "sprite_source_size": [offset[0], offset[1], expected[0], expected[1]],
                    "rotated": bool(entry.get("rotated", False))}


def parse_cell(value):
    try:
        width, height = (int(part) for part in value.lower().split("x", 1))
    except (ValueError, AttributeError):
        raise argparse.ArgumentTypeError("cell must look like 32x32")
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("cell dimensions must be positive")
    return width, height


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--atlas-json", required=True, type=Path,
                        help="TexturePacker JSON from the Phaser bundle")
    parser.add_argument("--atlas-image", type=Path,
                        help="Atlas PNG; inferred from the JSON when omitted")
    parser.add_argument("--frames", nargs="+", required=True,
                        help="Exact frame names, in animation order")
    parser.add_argument("--out", required=True, type=Path,
                        help="Horizontal RGBA strip to feed to build-sprites.py")
    parser.add_argument("--cell", type=parse_cell,
                        help="Optional fixed source cell, for example 32x32")
    parser.add_argument("--role", choices=("player", "enemy", "weapon"), default="player",
                        help="SNES sprite role used in the generated manifest snippet")
    parser.add_argument("--no-manifest-entry", action="store_true",
                        help="Export atlas art without suggesting an assets.json sprite")
    args = parser.parse_args()

    json_path = args.atlas_json.expanduser().resolve()
    texture, entries = read_atlas(json_path)
    image_path = args.atlas_image or (json_path.parent / texture.get("image", ""))
    image_path = image_path.expanduser().resolve()
    if not image_path.is_file():
        parser.error(f"atlas image not found: {image_path}")
    atlas = Image.open(image_path).convert("RGBA")

    unpacked = []
    metadata = []
    for name in args.frames:
        if name not in entries:
            matches = [key for key in entries if Path(key).name == Path(name).name]
            if len(matches) != 1:
                parser.error(f"frame not found or ambiguous: {name}")
            name = matches[0]
        frame, details = unpack_frame(atlas, entries[name])
        unpacked.append((name, frame))
        metadata.append({"name": name, **details})

    cell_w = args.cell[0] if args.cell else max(frame.width for _, frame in unpacked)
    cell_h = args.cell[1] if args.cell else max(frame.height for _, frame in unpacked)
    if any(frame.width > cell_w or frame.height > cell_h for _, frame in unpacked):
        parser.error(f"selected frame exceeds output cell {cell_w}x{cell_h}")

    strip = Image.new("RGBA", (cell_w * len(unpacked), cell_h), (0, 0, 0, 0))
    for i, (name, frame) in enumerate(unpacked):
        x = i * cell_w + (cell_w - frame.width) // 2
        y = (cell_h - frame.height) // 2
        strip.alpha_composite(frame, (x, y))
        metadata[i]["cell"] = i
        metadata[i]["cell_offset"] = [x - i * cell_w, y]

    out = args.out.expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    strip.save(out)
    sidecar = out.with_suffix(out.suffix + ".frames.json")
    sidecar.write_text(json.dumps({
        "source_atlas_json": str(json_path),
        "source_atlas_image": str(image_path),
        "cell_size": [cell_w, cell_h],
        "frames": metadata,
    }, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    try:
        manifest_src = out.relative_to(REPO_ROOT).as_posix()
    except ValueError:
        manifest_src = str(out)
    name = out.stem
    print(f"Extracted {len(unpacked)} frames to {out} ({strip.width}x{strip.height})")
    print(f"Frame provenance: {sidecar}")
    if args.no_manifest_entry:
        print("No assets.json entry emitted (atlas art exported for another pipeline).")
    else:
        print("Manifest entry for assets.json:")
        print(json.dumps({"name": name, "src": manifest_src, "role": args.role,
                          "layout": "phaser_strip", "frame_size": [cell_w, cell_h],
                          "resample": "nearest"}, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
