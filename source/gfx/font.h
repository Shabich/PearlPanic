// Police bitmap 6x11, Latin-1, textes en UTF-8 (tâche 1.8)
#ifndef PP_FONT_H
#define PP_FONT_H

#include <stdint.h>

#include "gfx/canvas.h"

#define FONT_W 6
#define FONT_H 11
#define FONT_LINE 11

extern const uint8_t FONT_DATA[];

typedef enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT } TextAlign;

// Dessine une ligne (s'arrête à '\n'). scale = 1 ou 2. Renvoie la largeur en px.
int text_draw(Surface *s, int x, int y, const char *utf8, uint16_t color, int scale);

// Comme text_draw avec une ombre portée d'1 px (lisibilité sur fond clair)
int text_draw_shadow(Surface *s, int x, int y, const char *utf8, uint16_t color,
                     uint16_t shadow, int scale);

// Largeur en pixels de la première ligne
int text_width(const char *utf8, int scale);

// Texte aligné dans une largeur donnée (une ligne)
void text_draw_aligned(Surface *s, int x, int y, int w, const char *utf8,
                       uint16_t color, TextAlign align);

// Texte avec retour à la ligne automatique sur les espaces et '\n'.
// Renvoie le nombre de lignes.
int text_draw_wrapped(Surface *s, int x, int y, int w, const char *utf8,
                      uint16_t color, TextAlign align);

// Nombre de lignes que prendrait text_draw_wrapped
int text_count_lines(int w, const char *utf8);

#endif
