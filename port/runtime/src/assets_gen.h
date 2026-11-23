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

#define ANIM_FRAME_TILE_STRIDE 4
#define ANIM_OBJ_SIZE 32
#define ANIM_OAM_PROFILE OBJ_SIZE16_L32
#define HP_BAR_TILE_BASE 32
#define HP_BAR_TILE_STRIDE 2
#define TILE_BULLET 42

#define TILEBASE_Pipeestrello 128
#define TILEBASE_Antonio_Belpaese 192
#define TILEBASE_Zombie 256
#define TILEBASE_Skeleton 320
#define TILEBASE_Ghost 384
#define TILEBASE_Mudman 448
#define TILEBASE_Whip 64
#define TILEBASE_Items 96
#define TILEBASE_King_Bible (96 + 4)
#define TILEBASE_Axe (96 + 6)
#define TILEBASE_Garlic (96 + 8)
#define TILEBASE_Magic_Wand (96 + 10)
#define TILEBASE_Santa_Water (96 + 12)
#define TILEBASE_Experience_Gem (96 + 0)
#define PLAYER_TILE_BASE 192
#define PLAYER_PALETTE 2
#define WEAPONS_PALETTE 3
#define ITEMS_PALETTE 4
#define TILE_ICON_EXPERIENCE_GEM 0
#define TILE_ICON_WHIP 2
#define TILE_ICON_KING_BIBLE 4
#define TILE_ICON_AXE 6
#define TILE_ICON_GARLIC 8
#define TILE_ICON_MAGIC_WAND 10
#define TILE_ICON_SANTA_WATER 12

#define ENEMY_TYPES 5

extern const u16 enemyTileBase[ENEMY_TYPES];
extern const u8 enemyPalette[ENEMY_TYPES];
extern const u8 enemyHP[ENEMY_TYPES];
extern const u8 enemySpeed[ENEMY_TYPES];
extern const u8 enemyScore[ENEMY_TYPES];
extern const u8 enemyAnimSpeed[ENEMY_TYPES];

void loadAllSprites(void);

#endif
