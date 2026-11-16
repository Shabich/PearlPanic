# Pearl Panic — build Nintendo DS (BlocksDS)
#
#   make          compile pearlpanic.nds (via Docker si BlocksDS n'est pas installé)
#   make clean    supprime les artefacts
#   make run      lance la ROM dans melonDS
#   make assets   régénère les graphismes placeholder et leur conversion en C
#   make test     tests unitaires de la logique, compilés pour PC
#   make snap     joue un scénario sur PC et écrit des captures dans build/snap/

NAME          := pearlpanic
GAME_TITLE    := Pearl Panic
GAME_SUBTITLE := Bubble tea rush
GAME_AUTHOR   := shabiche
GAME_ICON     := icon.png

LIBS          := -lmm9 -lnds9
LIBDIRS       := $(BLOCKSDS)/libs/maxmod

SOURCEDIRS    := source
INCLUDEDIRS   := source
AUDIODIRS     := audio

# -fno-tree-loop-distribute-patterns : interdit à GCC de transformer les
# boucles de dessin en memset/memcpy (écritures 8 bits interdites en VRAM).
CFLAGS        := -std=gnu11 -Wextra -Wno-unused-parameter -fno-tree-loop-distribute-patterns

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

.PHONY: run assets test snap
run: all
	"$(MELONDS)" "$(CURDIR)/$(NAME).nds"

assets:
	python3 tools/gen_assets.py
	python3 tools/png2c.py

# --- Build PC (logique + rendu, sans libnds) ---------------------------------

HOST_CC     ?= cc
HOST_CFLAGS := -std=gnu11 -Wall -Wextra -Wno-unused-parameter -O1 -g -Isource -Ihost
HOST_SRC    := $(filter-out source/plat/nds/%,$(shell find source -name '*.c')) host/plat_host.c
HOST_DIR    := build/host

$(HOST_DIR)/test_game: tests/test_game.c $(HOST_SRC) $(shell find source host -name '*.h')
	@mkdir -p $(HOST_DIR)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ tests/test_game.c $(HOST_SRC)

$(HOST_DIR)/snap: host/snap.c $(HOST_SRC) $(shell find source host -name '*.h')
	@mkdir -p $(HOST_DIR)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ host/snap.c $(HOST_SRC)

test: $(HOST_DIR)/test_game
	./$(HOST_DIR)/test_game

snap: $(HOST_DIR)/snap
	@mkdir -p build/snap
	./$(HOST_DIR)/snap build/snap
	python3 tools/ppm2png.py build/snap
