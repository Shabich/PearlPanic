// Gobelet animé : liquide qui ondule, filet qui coule, garnitures qui tombent,
// glaçons qui flottent, mélange quand on secoue, couvercle et paille qui se posent.
#ifndef PP_CUP_ANIM_H
#define PP_CUP_ANIM_H

#include <stdbool.h>
#include <stdint.h>

#include "game/drink.h"
#include "gfx/canvas.h"

#define CUP_ITEMS 64

typedef enum {
    ITEM_PEARL,
    ITEM_POP,
    ITEM_JELLY,
    ITEM_ICE,
    ITEM_BUBBLE,     // bulle d'air pendant le versement
} CupItemType;

typedef struct {
    uint8_t type;
    uint8_t index;   // rang parmi les objets du même type (position de repos)
    bool resting;
    int16_t x;       // px, relatif au centre du gobelet
    int32_t y;       // 1/16 px, absolu
    int16_t vy;      // 1/16 px par frame
    uint8_t life;    // bulles : durée de vie
} CupItem;

typedef struct {
    uint8_t spawned[ING_COUNT];   // garnitures déjà apparues
    CupItem items[CUP_ITEMS];
    int count;
    int16_t slosh;     // amplitude des vagues (1/16 px)
    int8_t tilt;       // inclinaison de la surface (secousses)
    uint8_t mix;       // 0..32 : progression du mélange après avoir secoué
    uint8_t seal_t;    // frames depuis la fermeture (plafonné)
    uint8_t drip_t;    // animation des coulures quand ça déborde
    int prev_wobble;
    bool was_sealed, was_shaken;
} CupAnim;

void cup_anim_reset(CupAnim *a);

// Avance l'animation d'une frame. `pouring` : un filet coule dans ce gobelet.
void cup_anim_update(CupAnim *a, const Drink *d, bool pouring, int wobble, uint32_t frame);

// Vrai si quelque chose bouge (pour redessiner plus souvent)
bool cup_anim_busy(const CupAnim *a, const Drink *d);

// Dessine le gobelet. `pour_ing` : ingrédient du filet (0 = aucun). `a` peut être NULL
// (rendu statique, ex. livre de recettes).
void draw_cup(Surface *s, int cx, int dy, const Drink *d, const CupAnim *a, uint32_t frame,
              int wobble, int pour_ing);

// Hauteur (y écran) de la surface du liquide
int cup_surface_y(const Drink *d, int dy);

#endif
