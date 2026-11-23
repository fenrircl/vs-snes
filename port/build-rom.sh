#!/usr/bin/env bash
set -Eeuo pipefail
trap 'printf "Error en línea %s al construir la ROM.\\n" "$LINENO" >&2' ERR

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
port_root="$repo_root/port"
pvsneslib_home="${PVSNESLIB_HOME:-/home/fenrir/snesdev/pvsneslib}"
gfx4snes="$pvsneslib_home/devkitsnes/tools/gfx4snes"

if [[ ! -d "$repo_root/mvp/src" || ! -d "$repo_root/mvp/gfx" ]]; then
    printf 'No encuentro la base del runtime en mvp/.\\n' >&2
    exit 1
fi
if [[ ! -x "$gfx4snes" ]]; then
    printf 'No encuentro gfx4snes ejecutable en %s.\\n' "$gfx4snes" >&2
    exit 1
fi
if [[ ! -f "$pvsneslib_home/devkitsnes/snes_rules" ]]; then
    printf 'No encuentro snes_rules en %s.\\n' "$pvsneslib_home" >&2
    exit 1
fi

if [[ ! -d "$port_root/runtime" ]]; then
    mkdir -p "$port_root/runtime"
    cp -a "$repo_root/mvp/src" "$repo_root/mvp/gfx" "$repo_root/mvp/res" \
        "$repo_root/mvp/data.asm" "$repo_root/mvp/hdr.asm" \
        "$repo_root/mvp/linkfile" "$repo_root/mvp/Makefile" \
        "$repo_root/mvp/sprites.asm" "$port_root/runtime/"
    sed -i 's/ROMNAME := vs-snes-mvp/ROMNAME := vs-snes-port/' "$port_root/runtime/Makefile"
fi

python3 "$port_root/build-assets.py" --gfx4snes "$gfx4snes"
python3 "$port_root/build-title-screen.py" --gfx4snes "$gfx4snes"
PVSNESLIB_HOME="$pvsneslib_home" make -C "$port_root/runtime" all
printf 'ROM lista: %s\n' "$port_root/runtime/vs-snes-port.sfc"
