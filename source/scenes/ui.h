// Petits éléments d'interface partagés par les scènes
#ifndef PP_UI_H
#define PP_UI_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx/canvas.h"
#include "gfx/screen.h"

// Détection de changement : une zone est redessinée quand son « hash » change
typedef struct {
    uint32_t last;
    bool valid;
} Watch;

uint32_t hash_begin(void);
uint32_t hash_add(uint32_t h, int32_t v);
uint32_t hash_ptr(uint32_t h, const void *p);
// Marque `r` sale si le hash a changé depuis la dernière frame
void watch(ScreenId id, Watch *w, Rect r, uint32_t h);
void watch_reset(Watch *w);

typedef enum {
    BTN_STYLE_NORMAL,
    BTN_STYLE_PRIMARY,
    BTN_STYLE_DISABLED,
    BTN_STYLE_PRESSED,
} ButtonStyle;

void ui_button(Surface *s, Rect r, const char *label, ButtonStyle style);
// Barre de progression (0..max)
void ui_gauge(Surface *s, int x, int y, int w, int h, int value, int max, uint16_t color);
// Bulle de dialogue avec petite pointe vers le bas
void ui_bubble(Surface *s, Rect r, uint16_t fill, uint16_t border);
// Couleur d'une jauge de patience selon le ratio restant (0..256)
uint16_t ui_patience_color(int ratio256);
// Fond pastel de base avec motif de pois
void ui_polka_background(Surface *s, uint16_t base, uint16_t dots);

#endif
