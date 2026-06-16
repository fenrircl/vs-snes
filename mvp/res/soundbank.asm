;************************************************
; snesmod soundbank data                        *
; total size:      29064 bytes                  *
;************************************************

.include "hdr.asm"

.BANK 5
.SECTION "SOUNDBANK" ; need dedicated bank(s)

SOUNDBANK__:
.incbin "forest_rockman_sb.bnk"
.ENDS
