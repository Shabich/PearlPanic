// Compositeur des deux écrans (tâche 1.6)
//
// Chaque écran a :
//  - un fond statique en RAM (dessiné une fois par la scène),
//  - une couche dynamique dessinée par une fonction de la scène.
// Chaque frame, seules les zones marquées « sales » sont recomposées :
// copie du fond puis appel de la fonction dynamique, découpée sur la zone.
#ifndef PP_SCREEN_H
#define PP_SCREEN_H

#include "gfx/canvas.h"

typedef enum { SCREEN_TOP = 0, SCREEN_BOTTOM = 1 } ScreenId;

typedef void (*DynamicDrawFn)(Surface *dst);

void screen_init(void);

// Surface du fond statique (RAM) à dessiner par la scène
Surface *screen_background(ScreenId id);

// Fonction de dessin de la couche dynamique (NULL = aucune)
void screen_set_dynamic(ScreenId id, DynamicDrawFn fn);

// Couche dessinée par-dessus tout (overlay de debug), indépendante des scènes
void screen_set_overlay(ScreenId id, DynamicDrawFn fn);

// Marque une zone à recomposer
void screen_dirty(ScreenId id, Rect r);
void screen_dirty_all(ScreenId id);

// Recompose les zones sales dans une image en RAM (avant le VBlank)…
void screen_compose(void);
// … puis copie seulement le résultat final vers l'affichage (juste après le VBlank).
void screen_flush(void);
// Les deux à la suite
void screen_present(void);

// Aide : suit la boîte englobante d'un objet mobile et marque les zones
// ancienne + nouvelle quand elle change (ou quand force est vrai).
typedef struct {
    Rect last;
    bool shown;
} Tracked;
void screen_track(ScreenId id, Tracked *t, Rect now, bool visible, bool force);

#endif
