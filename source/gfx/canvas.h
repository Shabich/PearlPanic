// Primitives de dessin logiciel sur une surface RGB15 (tâches 1.6 / 1.8)
//
// Une Surface est un tableau de pixels 16 bits (bit 15 = opaque, format des
// fonds bitmap 16 bits de la DS). Sur DS elle pointe en VRAM ; sur PC en RAM.
// Tout le dessin respecte le rectangle de découpe `clip`.
//
// IMPORTANT (DS) : la VRAM n'accepte pas les écritures 8 bits. Ce module
// n'écrit que des u16/u32 et le build utilise -fno-tree-loop-distribute-patterns
// pour que GCC ne remplace pas les boucles par memset/memcpy.
#ifndef PP_CANVAS_H
#define PP_CANVAS_H

#include <stdbool.h>
#include <stdint.h>

#define SCREEN_W 256
#define SCREEN_H 192

#define RGB(r, g, b) ((uint16_t)(((r) & 31) | (((g) & 31) << 5) | (((b) & 31) << 10) | 0x8000))
#define RGB888(r, g, b) RGB((r) >> 3, (g) >> 3, (b) >> 3)

typedef struct {
    int16_t x, y, w, h;
} Rect;

typedef struct {
    uint16_t *px;
    int w, h, stride;
    Rect clip;
} Surface;

typedef struct {
    int w, h;
    const uint16_t *px;   // 0 = transparent
} Image;

static inline Rect rect_make(int x, int y, int w, int h)
{
    Rect r = { (int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h };
    return r;
}
static inline bool rect_contains(Rect r, int x, int y)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}
Rect rect_intersect(Rect a, Rect b);
Rect rect_union(Rect a, Rect b);
static inline bool rect_empty(Rect r) { return r.w <= 0 || r.h <= 0; }

void surface_init(Surface *s, uint16_t *px, int w, int h, int stride);
void surface_reset_clip(Surface *s);

// Couleurs
uint16_t color_mix(uint16_t a, uint16_t b, int t256);   // t256 = 0 -> a, 256 -> b
uint16_t color_darken(uint16_t c, int amount);           // amount 0..31

// Formes
void draw_fill(Surface *s, uint16_t c);
void draw_rect(Surface *s, int x, int y, int w, int h, uint16_t c);
void draw_rect_outline(Surface *s, int x, int y, int w, int h, uint16_t c);
void draw_round_rect(Surface *s, int x, int y, int w, int h, uint16_t fill, uint16_t border);
void draw_hline(Surface *s, int x, int y, int w, uint16_t c);
void draw_vline(Surface *s, int x, int y, int h, uint16_t c);
void draw_pixel(Surface *s, int x, int y, uint16_t c);
void draw_disc(Surface *s, int cx, int cy, int r, uint16_t c);
void draw_ellipse(Surface *s, int cx, int cy, int rx, int ry, uint16_t c);
void draw_line(Surface *s, int x0, int y0, int x1, int y1, uint16_t c);

// Images (pixels 0 ignorés)
void draw_image(Surface *s, const Image *img, int x, int y);
void draw_image_flip(Surface *s, const Image *img, int x, int y, bool hflip);

// Peint `color` là où le masque est opaque : pixel clair = couleur,
// pixel sombre = couleur assombrie (ombre). Sert à teinter vitres et bouteilles.
void draw_mask(Surface *s, const Image *mask, int x, int y, uint16_t color);

// Copie d'une zone d'une surface vers une autre (mêmes coordonnées)
void surface_copy_rect(Surface *dst, const Surface *src, Rect r);

#endif
