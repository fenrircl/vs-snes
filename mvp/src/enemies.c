#include "enemies.h"

void initEnemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].oamSlot = OAM_ENEMY(i);
    }
}

/* enemyTileBase[] y enemyPalette[] se generan en assets_gen.c (desde assets.json) */

void spawnEnemy(void) {
    u8 i, side, type;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            u16 randOffset = (snes_vblank_count * 17) + (i * 31);

            /* Seleccionar tipo de enemigo según el minuto de juego (hasta el minuto 10) */
            if (gameMins == 0) {
                type = 0; // Bat only
            } else if (gameMins == 1) {
                type = ((randOffset % 10) < 3) ? 0 : 1; // Bat (30%), Zombie (70%)
            } else if (gameMins == 2) {
                type = ((randOffset % 10) < 6) ? 1 : 2; // Zombie (60%), Skeleton (40%)
            } else if (gameMins == 3) {
                type = ((randOffset % 10) < 4) ? 2 : 3; // Skeleton (40%), Ghost (60%)
            } else if (gameMins == 4) {
                u16 r = randOffset % 10;
                if (r < 3) type = 1;      // Zombie (30%)
                else if (r < 6) type = 2; // Skeleton (30%)
                else if (r < 9) type = 3; // Ghost (30%)
                else type = 4;            // Mudman (10%)
            } else if (gameMins == 5) {
                type = ((randOffset % 10) < 5) ? 3 : 4; // Ghost (50%), Mudman (50%)
            } else if (gameMins == 6) {
                type = ((randOffset % 10) < 6) ? 1 : 2; // Zombie (60%), Skeleton (40%)
            } else if (gameMins == 7) {
                type = 3; // Ghost swarm
            } else if (gameMins == 8) {
                type = ((randOffset % 10) < 2) ? 0 : 4; // Bat (20%), Mudman (80%)
            } else {
                u16 r = randOffset % 10;
                if (r < 2) type = 0;
                else if (r < 4) type = 1;
                else if (r < 6) type = 2;
                else if (r < 8) type = 3;
                else type = 4;
            }
            
            /* Elegir un lado off-screen (relativo a la cámara) que caiga DENTRO del mapa.
               Reject-and-retry en vez de clampear: clampear arrastraba el enemigo dentro
               de la pantalla / encima del player cerca de los bordes -> "aparecía y
               desaparecía". Si ningún lado es válido, no spawnear este frame. */
            {
                u8 t;
                s16 sx = 0, sy = 0;
                u8 placed = 0;
                for (t = 0; t < 4 && !placed; t++) {
                    side = (randOffset + t) & 3;
                    switch (side) {
                        case 0: /* arriba */
                            sx = cameraX + (randOffset % (SCREEN_W - 16)) + 8;
                            sy = cameraY - 8;
                            break;
                        case 1: /* derecha */
                            sx = cameraX + SCREEN_W + 8;
                            sy = cameraY + (randOffset % (SCREEN_H - 16)) + 8;
                            break;
                        case 2: /* abajo */
                            sx = cameraX + (randOffset % (SCREEN_W - 16)) + 8;
                            sy = cameraY + SCREEN_H + 8;
                            break;
                        case 3: /* izquierda */
                            sx = cameraX - 8;
                            sy = cameraY + (randOffset % (SCREEN_H - 16)) + 8;
                            break;
                    }
                    if (sx >= 8 && sx <= MAP_W - 8 && sy >= 8 && sy <= MAP_H - 8) {
                        placed = 1;
                    }
                }
                if (!placed) return;  /* ningún lado off-screen cae dentro del mapa */
                enemies[i].x = sx;
                enemies[i].y = sy;
            }

            enemies[i].active = 1;
            enemies[i].hp = enemyHP[type];
            enemies[i].vx = enemySpeed[type];
            enemies[i].vy = type;  // Guardar tipo
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
        
        u8 speedStep = enemies[i].vx;
        // Zombie y Mudman se mueven a la mitad de velocidad (1 pixel cada 4 frames)
        if (enemies[i].vy == 1 || enemies[i].vy == 4) {
            if (frameCount & 2) speedStep = 0;
        }
        
        if (speedStep > 0) {
            if (dx > 0) {
                enemies[i].x += speedStep;
                enemies[i].unused1 = 0; // Mirando a la derecha
            }
            else if (dx < 0) {
                enemies[i].x -= speedStep;
                enemies[i].unused1 = 1; // Mirando a la izquierda (flip)
            }
            
            if (dy > 0) enemies[i].y += speedStep;
            else if (dy < 0) enemies[i].y -= speedStep;
        }
        
        /* Si sale muy lejos del jugador, desactivar */
        s16 distPlayerX = dx < 0 ? -dx : dx;
        s16 distPlayerY = dy < 0 ? -dy : dy;
        if (distPlayerX > 280 || distPlayerY > 280) {
            enemies[i].active = 0;
            continue;
        }
        
        /* Colisión con jugador */
        if (distPlayerX <= 12 && distPlayerY <= 12) {
            enemies[i].active = 0;
            if (player.invincibilityTimer == 0) {
                if (player.lives > 0) player.lives--;
                player.invincibilityTimer = 30; // 0.5s de invencibilidad
            }
        }
    }
}

void renderEnemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        u16 slot = enemies[i].oamSlot;
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
        u8 animDiv = enemyAnimSpeed[enemies[i].vy];
        // Desfase de animación usando el índice del enemigo i
        u8 animFrame = ((frameCount + (i * 3)) / animDiv) & 3;
        
        u16 animTileBase = enemyTileBase[enemies[i].vy];
        u16 tileIndex = animTileBase + (animFrame * 2);
        u8 palette = enemyPalette[enemies[i].vy];
        
        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(palette);
        if (enemies[i].unused1) {
            oamAttr |= OBJ_FLIPX;
        }
        
        oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, oamAttr);
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

u8 findSecondNearestEnemy(u8 skip) {
    u8 i, nearest = MAX_ENEMIES;
    s16 bestDist = 32767;
    s16 dx, dy, dist;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active || i == skip) continue;
        
        dx = enemies[i].x - player.x;
        dy = enemies[i].y - player.y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        dist = dx + dy;
        
        if (dist < bestDist) {
            bestDist = dist;
            nearest = i;
        }
    }
    return nearest;
}

u8 findThirdNearestEnemy(u8 skip1, u8 skip2) {
    u8 i, nearest = MAX_ENEMIES;
    s16 bestDist = 32767;
    s16 dx, dy, dist;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active || i == skip1 || i == skip2) continue;
        
        dx = enemies[i].x - player.x;
        dy = enemies[i].y - player.y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        dist = dx + dy;
        
        if (dist < bestDist) {
            bestDist = dist;
            nearest = i;
        }
    }
    return nearest;
}
