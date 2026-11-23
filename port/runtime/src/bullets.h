#ifndef BULLETS_H
#define BULLETS_H

#include "common.h"

void initBullets(void);
void fireBullet(s16 targetX, s16 targetY);
void updateBullets(void);
void renderBullets(void);

#endif /* BULLETS_H */
