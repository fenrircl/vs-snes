// AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR
#include "common.h"
#include "assets_gen.h"

const u16 enemyTileBase[ENEMY_TYPES] = { 256, 320, 352, 384, 416 };
const u8 enemyPalette[ENEMY_TYPES] = { 1, 1, 1, 1, 1 };
const u8 enemyHP[ENEMY_TYPES] = { 1, 2, 3, 1, 4 };
const u8 enemySpeed[ENEMY_TYPES] = { 1, 1, 1, 2, 1 };
const u8 enemyScore[ENEMY_TYPES] = { 10, 20, 30, 15, 40 };
const u8 enemyAnimSpeed[ENEMY_TYPES] = { 6, 12, 8, 10, 10 };

void loadAllSprites(void) {
    /* Tiles de sprites -> VRAM */
    dmaCopyVram(&Animated_Pipeestrello_indexed_til, 4096, 1024);
    dmaCopyVram(&Animated_Antonio_Belpaese_indexed_til, 4608, 1024);
    dmaCopyVram(&Animated_Zombie_indexed_til, 5120, 1024);
    dmaCopyVram(&Animated_Skeleton_indexed_til, 5632, 1024);
    dmaCopyVram(&Animated_Ghost_indexed_til, 6144, 1024);
    dmaCopyVram(&Animated_Mudman_indexed_til, 6656, 1024);
    dmaCopyVram(&Animated_Whip_indexed_til, 7168, 1024);
    dmaCopyVram(&Animated_Items_indexed_til, 7680, 1024);

    /* Paletas compartidas por grupo -> CGRAM */
    dmaCopyCGram(&Animated_Pipeestrello_indexed_pal, 144, 32);  /* grupo enemies (slot 1) */
    dmaCopyCGram(&Animated_Zombie_indexed_pal, 144, 32);  /* grupo enemies (slot 1) */
    dmaCopyCGram(&Animated_Skeleton_indexed_pal, 144, 32);  /* grupo enemies (slot 1) */
    dmaCopyCGram(&Animated_Ghost_indexed_pal, 144, 32);  /* grupo enemies (slot 1) */
    dmaCopyCGram(&Animated_Mudman_indexed_pal, 144, 32);  /* grupo enemies (slot 1) */
    dmaCopyCGram(&Animated_Antonio_Belpaese_indexed_pal, 160, 32);  /* grupo player (slot 2) */
    dmaCopyCGram(&Animated_Whip_indexed_pal, 176, 32);  /* grupo weapons (slot 3) */
    dmaCopyCGram(&Animated_Items_indexed_pal, 192, 32);  /* grupo weapons (slot 4) */
}
