# 👾 VS-SNES

**Vampire Survivors-like para Super Nintendo (SNES) — Homebrew con PVSNESlib**

Explorando los límites del hardware SNES para ver si es posible hacer un juego tipo *Vampire Survivors* usando [PVSNESlib](https://github.com/alekmaul/pvsneslib).

## Estado

MVP en fase de investigación y prototipado. Consulta [plan.md](plan.md) para el roadmap.

## Stack

| Componente | Herramienta |
|---|---|
| Librería | PVSNESlib 4.3.0 |
| Compilador C | 816-tcc (fork de TCC para 65816) |
| Ensamblador | WLA-DX (wla-65816 / wlalink) |
| Conversor gráficos | gfx4snes (PNG/BMP → .pic .pal) |
| Emulador debug | Mesen-S / bsnes-plus / mednafen |
| Editor sprites | Aseprite / GIMP (imágenes indexadas 16 colores) |

## Objetivo técnico

Demostrar que se puede tener ~40+ enemigos simultáneos en SNES usando técnicas de flickering, sprites 8x8, update escalonado y colisiones eficientes, todo en C con PVSNESlib.

## Docs

- [Guía de instalación](docs/pvsneslib-setup.md)
- [API de PVSNESlib](docs/pvsneslib-api.md)
- [Ejemplos de código](docs/pvsneslib-examples.md)
- [Recursos y comunidades](docs/recursos.md)

## Licencia

MIT
