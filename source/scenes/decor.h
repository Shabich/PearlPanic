// Décor animé (tâche 2.3) : guirlande qui scintille, nuages qui passent derrière
// les fenêtres, plantes et lianes qui se balancent, ciel du titre.
#ifndef PP_DECOR_H
#define PP_DECOR_H

#include <stdint.h>

#include "gfx/canvas.h"

// Bar dont on dessine le décor (0..4) : vue par les fenêtres, guirlande, plantes
void decor_set_world(int world);

// Boutique (écran du haut) : à dessiner en premier dans la couche dynamique
void decor_shop_draw(Surface *s, uint32_t frame);
// Marque les zones du décor qui changent à cette frame
void decor_shop_dirty(uint32_t frame);

// Ciel du titre (nuages qui défilent)
void decor_title_sky(Surface *s, uint32_t frame);
void decor_title_dirty(uint32_t frame);

// Fond à pois qui glisse doucement (menus)
void decor_polka(Surface *s, uint16_t base, uint16_t dots, uint32_t frame);

#endif
