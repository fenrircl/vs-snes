#include "player.h"
#include "enemies.h"
#include "bullets.h"

void updatePlayer(void) {
    u16 pad = padsCurrent(0);
    
    /* Movimiento */
    if (pad & KEY_UP)    player.y -= PLAYER_SPEED;
    if (pad & KEY_DOWN)  player.y += PLAYER_SPEED;
    if (pad & KEY_LEFT)  player.x -= PLAYER_SPEED;
    if (pad & KEY_RIGHT) player.x += PLAYER_SPEED;
    
    /* Límites de pantalla */
    if (player.x < 8)     player.x = 8;
    if (player.x > SCREEN_W - 8)  player.x = SCREEN_W - 8;
    if (player.y < 8)     player.y = 8;
    if (player.y > SCREEN_H - 8)  player.y = SCREEN_H - 8;
    
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
    oamSetAttr(OAM_PLAYER, player.x - 8, player.y - 8, TILE_PLAYER,
        OBJ_PRIO(3) | OBJ_PAL(0));
}
