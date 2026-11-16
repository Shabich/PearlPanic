// Particules décoratives (tâche 2.10) : cœurs, étoiles, gouttes, fumée…
#ifndef PP_FX_H
#define PP_FX_H

#include "gfx/canvas.h"
#include "gfx/screen.h"

typedef enum {
    FX_HEART,     // monte en ondulant
    FX_STAR,      // jaillit puis retombe
    FX_SWEAT,     // goutte de sueur qui tombe
    FX_SMOKE,     // bouffée de colère qui grossit
    FX_SPARKLE,   // scintillement immobile
    FX_SPLASH,    // gouttelette projetée (couleur libre)
    FX_COIN,      // petite pièce qui saute
} FxType;

void fx_clear(void);
// Émet `count` particules autour de (x, y)
void fx_emit(ScreenId screen, FxType type, int x, int y, int count, uint16_t color);
// Avance d'une frame et marque les zones à redessiner
void fx_update(void);
// Dessine les particules d'un écran (à appeler à la fin du dessin dynamique)
void fx_draw(ScreenId screen, Surface *s);
int fx_count(void);

#endif
