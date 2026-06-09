.MEMORYMAP
    SLOTSIZE $8000
    DEFAULTSLOT 0
    SLOT 0 $8000
    SLOT 1 $0 $2000
    SLOT 2 $2000 $E000
    SLOT 3 $0 $10000
.ENDME

.ROMBANKSIZE $8000
.ROMBANKS 8

.BANK 0 SLOT 0
.SECTION "SpriteData" SEMIFREE
.include "gfx/sprites_data.as"
.include "gfx/enemigos/Animated-Pipeestrello_indexed_data.as"
.include "gfx/personajes/Animated-Antonio_Belpaese_indexed_data.as"
.ENDS
