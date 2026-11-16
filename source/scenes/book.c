// Livre de recettes (tâche 5.6) : ouvert depuis la pause
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/world.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"
#include "scenes/cup_anim.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

#define LIST_Y     22
#define ROW_H      14
#define ROWS       10

static int g_sel, g_scroll;     // position dans la liste du bar affiché
static int g_world;
static int g_list[RECIPE_MAX], g_n;
static const Scene *g_back = &SCENE_MAP;

void book_open_from(const Scene *back, int world)
{
    g_back = back;
    g_world = world;
}

static void build_list(void)
{
    g_n = 0;
    for (int i = 0; i < RECIPE_COUNT; i++)
        if (RECIPES[i].world == g_world)
            g_list[g_n++] = i;
}
#define CUR (g_list[g_sel])
static int g_page_drawn = -1;
static uint32_t g_frame, g_page_frame;
static CupAnim g_cup;
static Drink g_drink;
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

// Écran haut : la page (fond, redessiné quand la sélection change)…
static void draw_page_background(Surface *s)
{
    const Recipe *r = &RECIPES[CUR];
    bool known = progress_recipe_known(CUR);
    char buf[128];
    ui_polka_background(s, COL_CREAM, COL_CREAM_DARK);
    draw_round_rect(s, 6, 6, 244, 180, COL_WHITE, COL_LAV_DARK);
    if (known) {
        text_draw_shadow(s, 14, 12, r->name, COL_PINK_DARK, COL_PINK_PALE, 1);
        snprintf(buf, sizeof(buf), "%d pièces", r->price);
        text_draw(s, 240 - text_width(buf, 1), 12, buf, COL_INK_SOFT, 1);
        recipe_describe(CUR, buf, sizeof(buf));
        text_draw_wrapped(s, 14, 30, 130, buf, COL_INK, ALIGN_LEFT);
    } else {
        text_draw(s, 14, 12, "??? (recette inconnue)", COL_INK_SOFT, 1);
        if (r->secret) {
            text_draw_wrapped(s, 14, 34, 228,
                              "Une recette secrète… Expérimente au comptoir pour la trouver !",
                              COL_LAV_DARK, ALIGN_LEFT);
        } else if (!progress_level_unlocked(recipe_level(CUR))) {
            snprintf(buf, sizeof(buf), "Arrive au niveau %d-%d.", r->world + 1, r->level + 1);
            text_draw(s, 14, 34, buf, COL_INK_SOFT, 1);
        } else {
            text_draw(s, 14, 34, "Un client l'a décrite ainsi :", COL_INK_SOFT, 1);
            snprintf(buf, sizeof(buf), "« %s »", r->riddle);
            text_draw_wrapped(s, 14, 50, 228, buf, COL_LAV_DARK, ALIGN_LEFT);
        }
    }
}

// … et le gobelet animé : les garnitures retombent à chaque page
#define BOOK_CUP_X 196
#define BOOK_CUP_DY (-10)
static Rect book_cup_rect(void) { return rect_make(BOOK_CUP_X - 40, 40, 80, 140); }

static void draw_page(Surface *s)
{
    if (progress_recipe_known(CUR))
        draw_cup(s, BOOK_CUP_X, BOOK_CUP_DY, &g_drink, &g_cup, g_frame, 0, 0);
}

// Écran bas : la liste
static void draw_list(Surface *s)
{
    char buf[48];
    draw_rect(s, 0, 0, SCREEN_W, 18, COL_LAVENDER);
    int known = progress_known_in_world(g_world, NULL);
    snprintf(buf, sizeof(buf), "%s  %d/%d", WORLDS[g_world].name, known, g_n);
    text_draw_aligned(s, 0, 4, SCREEN_W, buf, COL_INK, ALIGN_CENTER);
    // L / R : changer de bar
    if (g_world > 0)
        text_draw(s, 4, 4, "<L", COL_LAV_DARK, 1);
    if (g_world + 1 < WORLD_COUNT && progress_world_unlocked(g_world + 1))
        text_draw(s, 238, 4, "R>", COL_LAV_DARK, 1);
    for (int row = 0; row < ROWS; row++) {
        int i = g_scroll + row;
        if (i >= g_n)
            break;
        int y = LIST_Y + row * ROW_H;
        bool known = progress_recipe_known(g_list[i]);
        if (i == g_sel)
            draw_rect(s, 4, y - 1, 248, ROW_H, COL_PINK_PALE);
        snprintf(buf, sizeof(buf), "%2d. %s", i + 1, known ? RECIPES[g_list[i]].name : "? ? ?");
        text_draw(s, 8, y + 1, buf, known ? COL_INK : COL_GREY_DARK, 1);
        if (known)
            draw_image(s, &IMG_icons_heart, 234, y - 2);
    }
    ui_button(s, back_rect(), "Retour", BTN_STYLE_PRIMARY);
    ui_button(s, up_rect(), "^", g_scroll > 0 ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, down_rect(), "v", g_scroll + ROWS < g_n ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
}

static void book_enter(void)
{
    g_sel = 0;
    g_scroll = 0;
    g_page_drawn = -1;
    build_list();
    ui_polka_background(screen_background(SCREEN_BOTTOM), COL_CREAM, COL_CREAM_DARK);
    screen_set_dynamic(SCREEN_TOP, draw_page);
    screen_set_dynamic(SCREEN_BOTTOM, draw_list);
    watch_reset(&w_list);
    watch_reset(&w_page);
}

static void select_recipe(int i)
{
    if (i < 0 || i >= g_n)
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
    g_frame++;
    int dw = (in->down & BTN_L) ? -1 : (in->down & BTN_R) ? 1 : 0;
    if (in->touch_down && in->y < 18)
        dw = in->x < 40 ? -1 : in->x > 216 ? 1 : 0;
    if (dw && g_world + dw >= 0 && g_world + dw < WORLD_COUNT && progress_world_unlocked(g_world + dw)) {
        g_world += dw;
        build_list();
        g_sel = g_scroll = 0;
        g_page_drawn = -1;
        plat_sfx(SFX_CLICK);
    }
    if (in->down & BTN_UP)
        select_recipe(g_sel - 1);
    if (in->down & BTN_DOWN)
        select_recipe(g_sel + 1);
    if (in->touch_down && in->y >= LIST_Y && in->y < LIST_Y + ROWS * ROW_H)
        select_recipe(g_scroll + (in->y - LIST_Y) / ROW_H);
    Rect u = up_rect(), d = down_rect(), b = back_rect();
    if (input_tapped_in(u.x, u.y, u.w, u.h) && g_scroll > 0)
        g_scroll -= ROWS / 2 < g_scroll ? ROWS / 2 : g_scroll;
    if (input_tapped_in(d.x, d.y, d.w, d.h) && g_scroll + ROWS < g_n) {
        g_scroll += ROWS / 2;
        if (g_scroll + ROWS > g_n)
            g_scroll = g_n - ROWS;
    }
    if (input_tapped_in(b.x, b.y, b.w, b.h) || (in->down & (BTN_B | BTN_START))) {
        plat_sfx(SFX_CLICK);
        scene_change(g_back);
        return;
    }
    if (g_page_drawn != g_sel) {
        g_page_drawn = g_sel;
        draw_page_background(screen_background(SCREEN_TOP));
        screen_dirty_all(SCREEN_TOP);
        // On « prépare » la boisson : les garnitures vont tomber dans le gobelet
        make_drink(CUR, &g_drink);
        g_drink.sealed = g_drink.shaken = false;   // fermé une fois les garnitures tombées
        cup_anim_reset(&g_cup);
        g_page_frame = g_frame;
    }
    if (g_frame - g_page_frame == 45) {
        g_drink.sealed = true;
        g_drink.shaken = RECIPES[CUR].shaken;
    }
    cup_anim_update(&g_cup, &g_drink, false, 0, g_frame);
    if ((g_frame & 1) == 0)
        screen_dirty(SCREEN_TOP, book_cup_rect());
    uint32_t h = hash_add(hash_begin(), g_sel);
    h = hash_add(h, g_scroll);
    watch(SCREEN_BOTTOM, &w_list, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
}

const Scene SCENE_BOOK = {
    .name = "book",
    .enter = book_enter,
    .update = book_update,
    .exit = NULL,
};
