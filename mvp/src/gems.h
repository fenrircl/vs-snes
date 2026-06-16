#ifndef GEMS_H
#define GEMS_H

#include "common.h"

#define TILE_GEM 34
#define GEM_MAGNET_DIST 40
#define GEM_COLLECT_DIST 8

void initGems(void);
void spawnGem(s16 x, s16 y);
void updateGems(void);
void renderGems(void);

#endif /* GEMS_H */
