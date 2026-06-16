// AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR
#ifndef ASSETS_GEN_H
#define ASSETS_GEN_H

#include <snes.h>

extern u8 Animated_Pipeestrello_indexed_til, Animated_Pipeestrello_indexed_tilend;
extern u8 Animated_Pipeestrello_indexed_pal, Animated_Pipeestrello_indexed_palend;
extern u8 Animated_Antonio_Belpaese_indexed_til, Animated_Antonio_Belpaese_indexed_tilend;
extern u8 Animated_Antonio_Belpaese_indexed_pal, Animated_Antonio_Belpaese_indexed_palend;
extern u8 Animated_Zombie_indexed_til, Animated_Zombie_indexed_tilend;
extern u8 Animated_Zombie_indexed_pal, Animated_Zombie_indexed_palend;
extern u8 Animated_Skeleton_indexed_til, Animated_Skeleton_indexed_tilend;
extern u8 Animated_Skeleton_indexed_pal, Animated_Skeleton_indexed_palend;
extern u8 Animated_Ghost_indexed_til, Animated_Ghost_indexed_tilend;
extern u8 Animated_Ghost_indexed_pal, Animated_Ghost_indexed_palend;
extern u8 Animated_Mudman_indexed_til, Animated_Mudman_indexed_tilend;
extern u8 Animated_Mudman_indexed_pal, Animated_Mudman_indexed_palend;
extern u8 Animated_Whip_indexed_til, Animated_Whip_indexed_tilend;
extern u8 Animated_Whip_indexed_pal, Animated_Whip_indexed_palend;
extern u8 Animated_Items_indexed_til, Animated_Items_indexed_tilend;
extern u8 Animated_Items_indexed_pal, Animated_Items_indexed_palend;

#define TILEBASE_Pipeestrello 256
#define TILEBASE_Antonio_Belpaese 288
#define TILEBASE_Zombie 320
#define TILEBASE_Skeleton 352
#define TILEBASE_Ghost 384
#define TILEBASE_Mudman 416
#define TILEBASE_Whip 448
#define TILEBASE_Items 480
#define TILEBASE_King_Bible (480 + 0)
#define TILEBASE_Axe (480 + 2)
#define TILEBASE_Garlic (480 + 4)
#define TILEBASE_Magic_Wand (480 + 6)
#define TILEBASE_Santa_Water (480 + 8)
#define TILEBASE_Experience_Gem (480 + 10)
#define PLAYER_TILE_BASE 288
#define PLAYER_PALETTE 2
#define WEAPONS_PALETTE 3
#define ITEMS_PALETTE 4
#define TILE_ICON_EXPERIENCE_GEM 34
#define TILE_ICON_WHIP 64
#define TILE_ICON_KING_BIBLE 68
#define TILE_ICON_AXE 72
#define TILE_ICON_GARLIC 76
#define TILE_ICON_MAGIC_WAND 96
#define TILE_ICON_SANTA_WATER 100

#define ENEMY_TYPES 5

extern const u16 enemyTileBase[ENEMY_TYPES];
extern const u8 enemyPalette[ENEMY_TYPES];
extern const u8 enemyHP[ENEMY_TYPES];
extern const u8 enemySpeed[ENEMY_TYPES];
extern const u8 enemyScore[ENEMY_TYPES];
extern const u8 enemyAnimSpeed[ENEMY_TYPES];

void loadAllSprites(void);

#endif
