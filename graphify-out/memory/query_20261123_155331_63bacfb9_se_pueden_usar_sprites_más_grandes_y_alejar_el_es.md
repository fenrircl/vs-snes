---
type: "query"
date: "2026-11-23T15:53:31.150528+00:00"
question: "¿Se pueden usar sprites más grandes y alejar el escenario con escalado en SNES?"
contributor: "graphify"
source_nodes: ["Combined SNES sprite atlas containing player, enemies, items and weapon graphics", "renderPlayer()", "PVSNESlib OAM Slot API", "sprites.inc", "main.c"]
---

# Q: ¿Se pueden usar sprites más grandes y alejar el escenario con escalado en SNES?

## Answer

Expanded from original query via vocab: [snes, background, sprite, sprites, tile, screen, size, pixel, player, oam]. El grafo conecta el atlas combinado de sprites con mvp/gfx/sprites.png, los assets generados y main.c; identifica renderPlayer() en mvp/src/player.c y la referencia PVSNESlib OAM Slot API en docs/pvsneslib-api.md. El grafo no documenta una escala global ya implementada; el prototipo debe revisar los límites de OAM y las capas de fondo.

## Source Nodes

- Combined SNES sprite atlas containing player, enemies, items and weapon graphics
- renderPlayer()
- PVSNESlib OAM Slot API
- sprites.inc
- main.c