# 👾 VS-SNES

**Vampire Survivors-like para Super Nintendo (SNES) — Homebrew con PVSNESlib**

Explorando los límites del hardware SNES para ver si es posible hacer un juego tipo *Vampire Survivors* usando [PVSNESlib](https://github.com/alekmaul/pvsneslib).

## Estado

MVP en fase de investigación y prototipado. El estado, checklist y próximos
pasos están reunidos en el [plan único de implementación](docs/PLAN-IMPLEMENTACION.md).

## Stack

| Componente | Herramienta |
|---|---|
| Librería | PVSNESlib (versión por fijar; ver el plan) |
| Compilador C | 816-tcc (fork de TCC para 65816) |
| Ensamblador | WLA-DX (wla-65816 / wlalink) |
| Conversor gráficos | gfx4snes (PNG/BMP → .pic .pal) |
| Emulador debug | Mesen-S / bsnes-plus / mednafen |
| Editor sprites | Aseprite / GIMP (imágenes indexadas 16 colores) |

## Objetivo técnico

Prototipar un juego de supervivencia para SNES y medir cuántos enemigos puede
mover y mostrar el hardware con estabilidad. Los objetivos de rendimiento se
definirán cuando exista un build reproducible.

## Scripts

| Script | Propósito |
|---|---|
| `scripts/setup-snesdev.sh` | Instala PVSNESlib + todas las herramientas |
| `scripts/placeholder-sprites.py` | Genera sprites placeholder para compilar sin arte |
| `scripts/run.sh` | Compila y corre la ROM en un emulador |

```bash
# Rápido: generar sprites, compilar y correr
python3 scripts/placeholder-sprites.py
cd mvp
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png
make
mednafen vs-snes-mvp.sfc
```

## Docs

- [Plan y checklist del MVP](docs/PLAN-IMPLEMENTACION.md)
- [Guía de instalación](docs/pvsneslib-setup.md)
- [API de PVSNESlib](docs/pvsneslib-api.md)
- [Ejemplos de código](docs/pvsneslib-examples.md)
- [Herramientas: sprites, debugging y desarrollo](docs/herramientas.md)
- [Recursos y comunidades](docs/recursos.md)

## Licencia

MIT
