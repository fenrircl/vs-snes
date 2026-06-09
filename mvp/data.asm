.include "hdr.asm"

.section ".rodata1" superfree

tilfont:
.incbin "gfx/pvsneslibfont.pic"

palfont:
.incbin "gfx/pvsneslibfont.pal"

.include "gfx/fondos/Mad_Forest_crop_64x64_indexed_data.as"

.ends
