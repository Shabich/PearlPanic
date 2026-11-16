// Écran titre (tâche 8.1)
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/fx.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

typedef enum { TB_CONTINUE, TB_NEW, TB_RUSH, TB_OPTIONS, TB_COUNT } TitleButton;

static uint32_t g_frame;
static bool g_has_save;
static int g_confirm_new;    // > 0 : « Nouvelle partie » attend une confirmation
static Watch w_buttons;

static bool rush_unlocked(void) { return progress()->day >= 2; }

static Rect button_rect(TitleButton b) { return rect_make(48, 40 + b * 32, 160, 26); }

static void draw_parade(Surface *s)
{
    // Les clients défilent devant la boutique
    for (int i = 0; i < CT_COUNT; i++) {
        int x = (int)((g_frame / 2 + i * 48) % (SCREEN_W + 48)) - 44;
        int hop = ((g_frame / 6 + i) & 3) == 0 ? -2 : 0;
        draw_image(s, customer_image(i, (g_frame / 90 + i) % 2 ? 0 : 1), x, 132 + hop);
    }
    fx_draw(SCREEN_TOP, s);
}

static void draw_title_bottom(Surface *s)
{
    char label[40];
    if (g_has_save) {
        snprintf(label, sizeof(label), "Continuer (jour %d)", progress()->day);
        ui_button(s, button_rect(TB_CONTINUE), label, BTN_STYLE_PRIMARY);
    } else {
        ui_button(s, button_rect(TB_CONTINUE), "Pas de sauvegarde", BTN_STYLE_DISABLED);
    }
    ui_button(s, button_rect(TB_NEW), g_confirm_new ? "Sûr ? Touche encore" : "Nouvelle partie",
              g_has_save ? BTN_STYLE_NORMAL : BTN_STYLE_PRIMARY);
    if (rush_unlocked()) {
        if (progress()->best_rush)
            snprintf(label, sizeof(label), "Mode Rush (record %lu)", (unsigned long)progress()->best_rush);
        else
            snprintf(label, sizeof(label), "Mode Rush");
        ui_button(s, button_rect(TB_RUSH), label, BTN_STYLE_NORMAL);
    } else {
        ui_button(s, button_rect(TB_RUSH), "Rush : finis le jour 1", BTN_STYLE_DISABLED);
    }
    ui_button(s, button_rect(TB_OPTIONS), "Options", BTN_STYLE_NORMAL);
}

static void title_enter(void)
{
    g_frame = 0;
    g_confirm_new = 0;
    plat_music(true);
    plat_music_tempo(1000);
    g_has_save = progress_has_save();
    fx_clear();

    Surface *top = screen_background(SCREEN_TOP);
    ui_polka_background(top, COL_PINK_PALE, COL_WHITE);
    const char *title = "Pearl Panic";
    int tw = text_width(title, 3);
    text_draw_shadow(top, (SCREEN_W - tw) / 2 + 1, 34, title, COL_PINK_DARK, COL_INK, 3);
    text_draw_aligned(top, 0, 74, SCREEN_W, "~ bubble tea rush ~", COL_INK_SOFT, ALIGN_CENTER);
    draw_rect(top, 0, 180, SCREEN_W, 12, COL_WOOD);
    draw_hline(top, 0, 180, SCREEN_W, COL_WOOD_DARK);

    Surface *bot = screen_background(SCREEN_BOTTOM);
    ui_polka_background(bot, COL_CREAM, COL_CREAM_DARK);
    text_draw_aligned(bot, 0, 14, SCREEN_W, "Touche un bouton avec le stylet", COL_INK_SOFT, ALIGN_CENTER);

    screen_set_dynamic(SCREEN_TOP, draw_parade);
    screen_set_dynamic(SCREEN_BOTTOM, draw_title_bottom);
    watch_reset(&w_buttons);
}

static void start_story(bool fresh)
{
    plat_sfx(SFX_BELL);
    if (fresh) {
        progress_new_game();
        progress_save();
    }
    shop_begin_day();
    scene_change(&SCENE_SHOP);
}

static bool tapped(TitleButton b)
{
    Rect r = button_rect(b);
    return input_tapped_in(r.x, r.y, r.w, r.h);
}

static void title_update(void)
{
    const Input *in = input_get();
    g_frame++;
    if ((g_frame & 1) == 0)
        screen_dirty(SCREEN_TOP, rect_make(0, 128, SCREEN_W, 52));
    if ((g_frame % 40) == 0)
        fx_emit(SCREEN_TOP, FX_SPARKLE, SCREEN_W / 2, 46, 2, 0);
    fx_update();
    if (g_confirm_new)
        g_confirm_new--;

    if (scene_transitioning())
        return;
    if (g_has_save && (tapped(TB_CONTINUE) || (in->down & (BTN_START | BTN_A)))) {
        start_story(false);
    } else if (tapped(TB_NEW) || (!g_has_save && (in->down & (BTN_START | BTN_A)))) {
        // Écraser une sauvegarde existante demande un second appui
        if (g_has_save && !g_confirm_new) {
            g_confirm_new = 120;
            plat_sfx(SFX_CLICK);
        } else {
            start_story(true);
        }
    } else if (tapped(TB_RUSH) && rush_unlocked()) {
        plat_sfx(SFX_BELL);
        shop_begin_rush();
        scene_change(&SCENE_SHOP);
    } else if (tapped(TB_OPTIONS)) {
        plat_sfx(SFX_CLICK);
        scene_change(&SCENE_OPTIONS);
    }
    watch(SCREEN_BOTTOM, &w_buttons, rect_make(0, 30, SCREEN_W, 140),
          hash_add(hash_begin(), g_confirm_new > 0));
}

const Scene SCENE_TITLE = {
    .name = "title",
    .enter = title_enter,
    .update = title_update,
    .exit = NULL,
};
