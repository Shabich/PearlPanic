// Carte du monde façon carte de niveaux : Perle avance de case en case
#include <stdio.h>
#include <stdlib.h>

#include "core/input.h"
#include "core/tween.h"
#include "game/journal.h"
#include "game/objectives.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/specials.h"
#include "game/world.h"
#include "gen/assets.h"
#include "gen/map_nodes.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/decor.h"
#include "scenes/fx.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

#define WALK_FRAMES 36

static int g_world;          // monde affiché
static int g_sel;            // niveau sélectionné dans ce monde (0..3)
static int g_from, g_to;     // marche de Perle entre deux cases
static int g_walk;           // frames de marche restantes
static uint32_t g_frame;
static Watch w_bottom;

static const Image *MAP_IMAGES[WORLD_COUNT] = {
    &IMG_maps_quartier, &IMG_maps_plage, &IMG_maps_montagne, &IMG_maps_ville, &IMG_maps_foret,
};

static int sel_level(void) { return level_id(g_world, g_sel); }

// Première visite d'un bar (hors le premier) : cinématique d'arrivée
static bool world_intro_needed(int world)
{
    return world > 0 && !(progress()->story & (STORY_WORLD_FIRST << world));
}

// ------------------------------------------------------------------ Boutons (écran bas)

typedef enum { MB_PREV, MB_PLAY, MB_NEXT, MB_JOURNAL, MB_BOOK, MB_MENU, MB_WPREV, MB_WNEXT, MB_COUNT } MapButton;

static Rect button_rect(MapButton b)
{
    switch (b) {
    case MB_PREV:    return rect_make(8, 112, 36, 30);
    case MB_PLAY:    return rect_make(52, 112, 152, 30);
    case MB_NEXT:    return rect_make(212, 112, 36, 30);
    case MB_JOURNAL: return rect_make(8, 152, 84, 30);
    case MB_BOOK:    return rect_make(100, 152, 72, 30);
    case MB_MENU:    return rect_make(180, 152, 68, 30);
    case MB_WPREV:   return rect_make(4, 4, 24, 20);
    case MB_WNEXT:   return rect_make(228, 4, 24, 20);
    default:         return rect_make(0, 0, 0, 0);
    }
}

static bool tapped(MapButton b)
{
    Rect r = button_rect(b);
    return input_tapped_in(r.x, r.y, r.w, r.h);
}

// ------------------------------------------------------------------ Dessin

// Étoile 7x7 (pleine ou vide)
static void draw_star(Surface *s, int cx, int cy, bool full, int scale)
{
    static const uint8_t STAR[7] = { 0x08, 0x08, 0x7F, 0x3E, 0x1C, 0x36, 0x63 };
    uint16_t c = full ? COL_YELLOW : COL_GREY;
    uint16_t edge = full ? RGB888(220, 150, 50) : COL_GREY_DARK;
    for (int j = 0; j < 7; j++)
        for (int i = 0; i < 7; i++)
            if (STAR[j] >> i & 1)
                draw_rect(s, cx - 3 * scale + i * scale, cy - 3 * scale + j * scale, scale, scale,
                          (j == 6 || i == 0 || i == 6) ? edge : c);
}

static void node_pos(int i, int *x, int *y)
{
    *x = MAP_NODES[g_world][i][0];
    *y = MAP_NODES[g_world][i][1];
}

static void otter_pos(int *x, int *y, int *hop, int *step)
{
    int x0, y0, x1, y1;
    node_pos(g_from, &x0, &y0);
    node_pos(g_to, &x1, &y1);
    int t = g_walk ? tw_progress(WALK_FRAMES - g_walk, WALK_FRAMES) : 256;
    int e = ease_out_quad(t);
    *x = tw_lerp(x0, x1, e);
    *y = tw_lerp(y0, y1, e);
    *hop = g_walk ? -((g_frame / 3) & 3 ? 2 : 0) : (((g_frame / 30) & 1) ? -1 : 0);
    *step = g_walk ? 1 + ((g_frame / 6) & 1) : 0;
}

// ------------------------------------------------------------------ Animations de carte

static const int8_t SIN64[64] = {
    0, 6, 12, 19, 24, 30, 35, 40, 45, 49, 53, 56, 59, 61, 62, 64,
    64, 64, 62, 61, 59, 56, 53, 49, 45, 40, 35, 30, 24, 19, 12, 6,
    0, -6, -12, -19, -24, -30, -35, -40, -45, -49, -53, -56, -59, -61, -62, -64,
    -64, -64, -62, -61, -59, -56, -53, -49, -45, -40, -35, -30, -24, -19, -12, -6,
};
static inline int sin64(int i) { return SIN64[i & 63]; }

// Petit hachage pour des positions pseudo-aléatoires stables
static uint32_t mix32(uint32_t x)
{
    x ^= x >> 16; x *= 0x7FEB352Du; x ^= x >> 15; x *= 0x846CA68Bu; x ^= x >> 16;
    return x;
}

// Le pixel du décor est-il de l'eau ? (plus bleu que rouge)
static bool is_water(int x, int y)
{
    if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H)
        return false;
    uint16_t p = screen_background(SCREEN_TOP)->px[y * SCREEN_W + x];
    int r = p & 31, b = (p >> 10) & 31;
    return b > r + 4;
}

static void anim_smoke(Surface *s, int x, int y)
{
    for (int k = 0; k < 4; k++) {
        int phase = (int)((g_frame / 2 + k * 12) % 48);
        int py = y - phase / 2;
        int px = x + sin64(phase * 3 + k * 16) / 24 + phase / 10;
        int r = 1 + phase / 14;
        uint16_t c = phase > 36 ? RGB888(236, 232, 240) : RGB888(214, 208, 220);
        draw_disc(s, px, py, r, c);
    }
}

static void anim_water(Surface *s, const MapAnim *a)
{
    // Reflets qui glissent et scintillent sur l'eau
    for (int k = 0; k < 18; k++) {
        uint32_t h = mix32((uint32_t)k * 2654435761u + (g_frame / 24) * 97 + (uint32_t)g_world * 131);
        int x = a->x + (int)(h % (uint32_t)a->w) + (int)((g_frame / 6) % 6);
        int y = a->y + (int)((h >> 12) % (uint32_t)a->h);
        if (!is_water(x, y) || !is_water(x + 3, y))
            continue;
        int life = (int)((g_frame + k * 5) % 24);
        if (life < 16)
            draw_hline(s, x, y, life < 8 ? 2 : 3, COL_WHITE);
    }
}

static void anim_lantern(Surface *s, int x, int y, int kind, int k)
{
    static const uint16_t COLS[3] = { RGB888(255, 180, 210), RGB888(255, 236, 160), RGB888(255, 220, 150) };
    uint16_t c = COLS[kind % 3];
    bool bright = ((g_frame / 12 + k * 3) % 7) != 0;
    if (kind != 1)
        draw_pixel(s, x, y - 2, COL_INK);
    draw_rect(s, x - 1, y - 1, 3, 4, bright ? c : color_mix(c, COL_INK_SOFT, 120));
    draw_pixel(s, x, y, COL_WHITE);
    if (bright && kind == 1) {   // halo des lampadaires
        for (int d = 0; d < 8; d++)
            draw_pixel(s, x + sin64(d * 8) / 16, y + 1 + sin64(d * 8 + 16) / 22, color_mix(c, COL_WHITE, 60));
    }
}

static void anim_window(Surface *s, const MapAnim *a, int k)
{
    // Des fenêtres s'éteignent puis se rallument de temps en temps
    if (((g_frame / 40 + k * 7) % 11) == 0)
        draw_rect(s, a->x, a->y, a->w, a->h, RGB888(60, 52, 90));
}

static void anim_neon(Surface *s, const MapAnim *a, int k)
{
    static const uint16_t COLS[4] = { RGB888(255, 120, 200), RGB888(120, 240, 255), RGB888(255, 230, 120),
                                      RGB888(180, 140, 255) };
    uint16_t c = COLS[a->param % 4];
    int t = (int)((g_frame + k * 17) % 90);
    bool off = t == 3 || t == 5 || t == 40;   // grésillement du néon
    uint16_t col = off ? RGB888(80, 70, 110) : c;
    draw_rect_outline(s, a->x, a->y, a->w, a->h, col);
    if (a->h > 4)
        draw_hline(s, a->x + 3, a->y + a->h / 2, a->w - 6, off ? col : COL_WHITE);
}

static void anim_boat(Surface *s, int x, int y)
{
    int bob = sin64((int)g_frame * 2) / 40;
    int drift = sin64((int)g_frame / 3) / 12;
    x += drift;
    y += bob;
    draw_rect(s, x - 8, y + 2, 17, 4, RGB888(184, 124, 92));
    draw_hline(s, x - 6, y + 6, 13, RGB888(136, 88, 72));
    draw_vline(s, x, y - 12, 14, RGB888(110, 74, 60));
    for (int j = 0; j < 11; j++)
        draw_hline(s, x + 1, y - 11 + j, j * 7 / 11 + 1, COL_WHITE);
    draw_pixel(s, x + 1, y - 13, COL_PINK_DARK);
    draw_hline(s, x - 10, y + 7, 21, COL_WHITE);   // écume
}

static void anim_cars(Surface *s, const MapAnim *a)
{
    static const uint16_t COLS[4] = { RGB888(255, 120, 150), RGB888(120, 200, 255), RGB888(255, 220, 120),
                                      RGB888(200, 255, 200) };
    for (int k = 0; k < 4; k++) {
        int dir = k & 1 ? -1 : 1;
        int span = a->w + 20;
        int pos = (int)((g_frame * (1 + k % 2) + k * 70) % (uint32_t)span) - 10;
        int x = dir > 0 ? a->x + pos : a->x + a->w - pos;
        int y = a->y + (dir > 0 ? 5 : 0);
        draw_rect(s, x - 4, y, 9, 4, COLS[k]);
        draw_hline(s, x - 2, y + 1, 5, RGB888(60, 60, 90));
        draw_pixel(s, dir > 0 ? x + 4 : x - 4, y + 1, COL_YELLOW);   // phares
        draw_pixel(s, dir > 0 ? x + 4 : x - 4, y + 2, COL_YELLOW);
    }
}

static void anim_fireflies(Surface *s, const MapAnim *a)
{
    for (int k = 0; k < 14; k++) {
        uint32_t h = mix32((uint32_t)k * 7919u);
        int x = a->x + (int)(h % (uint32_t)a->w) + sin64((int)g_frame + k * 9) / 10;
        int y = a->y + (int)((h >> 10) % (uint32_t)a->h) + sin64((int)g_frame / 2 + k * 13) / 14;
        int blink = (int)((g_frame / 4 + k * 5) % 16);
        if (blink < 10) {
            draw_pixel(s, x, y, RGB888(255, 250, 160));
            if (blink < 4) {
                draw_pixel(s, x - 1, y, RGB888(220, 240, 120));
                draw_pixel(s, x + 1, y, RGB888(220, 240, 120));
            }
        }
    }
}

static void anim_glow(Surface *s, int x, int y, int k)
{
    int t = (int)((g_frame / 6 + k * 4) % 16);
    int r = t < 8 ? t / 2 : (16 - t) / 2;
    uint16_t c = RGB888(255, 200, 240);
    for (int d = 0; d < 12; d++)
        draw_pixel(s, x + sin64(d * 5 + t) * (5 + r) / 64, y + sin64(d * 5 + 16 + t) * (3 + r / 2) / 64, c);
}

static void anim_fountain(Surface *s, int x, int y)
{
    for (int k = 0; k < 8; k++) {
        int t = (int)((g_frame + k * 5) % 20);
        int dx = (k - 4) * t / 8;
        int dy = -(t * (20 - t)) / 12;
        draw_pixel(s, x + dx, y + dy, k & 1 ? COL_WHITE : RGB888(170, 214, 240));
    }
}

static void anim_gulls(Surface *s, const MapAnim *a)
{
    for (int k = 0; k < 3; k++) {
        int x = (int)((g_frame / 2 + k * 90) % (uint32_t)(a->w + 40)) - 20;
        int y = a->y + 8 + k * 11 + sin64((int)g_frame + k * 20) / 22;
        int flap = (g_frame / 6 + k) & 1;
        draw_pixel(s, x - 2, y - flap, COL_WHITE);
        draw_pixel(s, x - 1, y - 1 + flap, COL_WHITE);
        draw_pixel(s, x, y, COL_INK_SOFT);
        draw_pixel(s, x + 1, y - 1 + flap, COL_WHITE);
        draw_pixel(s, x + 2, y - flap, COL_WHITE);
    }
}

// Particules d'ambiance par monde : pétales, neige
static void anim_ambient(Surface *s)
{
    if (g_world != WORLD_QUARTIER && g_world != WORLD_MONTAGNE)
        return;
    bool snow = g_world == WORLD_MONTAGNE;
    for (int k = 0; k < (snow ? 40 : 14); k++) {
        uint32_t h = mix32((uint32_t)k * 104729u);
        int speed = snow ? 1 + (int)(h % 2) : 1;
        int x = (int)(h % 256) + sin64((int)g_frame / 2 + k * 11) / (snow ? 20 : 10);
        int y = (int)((h >> 8) % 192 + g_frame * speed / (snow ? 2 : 3)) % 192;
        if (snow) {
            draw_pixel(s, x, y, COL_WHITE);
            if (k % 4 == 0)
                draw_pixel(s, x + 1, y, COL_WHITE);
        } else {
            draw_hline(s, x, y, 2, (k & 1) ? RGB888(255, 206, 222) : RGB888(255, 236, 242));
        }
    }
}

static void draw_map_anims(Surface *s)
{
    for (int i = 0; i < MAP_ANIM_COUNT[g_world]; i++) {
        const MapAnim *a = &MAP_ANIMS[g_world][i];
        switch (a->type) {
        case MA_SMOKE:     anim_smoke(s, a->x, a->y); break;
        case MA_WATER:     anim_water(s, a); break;
        case MA_LANTERN:   anim_lantern(s, a->x, a->y, a->param, i); break;
        case MA_WINDOW:    anim_window(s, a, i); break;
        case MA_NEON:      anim_neon(s, a, i); break;
        case MA_BOAT:      anim_boat(s, a->x, a->y); break;
        case MA_CARS:      anim_cars(s, a); break;
        case MA_FIREFLIES: anim_fireflies(s, a); break;
        case MA_GLOW:      anim_glow(s, a->x, a->y, i); break;
        case MA_FOUNTAIN:  anim_fountain(s, a->x, a->y); break;
        case MA_GULLS:     anim_gulls(s, a); break;
        default: break;
        }
    }
    anim_ambient(s);
}

static void draw_map_top(Surface *s)
{
    draw_map_anims(s);

    // Cases de niveau
    for (int i = 0; i < LEVELS_PER_WORLD; i++) {
        int lvl = level_id(g_world, i);
        int x, y;
        node_pos(i, &x, &y);
        bool open = progress_level_unlocked(lvl);
        int stars = progress()->stars[lvl - 1];
        bool sel = i == g_sel;
        int pulse = sel && ((g_frame / 15) & 1);
        uint16_t fill = !open ? COL_GREY : stars ? COL_MINT : COL_PINK;
        uint16_t edge = sel ? COL_INK : !open ? COL_GREY_DARK : stars ? COL_MINT_DARK : COL_PINK_DARK;
        draw_disc(s, x, y + 2, 10 + pulse, color_darken(edge, 3));
        draw_disc(s, x, y, 10 + pulse, edge);
        draw_disc(s, x, y, 8 + pulse, fill);
        draw_pixel(s, x - 4, y - 4, COL_WHITE);
        char num[4];
        snprintf(num, sizeof(num), "%d", i + 1);
        if (open)
            text_draw(s, x - 2, y - 5, num, COL_INK, 1);
        else
            draw_image(s, &IMG_icons_lock, x - 8, y - 8);
        for (int k = 0; k < 3; k++)
            draw_star(s, x - 9 + k * 9, y + 15, k < stars, 1);
    }

    // Perle
    int ox, oy, hop, step;
    otter_pos(&ox, &oy, &hop, &step);
    static const Image *const FRAMES[3] = { &IMG_heroine_map_0, &IMG_heroine_map_1, &IMG_heroine_map_2 };
    draw_image(s, FRAMES[step], ox - 10, oy - 30 + hop);

    // Bandeau du monde
    char buf[64];
    snprintf(buf, sizeof(buf), "Monde %d : %s", g_world + 1, WORLDS[g_world].name);
    int w = text_width(buf, 1) + 16;
    draw_round_rect(s, (SCREEN_W - w) / 2, 2, w, 15, COL_CREAM, COL_WOOD_DARK);
    text_draw(s, (SCREEN_W - w) / 2 + 8, 4, buf, COL_INK, 1);
    fx_draw(SCREEN_TOP, s);
}

static void draw_map_bottom(Surface *s)
{
    decor_polka(s, COL_CREAM, COL_CREAM_DARK, g_frame);
    int lvl = sel_level();
    bool open = progress_level_unlocked(lvl);
    char buf[96];

    // Changer de monde
    bool wprev = g_world > 0;
    bool wnext = g_world + 1 < WORLD_COUNT && progress_world_unlocked(g_world + 1);
    ui_button(s, button_rect(MB_WPREV), "<", wprev ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, button_rect(MB_WNEXT), ">", wnext ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    snprintf(buf, sizeof(buf), "%s", WORLDS[g_world].place);
    text_draw_aligned(s, 30, 9, 196, buf, COL_INK_SOFT, ALIGN_CENTER);

    // Fiche du niveau
    draw_round_rect(s, 8, 28, 240, 78, COL_WHITE, COL_LAV_DARK);
    snprintf(buf, sizeof(buf), "Niveau %d-%d", g_world + 1, g_sel + 1);
    text_draw_shadow(s, 16, 34, buf, COL_PINK_DARK, COL_PINK_PALE, 2);
    int stars = progress()->stars[lvl - 1];
    for (int k = 0; k < 3; k++)
        draw_star(s, 186 + k * 18, 44, k < stars, 2);
    if (!open) {
        text_draw_wrapped(s, 16, 62, 224, "Termine le niveau précédent pour débloquer celui-ci.",
                          COL_INK_SOFT, ALIGN_LEFT);
    } else {
        // Objectif (déterministe pour le niveau) et habitué attendu
        bool can_discover = false;
        for (int i = 0; i < RECIPE_COUNT; i++)
            if (!RECIPES[i].secret && recipe_available(i, lvl) && !progress_recipe_known(i))
                can_discover = true;
        Objective o = objective_for_day(lvl, can_discover);
        char obj[48];
        objective_describe(&o, obj, sizeof(obj));
        snprintf(buf, sizeof(buf), "Objectif : %s", obj);
        text_draw(s, 16, 58, buf, COL_INK, 1);
        int due[SPECIALS_PER_LEVEL];
        int n = specials_due(lvl, progress()->special_steps, due);
        if (n) {
            snprintf(buf, sizeof(buf), "Visite : %s%s%s", SPECIALS[due[0]].name, n > 1 ? " et " : "",
                     n > 1 ? SPECIALS[due[1]].name : "");
            text_draw(s, 16, 72, buf, COL_WOOD_DARK, 1);
        }
        int total;
        int known = progress_known_in_world(g_world, &total);
        snprintf(buf, sizeof(buf), "Recettes : %d/%d", known, total);
        text_draw(s, 16, 88, buf, COL_LAV_DARK, 1);
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)progress()->coins);
        draw_image(s, &IMG_icons_coin, 236 - 18 - text_width(buf, 1), 85);
        text_draw(s, 238 - text_width(buf, 1), 88, buf, COL_INK, 1);
    }

    bool can_prev = g_sel > 0;
    bool can_next = g_sel + 1 < LEVELS_PER_WORLD;
    ui_button(s, button_rect(MB_PREV), "<", can_prev ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, button_rect(MB_NEXT), ">", can_next ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    Rect play = button_rect(MB_PLAY);
    if (open && !g_walk && ((g_frame / 20) & 1))
        play.y -= 1;
    ui_button(s, play, open ? (stars ? "Rejouer" : "Jouer !") : "Verrouillé",
              open ? BTN_STYLE_PRIMARY : BTN_STYLE_DISABLED);

    int unread = journal_unread();
    snprintf(buf, sizeof(buf), unread ? "Journal (%d)" : "Journal", unread);
    ui_button(s, button_rect(MB_JOURNAL), buf, BTN_STYLE_NORMAL);
    if (unread && ((g_frame / 15) & 1)) {   // pastille « nouveau »
        Rect j = button_rect(MB_JOURNAL);
        draw_disc(s, j.x + j.w - 3, j.y + 3, 4, COL_BAD);
    }
    ui_button(s, button_rect(MB_BOOK), "Livre", BTN_STYLE_NORMAL);
    ui_button(s, button_rect(MB_MENU), "Menu", BTN_STYLE_NORMAL);
}

// ------------------------------------------------------------------ Scène

static void enter_world(int world)
{
    g_world = world;
    draw_image(screen_background(SCREEN_TOP), MAP_IMAGES[world], 0, 0);
    screen_dirty_all(SCREEN_TOP);
}

static void map_enter(void)
{
    int lvl = progress()->level;
    if (lvl < 1 || !progress_level_unlocked(lvl))
        lvl = progress_furthest_level();
    // Perle avance vers le prochain niveau débloqué juste après une réussite
    int target = lvl;
    if (lvl < LEVEL_COUNT && progress()->stars[lvl - 1] > 0 && progress_level_unlocked(lvl + 1))
        target = lvl + 1;
    if (level_world(target) != level_world(lvl)) {
        // Bar terminé : direction le bar suivant (sa cinématique d'arrivée suit)
        enter_world(level_world(target));
        g_sel = g_from = g_to = level_index(target);
        g_walk = 0;
    } else {
        enter_world(level_world(lvl));
        g_sel = level_index(target);
        g_from = level_index(lvl);
        g_to = g_sel;
        g_walk = g_from != g_to ? WALK_FRAMES : 0;
    }
    progress()->level = (uint8_t)level_id(g_world, g_sel);
    g_frame = 0;
    fx_clear();
    plat_music(true);
    plat_music_tempo(1000);
    screen_set_dynamic(SCREEN_TOP, draw_map_top);
    screen_set_dynamic(SCREEN_BOTTOM, draw_map_bottom);
    watch_reset(&w_bottom);
}

static void select_level(int i)
{
    if (i < 0 || i >= LEVELS_PER_WORLD || i == g_sel)
        return;
    g_from = g_walk ? g_to : g_sel;
    g_to = i;
    g_sel = i;
    g_walk = WALK_FRAMES;
    progress()->level = (uint8_t)sel_level();
    plat_sfx(SFX_CLICK);
}

static void change_world(int dir)
{
    int w = g_world + dir;
    if (w < 0 || w >= WORLD_COUNT || !progress_world_unlocked(w))
        return;
    enter_world(w);
    if (world_intro_needed(w)) {
        cutscene_play((CutsceneId)(CUT_WORLD_2 + w - 1), &SCENE_MAP);
        progress()->level = (uint8_t)level_id(w, 0);
        scene_change(&SCENE_CUTSCENE);
        return;
    }
    int furthest = progress_furthest_level();
    g_sel = level_world(furthest) == w ? level_index(furthest) : LEVELS_PER_WORLD - 1;
    g_from = g_to = g_sel;
    g_walk = 0;
    progress()->level = (uint8_t)sel_level();
    plat_sfx(SFX_BELL);
}

static void map_update(void)
{
    const Input *in = input_get();
    g_frame++;
    if (g_frame == 2 && world_intro_needed(g_world) && !scene_transitioning()) {
        cutscene_play((CutsceneId)(CUT_WORLD_2 + g_world - 1), &SCENE_MAP);
        scene_change(&SCENE_CUTSCENE);
        return;
    }
    if (g_walk) {
        g_walk--;
        if (g_walk == 0)
            fx_emit(SCREEN_TOP, FX_SPARKLE, MAP_NODES[g_world][g_to][0], MAP_NODES[g_world][g_to][1], 4, 0);
    }
    fx_update();
    // La carte bouge en permanence (Perle, cases, petites bêtes) : tout l'écran toutes les 2 frames
    if ((g_frame & 1) == 0)
        screen_dirty_all(SCREEN_TOP);
    if ((g_frame & 3) == 0)
        screen_dirty_all(SCREEN_BOTTOM);
    if (scene_transitioning())
        return;

    if (tapped(MB_PREV) || (in->down & BTN_LEFT))
        select_level(g_sel - 1);
    if (tapped(MB_NEXT) || (in->down & BTN_RIGHT))
        select_level(g_sel + 1);
    if (tapped(MB_WPREV) || (in->down & BTN_L))
        change_world(-1);
    if (tapped(MB_WNEXT) || (in->down & BTN_R))
        change_world(+1);

    int lvl = sel_level();
    if ((tapped(MB_PLAY) || (in->down & (BTN_A | BTN_START))) && progress_level_unlocked(lvl) && !g_walk) {
        plat_sfx(SFX_BELL);
        progress()->level = (uint8_t)lvl;
        progress_save();
        shop_begin_day();
        scene_change(&SCENE_SHOP);
    } else if (tapped(MB_JOURNAL) || (in->down & BTN_Y)) {
        plat_sfx(SFX_CLICK);
        journal_open_from(&SCENE_MAP);
        scene_change(&SCENE_JOURNAL);
    } else if (tapped(MB_BOOK) || (in->down & BTN_X)) {
        plat_sfx(SFX_CLICK);
        book_open_from(&SCENE_MAP, g_world);
        scene_change(&SCENE_BOOK);
    } else if (tapped(MB_MENU) || (in->down & BTN_B)) {
        progress_save();
        scene_change(&SCENE_TITLE);
    }
    (void)w_bottom;
}

const Scene SCENE_MAP = {
    .name = "map",
    .enter = map_enter,
    .update = map_update,
    .exit = NULL,
};
