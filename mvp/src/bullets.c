#include "bullets.h"
#include "gems.h"

void initBullets(void) {
    u8 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
        bullets[i].oamSlot = OAM_BULLET(i);
    }
}

void fireBullet(s16 targetX, s16 targetY) {
    u8 i;
    s16 dx, dy, adx, ady;
    
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x;
            bullets[i].y = player.y;
            
            dx = targetX - player.x;
            dy = targetY - player.y;
            adx = dx; if (adx < 0) adx = -adx;
            ady = dy; if (ady < 0) ady = -ady;
            
            /* Velocidad proporcional a la distancia en cada eje */
            if (adx + ady > 0) {
                bullets[i].vx = (dx * player.bulletSpeed) / (adx + ady);
                bullets[i].vy = (dy * player.bulletSpeed) / (adx + ady);
            } else {
                bullets[i].vx = 0;
                bullets[i].vy = -player.bulletSpeed;
            }
            
            bullets[i].active = 1;
            return;
        }
    }
}

void updateBullets(void) {
    u8 i;
    
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        /* Movimiento */
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        
        /* Fuera de cámara → desactivar */
        s16 screenX = bullets[i].x - cameraX;
        s16 screenY = bullets[i].y - cameraY;
        if (screenX < -16 || screenX > SCREEN_W + 16 ||
            screenY < -16 || screenY > SCREEN_H + 16) {
            bullets[i].active = 0;
            continue;
        }
        
        /* Colisión con enemigos */
        u8 j;
        for (j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            
            s16 dx = bullets[i].x - enemies[j].x;
            if (dx < -8 || dx > 8) continue;
            
            s16 dy = bullets[i].y - enemies[j].y;
            if (dy < -8 || dy > 8) continue;
            
            u8 damage = 1;
            if (player.cooldownLvl == 2) damage = 2;
            else if (player.cooldownLvl == 3) damage = 2;
            else if (player.cooldownLvl == 4) damage = 3;
            else if (player.cooldownLvl >= 5) damage = 4;
            
            bullets[i].active = 0;
            if (enemies[j].hp > damage) {
                enemies[j].hp -= damage;
            } else {
                enemies[j].active = 0;
                player.score += enemyScore[enemies[j].vy];
                player.kills++;
                spawnGem(enemies[j].x, enemies[j].y);
            }
            break;
        }
    }
}

void renderBullets(void) {
    u8 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        u16 slot = bullets[i].oamSlot;
        if (!bullets[i].active) {
            oamSetAttr(slot, 0, 240, 0, 0);
            continue;
        }
        s16 screenX = bullets[i].x - cameraX;
        s16 screenY = bullets[i].y - cameraY;
        oamSetAttr(slot, screenX - 4, screenY - 4, TILE_BULLET,
            OBJ_PRIO(3) | OBJ_PAL(0));
    }
}
