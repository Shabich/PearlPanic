// Écran titre (tâche 8.1)
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

static uint32_t g_frame;
static Watch w_parade;
static bool g_has_save;

static Rect continue_rect(void) { return rect_make(48, 70, 160, 30); }
static Rect new_game_rect(void) { return rect_make(48, g_has_save ? 112 : 80, 160, 30); }

static void draw_parade(Surface *s)
{
    // Les clients défilent devant la boutique
    for (int i = 0; i < CT_COUNT; i++) {
        int x = (int)((g_frame / 2 + i * 48) % (SCREEN_W + 48)) - 44;
        int hop = ((g_frame / 6 + i) & 3) == 0 ? -2 : 0;
        draw_image(s, customer_image(i, (g_frame / 90 + i) % 2 ? 0 : 1), x, 132 + hop);
    }
}

static void draw_title_bottom(Surface *s)
{
    char label[32];
    if (g_has_save) {
        snprintf(label, sizeof(label), "Continuer (jour %d)", progress()->day);
        ui_button(s, continue_rect(), label, BTN_STYLE_PRIMARY);
    }
    ui_button(s, new_game_rect(), "Nouvelle partie", g_has_save ? BTN_STYLE_NORMAL : BTN_STYLE_PRIMARY);
}

static void title_enter(void)
{
    g_frame = 0;
    plat_music(true);
    plat_music_tempo(1000);
    g_has_save = progress_has_save();

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
    text_draw_aligned(bot, 0, 20, SCREEN_W, "Touche un bouton avec le stylet", COL_INK_SOFT, ALIGN_CENTER);
    text_draw_aligned(bot, 0, 170, SCREEN_W, "START : jouer", COL_INK_SOFT, ALIGN_CENTER);

    screen_set_dynamic(SCREEN_TOP, draw_parade);
    screen_set_dynamic(SCREEN_BOTTOM, draw_title_bottom);
    watch_reset(&w_parade);
}

static void start(bool fresh)
{
    plat_sfx(SFX_BELL);
    if (fresh) {
        progress_new();
        progress_save();
    }
    shop_begin_day();
    scene_change(&SCENE_SHOP);
}

static void title_update(void)
{
    const Input *in = input_get();
    g_frame++;
    if ((g_frame & 1) == 0)
        screen_dirty(SCREEN_TOP, rect_make(0, 128, SCREEN_W, 52));

    Rect c = continue_rect(), n = new_game_rect();
    if (g_has_save && (input_tapped_in(c.x, c.y, c.w, c.h) || (in->down & (BTN_START | BTN_A))))
        start(false);
    else if (input_tapped_in(n.x, n.y, n.w, n.h) || (!g_has_save && (in->down & (BTN_START | BTN_A))))
        start(true);
}

const Scene SCENE_TITLE = {
    .name = "title",
    .enter = title_enter,
    .update = title_update,
    .exit = NULL,
};
