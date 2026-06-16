#ifndef HUD_H
#define HUD_H

#include "common.h"

extern u8 txt_pal_adr;

void consoleDrawTextColored(u16 x, u16 y, u8 palette, const char *fmt, ...);
void drawNum(u16 x, u16 y, u16 val);
void drawTime(u16 x, u16 y, u8 mins, u8 secs);

#endif /* HUD_H */
