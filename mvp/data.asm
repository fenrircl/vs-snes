.include "hdr.asm"

.section ".rodata_font" superfree
tilfont:
.incbin "gfx/pvsneslibfont.pic"
palfont:
.incbin "gfx/pvsneslibfont.pal"
.ends

.section ".rodata_map" superfree
.include "gfx/fondos/Mad_Forest_crop_64x64_indexed_data.as"
.ends

.section ".rodata_menu" superfree
.include "gfx/fondos/main_menu_data.as"
.ends
