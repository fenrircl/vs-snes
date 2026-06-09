#include "player.h"
#include "enemies.h"
#include "bullets.h"

void updatePlayer(void) {
    u16 pad = padsCurrent(0);
    
    /* Movimiento */
    if (pad & KEY_UP)    player.y -= PLAYER_SPEED;
    if (pad & KEY_DOWN)  player.y += PLAYER_SPEED;
    if (pad & KEY_LEFT)  { player.x -= PLAYER_SPEED; player.facingLeft = 1; }
    if (pad & KEY_RIGHT) { player.x += PLAYER_SPEED; player.facingLeft = 0; }
    
    /* Límites del mapa */
    if (player.x < 8)     player.x = 8;
    if (player.x > MAP_W - 8)  player.x = MAP_W - 8;
    if (player.y < 8)     player.y = 8;
    if (player.y > MAP_H - 8)  player.y = MAP_H - 8;
    
    if (player.shootTimer > 0) {
        player.shootTimer--;
    } else {
        u8 target = findNearestEnemy();
        if (target < MAX_ENEMIES) {
            fireBullet(enemies[target].x, enemies[target].y);
        }
        player.shootTimer = SHOOT_COOLDOWN;
    }
}

void renderPlayer(void) {
    s16 screenX = player.x - cameraX;
    s16 screenY = player.y - cameraY;
    
    // Calcular el tile base para Antonio Belpaese
    u16 baseSpritesSize = &sprites_tilend - &sprites_til;
    u16 batSpritesSize = &Animated_Pipeestrello_indexed_tilend - &Animated_Pipeestrello_indexed_til;
    u16 antonioTileBase = (baseSpritesSize + batSpritesSize) / 32;
    
    u8 animFrame = (frameCount / 8) & 3;
    u16 tileIndex = antonioTileBase + (animFrame * 2);
    
    u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(2);
    if (player.facingLeft) {
        oamAttr |= OBJ_FLIPX;
    }
    
    oamSetAttr(OAM_PLAYER, screenX - 8, screenY - 8, tileIndex, oamAttr);
    oamSetEx(OAM_PLAYER, OBJ_LARGE, OBJ_SHOW);
}
