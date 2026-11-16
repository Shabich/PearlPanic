#include "gfx/canvas.h"

static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }

Rect rect_intersect(Rect a, Rect b)
{
    int x0 = imax(a.x, b.x), y0 = imax(a.y, b.y);
    int x1 = imin(a.x + a.w, b.x + b.w), y1 = imin(a.y + a.h, b.y + b.h);
    if (x1 <= x0 || y1 <= y0)
        return rect_make(0, 0, 0, 0);
    return rect_make(x0, y0, x1 - x0, y1 - y0);
}

Rect rect_union(Rect a, Rect b)
{
    if (rect_empty(a))
        return b;
    if (rect_empty(b))
        return a;
    int x0 = imin(a.x, b.x), y0 = imin(a.y, b.y);
    int x1 = imax(a.x + a.w, b.x + b.w), y1 = imax(a.y + a.h, b.y + b.h);
    return rect_make(x0, y0, x1 - x0, y1 - y0);
}

void surface_init(Surface *s, uint16_t *px, int w, int h, int stride)
{
    s->px = px;
    s->w = w;
    s->h = h;
    s->stride = stride;
    surface_reset_clip(s);
}

void surface_reset_clip(Surface *s)
{
    s->clip = rect_make(0, 0, s->w, s->h);
}

uint16_t color_mix(uint16_t a, uint16_t b, int t)
{
    int ar = a & 31, ag = (a >> 5) & 31, ab = (a >> 10) & 31;
    int br = b & 31, bg = (b >> 5) & 31, bb = (b >> 10) & 31;
    return RGB(ar + (((br - ar) * t) >> 8), ag + (((bg - ag) * t) >> 8),
               ab + (((bb - ab) * t) >> 8));
}

uint16_t color_darken(uint16_t c, int amount)
{
    int r = (c & 31) - amount, g = ((c >> 5) & 31) - amount, b = ((c >> 10) & 31) - amount;
    return RGB(imax(r, 0), imax(g, 0), imax(b, 0));
}

static void fill_span(uint16_t *p, int n, uint16_t c)
{
    // Écritures 16 puis 32 bits alignées
    if (n > 0 && ((uintptr_t)p & 2)) {
        *p++ = c;
        n--;
    }
    uint32_t cc = (uint32_t)c | ((uint32_t)c << 16);
    uint32_t *p32 = (uint32_t *)p;
    for (int i = 0; i < n / 2; i++)
        p32[i] = cc;
    if (n & 1)
        p[n - 1] = c;
}

void draw_rect(Surface *s, int x, int y, int w, int h, uint16_t c)
{
    Rect r = rect_intersect(rect_make(x, y, w, h), s->clip);
    if (rect_empty(r))
        return;
    for (int j = 0; j < r.h; j++)
        fill_span(s->px + (r.y + j) * s->stride + r.x, r.w, c);
}

void draw_fill(Surface *s, uint16_t c)
{
    draw_rect(s, 0, 0, s->w, s->h, c);
}

void draw_hline(Surface *s, int x, int y, int w, uint16_t c) { draw_rect(s, x, y, w, 1, c); }
void draw_vline(Surface *s, int x, int y, int h, uint16_t c) { draw_rect(s, x, y, 1, h, c); }

void draw_pixel(Surface *s, int x, int y, uint16_t c)
{
    if (rect_contains(s->clip, x, y))
        s->px[y * s->stride + x] = c;
}

void draw_rect_outline(Surface *s, int x, int y, int w, int h, uint16_t c)
{
    draw_hline(s, x, y, w, c);
    draw_hline(s, x, y + h - 1, w, c);
    draw_vline(s, x, y, h, c);
    draw_vline(s, x + w - 1, y, h, c);
}

void draw_round_rect(Surface *s, int x, int y, int w, int h, uint16_t fill, uint16_t border)
{
    draw_rect(s, x + 1, y + 1, w - 2, h - 2, fill);
    draw_hline(s, x + 2, y, w - 4, border);
    draw_hline(s, x + 2, y + h - 1, w - 4, border);
    draw_vline(s, x, y + 2, h - 4, border);
    draw_vline(s, x + w - 1, y + 2, h - 4, border);
    draw_pixel(s, x + 1, y + 1, border);
    draw_pixel(s, x + w - 2, y + 1, border);
    draw_pixel(s, x + 1, y + h - 2, border);
    draw_pixel(s, x + w - 2, y + h - 2, border);
}

void draw_disc(Surface *s, int cx, int cy, int r, uint16_t c)
{
    for (int dy = -r; dy <= r; dy++) {
        int dx = 0;
        while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r)
            dx++;
        draw_hline(s, cx - dx, cy + dy, 2 * dx + 1, c);
    }
}

void draw_line(Surface *s, int x0, int y0, int x1, int y1, uint16_t c)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        draw_pixel(s, x0, y0, c);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_image_flip(Surface *s, const Image *img, int x, int y, bool hflip)
{
    Rect r = rect_intersect(rect_make(x, y, img->w, img->h), s->clip);
    if (rect_empty(r))
        return;
    for (int j = 0; j < r.h; j++) {
        const uint16_t *src = img->px + (r.y - y + j) * img->w;
        uint16_t *dst = s->px + (r.y + j) * s->stride + r.x;
        for (int i = 0; i < r.w; i++) {
            int sx = r.x - x + i;
            uint16_t p = src[hflip ? img->w - 1 - sx : sx];
            if (p)
                dst[i] = p;
        }
    }
}

void draw_image(Surface *s, const Image *img, int x, int y)
{
    draw_image_flip(s, img, x, y, false);
}

void surface_copy_rect(Surface *dst, const Surface *src, Rect r)
{
    r = rect_intersect(r, rect_make(0, 0, dst->w, dst->h));
    for (int j = 0; j < r.h; j++) {
        const uint16_t *a = src->px + (r.y + j) * src->stride + r.x;
        uint16_t *b = dst->px + (r.y + j) * dst->stride + r.x;
        int n = r.w;
        if (n > 0 && ((uintptr_t)b & 2)) {
            *b++ = *a++;
            n--;
        }
        // a et b ont la même parité d'alignement (mêmes coordonnées, stride pair)
        const uint32_t *a32 = (const uint32_t *)a;
        uint32_t *b32 = (uint32_t *)b;
        for (int i = 0; i < n / 2; i++)
            b32[i] = a32[i];
        if (n & 1)
            b[n - 1] = a[n - 1];
    }
}
