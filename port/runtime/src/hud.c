#include "hud.h"
#include <stdarg.h>
#include <stdio.h>

void consoleDrawTextColored(u16 x, u16 y, u8 palette, const char *fmt, ...) {
    va_list ap;
    char buffer[128];
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);
    
    u8 oldPal = txt_pal_adr;
    txt_pal_adr = (palette << 2) | 0x20;
    consoleDrawText(x, y, buffer);
    txt_pal_adr = oldPal;
}

void drawNum(u16 x, u16 y, u16 val) {
    char buf[6], *p = buf + 5;
    buf[5] = 0;
    if (val == 0) { *--p = '0'; }
    else { while (val > 0) { *--p = '0' + (val % 10); val /= 10; } }
    consoleDrawText(x, y, p);
}

void drawTime(u16 x, u16 y, u8 mins, u8 secs) {
    char buf[6];
    buf[0] = '0' + (mins / 10);
    buf[1] = '0' + (mins % 10);
    buf[2] = ':';
    buf[3] = '0' + (secs / 10);
    buf[4] = '0' + (secs % 10);
    buf[5] = 0;
    consoleDrawTextColored(x, y, 6, buf); // Palette 6 (Celeste)
}
