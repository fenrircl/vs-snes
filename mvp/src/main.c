/*
 * VS-SNES MVP — Main bootstrap & Game Loop
 */

#include "common.h"
#include "hud.h"
#include "player.h"
#include "enemies.h"
#include "bullets.h"

/* ============================================================
 *  DEFINICIONES DE VARIABLES GLOBALES
 * ============================================================ */

Player player;
Entity enemies[MAX_ENEMIES];
Entity bullets[MAX_BULLETS];
u8 spawnTimer;
u8 frameCount;
u16 gameTimer;
u8 wave, gameMins, gameSecs;
u32 gameStartTime;

/* ============================================================
 *  SPRITES & FONTS EXTERNOS
 * ============================================================ */

extern u8 sprites_til, sprites_tilend;     // tiles de sprites
extern u8 sprites_pal, sprites_palend;     // paleta de sprites
extern char tilfont, palfont;              // fuente de consola (definida en data.asm)

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    /* Inicializar sistema */
    consoleInit();
    consoleSetTextGfxPtr(0x3000);
    consoleSetTextMapPtr(0x6800);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);
    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_32x32);
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);
    
    /* Cargar sprites */
    oamInitGfxSet(
        &sprites_til,
        (&sprites_tilend - &sprites_til),
        &sprites_pal,
        (&sprites_palend - &sprites_pal),
        0, 0, OBJ_SIZE8_L16
    );
    
    /* Init estado */
    player.x = SCREEN_W / 2;
    player.y = SCREEN_H / 2;
    player.lives = 5;
    player.score = 0;
    player.kills = 0;
    player.shootTimer = 0;
    initEnemies();
    initBullets();
    spawnTimer = 0;
    frameCount = 0;
    gameTimer = 0;
    wave = 0;
    gameMins = 0;
    gameSecs = 0;
    
    setScreenOn();
    
    /* Mensaje de inicio en consola */
    consoleDrawText(12, 5, "VAMPIRE");
    consoleDrawText(11, 7, "SURVIVORS");
    consoleDrawText(9, 14, "PRESS TO START");
    
    /* Esperar START */
    while (!(padsDown(0) & KEY_START)) {
        WaitForVBlank();
    }
    
    // Clear start screen texts
    consoleDrawText(12, 5, "       ");
    consoleDrawText(11, 7, "         ");
    consoleDrawText(9, 14, "              ");
    
    // Draw clean HUD on Row 1 (starting at column 2 to avoid overscan cut-off)
    consoleDrawText(2, 1, "HP:#####   00:00  S:0  K:0  ");
    consoleDrawText(0, 0, "                                ");
    consoleDrawText(0, 2, "                                ");
    
start:
    gameStartTime = snes_vblank_count;
    while (1) {
        u32 elapsed = snes_vblank_count - gameStartTime;
        gameSecs = (elapsed / 60) % 60;
        gameMins = (elapsed / 3600);
        wave = elapsed / 900;
        
        updatePlayer();
        if (spawnTimer == 0) {
            spawnEnemy();
            /* Spawneo extra en waves altas */
            { u8 s; for (s = 0; s < wave; s++) spawnEnemy(); }
            spawnTimer = SPAWN_COOLDOWN - (wave * 2);
            if (spawnTimer < 4) spawnTimer = 4;
        } else spawnTimer--;
        updateEnemies();
        updateBullets();
        
        renderPlayer();
        renderEnemies();
        renderBullets();
        
        { u8 h; for (h = 0; h < 5; h++)
            consoleDrawText(5 + h, 1, h < player.lives ? "#" : " "); }
        drawTime(13, 1, gameMins, gameSecs);
        consoleDrawText(22, 1, "%u ", player.score);
        consoleDrawText(27, 1, "%u ", player.kills);
        
        WaitForVBlank();
        oamUpdate();
        frameCount++;
        
        if (player.lives == 0) {
            consoleDrawText(6, 10, "GAME OVER");
            consoleDrawText(2, 11, "SCORE: %u  KILLS: %u", player.score, player.kills);
            consoleDrawText(2, 12, "TIEMPO: ");
            drawTime(10, 12, gameMins, gameSecs);
            consoleDrawText(2, 14, "PRESIONA START");
            while (!(padsDown(0) & KEY_START)) { WaitForVBlank(); }
            player.x = SCREEN_W / 2; player.y = SCREEN_H / 2;
            player.lives = 5; player.score = 0; player.kills = 0; player.shootTimer = 0;
            spawnTimer = 0; frameCount = 0; gameTimer = 0; wave = 0;
            gameMins = 0; gameSecs = 0;
            initEnemies(); initBullets();
            consoleDrawText(6, 10, "         ");
            consoleDrawText(2, 11, "                         ");
            consoleDrawText(2, 12, "                 ");
            consoleDrawText(2, 14, "              ");
            // Redraw clean HUD on Row 1
            consoleDrawText(2, 1, "HP:#####   00:00  S:0  K:0  ");
            goto start;
        }
    }
}
