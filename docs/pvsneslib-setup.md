# Instalación de PVSNESlib

## Requisitos

- Linux (Debian/Ubuntu recomendado), macOS o Windows (con msys2)
- git, make, build-essential, cmake
- Conexión a Internet

## Instalación rápida (release binario)

```bash
# 1. Descargar release 4.3.0
wget https://github.com/alekmaul/pvsneslib/releases/download/4.3.0/pvsneslib_430_64b_linux_release.zip

# 2. Extraer en ~/snesdev
unzip pvsneslib_430_64b_linux_release.zip -d ~/snesdev

# 3. Setear variable de entorno (OBLIGATORIO)
export PVSNESLIB_HOME=~/snesdev/pvsneslib

# 4. Agregar a ~/.bashrc para persistencia
echo 'export PVSNESLIB_HOME=~/snesdev/pvsneslib' >> ~/.bashrc
```

## Instalación desde source

```bash
# 1. Clonar repo
git clone https://github.com/alekmaul/pvsneslib.git
cd pvsneslib

# 2. Inicializar submodules
git submodule update --init --recursive

# 3. Instalar dependencias
sudo apt-get install -y build-essential gcc-12 cmake make git doxygen

# 4. Setear PVSNESLIB_HOME
export PVSNESLIB_HOME=$(pwd)

# 5. Compilar todo (compiler → tools → library → examples)
make
```

Esto puede tomar bastante tiempo (compila 816-tcc y wla-dx desde cero).

## Verificar instalación

```bash
cd $PVSNESLIB_HOME/snes-examples/hello_world
make all
ls -la hello_world.sfc
```

Si se generó `hello_world.sfc`, la instalación funciona.

## Instalar emulador

```bash
# Opción 1: mednafen (rápido, línea de comandos)
sudo apt-get install -y mednafen

# Opción 2: bsnes-plus (tiene debugger)
# Descargar desde https://github.com/devinacker/bsnes-plus/releases

# Opción 3: Mesen-S (el mejor debugger, requiere Wine o Windows)
# https://github.com/SourMesen/Mesen-S
```

## Probar la ROM

```bash
mednafen hello_world.sfc
```

## Estructura de PVSNESlib

```
pvsneslib/
├── include/
│   ├── snes.h              # Master include
│   └── snes/
│       ├── background.h    # Fondos (BG modes)
│       ├── console.h       # Consola de texto
│       ├── dma.h           # DMA
│       ├── input.h          # Input (pads, mouse, superscope)
│       ├── interrupt.h     # Interrupciones
│       ├── map.h            # Tile maps
│       ├── object.h        # Objetos / sprites por coordenadas
│       ├── scores.h        # Sistema de puntuación
│       ├── sound.h         # Música y efectos
│       ├── sprite.h        # Sprites / OAM
│       └── video.h         # Modos de video, init
├── devkitsnes/
│   ├── 816-tcc             # Compilador C → 65816
│   ├── wla-65816           # Ensamblador
│   ├── wlalink             # Linker
│   ├── tools/
│   │   ├── gfx4snes        # Conversor PNG/BMP → .pic .pal .map
│   │   ├── smconv          # Conversor .it → soundbank SNES
│   │   ├── snesbrr         # Conversor WAV → BRR
│   │   └── snestools       # Utilidades ROM
│   └── snes_rules          # Makefile include (reglas de build)
├── snes-examples/          # 19 categorías de ejemplos
├── vscode-template/        # Plantilla VS Code
└── lib/                    # Código fuente de la librería
```

**Nota importante:** PVSNESlib **NO** usa cc65. Usa su propio toolchain: **816-tcc** (C → 65816) + **wla-dx** (assembler/linker). Todo viene incluido en el release.
