#include "scenes/decor.h"

#include "gfx/palette.h"
#include "gfx/screen.h"

// Couleurs reprises de tools/art/palette.py
#define SKY0   RGB888(232, 246, 255)
#define SKY1   RGB888(196, 228, 250)
#define SKY2   RGB888(214, 238, 236)
#define LEAF0  RGB888(190, 226, 150)
#define LEAF1  RGB888(140, 196, 116)
#define LEAF2  RGB888(92, 152, 96)
#define LEAF3  RGB888(60, 108, 80)
#define FRAME2 RGB888(184, 124, 92)

static const int8_t SIN64[64] = {
    0, 6, 12, 19, 24, 30, 35, 40, 45, 49, 53, 56, 59, 61, 62, 64,
    64, 64, 62, 61, 59, 56, 53, 49, 45, 40, 35, 30, 24, 19, 12, 6,
    0, -6, -12, -19, -24, -30, -35, -40, -45, -49, -53, -56, -59, -61, -62, -64,
    -64, -64, -62, -61, -59, -56, -53, -49, -45, -40, -35, -30, -24, -19, -12, -6,
};
static inline int sin64(int i) { return SIN64[i & 63]; }

static int g_world;

void decor_set_world(int world) { g_world = world; }

// Couleur du cadre des fenêtres de chaque bar (cf. tools/art/interiors.py)
static const uint16_t FRAME_COL[5] = {
    RGB888(220, 164, 120), RGB888(226, 226, 230), RGB888(120, 80, 60), RGB888(140, 140, 160),
    RGB888(126, 92, 66),
};
static const uint16_t BAR_COL[5] = {
    RGB888(184, 124, 92), RGB888(190, 190, 200), RGB888(96, 62, 50), RGB888(110, 110, 130),
    RGB888(100, 72, 54),
};

static bool visible(const Surface *s, Rect r)
{
    return !rect_empty(rect_intersect(s->clip, r));
}

// Feuille ombrée : contour sombre, corps, reflet
static void leaf(Surface *s, int x, int y, int rx, int ry)
{
    draw_ellipse(s, x, y, rx, ry, LEAF3);
    draw_ellipse(s, x, y, rx - 1, ry - 1, LEAF1);
    draw_ellipse(s, x + 1, y + 1, rx / 2, ry / 2, LEAF2);
    draw_pixel(s, x - rx / 2, y - ry / 2, LEAF0);
}

static void cloud(Surface *s, int cx, int cy, int size)
{
    draw_ellipse(s, cx, cy, 6 * size / 4, 3 * size / 4 + 1, COL_WHITE);
    draw_ellipse(s, cx - 4 * size / 4, cy + 1, 4 * size / 4, 2 * size / 4 + 1, COL_WHITE);
    draw_ellipse(s, cx + 4 * size / 4, cy + 1, 4 * size / 4, 2 * size / 4 + 1, COL_WHITE);
}

// ---------------------------------------------------------------- Fenêtres

typedef struct { int x0, y0, w, h; } Win;
static const Win WINDOWS[2] = { { 14, 30, 44, 50 }, { 198, 30, 44, 50 } };

static Rect glass_rect(const Win *w)
{
    return rect_make(w->x0 + 3, w->y0 + 3, w->w - 6, w->h - 6);
}

static void window_anim(Surface *s, const Win *w, uint32_t frame, int seed)
{
    Rect g = glass_rect(w);
    if (!visible(s, g))
        return;
    Rect saved = s->clip;
    s->clip = rect_intersect(s->clip, g);
    switch (g_world) {
    case 1: {   // la mer : ciel, soleil, vagues qui défilent, voilier
        for (int y = g.y; y < g.y + g.h; y++)
            draw_hline(s, g.x, y, g.w, y < g.y + g.h / 2 ? SKY0 : RGB888(120, 190, 232));
        draw_disc(s, g.x + 8, g.y + 8, 4, RGB888(255, 240, 170));
        for (int k = 0; k < 6; k++) {
            int y = g.y + g.h / 2 + 3 + k * 4;
            int off = (int)((frame / (3 + k % 2)) + k * 9) % 12;
            for (int x = g.x - 12 + off; x < g.x + g.w; x += 12)
                draw_hline(s, x, y, 4, COL_WHITE);
        }
        int bx = g.x + (int)((frame / 8 + seed * 20) % (g.w + 20)) - 10;
        int by = g.y + g.h / 2 - 1 + (((frame / 20) & 1) ? 1 : 0);
        draw_rect(s, bx - 4, by, 9, 2, RGB888(184, 124, 92));
        for (int j = 0; j < 6; j++)
            draw_hline(s, bx, by - 6 + j, j / 2 + 1, COL_WHITE);
        break;
    }
    case 2: {   // montagnes enneigées, neige qui tombe
        for (int y = g.y; y < g.y + g.h; y++)
            draw_hline(s, g.x, y, g.w, y < g.y + g.h * 2 / 3 ? RGB888(220, 232, 250) : COL_WHITE);
        for (int x = g.x; x < g.x + g.w; x++) {
            int peak = g.y + g.h / 3 + (x * 7 % 23 < 12 ? (x * 7 % 23) : 23 - x * 7 % 23);
            for (int y = peak; y < g.y + g.h * 2 / 3; y++)
                draw_pixel(s, x, y, y < peak + 4 ? COL_WHITE : RGB888(170, 176, 214));
        }
        for (int k = 0; k < 10; k++) {
            int x = g.x + (k * 11 + (int)(frame / 8) * (k % 3 ? 1 : -1)) % g.w;
            if (x < g.x)
                x += g.w;
            int y = g.y + (int)((frame / (2 + k % 2) + k * 13) % (uint32_t)g.h);
            draw_pixel(s, x, y, COL_WHITE);
        }
        break;
    }
    case 3: {   // la ville la nuit : étoiles qui scintillent, fenêtres qui s'allument
        for (int y = g.y; y < g.y + g.h; y++)
            draw_hline(s, g.x, y, g.w, y < g.y + g.h / 2 ? RGB888(36, 32, 76) : RGB888(54, 46, 96));
        for (int k = 0; k < 8; k++)
            if (((frame / 12) + k * 3) % 5)
                draw_pixel(s, g.x + (k * 13 + seed * 7) % g.w, g.y + (k * 7) % (g.h / 2), RGB888(250, 240, 200));
        for (int b = 0; b < 4; b++) {
            int bx = g.x + b * (g.w / 4), bw = g.w / 4 - 2, bh = 14 + (b * 11 + seed * 5) % 18;
            draw_rect(s, bx, g.y + g.h - bh, bw, bh, RGB888(70, 60, 120));
            for (int wy = g.y + g.h - bh + 3; wy < g.y + g.h - 2; wy += 5)
                for (int wx = bx + 2; wx < bx + bw - 2; wx += 4)
                    if ((((frame / 30) + wx * 3 + wy) % 7) < 4)
                        draw_rect(s, wx, wy, 2, 2, (wx + wy) % 3 ? RGB888(255, 230, 140) : RGB888(255, 150, 210));
        }
        break;
    }
    case 4: {   // la forêt : troncs, feuillage, lucioles
        for (int y = g.y; y < g.y + g.h; y++)
            draw_hline(s, g.x, y, g.w, y < g.y + g.h / 3 ? RGB888(190, 226, 170) : RGB888(150, 200, 130));
        for (int k = 0; k < 3; k++)
            draw_rect(s, g.x + 4 + k * 13 + seed * 3, g.y + 10, 4, g.h - 10, RGB888(120, 90, 70));
        for (int k = 0; k < 4; k++)
            draw_ellipse(s, g.x + 6 + k * 11, g.y + 6 + (k & 1) * 4, 9, 7, LEAF1);
        for (int k = 0; k < 5; k++) {
            int x = g.x + (k * 9 + sin64((int)frame + k * 13) / 16 + 64) % g.w;
            int y = g.y + 14 + (k * 7 + sin64((int)frame / 2 + k * 9) / 20 + 40) % (g.h - 16);
            if (((frame / 6) + k) % 3)
                draw_pixel(s, x, y, RGB888(255, 250, 160));
        }
        break;
    }
    default:
        // Ciel par paliers
        for (int y = g.y; y < g.y + g.h; y++) {
            int t = (y - g.y) * 100 / (g.h - 1);
            draw_hline(s, g.x, y, g.w, t < 35 ? SKY0 : t < 75 ? SKY1 : SKY2);
        }
        // Deux nuages qui dérivent vers la droite, à des vitesses différentes
        for (int k = 0; k < 2; k++) {
            int span = g.w + 30;
            int x = g.x - 15 + (int)((frame / (6 + k * 4) + seed * 13 + k * 23) % span);
            cloud(s, x, g.y + 6 + k * 10, 4 - k);
        }
        // Collines
        for (int x = g.x; x < g.x + g.w; x++) {
            int top = g.y + g.h - 6 + ((x * 3) % 7 == 0);
            for (int y = top; y < g.y + g.h; y++)
                draw_pixel(s, x, y, (x + y) % 5 ? LEAF0 : LEAF1);
        }
        break;
    }
    // Coins arrondis : on rend le bois du cadre
    s->clip = saved;
    // Croisillons et reflet
    draw_vline(s, w->x0 + w->w / 2, w->y0 + 3, w->h - 6, BAR_COL[g_world]);
    draw_hline(s, w->x0 + 3, w->y0 + w->h / 2, w->w - 6, BAR_COL[g_world]);
    for (int k = 0; k < 5; k++)
        draw_pixel(s, g.x + 2 + k, g.y + 8 - k, COL_WHITE);
    // Coins arrondis de la vitre (rayon 4)
    static const uint8_t CORNER[4] = { 3, 2, 1, 1 };
    for (int j = 0; j < 4; j++) {
        int n = CORNER[j];
        uint16_t wood = FRAME_COL[g_world];
        draw_hline(s, g.x, g.y + j, n, wood);
        draw_hline(s, g.x + g.w - n, g.y + j, n, wood);
        draw_hline(s, g.x, g.y + g.h - 1 - j, n, wood);
        draw_hline(s, g.x + g.w - n, g.y + g.h - 1 - j, n, wood);
    }
}

// ---------------------------------------------------------------- Guirlande

static const uint16_t BULBS[5][5] = {
    { RGB888(255, 194, 210), RGB888(255, 228, 140), RGB888(178, 230, 206), RGB888(212, 198, 248), RGB888(255, 210, 172) },
    { RGB888(150, 210, 240), RGB888(255, 170, 150), RGB888(255, 240, 180), RGB888(150, 230, 210), RGB888(255, 255, 255) },
    { RGB888(255, 220, 140), RGB888(240, 110, 100), RGB888(255, 240, 200), RGB888(140, 200, 150), RGB888(255, 200, 120) },
    { RGB888(255, 120, 200), RGB888(120, 240, 255), RGB888(255, 230, 120), RGB888(180, 140, 255), RGB888(140, 255, 180) },
    { RGB888(255, 236, 150), RGB888(190, 236, 140), RGB888(255, 200, 220), RGB888(255, 220, 120), RGB888(200, 180, 255) },
};

static void lights(Surface *s, uint32_t frame)
{
    if (!visible(s, rect_make(0, 16, SCREEN_W, 14)))
        return;
    for (int x = 8, i = 0; x < SCREEN_W; x += 16, i++) {
        int t = (x % 64) - 32;
        if (t < 0)
            t = -t;
        int yy = 18 + (4 * 1024 - 4 * t * t) / 1024;
        uint16_t col = BULBS[g_world][i % 5];
        // Scintillement : chaque ampoule s'allume / s'atténue à son rythme
        int phase = (int)((frame / 10 + i * 3) % 9);
        bool bright = phase < 6;
        uint16_t body = bright ? col : color_mix(col, RGB888(150, 120, 140), 110);
        draw_pixel(s, x, yy + 1, COL_INK);
        draw_rect(s, x - 1, yy + 2, 4, 4, body);
        draw_hline(s, x, yy + 6, 2, body);
        if (bright) {
            draw_pixel(s, x, yy + 3, COL_WHITE);
            if (phase == 0 || phase == 3) {   // petit éclat
                draw_pixel(s, x - 2, yy + 3, col);
                draw_pixel(s, x + 3, yy + 3, col);
            }
        }
    }
}

// ---------------------------------------------------------------- Plantes

// Mobile de coquillages (paillote) ou lampes néon (kiosque) qui se balancent
static void hanging_mobile(Surface *s, int x, int y, uint32_t frame, bool neon)
{
    static const uint16_t NEON[3] = { RGB888(255, 120, 200), RGB888(120, 240, 255), RGB888(255, 230, 120) };
    static const uint16_t SHELL[3] = { RGB888(255, 200, 190), RGB888(255, 240, 220), RGB888(200, 230, 250) };
    for (int k = 0; k < 3; k++) {
        int len = 10 + k * 5;
        int sway = sin64((int)frame + k * 15) * len / (64 * 6);
        int bx = x - 6 + k * 6;
        draw_line(s, bx, y + 9, bx + sway, y + 9 + len, COL_INK_SOFT);
        int ex = bx + sway, ey = y + 9 + len;
        if (neon) {
            bool on = ((frame / 9 + k) % 11) != 0;
            draw_rect(s, ex - 2, ey, 5, 4, on ? NEON[k] : RGB888(80, 70, 110));
            draw_pixel(s, ex, ey + 1, on ? COL_WHITE : RGB888(80, 70, 110));
        } else {
            draw_ellipse(s, ex, ey + 2, 3, 2, SHELL[k]);
            draw_pixel(s, ex - 1, ey + 1, COL_WHITE);
            draw_pixel(s, ex + 1, ey + 3, color_darken(SHELL[k], 6));
        }
    }
}

static void hanging_vines(Surface *s, int x, int y, uint32_t frame)
{
    Rect area = rect_make(x - 14, y - 4, 28, 40);
    if (!visible(s, area))
        return;
    if (g_world == 1 || g_world == 3) {
        hanging_mobile(s, x, y, frame, g_world == 3);
        return;
    }
    static const uint8_t LENGTHS[5] = { 10, 18, 14, 22, 12 };
    for (int k = 0; k < 5; k++) {
        int vx = x - 7 + (k * 7) / 2;
        for (int j = 0; j < LENGTHS[k]; j++) {
            // Le bout des lianes se balance plus que le haut
            int sway = sin64((int)frame + k * 9 + j * 2) * j / (64 * 7);
            int xx = vx + sway + ((j / 4) % 2 * 2 - 1) * (j > 2);
            int yy = y + 9 + j;
            draw_pixel(s, xx, yy, LEAF3);
            if (j % 3 == 1) {
                int side = ((j / 3 + k) % 2) ? 1 : -1;
                draw_ellipse(s, xx + side * 2, yy, 2, 1, LEAF1);
                draw_pixel(s, xx + side * 2 - 1, yy - 1, LEAF0);
                draw_pixel(s, xx + side * 3, yy, LEAF2);
            }
        }
    }
    for (int k = 0; k < 4; k++)
        leaf(s, x - 6 + k * 4, y + 1 - (k % 2), 3, 2);
}

static void floor_plant(Surface *s, int cx, int base_y, uint32_t frame, int seed)
{
    Rect area = rect_make(cx - 16, base_y - 44, 32, 30);
    if (!visible(s, area))
        return;
    int sway = sin64((int)frame / 2 + seed * 20) / 40;
    if (g_world == 1) {          // petit palmier
        draw_vline(s, cx, base_y - 28, 13, RGB888(150, 110, 80));
        for (int k = 0; k < 5; k++) {
            int ang = -2 + k;
            draw_line(s, cx, base_y - 28, cx + ang * 7 + sway, base_y - 32 + (ang < 0 ? -ang : ang) * 3, LEAF2);
            draw_line(s, cx, base_y - 29, cx + ang * 7 + sway, base_y - 33 + (ang < 0 ? -ang : ang) * 3, LEAF1);
        }
        return;
    }
    if (g_world == 2) {          // sapin en pot, avec un peu de neige
        for (int k = 0; k < 3; k++) {
            int w = 4 + k * 3, top = base_y - 36 + k * 6;
            for (int j = 0; j < 7; j++)
                draw_hline(s, cx - w * j / 7 + sway * (3 - k) / 3, top + j, 2 * w * j / 7 + 1, j < 2 ? COL_WHITE : LEAF3);
        }
        return;
    }
    if (g_world == 3) {          // cactus
        draw_rect(s, cx - 3 + sway, base_y - 34, 6, 18, LEAF2);
        draw_rect(s, cx - 8 + sway, base_y - 28, 4, 6, LEAF2);
        draw_rect(s, cx + 4 + sway, base_y - 30, 4, 7, LEAF2);
        draw_vline(s, cx - 1 + sway, base_y - 33, 16, LEAF1);
        draw_pixel(s, cx + sway, base_y - 36, RGB888(255, 120, 200));   // fleur
        return;
    }
    if (g_world == 4) {          // champignons lumineux
        for (int k = 0; k < 3; k++) {
            int mx = cx - 6 + k * 6 + sway, my = base_y - 18 - (k % 2) * 6;
            draw_rect(s, mx - 1, my, 2, 5, RGB888(250, 240, 220));
            bool glow = ((frame / 12) + k) % 4 == 0;
            draw_ellipse(s, mx, my, 4, 2, glow ? RGB888(255, 170, 220) : RGB888(236, 110, 170));
            draw_pixel(s, mx - 1, my - 1, COL_WHITE);
        }
        return;
    }
    draw_vline(s, cx, base_y - 22, 7, LEAF3);
    for (int i = 0; i < 7; i++) {
        int ang = -120 + i * 40;                 // centièmes de radian
        int a = ang < 0 ? -ang : ang;
        int lx = cx + ang * 7 / 100;
        int ly = base_y - 20 - (140 - a) * 12 / 100 - ((seed * 7 + i * 3) % 4);
        // Balancement doux, plus marqué sur les feuilles extérieures
        lx += sin64((int)frame / 2 + i * 9 + seed * 20) * (1 + a / 60) / 64;
        leaf(s, lx, ly, 4, 6);
    }
}

// ---------------------------------------------------------------- API

void decor_shop_draw(Surface *s, uint32_t frame)
{
    window_anim(s, &WINDOWS[0], frame, 0);
    window_anim(s, &WINDOWS[1], frame, 1);
    lights(s, frame);
    hanging_vines(s, 70, 58, frame);
    hanging_vines(s, 186, 58, frame + 20);
    floor_plant(s, 10, 122, frame, 0);
    floor_plant(s, 246, 122, frame, 1);
}

void decor_shop_dirty(uint32_t frame)
{
    if (frame % 6 == 0) {
        screen_dirty(SCREEN_TOP, glass_rect(&WINDOWS[0]));
        screen_dirty(SCREEN_TOP, glass_rect(&WINDOWS[1]));
    }
    if (frame % 10 == 0)
        screen_dirty(SCREEN_TOP, rect_make(0, 18, SCREEN_W, 10));
    if (frame % 4 == 0) {
        screen_dirty(SCREEN_TOP, rect_make(56, 54, 28, 42));
        screen_dirty(SCREEN_TOP, rect_make(172, 54, 28, 42));
    }
    if (frame % 8 == 4) {
        screen_dirty(SCREEN_TOP, rect_make(0, 76, 28, 34));
        screen_dirty(SCREEN_TOP, rect_make(228, 76, 28, 34));
    }
}

void decor_title_sky(Surface *s, uint32_t frame)
{
    static const struct { int y, speed, size, offset; } CLOUDS[5] = {
        { 10, 3, 6, 0 }, { 26, 5, 4, 90 }, { 44, 2, 7, 170 }, { 62, 4, 5, 40 }, { 74, 6, 4, 210 },
    };
    for (int i = 0; i < 5; i++) {
        int span = SCREEN_W + 60;
        int x = (int)((frame / CLOUDS[i].speed + CLOUDS[i].offset) % span) - 30;
        cloud(s, x, CLOUDS[i].y, CLOUDS[i].size);
        draw_hline(s, x - CLOUDS[i].size * 2, CLOUDS[i].y + CLOUDS[i].size, CLOUDS[i].size * 4, SKY1);
    }
}

void decor_title_dirty(uint32_t frame)
{
    if (frame % 2 == 0)
        screen_dirty(SCREEN_TOP, rect_make(0, 0, SCREEN_W, 88));
}

void decor_polka(Surface *s, uint16_t base, uint16_t dots, uint32_t frame)
{
    draw_rect(s, s->clip.x, s->clip.y, s->clip.w, s->clip.h, base);
    int off = (int)(frame / 4) % 16;
    for (int y = 6 - 16 + off; y < s->h + 16; y += 16)
        for (int x = (((y - off) / 16) & 1) * 8 + 4 - 16 + off; x < s->w + 16; x += 16)
            draw_disc(s, x, y, 1, dots);
}
