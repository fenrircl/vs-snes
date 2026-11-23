#ifndef ENEMIES_H
#define ENEMIES_H

#include "common.h"

void initEnemies(void);
void spawnEnemy(void);
void updateEnemies(void);
void renderEnemies(void);
u8 findNearestEnemy(void);
u8 findSecondNearestEnemy(u8 skip);
u8 findThirdNearestEnemy(u8 skip1, u8 skip2);

#endif /* ENEMIES_H */
