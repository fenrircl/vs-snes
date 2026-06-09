#include "hud.h"

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
    consoleDrawText(x, y, buf);
}
