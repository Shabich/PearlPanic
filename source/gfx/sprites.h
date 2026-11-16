// Gestionnaire de sprites (tâche 1.5)
//
// Un SpriteGfx = des tuiles + une palette 16 couleurs chargées en VRAM sur un
// écran. Les entrées OAM (0..127) sont allouées par écran via sprite_alloc().
#ifndef PP_SPRITES_H
#define PP_SPRITES_H

#include <stdbool.h>

#include "gfx/video.h"

typedef struct {
    Screen screen;
    SpriteSize size;
    u16 *gfx;       // adresse VRAM des tuiles
    int palette;    // slot de palette 0..15
} SpriteGfx;

void sprites_init(void);

// Charge des tuiles 16 couleurs (sortie grit) dans la VRAM de l'écran
SpriteGfx sprite_gfx_load(Screen s, SpriteSize size,
                          const void *tiles, u32 tiles_len,
                          const void *pal, u32 pal_len);
void sprite_gfx_free(SpriteGfx *g);

// Réserve une entrée OAM ; renvoie -1 si l'écran est plein
int  sprite_alloc(Screen s);
void sprite_free(Screen s, int id);

// Positionne / affiche une entrée OAM avec un graphisme
void sprite_show(int id, const SpriteGfx *g, int x, int y, int priority, bool hflip);
void sprite_hide(Screen s, int id);

// Libère toutes les entrées et graphismes (changement de scène)
void sprites_reset(void);

// À appeler juste après le VBlank
void sprites_commit(void);

#endif
