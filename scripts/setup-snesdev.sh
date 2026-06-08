#!/bin/bash
# setup-snesdev.sh — Instala PVSNESlib + todas las herramientas de desarrollo SNES
#
# Uso: bash scripts/setup-snesdev.sh
# Ejecutar desde la raíz del proyecto (vs-snes/)
#
# Detecta automáticamente si tu sistema es compatible con los binarios
# pre-compilados (glibc >= 2.34) y si no, compila todo desde source.

set -e

# Resolver ruta absoluta del script al inicio (antes de cualquier cd)
SCRIPT_DIR=""
for src in "${BASH_SOURCE[0]}" "$0"; do
    [ -z "$src" ] && continue
    candidate="$(command -v realpath 2>/dev/null && realpath "$src" 2>/dev/null)"
    [ -z "$candidate" ] && candidate="$(readlink -f "$src" 2>/dev/null)"
    [ -z "$candidate" ] && candidate="$src"
    candidate="$(dirname "$candidate")"
    if [ -d "$candidate" ]; then
        SCRIPT_DIR="$candidate"
        break
    fi
done
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
[ ! -d "$PROJECT_DIR" ] && PROJECT_DIR="$(pwd -P)"

echo "========================================"
echo "  VS-SNES: Instalación de herramientas"
echo "========================================"
echo ""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# ──────────────────────────────────────────────
# 1. Detectar glibc
# ──────────────────────────────────────────────
GLIBC_VERSION=$(ldd --version 2>&1 | head -1 | grep -oP '(\d+\.\d+)$' || echo "0.0")
GLIBC_MAJOR=$(echo "$GLIBC_VERSION" | cut -d. -f1)
GLIBC_MINOR=$(echo "$GLIBC_VERSION" | cut -d. -f2)

info "Sistema: $(lsb_release -sd 2>/dev/null || cat /etc/os-release 2>/dev/null | grep PRETTY_NAME | cut -d= -f2 | tr -d '\"')"
info "glibc: $GLIBC_VERSION"

# glibc >= 2.34? (para los binarios pre-compilados de 816-tcc)
if [ "$GLIBC_MAJOR" -gt 2 ] || { [ "$GLIBC_MAJOR" -eq 2 ] && [ "$GLIBC_MINOR" -ge 34 ]; }; then
    GLIBC_OK=true
    info "glibc OK → se puede usar release binario"
else
    GLIBC_OK=false
    warn "glibc $GLIBC_VERSION < 2.34 → se compilará desde source"
fi

# ──────────────────────────────────────────────
# 2. Dependencias del sistema
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
# 3. PVSNESlib — release o source según glibc
# ──────────────────────────────────────────────
PVSNESLIB_HOME="${PVSNESLIB_HOME:-$HOME/snesdev/pvsneslib}"

# Helper: check if PVSNESlib is installed (handles both release and source layout)
have_pvsneslib() {
    [ -f "$1/include/snes.h" ] || [ -f "$1/pvsneslib/include/snes.h" ]
}

install_pvsneslib_release() {
    local dest="$HOME/snesdev/pvsneslib-release"
    mkdir -p "$dest"
    cd "$HOME/snesdev"

    if [ ! -f "pvsneslib_430_64b_linux_release.zip" ]; then
        info "Descargando PVSNESlib 4.3.0 release..."
        wget https://github.com/alekmaul/pvsneslib/releases/download/4.3.0/pvsneslib_430_64b_linux_release.zip
    fi

    info "Extrayendo release..."
    unzip -o pvsneslib_430_64b_linux_release.zip -d "$dest"

    # The zip contains a pvsneslib/ top-level directory
    if [ -d "$dest/pvsneslib" ]; then
        rm -rf "$PVSNESLIB_HOME" 2>/dev/null || true
        mv "$dest/pvsneslib" "$PVSNESLIB_HOME"
    else
        # Flat extraction fallback
        rm -rf "$PVSNESLIB_HOME" 2>/dev/null || true
        mv "$dest"/* "$PVSNESLIB_HOME" 2>/dev/null || mv "$dest" "$PVSNESLIB_HOME"
    fi
    rm -rf "$dest" 2>/dev/null || true

    # Probar el binario
    TEST_BIN=
    if [ -f "$PVSNESLIB_HOME/devkitsnes/bin/816-tcc" ]; then
        TEST_BIN="$PVSNESLIB_HOME/devkitsnes/bin/816-tcc"
    elif [ -f "$PVSNESLIB_HOME/devkitsnes/816-tcc" ]; then
        TEST_BIN="$PVSNESLIB_HOME/devkitsnes/816-tcc"
    fi

    if [ -z "$TEST_BIN" ] || ! ("$TEST_BIN" -v >/dev/null 2>&1 || "$TEST_BIN" < /dev/null >/dev/null 2>&1); then
        warn "El binario 816-tcc no funciona en este sistema (glibc incompatible)"
        return 1
    fi
    return 0
}

install_pvsneslib_source() {
    local src_dir="$HOME/snesdev/pvsneslib-source"
    local symlink_target="${1:-$PVSNESLIB_HOME}"

    info "Clonando PVSNESlib desde source..."
    if [ -d "$src_dir" ]; then
        info "Source ya existe, actualizando..."
        cd "$src_dir"
        git pull
    else
        git clone https://github.com/alekmaul/pvsneslib.git "$src_dir"
        cd "$src_dir"
    fi

    info "Inicializando submodules (816-tcc, wla-dx)..."
    git submodule update --init --recursive

    info "Compilando toolchain + librería + ejemplos..."
    info "Esto toma entre 3 y 10 minutos dependiendo de tu máquina."
    export PVSNESLIB_HOME="$src_dir"
    make

    # Crear symlink desde el path canónico (ej: ~/snesdev/pvsneslib) hacia el source
    if [ "$src_dir" != "$symlink_target" ]; then
        rm -rf "$symlink_target" 2>/dev/null || true
        ln -sf "$src_dir" "$symlink_target"
        info "Link simbólico: $symlink_target → $src_dir"
    fi

    if have_pvsneslib "$src_dir"; then
        return 0
    else
        return 1
    fi
}

# ──────────────────────────────────────────────
# 3b. Ejecutar instalación de PVSNESlib
# ──────────────────────────────────────────────
# Path canónico (donde otros scripts buscarán PVSNESlib)
CANONICAL_HOME="$HOME/snesdev/pvsneslib"

if have_pvsneslib "$PVSNESLIB_HOME"; then
    info "PVSNESlib ya está instalado en $PVSNESLIB_HOME"
else
    mkdir -p "$HOME/snesdev"

    if $GLIBC_OK; then
        info "Intentando instalación via release binario..."
        if install_pvsneslib_release; then
            info "✓ Release binario instalado correctamente"
        else
            warn "Falló release binario — compilando desde source..."
            install_pvsneslib_source "$CANONICAL_HOME"
        fi
    else
        info "glibc < 2.34 — compilando desde source directamente..."
        install_pvsneslib_source "$CANONICAL_HOME"
    fi

    if ! have_pvsneslib "$CANONICAL_HOME"; then
        error "No se pudo instalar PVSNESlib. Revisa los errores arriba."
        error "Intenta manualmente: https://github.com/alekmaul/pvsneslib/wiki/Installation"
        exit 1
    fi
fi

# ──────────────────────────────────────────────
# 4. Variable de entorno PVSNESLIB_HOME — siempre apuntar al path canónico
# ──────────────────────────────────────────────
if grep -q "^export PVSNESLIB_HOME=" ~/.bashrc 2>/dev/null; then
    sed -i "s|^export PVSNESLIB_HOME=.*|export PVSNESLIB_HOME=$CANONICAL_HOME|" ~/.bashrc
    info "PVSNESLIB_HOME actualizado en ~/.bashrc → $CANONICAL_HOME"
else
    echo "" >> ~/.bashrc
    echo "# PVSNESlib (SNES development toolchain)" >> ~/.bashrc
    echo "export PVSNESLIB_HOME=$CANONICAL_HOME" >> ~/.bashrc
    info "PVSNESLIB_HOME=$CANONICAL_HOME agregado a ~/.bashrc"
fi

export PVSNESLIB_HOME="$CANONICAL_HOME"

# ──────────────────────────────────────────────
# 5. Verificar toolchain
# ──────────────────────────────────────────────
info "Verificando toolchain..."

# Buscar 816-tcc en cualquiera de los paths posibles
TCC_PATH=$(find "$PVSNESLIB_HOME/devkitsnes" -name "816-tcc" -type f 2>/dev/null | head -1)
TOOLS_PATH=$(find "$PVSNESLIB_HOME/devkitsnes" -name "gfx4snes" -type f 2>/dev/null | head -1)

if [ -n "$TCC_PATH" ]; then
    info "✓ 816-tcc: $TCC_PATH"
else
    warn "✗ 816-tcc no encontrado"
    find "$PVSNESLIB_HOME/devkitsnes" -maxdepth 3 -type f 2>/dev/null | head -20
fi

if [ -n "$TOOLS_PATH" ]; then
    info "✓ gfx4snes: $TOOLS_PATH"
else
    warn "✗ gfx4snes no encontrado"
fi

# ──────────────────────────────────────────────
# 6. Verificar toolchain — probar 816-tcc
# ──────────────────────────────────────────────
TCC_WORKS=false
if [ -n "$TCC_PATH" ]; then
    if "$TCC_PATH" -v >/dev/null 2>&1 || "$TCC_PATH" < /dev/null >/dev/null 2>&1; then
        TCC_WORKS=true
    else
        warn "816-tcc existe pero NO funciona (probablemente glibc incompatible)"
    fi
fi

# ──────────────────────────────────────────────
# 6b. Compilar hello_world como verificación
# ──────────────────────────────────────────────
if [ -d "$PVSNESLIB_HOME/snes-examples/hello_world" ] && $TCC_WORKS; then
    (
        info "Compilando hello_world para verificar toolchain..."
        cd "$PVSNESLIB_HOME/snes-examples/hello_world"
        make clean 2>/dev/null || true
        if make all; then
            info "✓ hello_world compilado correctamente"
            ls -la hello_world.sfc 2>/dev/null
        else
            warn "hello_world no compiló. Revisa los errores."
            warn "A veces la compilación de ejemplos necesita make completo primero."
        fi
    )
fi

# ──────────────────────────────────────────────
# 7. Instalar Pillow
# ──────────────────────────────────────────────
info "Instalando Python Pillow..."
pip3 install Pillow 2>/dev/null || sudo pip3 install Pillow 2>/dev/null || true

# ──────────────────────────────────────────────
# 8. Generar sprites placeholder
# ──────────────────────────────────────────────
info "Generando sprites placeholder..."

if [ -f "$PROJECT_DIR/scripts/placeholder-sprites.py" ]; then
    cd "$PROJECT_DIR"
    python3 scripts/placeholder-sprites.py || warn "No se pudo generar sprites placeholder"
else
    # Si el script no está, buscar en el directorio actual
    if [ -f "scripts/placeholder-sprites.py" ]; then
        python3 scripts/placeholder-sprites.py || warn "No se pudo generar sprites placeholder"
    fi
fi

# ──────────────────────────────────────────────
# 9. Resumen final
# ──────────────────────────────────────────────
echo ""
echo "========================================"
echo -e "${GREEN}  Instalación completada${NC}"
echo "========================================"
echo ""
echo "  PVSNESLIB_HOME=$PVSNESLIB_HOME"
echo "  816-tcc:      ${TCC_PATH:-NO ENCONTRADO}"
echo "  gfx4snes:     ${TOOLS_PATH:-NO ENCONTRADO}"
echo ""

if [ -z "$TCC_PATH" ] || [ -z "$TOOLS_PATH" ]; then
    warn "Hay herramientas faltantes. Revisa los errores arriba."
    echo "Si el build desde source falló, prueba:"
    echo "  cd ~/snesdev/pvsneslib-source"
    echo "  make clean 2>/dev/null; make"
    echo ""
elif ! $TCC_WORKS; then
    warn "816-tcc requiere GLIBC 2.34+ pero tu sistema tiene una versión anterior."
    echo "Solución: compilar toolchain desde source:"
    echo "  1. rm -rf ~/snesdev/pvsneslib"
    echo "  2. bash scripts/setup-snesdev.sh   (compilará desde source automáticamente)"
    echo ""
    echo "O manualmente:"
    echo "  cd ~/snesdev/pvsneslib-source"
    echo "  export PVSNESLIB_HOME=\$(pwd)"
    echo "  make"
    echo ""
fi

echo "Para empezar a desarrollar:"
echo "  source ~/.bashrc"
echo "  cd mvp/"
echo "  gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png"
echo "  make"
echo "  mednafen vs-snes-mvp.sfc"
echo ""
echo "O directamente:"
echo "  bash scripts/run.sh"
echo ""
