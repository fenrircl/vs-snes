ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Error: Debes definir PVSNESLIB_HOME. Ej: export PVSNESLIB_HOME=~/snesdev/pvsneslib")
endif

# Músicas
AUDIOFILES := res/bgm.it
export SOUNDBANK := res/soundbank

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all musics

ROMNAME := vs-snes-mvp

# Regla para compilar músicas
SMCONVFLAGS := -s -o $(SOUNDBANK) -V -b 5
musics: $(SOUNDBANK).obj

all: musics bitmaps $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio

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
