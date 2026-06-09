#include "enemies.h"

void initEnemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].oamSlot = OAM_ENEMY(i);
    }
}

void spawnEnemy(void) {
    u8 i, side, type;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            side = snes_vblank_count & 3;
            /* Oleadas por tipo segun wave */
            if (wave == 0) type = 0;                           // solo rojos
            else if (wave == 1) type = (snes_vblank_count & 1) ? 0 : 1; // rojo+morado
            else if (wave == 2) type = (snes_vblank_count & 1) ? 1 : 2; // morado+naranja
            else if (wave == 3) type = (snes_vblank_count & 1) ? 2 : 3; // naranja+verde
            else type = (snes_vblank_count & 3);               // mezcla de todos
            
            switch (side) {
                case 0:
                    enemies[i].x = cameraX + (snes_vblank_count % (SCREEN_W - 16)) + 8;
                    enemies[i].y = cameraY - 8;
                    break;
                case 1:
                    enemies[i].x = cameraX + SCREEN_W + 8;
                    enemies[i].y = cameraY + (snes_vblank_count % (SCREEN_H - 16)) + 8;
                    break;
                case 2:
                    enemies[i].x = cameraX + (snes_vblank_count % (SCREEN_W - 16)) + 8;
                    enemies[i].y = cameraY + SCREEN_H + 8;
                    break;
                case 3:
                    enemies[i].x = cameraX - 8;
                    enemies[i].y = cameraY + (snes_vblank_count % (SCREEN_H - 16)) + 8;
                    break;
            }
            
            enemies[i].active = 1;
            enemies[i].hp = enemyHP[type];
            enemies[i].vx = enemySpeed[type];
            enemies[i].vy = type;  // guardar tipo para el tile
            return;
        }
    }
}

void updateEnemies(void) {
    u8 i;
    s16 dx, dy;
    u8 frameOdd = frameCount & 1;
    
    for (i = frameOdd; i < MAX_ENEMIES; i += 2) {
        if (!enemies[i].active) continue;
        
        /* Movimiento hacia el jugador */
        dx = player.x - enemies[i].x;
        dy = player.y - enemies[i].y;
        
        if (dx > 0) enemies[i].x += (enemies[i].vx << 1);
        else if (dx < 0) enemies[i].x -= (enemies[i].vx << 1);
        
        if (dy > 0) enemies[i].y += (enemies[i].vx << 1);
        else if (dy < 0) enemies[i].y -= (enemies[i].vx << 1);
        
        /* Si sale muy lejos del jugador, desactivar */
        s16 distPlayerX = dx < 0 ? -dx : dx;
        s16 distPlayerY = dy < 0 ? -dy : dy;
        if (distPlayerX > 200 || distPlayerY > 200) {
            enemies[i].active = 0;
            continue;
        }
        
        /* Colisión con jugador */
        if (distPlayerX <= 12 && distPlayerY <= 12) {
            enemies[i].active = 0;
            if (player.lives > 0) player.lives--;
        }
    }
}

void renderEnemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        u8 slot = enemies[i].oamSlot;
        if (!enemies[i].active) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_SMALL, OBJ_HIDE);
            continue;
        }
        s16 screenX = enemies[i].x - cameraX;
        s16 screenY = enemies[i].y - cameraY;
        
        if (screenX < -16 || screenX > SCREEN_W + 16 ||
            screenY < -16 || screenY > SCREEN_H + 16) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_SMALL, OBJ_HIDE);
            continue;
        }
        
        if (enemies[i].vy == 0) {
            // Primer enemigo (Type 0): Pipeestrello 16x16 animado
            u8 animFrame = (frameCount / 8) & 3;
            u16 animTileBase = 256;
            u16 tileIndex = animTileBase + (animFrame * 2);
            
            oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, OBJ_PRIO(3) | OBJ_PAL(1));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else {
            // Otros enemigos: 8x8 estáticos
            oamSetAttr(slot, screenX - 4, screenY - 4,
                enemyTile[enemies[i].vy], OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_SMALL, OBJ_SHOW);
        }
    }
}

u8 findNearestEnemy(void) {
    u8 i, nearest = MAX_ENEMIES;
    s16 bestDist = 32767;
    s16 dx, dy, dist;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        dx = enemies[i].x - player.x;
        dy = enemies[i].y - player.y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        dist = dx + dy;  // Manhattan distance
        
        if (dist < bestDist) {
            bestDist = dist;
            nearest = i;
        }
    }
    return nearest;
}
