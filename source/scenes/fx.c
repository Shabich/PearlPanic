#include "scenes/fx.h"

#include <stddef.h>

#include "core/rng.h"
#include "gfx/palette.h"

#define MAX_FX 64
#define SUB 16          // positions en 1/16 de pixel

typedef struct {
    bool alive;
    uint8_t screen, type;
    int16_t life, age;
    int32_t x, y, vx, vy;   // 1/16 px
    uint16_t color;
    Rect last;
} Particle;

static Particle g_fx[MAX_FX];
static Rng g_rng = { 0x1234567u };

void fx_clear(void)
{
    for (int i = 0; i < MAX_FX; i++)
        g_fx[i].alive = false;
}

int fx_count(void)
{
    int n = 0;
    for (int i = 0; i < MAX_FX; i++)
        n += g_fx[i].alive;
    return n;
}

static int rnd(int lo, int hi) { return lo + rng_range(&g_rng, hi - lo + 1); }

static Rect bounds(const Particle *p)
{
    int x = p->x / SUB, y = p->y / SUB;
    int r = p->type == FX_SMOKE ? 2 + p->age / 6 : 4;
    return rect_make(x - r - 1, y - r - 1, 2 * r + 3, 2 * r + 3);
}

void fx_emit(ScreenId screen, FxType type, int x, int y, int count, uint16_t color)
{
    for (int k = 0; k < count; k++) {
        Particle *p = NULL;
        for (int i = 0; i < MAX_FX; i++)
            if (!g_fx[i].alive) { p = &g_fx[i]; break; }
        if (!p)
            return;
        p->alive = true;
        p->screen = (uint8_t)screen;
        p->type = (uint8_t)type;
        p->age = 0;
        p->x = (x + rnd(-6, 6)) * SUB;
        p->y = (y + rnd(-3, 3)) * SUB;
        p->color = color;
        switch (type) {
        case FX_HEART:   p->vx = rnd(-6, 6);   p->vy = rnd(-16, -10); p->life = rnd(40, 60); break;
        case FX_STAR:    p->vx = rnd(-24, 24); p->vy = rnd(-40, -24); p->life = rnd(35, 50); break;
        case FX_SWEAT:   p->vx = rnd(4, 10);   p->vy = rnd(-14, -6);  p->life = 30; break;
        case FX_SMOKE:   p->vx = rnd(-8, 8);   p->vy = rnd(-12, -6);  p->life = rnd(30, 40); break;
        case FX_SPARKLE: p->vx = rnd(-10, 10); p->vy = rnd(-10, 10);  p->life = rnd(25, 45);
                         p->x = (x + rnd(-40, 40)) * SUB; p->y = (y + rnd(-20, 20)) * SUB; break;
        case FX_SPLASH:  p->vx = rnd(-20, 20); p->vy = rnd(-36, -16); p->life = 24; break;
        case FX_COIN:    p->vx = rnd(-14, 14); p->vy = rnd(-44, -30); p->life = 40; break;
        }
        p->last = rect_make(0, 0, 0, 0);
    }
}

void fx_update(void)
{
    for (int i = 0; i < MAX_FX; i++) {
        Particle *p = &g_fx[i];
        if (!p->alive)
            continue;
        p->age++;
        p->x += p->vx;
        p->y += p->vy;
        switch (p->type) {
        case FX_HEART:   p->vx = ((p->age / 8) & 1) ? 5 : -5; break;
        case FX_STAR:
        case FX_SPLASH:
        case FX_COIN:    p->vy += 3; break;
        case FX_SWEAT:   p->vy += 2; break;
        case FX_SMOKE:   p->vx = p->vx * 15 / 16; break;
        default:         p->vx = p->vx * 14 / 16; p->vy = p->vy * 14 / 16; break;
        }
        Rect now = bounds(p);
        if (p->last.w)
            screen_dirty(p->screen, p->last);
        if (p->age >= p->life) {
            p->alive = false;
            continue;
        }
        screen_dirty(p->screen, now);
        p->last = now;
    }
}

// Petits motifs 7x6 et 5x5 (bit 0 = colonne de gauche)
static const uint8_t HEART[6] = { 0x36, 0x7F, 0x7F, 0x3E, 0x1C, 0x08 };
static const uint8_t STAR[5]  = { 0x04, 0x0E, 0x1F, 0x0E, 0x0A };
static const uint8_t COIN[5]  = { 0x0E, 0x1F, 0x1B, 0x1F, 0x0E };

static void draw_pattern(Surface *s, const uint8_t *rows, int h, int w, int x, int y, uint16_t c)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            if (rows[j] >> i & 1)
                draw_pixel(s, x + i, y + j, c);
}

void fx_draw(ScreenId screen, Surface *s)
{
    for (int i = 0; i < MAX_FX; i++) {
        const Particle *p = &g_fx[i];
        if (!p->alive || p->screen != screen)
            continue;
        Rect b = bounds(p);
        if (rect_empty(rect_intersect(b, s->clip)))
            continue;
        int x = p->x / SUB, y = p->y / SUB;
        bool fading = p->life - p->age < 8;
        switch (p->type) {
        case FX_HEART:
            draw_pattern(s, HEART, 6, 7, x - 3, y - 3, fading ? COL_PINK : RGB888(255, 110, 140));
            break;
        case FX_STAR:
            draw_pattern(s, STAR, 5, 5, x - 2, y - 2, (p->age / 3) & 1 ? COL_YELLOW : COL_WHITE);
            break;
        case FX_COIN:
            draw_pattern(s, COIN, 5, 5, x - 2, y - 2, RGB888(250, 210, 90));
            draw_pixel(s, x - 1, y - 1, COL_WHITE);
            break;
        case FX_SWEAT:
            draw_rect(s, x, y - 1, 2, 3, RGB888(140, 200, 255));
            draw_pixel(s, x, y - 2, RGB888(140, 200, 255));
            break;
        case FX_SMOKE: {
            int r = 2 + p->age / 6;
            draw_disc(s, x, y, r, fading ? COL_GREY : COL_GREY_DARK);
            draw_disc(s, x - r / 3, y - r / 3, r / 3, COL_GREY);
            break;
        }
        case FX_SPARKLE: {
            uint16_t c = (p->age / 4) & 1 ? COL_WHITE : COL_LAVENDER;
            int len = fading ? 1 : 2;
            draw_hline(s, x - len, y, 2 * len + 1, c);
            draw_vline(s, x, y - len, 2 * len + 1, c);
            break;
        }
        case FX_SPLASH:
            draw_rect(s, x, y, 2, 2, p->color);
            break;
        }
    }
}
