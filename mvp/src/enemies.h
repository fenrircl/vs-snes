#ifndef ENEMIES_H
#define ENEMIES_H

#include "common.h"

void initEnemies(void);
void spawnEnemy(void);
void updateEnemies(void);
void renderEnemies(void);
u8 findNearestEnemy(void);

#endif /* ENEMIES_H */
