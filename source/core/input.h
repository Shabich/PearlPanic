// Module d'entrées : boutons + stylet (tâche 1.3)
#ifndef PP_INPUT_H
#define PP_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "plat/plat.h"

typedef struct {
    // Boutons (masques BTN_*)
    uint32_t held;
    uint32_t down;     // pressés cette frame
    uint32_t up;       // relâchés cette frame

    // Stylet
    bool touching;     // stylet posé
    bool touch_down;   // posé cette frame
    bool touch_up;     // levé cette frame
    bool tapped;       // levé rapidement sans avoir bougé
    bool dragging;     // a bougé au-delà du seuil depuis la pose
    int x, y;          // dernière position connue (conservée après le lever)
    int start_x, start_y;
    int dx, dy;        // déplacement depuis la frame précédente
    int touch_frames;  // durée de la pose en cours (ou de la dernière)
} Input;

// Met à jour l'état à partir de l'entrée brute de la frame
void input_feed(const RawInput *raw);
const Input *input_get(void);

// Vrai si le stylet est posé dans le rectangle [x, x+w) x [y, y+h)
bool input_touch_in(int x, int y, int w, int h);
// Vrai si le stylet vient d'être posé dans le rectangle
bool input_pressed_in(int x, int y, int w, int h);
// Vrai si un tap vient de se terminer dans le rectangle
bool input_tapped_in(int x, int y, int w, int h);

#endif
