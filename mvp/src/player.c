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
    if (player.x < 16)    player.x = 16;
    if (player.x > MAP_W - 16) player.x = MAP_W - 16;
    if (player.y < 16)    player.y = 16;
    if (player.y > MAP_H - 16) player.y = MAP_H - 16;
    
    if (player.invincibilityTimer > 0) {
        player.invincibilityTimer--;
    }
}

void renderPlayer(void) {
    s16 screenX = CAMERA_X(player.x);
    s16 screenY = CAMERA_Y(player.y);
    
    // Tile base del jugador (autogenerado desde assets.json)
    u16 antonioTileBase = PLAYER_TILE_BASE;
    
    u16 pad = padsCurrent(0);
    u8 isMoving = (pad & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) ? 1 : 0;
    u8 animDiv = isMoving ? 6 : 12; // Animación rápida (6) en movimiento, lenta (12) en reposo
    u8 animFrame = (frameCount / animDiv) & 3;
    u16 tileIndex = antonioTileBase + (animFrame * ANIM_FRAME_TILE_STRIDE);
    
    u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(PLAYER_PALETTE);
    if (player.facingLeft) {
        oamAttr |= OBJ_FLIPX;
    }
    
    // Flashing effect when invincible
    if (player.invincibilityTimer > 0 && (frameCount & 4)) {
        oamSetAttr(OAM_PLAYER, 0, 240, 0, 0);
    } else {
        oamSetEx(OAM_PLAYER, OBJ_LARGE, OBJ_SHOW);
        oamSetAttr(OAM_PLAYER, screenX - (ANIM_OBJ_SIZE / 2),
                   screenY - (ANIM_OBJ_SIZE / 2), tileIndex, oamAttr);
    }

    // Render HP bar sprite under player feet (only if alive)
    if (player.lives > 0 && player.lives <= 5) {
        u16 tileHp = HP_BAR_TILE_BASE + ((player.lives - 1) * HP_BAR_TILE_STRIDE);
        oamSetEx(OAM_HP_BAR, OBJ_SMALL, OBJ_SHOW);
        oamSetAttr(OAM_HP_BAR, screenX - 8, screenY + 11, tileHp,
                   OBJ_PRIO(3) | OBJ_PAL(0));
    } else {
        oamSetAttr(OAM_HP_BAR, 0, 240, 0, 0);
        oamSetEx(OAM_HP_BAR, OBJ_SMALL, OBJ_HIDE);
    }
}
