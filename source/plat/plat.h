// Couche plateforme : tout ce qui dépend de la DS (ou du PC pour les tests)
//
// Implémentations :
//   source/plat/nds/plat_nds.c  (console, libnds + maxmod)
//   host/plat_host.c            (PC : tests et captures d'écran)
#ifndef PP_PLAT_H
#define PP_PLAT_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx/canvas.h"

// Bits de boutons (mêmes valeurs que libnds)
enum {
    BTN_A = 1 << 0,
    BTN_B = 1 << 1,
    BTN_SELECT = 1 << 2,
    BTN_START = 1 << 3,
    BTN_RIGHT = 1 << 4,
    BTN_LEFT = 1 << 5,
    BTN_UP = 1 << 6,
    BTN_DOWN = 1 << 7,
    BTN_R = 1 << 8,
    BTN_L = 1 << 9,
    BTN_X = 1 << 10,
    BTN_Y = 1 << 11,
};

typedef struct {
    uint32_t keys;
    bool touching;
    int tx, ty;
} RawInput;

// Effets sonores (tâche 7.3)
typedef enum {
    SFX_POUR,
    SFX_PLOP,
    SFX_PUMP,
    SFX_SEAL,
    SFX_SHAKE,
    SFX_TRASH,
    SFX_CASH,
    SFX_HAPPY,
    SFX_ANGRY,
    SFX_UNLOCK,
    SFX_BELL,
    SFX_CLICK,
    SFX_COUNT
} SfxId;

void plat_init(void);
Surface *plat_screen(int screen);     // 0 = haut, 1 = bas (tactile)
void plat_poll(RawInput *in);
void plat_wait_vblank(void);
uint32_t plat_entropy(void);

void plat_sfx(SfxId id);
void plat_pour_loop(bool on);         // son continu pendant le versement
void plat_music(bool on);             // musique de la boutique (tâche 7.2)
void plat_music_tempo(int permille);  // 1000 = normal ; plus rapide quand ça stresse

// Sauvegarde (tâche 6.5) ; renvoie false si indisponible
bool plat_save_read(void *buf, uint32_t size);
bool plat_save_write(const void *buf, uint32_t size);

#endif
