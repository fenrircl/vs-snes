#!/bin/bash
# run.sh — Compila y ejecuta el MVP de VS-SNES
# Uso: bash scripts/run.sh [mednafen|bsnes|mesen]

set -e

EMULATOR="${1:-mednafen}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
MVP_DIR="$PROJECT_DIR/mvp"
ROM="$MVP_DIR/vs-snes-mvp.sfc"

# Verificar PVSNESLIB_HOME
if [ -z "$PVSNESLIB_HOME" ]; then
    if [ -f "$HOME/.bashrc" ]; then
        export PVSNESLIB_HOME=$(grep 'export PVSNESLIB_HOME=' "$HOME/.bashrc" | tail -1 | cut -d= -f2)
    fi
    if [ -z "$PVSNESLIB_HOME" ] && [ -d "$HOME/snesdev/pvsneslib" ]; then
        export PVSNESLIB_HOME="$HOME/snesdev/pvsneslib"
    fi
    if [ -z "$PVSNESLIB_HOME" ]; then
        echo "ERROR: Define PVSNESLIB_HOME primero"
        echo "  export PVSNESLIB_HOME=~/snesdev/pvsneslib"
        exit 1
    fi
    echo "PVSNESLIB_HOME=$PVSNESLIB_HOME"
fi

cd "$MVP_DIR"

# Verificar sprites
if [ ! -f "gfx/sprites.png" ]; then
    echo "No hay sprites. Generando placeholders..."
    python3 "$SCRIPT_DIR/placeholder-sprites.py"
fi

# Convertir sprites
if [ ! -f "sprites.pic" ] || [ "gfx/sprites.png" -nt "sprites.pic" ]; then
    echo "Convirtiendo sprites..."
    "$PVSNESLIB_HOME/devkitsnes/tools/gfx4snes" -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png
fi

# Compilar
echo "Compilando..."
make clean 2>/dev/null || true
make all

# Verificar ROM
if [ ! -f "$ROM" ]; then
    echo "ERROR: No se generó la ROM"
    ls -la *.sfc 2>/dev/null || echo "  (no hay .sfc)"
    exit 1
fi

SIZE=$(stat --format=%s "$ROM" 2>/dev/null || stat -f%z "$ROM" 2>/dev/null)
echo "ROM generada: $ROM ($SIZE bytes)"

# Ejecutar
echo "Ejecutando con $EMULATOR..."
case "$EMULATOR" in
    mednafen)
        mednafen "$ROM"
        ;;
    bsnes)
        bsnes-plus "$ROM" 2>/dev/null || bsnes "$ROM" 2>/dev/null || \
            echo "bsnes no encontrado. Instálalo o usa: bash run.sh mednafen"
        ;;
    mesen)
        wine mesen-s.exe "$ROM" 2>/dev/null || \
            echo "Mesen-S no encontrado (necesita Wine)"
        ;;
    *)
        echo "Emulador no reconocido: $EMULATOR"
        echo "Usa: mednafen, bsnes, o mesen"
        exit 1
        ;;
esac
