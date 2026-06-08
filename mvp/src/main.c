/*
 * VS-SNES MVP — Template base
 * 
 * Personaje que se mueve + pool de enemigos + colisiones
 * 
 * Para compilar:
 *   export PVSNESLIB_HOME=~/snesdev/pvsneslib
 *   make
 *   mednafen vs-snes-mvp.sfc
 */

#include <snes.h>

/* ============================================================
 *  CONSTANTES
 * ============================================================ */

#define SCREEN_W       256
#define SCREEN_H       224
#define PLAYER_SPEED    2
#define ENEMY_SPEED     1
#define MAX_ENEMIES     64
#define MAX_BULLETS     16
#define SHOOT_COOLDOWN  12    // frames entre disparos
#define SPAWN_COOLDOWN  30    // frames entre spawn de enemigos
#define BULLET_SPEED    3
#define KILL_SCORE      10

/* Tamaño de sprites en tiles */
#define TILE_8X8  0
#define TILE_16X16 4

/* ============================================================
 *  ESTRUCTURAS
 * ============================================================ */

typedef struct {
    s16 x, y;          // Posición centrada (píxeles)
    s16 vx, vy;        // Velocidad
    u8 active;         // 1 = vivo, 0 = inactivo
    u8 hp;             // Vida / hits que recibe
    u8 animFrame;      // Frame de animación
    u8 oamSlot;        // Slot de OAM asignado
} Entity;

/* Pool de entidades */
Entity enemies[MAX_ENEMIES];
Entity bullets[MAX_BULLETS];

/* Estado del jugador */
typedef struct {
    s16 x, y;
    u8 lives;
    u16 score;
    u8 shootTimer;
} Player;

Player player;

/* Spawn timer */
u8 spawnTimer;

/* Contador de frames para update escalonado */
u8 frameCount;

/* ============================================================
 *  DECLARACIONES DE SPRITES (definidos en sprites.pic)
 * ============================================================ */

extern u8 sprites_pic;     // tiles
extern u8 sprites_pal;     // paleta

/* Offsets de tiles dentro del spritesheet (dependen de tu PNG) */
#define TILE_PLAYER     0   // tile 0-3 = jugador 16x16
#define TILE_ENEMY      4   // tile 4 = enemigo 8x8
#define TILE_BULLET     5   // tile 5 = bala 8x8
#define TILE_GEM        6   // tile 6 = gema 8x8

/* ============================================================
 *  FUNCIONES DE ENTIDADES
 * ============================================================ */

/* Inicializar pool de enemigos */
void initEnemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].oamSlot = 4 + i;  // Slots 4..67 para enemigos
    }
}

/* Inicializar pool de balas */
void initBullets(void) {
    u8 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
        bullets[i].oamSlot = 4 + MAX_ENEMIES + i;  // Slots 68..83 para balas
    }
}

/* Spawnear enemigo desde un borde */
void spawnEnemy(void) {
    u8 i;
    u8 side;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            side = GetTickCount() & 3;  // 0-3: arriba, derecha, abajo, izq
            
            switch (side) {
                case 0: // arriba
                    enemies[i].x = (GetTickCount() % (SCREEN_W - 16)) + 8;
                    enemies[i].y = -8;
                    break;
                case 1: // derecha
                    enemies[i].x = SCREEN_W + 8;
                    enemies[i].y = (GetTickCount() % (SCREEN_H - 16)) + 8;
                    break;
                case 2: // abajo
                    enemies[i].x = (GetTickCount() % (SCREEN_W - 16)) + 8;
                    enemies[i].y = SCREEN_H + 8;
                    break;
                case 3: // izquierda
                    enemies[i].x = -8;
                    enemies[i].y = (GetTickCount() % (SCREEN_H - 16)) + 8;
                    break;
            }
            
            enemies[i].vx = 0;
            enemies[i].vy = 0;
            enemies[i].active = 1;
            enemies[i].hp = 1;
            return;
        }
    }
}

/* Disparar bala desde el jugador hacia objetivo */
void fireBullet(s16 targetX, s16 targetY) {
    u8 i;
    s16 dx, dy;
    
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x;
            bullets[i].y = player.y;
            
            /* Calcular dirección hacia el objetivo */
            dx = targetX - player.x;
            dy = targetY - player.y;
            
            /* Normalización simple (movimiento en 8 direcciones) */
            if (dx > 0) bullets[i].vx = BULLET_SPEED;
            else if (dx < 0) bullets[i].vx = -BULLET_SPEED;
            else bullets[i].vx = 0;
            
            if (dy > 0) bullets[i].vy = BULLET_SPEED;
            else if (dy < 0) bullets[i].vy = -BULLET_SPEED;
            else bullets[i].vy = 0;
            
            bullets[i].active = 1;
            return;
        }
    }
}

/* Encontrar enemigo más cercano */
u8 findNearestEnemy(void) {
    u8 i, nearest = MAX_ENEMIES;  // MAX_ENEMIES = no encontrado
    s16 bestDist = 32000;
    s16 dx, dy, dist;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        dx = enemies[i].x - player.x;
        dy = enemies[i].y - player.y;
        
        /* Distancia al cuadrado (evita sqrt) */
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        dist = (dx * dx) + (dy * dy);
        
        if (dist < bestDist) {
            bestDist = dist;
            nearest = i;
        }
    }
    
    return nearest;
}

/* ============================================================
 *  COLISIONES
 * ============================================================ */

/* Colisión entre dos entidades (AABB simple) */
u8 checkCollision(s16 ax, s16 ay, u8 asize, s16 bx, s16 by, u8 bsize) {
    s16 dx, dy;
    
    dx = ax - bx;
    if (dx < 0) dx = -dx;
    if (dx > (s16)(asize + bsize)) return 0;
    
    dy = ay - by;
    if (dy < 0) dy = -dy;
    if (dy > (s16)(asize + bsize)) return 0;
    
    return 1;  // Colisión!
}

/* ============================================================
 *  UPDATE
 * ============================================================ */

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
    
    /* Disparo automático */
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

void updateEnemies(void) {
    u8 i;
    s16 dx, dy;
    
    /* Update escalonado: solo ~16 enemigos por frame */
    u8 start = (frameCount * 16) % MAX_ENEMIES;
    u8 end = start + 16;
    if (end > MAX_ENEMIES) end = MAX_ENEMIES;
    
    for (i = start; i < end; i++) {
        if (!enemies[i].active) continue;
        
        /* Movimiento hacia el jugador */
        dx = player.x - enemies[i].x;
        dy = player.y - enemies[i].y;
        
        if (dx > 0) enemies[i].x += ENEMY_SPEED;
        else if (dx < 0) enemies[i].x -= ENEMY_SPEED;
        
        if (dy > 0) enemies[i].y += ENEMY_SPEED;
        else if (dy < 0) enemies[i].y -= ENEMY_SPEED;
        
        /* Si sale muy lejos de pantalla, desactivar */
        if (enemies[i].x < -32 || enemies[i].x > SCREEN_W + 32 ||
            enemies[i].y < -32 || enemies[i].y > SCREEN_H + 32) {
            enemies[i].active = 0;
        }
        
        /* Colisión con jugador */
        if (checkCollision(player.x, player.y, 8, enemies[i].x, enemies[i].y, 4)) {
            enemies[i].active = 0;
            player.lives--;
            if (player.lives > 99) player.lives = 0;  // clamp mínimo
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
        
        /* Fuera de pantalla → desactivar */
        if (bullets[i].x < -16 || bullets[i].x > SCREEN_W + 16 ||
            bullets[i].y < -16 || bullets[i].y > SCREEN_H + 16) {
            bullets[i].active = 0;
            continue;
        }
        
        /* Colisión con enemigos */
        u8 j;
        for (j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            
            if (checkCollision(bullets[i].x, bullets[i].y, 4, enemies[j].x, enemies[j].y, 4)) {
                bullets[i].active = 0;
                enemies[j].active = 0;
                player.score += KILL_SCORE;
                break;
            }
        }
    }
}

/* ============================================================
 *  RENDER (sprites)
 * ============================================================ */

void renderPlayer(void) {
    oamSet(
        0,                    // slot 0 — tile sup-izq
        player.x - 8,         // X (top-left)
        player.y - 8,         // Y (top-left)
        OBJ_PRIO(1),          // prioridad
        OBJ_FLIPX(0),         // flip X
        OBJ_FLIPY(0),         // flip Y
        TILE_PLAYER,          // tile offset
        OBJ_PAL(0)            // paleta 0
    );
    
    oamSet(1, player.x, player.y - 8, OBJ_PRIO(1), 0, 0, TILE_PLAYER + 1, OBJ_PAL(0));  // sup-der
    oamSet(2, player.x - 8, player.y, OBJ_PRIO(1), 0, 0, TILE_PLAYER + 2, OBJ_PAL(0));  // inf-izq
    oamSet(3, player.x, player.y, OBJ_PRIO(1), 0, 0, TILE_PLAYER + 3, OBJ_PAL(0));       // inf-der
}

void renderEnemies(void) {
    u8 i;
    
    for (i = 0; i < MAX_ENEMIES; i++) {
        u8 slot = enemies[i].oamSlot;
        
        if (!enemies[i].active) {
            oamSetVisible(slot, OBJ_HIDE);
            continue;
        }
        
        /* Flickering básico: ocultar sprites lejanos en frames alternos */
        s16 dx = player.x - enemies[i].x;
        s16 dy = player.y - enemies[i].y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        u16 dist = (u16)(dx + dy);
        
        if (dist > 300 && (frameCount & 1)) {
            oamSetVisible(slot, OBJ_HIDE);
            continue;
        }
        
        oamSet(
            slot,
            enemies[i].x - 4,
            enemies[i].y - 4,
            OBJ_PRIO(2),
            0, 0,
            TILE_ENEMY,
            OBJ_PAL(1)  // paleta 1 para enemigos
        );
    }
}

void renderBullets(void) {
    u8 i;
    
    for (i = 0; i < MAX_BULLETS; i++) {
        u8 slot = bullets[i].oamSlot;
        
        if (!bullets[i].active) {
            oamSetVisible(slot, OBJ_HIDE);
            continue;
        }
        
        oamSet(
            slot,
            bullets[i].x - 4,
            bullets[i].y - 4,
            OBJ_PRIO(0),
            0, 0,
            TILE_BULLET,
            OBJ_PAL(2)  // paleta 2 para balas
        );
    }
}

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    u8 i;
    
    /* Inicializar consola y pantalla */
    consoleInit();
    
    /* Inicializar sprites */
    oamInit();
    
    /* Cargar tiles y paleta de sprites */
    oamInitGfxSet(
        &sprites_pic,          // tiles
        512,                   // tamaño (512 tiles 8x8 = 128*4 bytes)
        &sprites_pal,          // paleta
        32,                    // 16 colores * 2 bytes
        0,                     // paleta 0
        0,                     // VRAM address 0
        OBJ_SIZE8_L16          // small=8x8, large=16x16
    );
    
    /* Inicializar estado */
    player.x = SCREEN_W / 2;
    player.y = SCREEN_H / 2;
    player.lives = 5;
    player.score = 0;
    player.shootTimer = 0;
    
    initEnemies();
    initBullets();
    spawnTimer = 0;
    frameCount = 0;
    
    /* Mensaje de inicio en consola */
    consoleDrawText(2, 2, "VS-SNES MVP");
    consoleDrawText(2, 3, "Presiona START");
    consoleDrawText(2, 4, "para comenzar");
    
    /* Esperar START */
    while (!(padsDown(0) & KEY_START)) {
        WaitForVBlank();
    }
    
    consoleClear();
    
    /* ============================================
     *  BUCLE PRINCIPAL DEL JUEGO
     * ============================================ */
    while(1) {
        /* INPUT */
        updatePlayer();
        
        /* SPAWN */
        if (spawnTimer == 0) {
            spawnEnemy();
            spawnTimer = SPAWN_COOLDOWN;
            /* Acelerar spawn con el score */
            if (player.score > 100) spawnTimer = 20;
            if (player.score > 300) spawnTimer = 12;
            if (player.score > 500) spawnTimer = 8;
        } else {
            spawnTimer--;
        }
        
        /* UPDATE */
        updateEnemies();
        updateBullets();
        
        /* RENDER */
        renderPlayer();
        renderEnemies();
        renderBullets();
        
        /* HUD en consola */
        consoleDrawText(0, 0, "SCORE:");
        consoleDrawInt(7, 0, player.score);
        consoleDrawText(0, 1, "LIVES:");
        consoleDrawInt(7, 1, player.lives);
        
        /* Sincronizar con VBlank */
        WaitForVBlank();
        oamUpdate();
        
        frameCount++;
        
        /* Game Over */
        if (player.lives == 0) {
            consoleDrawText(8, 12, "GAME OVER");
            break;
        }
    }
    
    /* Loop infinito después de game over */
    while(1) {
        WaitForVBlank();
    }
    
    return 0;
}
