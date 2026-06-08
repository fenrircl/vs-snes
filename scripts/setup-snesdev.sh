#!/bin/bash
# setup-snesdev.sh — Instala PVSNESlib + todas las herramientas de desarrollo SNES
# Uso: bash scripts/setup-snesdev.sh
# Ejecutar desde la raíz del proyecto (vs-snes/)

set -e

echo "========================================"
echo "  VS-SNES: Instalación de herramientas"
echo "========================================"
echo ""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# ──────────────────────────────────────────────
# 1. Dependencias del sistema
# ──────────────────────────────────────────────
info "Instalando dependencias del sistema..."
sudo apt-get update -y
sudo apt-get install -y \
    build-essential \
    cmake \
    make \
    git \
    wget \
    unzip \
    doxygen \
    python3 \
    python3-pip \
    python3-pil \
    mednafen \
    gimp \
    imagemagick \
    xxd

# ──────────────────────────────────────────────
# 2. PVSNESlib
# ──────────────────────────────────────────────
if [ -z "$PVSNESLIB_HOME" ]; then
    PVSNESLIB_HOME="$HOME/snesdev/pvsneslib"
fi

if [ -f "$PVSNESLIB_HOME/include/snes.h" ]; then
    info "PVSNESlib ya está instalado en $PVSNESLIB_HOME"
else
    info "Descargando PVSNESlib 4.3.0..."
    mkdir -p ~/snesdev
    cd ~/snesdev
    
    if [ ! -f "pvsneslib_430_64b_linux_release.zip" ]; then
        wget https://github.com/alekmaul/pvsneslib/releases/download/4.3.0/pvsneslib_430_64b_linux_release.zip
    fi
    
    info "Extrayendo..."
    unzip -o pvsneslib_430_64b_linux_release.zip -d ~/snesdev/ 2>/dev/null
    
    if [ -d "$HOME/snesdev/pvsneslib" ]; then
        info "PVSNESlib instalado en ~/snesdev/pvsneslib"
    else
        # Intentar con nombre diferente
        RELEASE_DIR=$(find ~/snesdev -maxdepth 1 -type d -name "pvsneslib*" | head -1)
        if [ -n "$RELEASE_DIR" ]; then
            mv "$RELEASE_DIR" ~/snesdev/pvsneslib
            info "PVSNESlib movido a ~/snesdev/pvsneslib"
        else
            warn "No se encontró el directorio pvsneslib después de extraer."
            warn "Buscando manualmente..."
            ls -la ~/snesdev/
        fi
    fi
fi

# ──────────────────────────────────────────────
# 3. Variable de entorno PVSNESLIB_HOME
# ──────────────────────────────────────────────
if ! grep -q "PVSNESLIB_HOME" ~/.bashrc 2>/dev/null; then
    echo "" >> ~/.bashrc
    echo "# PVSNESlib (SNES development)" >> ~/.bashrc
    echo "export PVSNESLIB_HOME=$HOME/snesdev/pvsneslib" >> ~/.bashrc
    info "PVSNESLIB_HOME agregado a ~/.bashrc"
fi

export PVSNESLIB_HOME=$HOME/snesdev/pvsneslib

# ──────────────────────────────────────────────
# 4. Verificar toolchain
# ──────────────────────────────────────────────
info "Verificando toolchain..."

TOOLS="$PVSNESLIB_HOME/devkitsnes/tools"
COMPILER="$PVSNESLIB_HOME/devkitsnes"

if [ -f "$TOOLS/gfx4snes" ]; then
    info "✓ gfx4snes encontrado"
else
    warn "gfx4snes no encontrado en $TOOLS"
    ls "$TOOLS/" 2>/dev/null || echo "  (directorio tools no existe)"
fi

if [ -f "$COMPILER/816-tcc" ]; then
    info "✓ 816-tcc encontrado"
else
    warn "816-tcc no encontrado en $COMPILER"
fi

if [ -f "$COMPILER/wla-65816" ]; then
    info "✓ wla-65816 encontrado"
else
    warn "wla-65816 no encontrado en $COMPILER"
fi

# ──────────────────────────────────────────────
# 5. Verificar ejemplos (compilar hello_world)
# ──────────────────────────────────────────────
if [ -d "$PVSNESLIB_HOME/snes-examples/hello_world" ]; then
    info "Compilando hello_world para verificar..."
    cd "$PVSNESLIB_HOME/snes-examples/hello_world"
    make clean 2>/dev/null
    if make all 2>/dev/null; then
        info "✓ hello_world compilado correctamente"
    else
        warn "hello_world no compiló (puede que necesites compilar desde source)"
    fi
fi

# ──────────────────────────────────────────────
# 6. Instalar Pillow para scripts de assets
# ──────────────────────────────────────────────
info "Instalando Python Pillow..."
pip3 install Pillow 2>/dev/null || sudo pip3 install Pillow 2>/dev/null || true

# ──────────────────────────────────────────────
# 7. Generar sprites placeholder
# ──────────────────────────────────────────────
info "Generando sprites placeholder..."
cd ~/vs-snes 2>/dev/null || cd /tmp 2>/dev/null
if [ -f "scripts/placeholder-sprites.py" ]; then
    python3 scripts/placeholder-sprites.py 2>/dev/null || \
        warn "No se pudo generar sprites (ejecuta manualmente después)"
fi

echo ""
echo "========================================"
echo -e "${GREEN}  Instalación completada${NC}"
echo "========================================"
echo ""
echo "Para empezar a desarrollar:"
echo "  1. export PVSNESLIB_HOME=\$HOME/snesdev/pvsneslib"
echo "  2. source ~/.bashrc"
echo "  3. cd mvp/"
echo "  4. python3 ../scripts/placeholder-sprites.py  # si no se generaron"
echo "  5. gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png"
echo "  6. make"
echo "  7. mednafen vs-snes-mvp.sfc"
echo ""
