#include "player.h"
#include "enemies.h"
#include "bullets.h"

void updatePlayer(void) {
    u16 pad = padsCurrent(0);
    
    /* Movimiento */
    if (pad & KEY_UP)    player.y -= player.speed;
    if (pad & KEY_DOWN)  player.y += player.speed;
    if (pad & KEY_LEFT)  { player.x -= player.speed; player.facingLeft = 1; }
    if (pad & KEY_RIGHT) { player.x += player.speed; player.facingLeft = 0; }
    
    /* Límites del mapa */
    if (player.x < 8)     player.x = 8;
    if (player.x > MAP_W - 8)  player.x = MAP_W - 8;
    if (player.y < 8)     player.y = 8;
    if (player.y > MAP_H - 8)  player.y = MAP_H - 8;
    
    if (player.invincibilityTimer > 0) {
        player.invincibilityTimer--;
    }
}

void renderPlayer(void) {
    s16 screenX = player.x - cameraX;
    s16 screenY = player.y - cameraY;
    
    // Tile base del jugador (autogenerado desde assets.json)
    u16 antonioTileBase = PLAYER_TILE_BASE;
    
    u16 pad = padsCurrent(0);
    u8 isMoving = (pad & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) ? 1 : 0;
    u8 animDiv = isMoving ? 6 : 12; // Animación rápida (6) en movimiento, lenta (12) en reposo
    u8 animFrame = (frameCount / animDiv) & 3;
    u16 tileIndex = antonioTileBase + (animFrame * 2);
    
    u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(PLAYER_PALETTE);
    if (player.facingLeft) {
        oamAttr |= OBJ_FLIPX;
    }
    
    // Flashing effect when invincible
    if (player.invincibilityTimer > 0 && (frameCount & 4)) {
        oamSetAttr(OAM_PLAYER, 0, 240, 0, 0);
    } else {
        oamSetAttr(OAM_PLAYER, screenX - 8, screenY - 8, tileIndex, oamAttr);
    }
    oamSetEx(OAM_PLAYER, OBJ_LARGE, OBJ_SHOW);

    // Render HP bar sprite under player feet (only if alive)
    if (player.lives > 0 && player.lives <= 5) {
        u16 tileHp = 37 + (5 - player.lives);
        oamSetAttr(OAM_HP_BAR, screenX - 4, screenY + 9, tileHp, OBJ_PRIO(3) | OBJ_PAL(0));
    } else {
        oamSetAttr(OAM_HP_BAR, 0, 240, 0, 0);
    }
}
