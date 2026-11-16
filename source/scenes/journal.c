// Le Journal : « L'Écho des Perles » (articles qui arrivent au fil des niveaux)
#include <stdio.h>

#include "core/input.h"
#include "game/journal.h"
#include "game/progress.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/decor.h"
#include "scenes/fx.h"
#include "scenes/scenes.h"
#include "scenes/ui.h"

#define LIST_Y  26
#define ROW_H   22
#define ROWS    6

static const Scene *g_back = &SCENE_MAP;
static int g_sel, g_scroll, g_count, g_new_count;
static int g_page_drawn = -1;
static uint32_t g_frame;

void journal_open_from(const Scene *back) { g_back = back; }

static Rect back_rect(void) { return rect_make(4, 160, 80, 26); }
static Rect up_rect(void) { return rect_make(172, 160, 38, 26); }
static Rect down_rect(void) { return rect_make(214, 160, 38, 26); }

static uint16_t kind_color(NewsKind k)
{
    return k == NEWS_MYSTERY ? COL_LAV_DARK : k == NEWS_BAR ? COL_PINK_DARK : COL_MINT_DARK;
}

static const char *kind_label(NewsKind k)
{
    return k == NEWS_MYSTERY ? "Mystère" : k == NEWS_BAR ? "Pearl Panic" : "Vie locale";
}

// Une : page de journal (redessinée quand on change d'article)
static void draw_page(Surface *s)
{
    draw_fill(s, RGB888(246, 240, 226));
    // Bandeau du titre du journal
    draw_rect(s, 0, 0, SCREEN_W, 26, RGB888(70, 60, 70));
    text_draw_shadow(s, (SCREEN_W - text_width("L'Écho des Perles", 2)) / 2, 3, "L'Écho des Perles",
                     COL_CREAM, COL_INK, 2);
    draw_hline(s, 0, 27, SCREEN_W, COL_INK);
    draw_hline(s, 0, 29, SCREEN_W, COL_INK);
    if (g_count == 0) {
        text_draw_aligned(s, 0, 90, SCREEN_W, "Aucun article pour l'instant.", COL_INK_SOFT, ALIGN_CENTER);
        return;
    }
    const Article *a = &ARTICLES[journal_nth_recent(g_sel)];
    char buf[48];
    snprintf(buf, sizeof(buf), "%s", kind_label(a->kind));
    draw_round_rect(s, 8, 34, text_width(buf, 1) + 10, 13, kind_color(a->kind), color_darken(kind_color(a->kind), 5));
    text_draw(s, 13, 35, buf, COL_WHITE, 1);
    int lines = text_draw_wrapped(s, 8, 52, 240, a->title, COL_INK, ALIGN_LEFT);
    // Titre en « gras » : on le redessine décalé d'un pixel
    text_draw_wrapped(s, 9, 52, 240, a->title, COL_INK, ALIGN_LEFT);
    int y = 52 + lines * FONT_LINE + 4;
    draw_hline(s, 8, y, 240, COL_GREY);
    // Colonne de texte + petite illustration
    const Image *pic = a->kind == NEWS_MYSTERY ? &IMG_icons_lock
                     : a->kind == NEWS_BAR ? &IMG_heroine_map_0 : &IMG_icons_heart;
    draw_round_rect(s, 216, y + 6, 32, 36, COL_WHITE, COL_GREY);
    draw_image(s, pic, 232 - pic->w / 2, y + 24 - pic->h / 2);
    text_draw_wrapped(s, 8, y + 6, 204, a->body, COL_INK_SOFT, ALIGN_LEFT);
}

static void draw_list(Surface *s)
{
    decor_polka(s, COL_CREAM, COL_CREAM_DARK, g_frame);
    char buf[64];
    draw_rect(s, 0, 0, SCREEN_W, 20, RGB888(70, 60, 70));
    snprintf(buf, sizeof(buf), "Journal - %d article%s", g_count, g_count > 1 ? "s" : "");
    text_draw_aligned(s, 0, 5, SCREEN_W, buf, COL_CREAM, ALIGN_CENTER);
    for (int row = 0; row < ROWS; row++) {
        int n = g_scroll + row;
        if (n >= g_count)
            break;
        const Article *a = &ARTICLES[journal_nth_recent(n)];
        int y = LIST_Y + row * ROW_H;
        bool sel = n == g_sel;
        draw_round_rect(s, 4, y, 248, ROW_H - 2, sel ? COL_PINK_PALE : COL_WHITE,
                        sel ? COL_PINK_DARK : COL_GREY);
        draw_rect(s, 8, y + 4, 3, ROW_H - 10, kind_color(a->kind));
        // Titre tronqué sur une ligne
        char title[40];
        int k = 0;
        for (const char *p = a->title; *p && k < (int)sizeof(title) - 4; p++)
            title[k++] = *p;
        title[k] = 0;
        if (text_width(a->title, 1) > 200) {
            // coupe proprement à ~33 caractères
            int chars = 0, cut = 0;
            for (int i = 0; title[i]; i++) {
                if ((title[i] & 0xC0) != 0x80)
                    chars++;
                if (chars > 31) { cut = i; break; }
            }
            if (cut) {
                title[cut] = 0;
                snprintf(title + cut, sizeof(title) - cut, "...");
            }
        }
        text_draw(s, 16, y + 5, title, COL_INK, 1);
        if (n < g_new_count && ((g_frame / 15) & 1) == 0) {
            draw_round_rect(s, 206, y + 4, 42, 12, COL_BAD, color_darken(COL_BAD, 6));
            text_draw(s, 209, y + 5, "NEUF", COL_WHITE, 1);
        }
    }
    ui_button(s, back_rect(), "Retour", BTN_STYLE_PRIMARY);
    ui_button(s, up_rect(), "^", g_scroll > 0 ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, down_rect(), "v", g_scroll + ROWS < g_count ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
}

static void draw_top(Surface *s)
{
    fx_draw(SCREEN_TOP, s);
}

static void journal_enter(void)
{
    g_count = journal_available();
    g_new_count = journal_unread();
    progress()->journal_read = (uint8_t)g_count;   // tout est lu une fois ouvert
    g_sel = 0;
    g_scroll = 0;
    g_page_drawn = -1;
    g_frame = 0;
    fx_clear();
    screen_set_dynamic(SCREEN_TOP, draw_top);
    screen_set_dynamic(SCREEN_BOTTOM, draw_list);
}

static void select_article(int n)
{
    if (n < 0 || n >= g_count)
        return;
    g_sel = n;
    if (g_sel < g_scroll)
        g_scroll = g_sel;
    if (g_sel >= g_scroll + ROWS)
        g_scroll = g_sel - ROWS + 1;
    plat_sfx(SFX_CLICK);
}

static void journal_update(void)
{
    const Input *in = input_get();
    g_frame++;
    if (g_page_drawn != g_sel) {
        g_page_drawn = g_sel;
        draw_page(screen_background(SCREEN_TOP));
        screen_dirty_all(SCREEN_TOP);
        if (g_sel < g_new_count)
            fx_emit(SCREEN_TOP, FX_SPARKLE, 128, 60, 6, 0);
    }
    fx_update();
    if ((g_frame & 3) == 0)
        screen_dirty_all(SCREEN_BOTTOM);
    if (scene_transitioning())
        return;
    if (in->down & BTN_UP)
        select_article(g_sel - 1);
    if (in->down & BTN_DOWN)
        select_article(g_sel + 1);
    if (in->touch_down && in->y >= LIST_Y && in->y < LIST_Y + ROWS * ROW_H)
        select_article(g_scroll + (in->y - LIST_Y) / ROW_H);
    Rect u = up_rect(), d = down_rect(), b = back_rect();
    if (input_tapped_in(u.x, u.y, u.w, u.h) && g_scroll > 0)
        g_scroll--;
    if (input_tapped_in(d.x, d.y, d.w, d.h) && g_scroll + ROWS < g_count)
        g_scroll++;
    if (input_tapped_in(b.x, b.y, b.w, b.h) || (in->down & (BTN_B | BTN_START))) {
        plat_sfx(SFX_CLICK);
        progress_save();
        scene_change(g_back);
    }
}

const Scene SCENE_JOURNAL = {
    .name = "journal",
    .enter = journal_enter,
    .update = journal_update,
    .exit = NULL,
};
