# Pearl Panic — build Nintendo DS (BlocksDS)
#
#   make          compile pearlpanic.nds (via Docker si BlocksDS n'est pas installé)
#   make clean    supprime les artefacts
#   make run      lance la ROM dans melonDS
#   make assets   régénère les placeholders graphiques (Python + Pillow)

NAME          := pearlpanic
GAME_TITLE    := Pearl Panic
GAME_SUBTITLE := Bubble tea rush
GAME_AUTHOR   := shabiche
GAME_ICON     := icon.png

SOURCEDIRS    := source
INCLUDEDIRS   := source
GFXDIRS       := gfx
AUDIODIRS     := audio

CFLAGS        := -std=gnu11 -Wextra -Wno-unused-parameter

DOCKER_IMAGE  ?= skylyrac/blocksds:slim-latest
MELONDS       ?= $(firstword $(wildcard $(HOME)/Applications/melonDS.app/Contents/MacOS/melonDS /Applications/melonDS.app/Contents/MacOS/melonDS))

# BLOCKSDS est défini dans l'environnement par l'image Docker ou par une
# installation native. Sinon, on délègue chaque cible au conteneur.
ifeq ($(origin BLOCKSDS),environment)

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile

else

.PHONY: all clean dump
all clean dump:
	docker run --rm -v "$(CURDIR)":/work -w /work $(DOCKER_IMAGE) make $@

endif

.PHONY: run assets
run: all
	"$(MELONDS)" "$(CURDIR)/$(NAME).nds"

assets:
	python3 tools/gen_placeholders.py
