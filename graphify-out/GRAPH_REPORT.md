# Graph Report - vs-snes  (2026-11-23)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 195 nodes · 242 edges · 38 communities (12 shown, 26 thin omitted)
- Extraction: 93% EXTRACTED · 7% INFERRED · 0% AMBIGUOUS · INFERRED: 16 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `d1eca304`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Community 0
- Community 1
- Community 2
- Community 3
- Community 4
- Community 5
- Community 6
- Community 7
- Community 8
- Community 33
- Community 34
- Community 35
- Community 36
- Community 37

## God Nodes (most connected - your core abstractions)
1. `main()` - 16 edges
2. `spawnGem()` - 9 edges
3. `main()` - 7 edges
4. `setup-snesdev.sh script` - 7 edges
5. `consoleDrawTextColored()` - 5 edges
6. `index_sheet()` - 4 edges
7. `drawTime()` - 4 edges
8. `info()` - 4 edges
9. `install_pvsneslib_release()` - 4 edges
10. `install_pvsneslib_source()` - 4 edges

## Surprising Connections (you probably didn't know these)
- `spawnGem()` --calls--> `updateBullets()`  [INFERRED]
  mvp/src/gems.c → mvp/src/bullets.c
- `PVSNESlib Installation Workflow` --conceptually_related_to--> `Defensive Bash Script Practices`  [INFERRED]
  docs/pvsneslib-setup.md → .agents/skills/bash-defensive-patterns/SKILL.md
- `Gameplay reference: crowded forest arena with player, bat enemies, XP gems and weapon effects` --conceptually_related_to--> `Mad Forest grass texture: repeating dark green ground pattern`  [INFERRED]
  referencia.jpg → sprites/fondos/Bestiary-Mad_Forest.png
- `Mad Forest grass texture: repeating dark green ground pattern` --conceptually_related_to--> `Mad Forest indexed 64x64 background crop used as SNES tile graphics`  [INFERRED]
  sprites/fondos/Bestiary-Mad_Forest.png → mvp/gfx/fondos/Mad_Forest_crop_64x64_indexed.png
- `Antonio Belpaese player sprite: pixel-art hunter in green clothing carrying a whip` --conceptually_related_to--> `Combined SNES sprite atlas for player, enemies, items and weapon graphics`  [INFERRED]
  sprites/personajes/Animated-Antonio_Belpaese.gif → mvp/gfx/sprites.png

## Import Cycles
- None detected.

## Communities (38 total, 26 thin omitted)

### Community 0 - "Community 0"
Cohesion: 0.07
Nodes (9): updateAxe(), updateBible(), updateGarlic(), s16, spawnGem(), fireWandProjectile(), updateWand(), updateWater() (+1 more)

### Community 1 - "Community 1"
Cohesion: 0.13
Nodes (22): json, build_group_palette(), gen_assets_c(), gen_assets_h(), gen_sprites_asm(), gfx4snes_path(), index_sheet(), load_frames() (+14 more)

### Community 2 - "Community 2"
Cohesion: 0.14
Nodes (18): create_palette(), draw_bullet_tile(), draw_enemy_tile(), draw_gem_tile(), draw_hp_bar_tile(), draw_player_tile(), draw_tile(), main() (+10 more)

### Community 3 - "Community 3"
Cohesion: 0.10
Nodes (7): s16, fireBullet(), updateBullets(), u8, countOwnedWeapons(), levelUpMenu(), snes

### Community 4 - "Community 4"
Cohesion: 0.28
Nodes (4): u8, findNearestEnemy(), findSecondNearestEnemy(), findThirdNearestEnemy()

### Community 5 - "Community 5"
Cohesion: 0.22
Nodes (5): glob, os, pil, subprocess, sys

### Community 6 - "Community 6"
Cohesion: 0.33
Nodes (7): u8, consoleDrawTextColored(), drawNum(), drawTime(), stdarg, stdio, u16

### Community 7 - "Community 7"
Cohesion: 0.50
Nodes (8): error(), have_pvsneslib(), info(), install_pvsneslib_release(), install_pvsneslib_source(), PVSNESLIB_HOME, setup-snesdev.sh script, warn()

### Community 33 - "Community 33"
Cohesion: 0.18
Nodes (11): Defensive Bash Script Practices, SimpleSprite PVSNESlib Example, Survival Gameplay Loop, MVP Scope and Acceptance, Reproducible ROM Build, SPC Audio Path, VS-SNES Homebrew Project, HUD Screen Layout Reference (+3 more)

### Community 34 - "Community 34"
Cohesion: 0.50
Nodes (4): Antonio Belpaese player sprite: pixel-art hunter in green clothing carrying a whip, Combined item icon strip containing the XP gem and weapon inventory icons, Combined SNES sprite atlas for player, enemies, items and weapon graphics, Zombie enemy sprite sheet with four small animation frames for the SNES build

### Community 35 - "Community 35"
Cohesion: 0.50
Nodes (4): PVSNESlib OAM Slot API, Historical OAM_BASE Workaround (Unverified), OAM Slot Budget and Validation, Measured Performance Targets

### Community 36 - "Community 36"
Cohesion: 0.67
Nodes (3): Mad Forest indexed 64x64 background crop used as SNES tile graphics, Gameplay reference: crowded forest arena with player, bat enemies, XP gems and weapon effects, Mad Forest grass texture: repeating dark green ground pattern

## Knowledge Gaps
- **15 isolated node(s):** `PVSNESLIB_HOME`, `run.sh script`, `VS-SNES Homebrew Project`, `SPC Audio Path`, `PVSNESlib OAM Slot API` (+10 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 94 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **26 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `spawnGem()` connect `Community 0` to `Community 3`?**
  _High betweenness centrality (0.019) - this node is a cross-community bridge._
- **Why does `levelUpMenu()` connect `Community 3` to `Community 6`?**
  _High betweenness centrality (0.012) - this node is a cross-community bridge._
- **Why does `consoleDrawTextColored()` connect `Community 6` to `Community 3`?**
  _High betweenness centrality (0.009) - this node is a cross-community bridge._
- **Are the 7 inferred relationships involving `spawnGem()` (e.g. with `updateAxe()` and `updateBible()`) actually correct?**
  _`spawnGem()` has 7 INFERRED edges - model-reasoned connections that need verification._
- **What connects `PVSNESLIB_HOME`, `run.sh script`, `VS-SNES Homebrew Project` to the rest of the system?**
  _15 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Community 0` be split into smaller, more focused modules?**
  _Cohesion score 0.07196969696969698 - nodes in this community are weakly interconnected._
- **Should `Community 1` be split into smaller, more focused modules?**
  _Cohesion score 0.13333333333333333 - nodes in this community are weakly interconnected._