// Livre de recettes (tâche 5.6) : ouvert depuis la pause
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

#define LIST_Y     22
#define ROW_H      14
#define ROWS       10

static int g_sel, g_scroll;
static Watch w_list, w_page;

static Rect back_rect(void) { return rect_make(4, 166, 80, 22); }
static Rect up_rect(void) { return rect_make(172, 166, 38, 22); }
static Rect down_rect(void) { return rect_make(214, 166, 38, 22); }

static void make_drink(int recipe, Drink *d)
{
    const Recipe *r = &RECIPES[recipe];
    drink_reset(d);
    for (int i = 1; i < ING_COUNT; i++) {
        if (ing_info(i)->kind == KIND_LIQUID)
            drink_pour(d, i, r->qty[i]);
        else
            for (int k = 0; k < r->qty[i]; k++)
                drink_add(d, i);
    }
    drink_seal(d);
    if (r->shaken)
        drink_shake(d);
}

// Écran haut : la recette sélectionnée
static void draw_page(Surface *s)
{
    const Recipe *r = &RECIPES[g_sel];
    bool known = progress_recipe_known(g_sel);
    char buf[128];
    ui_polka_background(s, COL_CREAM, COL_CREAM_DARK);
    draw_round_rect(s, 6, 6, 244, 180, COL_WHITE, COL_LAV_DARK);
    if (known) {
        text_draw_shadow(s, 14, 12, r->name, COL_PINK_DARK, COL_PINK_PALE, 1);
        snprintf(buf, sizeof(buf), "%d pièces", r->price);
        text_draw(s, 240 - text_width(buf, 1), 12, buf, COL_INK_SOFT, 1);
        recipe_describe(g_sel, buf, sizeof(buf));
        text_draw_wrapped(s, 14, 30, 130, buf, COL_INK, ALIGN_LEFT);
        Drink d;
        make_drink(g_sel, &d);
        draw_cup(s, 196, -10, &d, 0, 0);
    } else {
        text_draw(s, 14, 12, "??? (recette inconnue)", COL_INK_SOFT, 1);
        if (r->secret) {
            text_draw_wrapped(s, 14, 34, 228,
                              "Une recette secrète… Expérimente au comptoir pour la trouver !",
                              COL_LAV_DARK, ALIGN_LEFT);
        } else if (r->day > progress()->day) {
            snprintf(buf, sizeof(buf), "Arrive au jour %d.", r->day);
            text_draw(s, 14, 34, buf, COL_INK_SOFT, 1);
        } else {
            text_draw(s, 14, 34, "Un client l'a décrite ainsi :", COL_INK_SOFT, 1);
            snprintf(buf, sizeof(buf), "« %s »", r->riddle);
            text_draw_wrapped(s, 14, 50, 228, buf, COL_LAV_DARK, ALIGN_LEFT);
        }
    }
}

// Écran bas : la liste
static void draw_list(Surface *s)
{
    char buf[48];
    draw_rect(s, 0, 0, SCREEN_W, 18, COL_LAVENDER);
    snprintf(buf, sizeof(buf), "Livre de recettes  %d / %d", progress_known_count(), RECIPE_COUNT);
    text_draw_aligned(s, 0, 4, SCREEN_W, buf, COL_INK, ALIGN_CENTER);
    for (int row = 0; row < ROWS; row++) {
        int i = g_scroll + row;
        if (i >= RECIPE_COUNT)
            break;
        int y = LIST_Y + row * ROW_H;
        bool known = progress_recipe_known(i);
        if (i == g_sel)
            draw_rect(s, 4, y - 1, 248, ROW_H, COL_PINK_PALE);
        snprintf(buf, sizeof(buf), "%2d. %s", i + 1, known ? RECIPES[i].name : "? ? ?");
        text_draw(s, 8, y + 1, buf, known ? COL_INK : COL_GREY_DARK, 1);
        if (known)
            draw_image(s, &IMG_icons_heart, 234, y - 2);
    }
    ui_button(s, back_rect(), "Retour", BTN_STYLE_PRIMARY);
    ui_button(s, up_rect(), "^", g_scroll > 0 ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, down_rect(), "v", g_scroll + ROWS < RECIPE_COUNT ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
}

static void book_enter(void)
{
    g_sel = 0;
    g_scroll = 0;
    ui_polka_background(screen_background(SCREEN_BOTTOM), COL_CREAM, COL_CREAM_DARK);
    screen_set_dynamic(SCREEN_TOP, draw_page);
    screen_set_dynamic(SCREEN_BOTTOM, draw_list);
    watch_reset(&w_list);
    watch_reset(&w_page);
}

static void select_recipe(int i)
{
    if (i < 0 || i >= RECIPE_COUNT)
        return;
    g_sel = i;
    if (g_sel < g_scroll)
        g_scroll = g_sel;
    if (g_sel >= g_scroll + ROWS)
        g_scroll = g_sel - ROWS + 1;
}

static void book_update(void)
{
    const Input *in = input_get();
    if (in->down & BTN_UP)
        select_recipe(g_sel - 1);
    if (in->down & BTN_DOWN)
        select_recipe(g_sel + 1);
    if (in->touch_down && in->y >= LIST_Y && in->y < LIST_Y + ROWS * ROW_H)
        select_recipe(g_scroll + (in->y - LIST_Y) / ROW_H);
    Rect u = up_rect(), d = down_rect(), b = back_rect();
    if (input_tapped_in(u.x, u.y, u.w, u.h) && g_scroll > 0)
        g_scroll -= ROWS / 2 < g_scroll ? ROWS / 2 : g_scroll;
    if (input_tapped_in(d.x, d.y, d.w, d.h) && g_scroll + ROWS < RECIPE_COUNT) {
        g_scroll += ROWS / 2;
        if (g_scroll + ROWS > RECIPE_COUNT)
            g_scroll = RECIPE_COUNT - ROWS;
    }
    if (input_tapped_in(b.x, b.y, b.w, b.h) || (in->down & (BTN_B | BTN_START))) {
        plat_sfx(SFX_CLICK);
        scene_change(&SCENE_SHOP);
        return;
    }
    uint32_t h = hash_add(hash_begin(), g_sel);
    watch(SCREEN_TOP, &w_page, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
    h = hash_add(h, g_scroll);
    watch(SCREEN_BOTTOM, &w_list, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
}

const Scene SCENE_BOOK = {
    .name = "book",
    .enter = book_enter,
    .update = book_update,
    .exit = NULL,
};
