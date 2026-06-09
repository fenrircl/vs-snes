ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Error: Debes definir PVSNESLIB_HOME. Ej: export PVSNESLIB_HOME=~/snesdev/pvsneslib")
endif

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

ROMNAME := vs-snes-mvp

all: bitmaps $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx

# Font para la consola de texto
gfx/pvsneslibfont.pic: gfx/pvsneslibfont.png
	@echo convert font ... $(notdir $@)
	$(GFXCONV) -s 8 -o 16 -u 16 -p -e 0 -i $<

# Sprites del juego: jugador + enemigos + gemas + balas
sprites.pic: gfx/sprites.png
	@echo convert sprites ... $(notdir $@)
	$(GFXCONV) -s 8 -o 16 -p -e 0 -i $<

# Background (opcional en MVP)
# bg.pic: gfx/background.png
# 	$(GFXCONV) -s 16 -o 16 -u 16 -p -m -e 0 -i $<

bitmaps : gfx/pvsneslibfont.pic sprites.pic
