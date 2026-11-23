# Graph Report - vs-snes  (2026-11-23)

## Corpus Check
- 133 files · ~74,516 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 55 file(s) not represented in the graph (top: .as 24, .Identifier 21, (none) 4)

## Summary
- 397 nodes · 641 edges · 32 communities (15 shown, 17 thin omitted)
- Extraction: 73% EXTRACTED · 27% INFERRED · 0% AMBIGUOUS · INFERRED: 171 edges (avg confidence: 0.81)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- Weapon Modules & Asset Manifest
- Asset Pipeline & Conversion Tools
- MVP Roadmap & Sprite Rendering
- Character & Stage Art
- OAM API & Bug Notes
- Placeholder Sprite Generator
- Dev Tools & Workflow
- Bash Defensive Patterns
- PVSnesLib Toolchain Setup
- Enemy Bestiary
- Weapon Sprite Bank
- Playable Cast Art
- HUD & Text Rendering
- Menus & Video Modes
- Mednafen Emulator
- Soundbank Playback
- Debugger Emulators
- Controller Input API
- SNES Hardware References

## God Nodes (most connected - your core abstractions)
1. `Bash Defensive Patterns Skill` - 20 edges
2. `Plan MVP Roadmap` - 17 edges
3. `VS-SNES Project Overview` - 15 edges
4. `updateWater()` - 10 edges
5. `gfx4snes Asset Converter` - 10 edges
6. `Indexed 4bpp SNES Palette Asset Format (.pic/.pal)` - 10 edges
7. `spawnGem()` - 9 edges
8. `updateWand()` - 9 edges
9. `16-bit Console-Style Pixel Art Character Sprite (transparent background, limited palette)` - 9 edges
10. `Animated Antonio Belpaese Walk Cycle (4 of 8 16x16 frames)` - 8 edges

## Surprising Connections (you probably didn't know these)
- `MVP Frame 0 - Zoomed Antonio Render` --semantically_similar_to--> `Animated Antonio Belpaese Walk Cycle (4 of 8 16x16 frames)`  [INFERRED] [semantically similar]
  mvp/frame0.png → gfx/personajes/Animated_Antonio_Belpaese_indexed.png
- `Antonio Belpaese Walk Sheet Variant 128x16 (hyphen-named duplicate, alternate light-color palette, not referenced by assets_gen)` --semantically_similar_to--> `Animated Antonio Belpaese Walk Cycle (4 of 8 16x16 frames)`  [INFERRED] [semantically similar]
  mvp/gfx/personajes/Animated-Antonio_Belpaese_indexed.png → gfx/personajes/Animated_Antonio_Belpaese_indexed.png
- `updateWater()` --implements--> `No Division or Modulo in Main Loop`  [INFERRED]
  mvp/src/water.c → plan.md
- `updateWater()` --implements--> `Radial Distance Collision (dx*dx+dy*dy)`  [INFERRED]
  mvp/src/water.c → plan.md
- `~40+ Simultaneous Enemies Objective` --semantically_similar_to--> `Enemy Count Estimates Table`  [INFERRED] [semantically similar]
  README.md → plan.md

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **OAM Slot Management Pattern** — docs_devlog_oammask_bug, docs_devlog_oam_base_256, docs_devlog_oam_slot_u16, docs_pvsneslib_api_oamset, docs_pendientes_oam_dirty_flags [INFERRED 0.85]
- **Asset Conversion Pipeline** — docs_herramientas_gfx4snes, docs_herramientas_smconv, docs_herramientas_snesbrr, docs_herramientas_tmx2snes, docs_recursos_asset_conversion [EXTRACTED 1.00]
- **HUD Row 0 Layout Flow** — docs_referencia_hud_row0, docs_referencia_console_32_columns, docs_pendientes_heart_hud, docs_pvsneslib_api_console [INFERRED 0.75]
- **SNES Enemy Rendering Strategy** — plan_flickering, plan_staggered_update, plan_sprite_pool, plan_enemy_count_estimates, plan_snes_hardware_limits [INFERRED 0.85]
- **PVSNESlib Toolchain Stack** — readme_pvsneslib, readme_tcc_816, readme_wla_dx, readme_gfx4snes, readme_mednafen, readme_aseprite [EXTRACTED 1.00]
- **Bash Script Safety Baseline** — bash_defensive_patterns_strict_mode, bash_defensive_patterns_error_trapping, bash_defensive_patterns_variable_quoting, bash_defensive_patterns_argument_parsing, bash_defensive_patterns_structured_logging [INFERRED 0.85]
- **Manifest-Driven Code Generation Flow** — assets_json, scripts_build_sprites, scripts_build_sprites_gen_assets_h, scripts_build_sprites_gen_assets_c, mvp_gfx_sprites_gen_asm [EXTRACTED 1.00]
- **SNES 4bpp / 16-Color Indexing Pipeline** — scripts_build_sprites_index_sheet, scripts_build_sprites_build_group_palette, scripts_convert_asset_main, scripts_convert_menu_main, scripts_placeholder_sprites_create_palette, concept_gfx4snes [INFERRED 0.85]
- **Toolchain Bootstrap to ROM Launch Flow** — scripts_setup_snesdev, scripts_run, scripts_placeholder_sprites, concept_pvsneslib_toolchain, mvp_gfx_sprites_png [EXTRACTED 0.95]
- **Antonio Player Character Art Family** — gfx_personajes_animated_antonio_belpaese_indexed, mvp_frame0, sprites_ataques_whip [INFERRED 0.85]
- **Mad Forest Stage Visuals** — gfx_fondos_mad_forest_crop_64x64_indexed, referencia, mvp_frame0 [INFERRED 0.65]
- **Weapon / Trigger Item Sprite Family (base arsenal)** — mvp_gfx_ataques_animated_axe_indexed, mvp_gfx_ataques_animated_garlic_indexed, mvp_gfx_ataques_animated_king_bible_indexed, mvp_gfx_ataques_animated_magic_wand_indexed, mvp_gfx_ataques_animated_whip_indexed [INFERRED 0.95]
- **Ranged / Lobbed Projectile Attack Sprites** — mvp_gfx_ataques_animated_axe_indexed, mvp_gfx_ataques_animated_magic_wand_indexed, mvp_gfx_ataques_animated_santa_water_indexed [INFERRED 0.85]
- **Persistent Area-of-Effect Attack Sprites** — mvp_gfx_ataques_animated_garlic_indexed, mvp_gfx_ataques_animated_king_bible_indexed, mvp_gfx_ataques_animated_whip_indexed [INFERRED 0.75]
- **Enemy Bestiary Sprite Set (Ghost, Mudman, Pipeestrello, Skeleton, Zombie)** — mvp_gfx_enemigos_animated_ghost_indexed, mvp_gfx_enemigos_animated_mudman_indexed, gfx_enemigos_animated_pipeestrello_indexed, mvp_gfx_enemigos_animated_skeleton_indexed, mvp_gfx_enemigos_animated_zombie_indexed [EXTRACTED 1.00]
- **Animated Enemy Asset Pipeline (4-frame sheets exported as .pic/.pal/.inc)** — mvp_gfx_enemigos_animated_ghost_indexed, mvp_gfx_enemigos_animated_mudman_indexed, gfx_enemigos_animated_pipeestrello_indexed, mvp_gfx_enemigos_animated_skeleton_indexed, mvp_gfx_enemigos_animated_zombie_indexed, indexed_4bpp_snes_palette [INFERRED 0.85]
- **HUD & Menu Visual Assets (text font, weapon icons, title screen)** — mvp_gfx_pvsneslibfont, mvp_gfx_sprites, mvp_gfx_fondos_main_menu [INFERRED 0.85]
- **Indexed 4bpp Art Pipeline (PNG -> .pic/.pal/.inc via build scripts)** — gfx_fondos_mad_forest_crop_64x64_indexed, mvp_gfx_fondos_main_menu, gfx_personajes_animated_antonio_belpaese_indexed, mvp_gfx_personajes_animated_antonio_belpaese_indexed_variant, mvp_gfx_pvsneslibfont, mvp_gfx_sprites, indexed_4bpp_snes_palette [INFERRED 0.85]
- **MVP Game Visual Assets (backgrounds, player, font, sprite atlas)** — gfx_fondos_mad_forest_crop_64x64_indexed, mvp_gfx_fondos_main_menu, gfx_personajes_animated_antonio_belpaese_indexed, mvp_gfx_pvsneslibfont, mvp_gfx_sprites [EXTRACTED 1.00]
- **Undead Humanoid Enemy Family (Ghost, Skeleton, Zombie)** — enemigos_ghost_enemy, enemigos_skeleton_enemy, enemigos_zombie_enemy, sprites_enemigos_animated_ghost, sprites_enemigos_animated_skeleton, sprites_enemigos_animated_skeleton_2, sprites_enemigos_animated_zombie, sprites_enemigos_animated_zombie_2, sprites_enemigos_animated_zombie_3 [INFERRED 0.85]
- **Pipeestrello Bat Enemy Variant Set** — enemigos_pipeestrello_enemy, sprites_enemigos_animated_pipeestrello, sprites_enemigos_animated_pipeestrello_2, sprites_enemigos_animated_pipeestrello_3, sprites_enemigos_animated_pipeestrello_4 [EXTRACTED 1.00]
- **Green Brute/Gel Enemy Family (Mudman, Zombie brute)** — enemigos_mudman_enemy, sprites_enemigos_animated_mudman, sprites_enemigos_animated_mudman_2, sprites_enemigos_animated_zombie_2, sprites_enemigos_animated_zombie_3 [INFERRED 0.75]
- **Trigger Item Pickup Icons (weapons plus experience gem atlas)** — sprites_items_sprite_axe, sprites_items_sprite_garlic, sprites_items_sprite_king_bible, sprites_items_sprite_magic_wand, sprites_items_sprite_santa_water, sprites_items_sprite_whip, sprites_items_sprite_experience_gem, sprites_items_combined_items [EXTRACTED 1.00]
- **Background Art Family (Mad Forest textures and main menu cover art)** — sprites_fondos_bestiary_mad_forest, sprites_fondos_mad_forest_crop_64x64, sprites_fondos_main [INFERRED 0.85]
- **Dommario Sprite Asset Family (static + 4-frame animated variants)** — sprites_personajes_120px_sprite_dommario, sprites_personajes_120px_animated_dommario, sprites_personajes_animated_dommario, character_dommario [EXTRACTED 1.00]
- **Playable Cast Sprite Set (Antonio, Dommario, Imelda, Lama)** — sprites_personajes_120px_sprite_antonio_belpaese, sprites_personajes_animated_antonio_belpaese, sprites_personajes_120px_sprite_dommario, sprites_personajes_120px_animated_dommario, sprites_personajes_animated_dommario, sprites_personajes_120px_sprite_imelda_belpaese, sprites_personajes_animated_imelda_belpaese, sprites_personajes_120px_sprite_lama_ladonna, sprites_personajes_animated_lama_ladonna, character_antonio_belpaese, character_dommario, character_imelda_belpaese, character_lama_ladonna [INFERRED 0.95]

## Communities (32 total, 17 thin omitted)

### Community 0 - "Weapon Modules & Asset Manifest"
Cohesion: 0.06
Nodes (44): assets.json sprite manifest (single source of truth), Per-weapon level scaling pattern (lvl -> damage/cooldown/count), Fixed OAM Slot Allocation Map, initAxe(), updateAxe(), initBible(), updateBible(), s16 (+36 more)

### Community 1 - "Asset Pipeline & Conversion Tools"
Cohesion: 0.06
Nodes (37): assets.json Sprite Manifest, Claude Code permission allowlist (.claude/settings.local.json), gfx4snes 4bpp Tile Converter Tool, Shared Per-Group 15-Color Palette Strategy, Glob, Json, sprites_gen.asm (generated includes), sprites.pic converted 4bpp tile data (+29 more)

### Community 2 - "MVP Roadmap & Sprite Rendering"
Cohesion: 0.08
Nodes (38): Enemy Count Estimates Table, Frame-Alternating Flickering, Game Over Condition and Screen, Gem Attraction Towards Player, hdr.asm ROM Header, Mode 1 16x16 Tile Background, Plan MVP Roadmap, One OBJ Per Enemy (8x8) (+30 more)

### Community 3 - "Character & Stage Art"
Cohesion: 0.09
Nodes (34): Antonio Belpaese (player character, whip wielder), Antonio Belpaese (Player Character), Enemy Walk-Cycle Animation (4-frame loop), Gem Pickup Sprites (multi-color XP gems), Animated Pipeestrello Enemy Sheet (4 sparse 16x16 frames, mostly blank export), Mad Forest 64x64 Indexed Canopy Tile, Animated Antonio Belpaese Walk Cycle (4 of 8 16x16 frames), Ghost Enemy (+26 more)

### Community 4 - "OAM API & Bug Notes"
Cohesion: 0.10
Nodes (23): main.c Compilation Bugs, OAM_BASE = 256 Slot Offset, oamSlot as u16, oammask Bug in oamSetAttr, Circular Debug Log Buffer, Frame Time Budget (16.7ms, VBlank ~4.2ms), Mini HP Bars for Resistant Enemies, OAM Heart Lives HUD (+15 more)

### Community 5 - "Placeholder Sprite Generator"
Cohesion: 0.10
Nodes (20): bash-defensive-patterns Skill, mvp/gfx/sprites.png base tilesheet, create_palette(), draw_bullet_tile(), draw_enemy_tile(), draw_gem_tile(), draw_hp_bar_tile(), draw_player_tile() (+12 more)

### Community 6 - "Dev Tools & Workflow"
Cohesion: 0.12
Nodes (20): gfx4snes .pic Format Quirk (-u 16), Key Project Files Layout, sprites.asm Wrapper, Aseprite Pixel Art Editor, gfx4snes Asset Converter, GIMP Alternative Image Editor, placeholder-sprites.py Generator, smconv Impulse Tracker to Soundbank (+12 more)

### Community 7 - "Bash Defensive Patterns"
Cohesion: 0.17
Nodes (19): Bash Defensive Patterns Skill, Robust Argument Parsing, Safe Array Handling, Bash Best Practices Summary, Safe Command Substitution, Conditional Safety ([[ ]] vs [ ]), Dependency Checking with command -v, Dry-Run Support (+11 more)

### Community 8 - "PVSnesLib Toolchain Setup"
Cohesion: 0.21
Nodes (16): PVSNESlib Toolchain (816-tcc, gfx4snes), hdr.asm Bugs and Fixes, setup-snesdev.sh Bugs and Fixes, Compile PVSNESlib from Source, glibc >= 2.34 Binary Requirement, 816-tcc + wla-dx Toolchain (not cc65), PVSNESlib Official Repository, SNESLab / PVSNESlib Discord (+8 more)

### Community 9 - "Enemy Bestiary"
Cohesion: 0.19
Nodes (17): Ghost Enemy Concept (floating spectral enemy), Mudman Enemy Concept (mud/slime creature enemy), Pipeestrello Enemy Concept (arc-flying bat enemy), Skeleton Enemy Concept (walking undead bone enemy), Zombie Enemy Concept (shambling undead enemy), Animated Ghost Enemy (translucent cyan wisp), Animated Mudman Enemy (bulky gray-brown humanoid), Animated Mudman Enemy Variant 2 (green blob with tentacle arms) (+9 more)

### Community 10 - "Weapon Sprite Bank"
Cohesion: 0.16
Nodes (15): ataques/ Weapon & Trigger Sprite Bank (4-frame indexed animated sheets), Axe Weapon Sprite Sheet (4 frames, rotated crescent axe head on haft), Garlic Aura Sprite Sheet (4 frames, pale bulb glow), Pickup Items Sprite Sheet (4 frames, tiny pale-yellow item/pickup glyphs), King Bible Weapon Sprite Sheet (4 frames, blue book with white cross), Magic Wand Weapon Sprite Sheet (4 frames, cyan orb on short rod with sparkle), Santa Water Flask Sprite Sheet (4 frames, blue bottle/lobbed projectile), Whip Weapon Sprite Sheet (4 frames, white lash arc sweeping horizontally) (+7 more)

### Community 11 - "Playable Cast Art"
Cohesion: 0.31
Nodes (14): 16-bit Console-Style Pixel Art Character Sprite (transparent background, limited palette), Antonio Belpaese (playable character, whip wielder), Dommario (playable character, hooded robed elder), Imelda Belpaese (playable character, magic caster), Lama Ladonna (playable character, armored sword fighter), 120px Animated Dommario (120x120, 4-frame GIF), 120px Sprite Antonio Belpaese (120x120 PNG), 120px Sprite Dommario (120x120 PNG) (+6 more)

### Community 12 - "HUD & Text Rendering"
Cohesion: 0.21
Nodes (11): u8, consoleDrawTextColored(), drawNum(), drawTime(), u8, countOwnedWeapons(), levelUpMenu(), txt_pal_adr Palette Swap Technique (+3 more)

### Community 13 - "Menus & Video Modes"
Cohesion: 0.29
Nodes (7): Character Select Screen, Fade In/Out via INIDISP $2100, Main Menu (JUGAR/CONTROLES/CREDITOS), Minute 5 Background Transition, bgInitSet Background Init, DMA Copy Functions, setMode Video Modes

### Community 14 - "Mednafen Emulator"
Cohesion: 0.67
Nodes (3): Build and Run Shortcuts, Mednafen Keyboard to SNES Mapping, mednafen CLI Emulator

## Ambiguous Edges - Review These
- `Pickup Items Sprite Sheet (4 frames, tiny pale-yellow item/pickup glyphs)` → `Santa Water Weapon Concept (thrown flask leaving damaging pools)`  [AMBIGUOUS]
  mvp/gfx/ataques/Animated_Items_indexed.png · relation: conceptually_related_to

## Knowledge Gaps
- **59 isolated node(s):** `run.sh script`, `PVSNESLIB_HOME`, `Build and Run Shortcuts`, `Mednafen Keyboard to SNES Mapping`, `Key Project Files Layout` (+54 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 126 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **17 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `Pickup Items Sprite Sheet (4 frames, tiny pale-yellow item/pickup glyphs)` and `Santa Water Weapon Concept (thrown flask leaving damaging pools)`?**
  _Edge tagged AMBIGUOUS (relation: conceptually_related_to) - confidence is low._
- **Why does `PVSNESLIB_HOME Environment Variable` connect `Dev Tools & Workflow` to `PVSnesLib Toolchain Setup`?**
  _High betweenness centrality (0.130) - this node is a cross-community bridge._
- **Why does `gfx4snes Asset Converter` connect `Dev Tools & Workflow` to `OAM API & Bug Notes`?**
  _High betweenness centrality (0.128) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `Bash Defensive Patterns Skill` (e.g. with `run.sh` and `setup-snesdev.sh`) actually correct?**
  _`Bash Defensive Patterns Skill` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Are the 5 inferred relationships involving `updateWater()` (e.g. with `updateWand()` and `Per-weapon level scaling pattern (lvl -> damage/cooldown/count)`) actually correct?**
  _`updateWater()` has 5 INFERRED edges - model-reasoned connections that need verification._
- **Are the 4 inferred relationships involving `gfx4snes Asset Converter` (e.g. with `sprites.asm Wrapper` and `GIMP Alternative Image Editor`) actually correct?**
  _`gfx4snes Asset Converter` has 4 INFERRED edges - model-reasoned connections that need verification._
- **What connects `run.sh script`, `PVSNESLIB_HOME`, `Build and Run Shortcuts` to the rest of the system?**
  _59 weakly-connected nodes found - possible documentation gaps or missing edges._