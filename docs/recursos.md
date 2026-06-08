# Recursos y Comunidades

## PVSNESlib

| Recurso | URL |
|---|---|
| Repositorio oficial | https://github.com/alekmaul/pvsneslib |
| Wiki | https://github.com/alekmaul/pvsneslib/wiki |
| API Docs (auto-generadas) | https://alekmaul.github.io/pvsneslib/ |
| Guía de instalación | https://github.com/alekmaul/pvsneslib/wiki/Installation |
| Compilar desde source | https://github.com/alekmaul/pvsneslib/wiki/Compiling-from-sources |
| VS Code integration | https://github.com/alekmaul/pvsneslib/wiki/PVSneslib-and-Visual-Studio-Code |
| Discord | https://discord.gg/DzEFnhB |
| Release 4.3.0 | https://github.com/alekmaul/pvsneslib/releases/download/4.3.0/pvsneslib_430_64b_linux_release.zip |

## Documentación técnica SNES

| Recurso | URL |
|---|---|
| Fullsnes (hardware reference) | https://problemkaputt.de/fullsnes.htm |
| SNES Development Wiki | https://wiki.superfamicom.org/ |
| Wikibooks SNES Programming | https://en.wikibooks.org/wiki/Super_NES_Programming |
| SPC700 Reference | https://wiki.superfamicom.org/spc700-reference |
| 65816 Opcodes | https://wiki.superfamicom.org/65816-reference |
| SNES Hardware Specs | https://en.wikipedia.org/wiki/Super_NES_technical_specifications |

## Comunidades

| Comunidad | URL | Idioma |
|---|---|---|
| NesDev Forums | https://forums.nesdev.org/ | EN |
| SNESLab Discord | https://discord.gg/DzEFnhB (mismo que PVSNESlib) | EN |
| SMW Central Homebrew | https://www.smwcentral.net/?p=section&s=smwhomebrew | EN |
| Riot / El Otro Lado | https://elotrolado.net/ (foros SNES) | ES |
| PVSNESlib Discord | https://discord.gg/DzEFnhB | EN/FR |

## Herramientas

| Herramienta | Uso | URL |
|---|---|---|
| **Mesen-S** | Emulador + debugger (el mejor) | https://github.com/SourMesen/Mesen-S |
| **bsnes-plus** | Emulador + debugger | https://github.com/devinacker/bsnes-plus |
| **mednafen** | Emulador CLI rápido | `sudo apt install mednafen` |
| **Aseprite** | Editor de sprites (pixel art) | https://www.aseprite.org/ |
| **GIMP** | Editor de imágenes (gratis) | `sudo apt install gimp` |
| **Tiled** | Editor de mapas (tilemaps) | https://www.mapeditor.org/ |
| **SuperFamistudio** | Editor de música tracker SPC700 | https://github.com/bbbbr/superfamistudio |
| **YY-CHR** | Editor de tiles SNES | https://www.romhacking.net/utilities/119/ |

## Juegos homebrew SNES de referencia

| Juego | Descripción | Enlace |
|---|---|---|
| **Yo-Yo Shuriken** | Hecho con PVSNESlib | https://pineappledev.itch.io/yo-yo-shuriken |
| **Eyra, The Crow Maiden** | Hecho con PVSNESlib | https://sebastianserpa.itch.io/eyra-the-crow-maiden |
| **Sydney Hunter** | Hecho con PVSNESlib | https://sydneysentinel.itch.io/ |
| **Super Boss Gaiden** | Homebrew con flickering avanzado | https://www.smwcentral.net/ |
| **Breakout (ejemplo)** | Viene con PVSNESlib | `snes-examples/games/breakout` |
| **LikeMario (ejemplo)** | Viene con PVSNESlib | `snes-examples/games/likemario` |

## Guías y tutoriales

1. **PVSNESlib Wiki** — https://github.com/alekmaul/pvsneslib/wiki
   - Installation Guide
   - Compiling from sources
   - VS Code Setup

2. **SNESdev Wiki** — https://wiki.superfamicom.org/
   - Getting Started
   - Hardware reference
   - Programming guides

3. **NesDev SNES Tutorial** — https://forums.nesdev.org/viewtopic.php?t=10657
   - Guía paso a paso de desarrollo SNES desde cero

## Conversión de assets

```bash
# PNG/BMP → SNES tiles + paleta
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i sprites.png
# Genera: sprites.pic (tiles), sprites.pal (paleta)

# Para sprites 16x16 con tiles de 8x8:
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i player.png
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i enemies.png

# PNG para background (Mode 1, 16 colores)
gfx4snes -s 16 -o 16 -u 16 -p -m -e 0 -i bg.png
# -m genera también el .map (tilemap)

# WAV → BRR (audio SNES)
snesbrr efecto.wav

# IT (Impulse Tracker) → soundbank SNES
smconv musica.it
```
