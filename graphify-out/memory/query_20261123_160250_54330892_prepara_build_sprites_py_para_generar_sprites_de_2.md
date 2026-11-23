---
type: "query"
date: "2026-11-23T16:02:50.684962+00:00"
question: "Prepara build-sprites.py para generar sprites de 24x24"
contributor: "graphify"
source_nodes: ["build-sprites.py", "process_frame()", "Combined SNES sprite atlas containing player, enemies, items and weapon graphics", "assets_gen.c", "renderPlayer()", "PVSNESlib OAM Slot API"]
---

# Q: Prepara build-sprites.py para generar sprites de 24x24

## Answer

Expanded from original query via vocab: [sprite, sprites, assets, atlas, frame, size, tile, oam, player, enemy]. El grafo relaciona scripts/build-sprites.py y process_frame() con el atlas combinado mvp/gfx/sprites.png, assets_gen.c y player.c/renderPlayer(). La herramienta produce los frames indexados y glue de assets; la vista previa nueva mantiene esa etapa separada del runtime, cuyo contrato OAM debe ajustarse antes de activar celdas 32x32.

## Source Nodes

- build-sprites.py
- process_frame()
- Combined SNES sprite atlas containing player, enemies, items and weapon graphics
- assets_gen.c
- renderPlayer()
- PVSNESlib OAM Slot API