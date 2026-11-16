// Options (tâche 8.5) : volumes, mode gaucher, effacement de la sauvegarde
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/decor.h"
#include "scenes/scenes.h"
#include "scenes/ui.h"

typedef enum { OPT_MUSIC, OPT_SFX, OPT_LEFTY, OPT_ERASE, OPT_COUNT } OptionRow;

static int g_confirm_erase;
static bool g_erased;
static Watch w_opts;
static uint32_t g_frame;

static int row_y(int r) { return 30 + r * 30; }
static Rect minus_rect(int r) { return rect_make(124, row_y(r), 28, 24); }
static Rect plus_rect(int r) { return rect_make(220, row_y(r), 28, 24); }
static Rect toggle_rect(int r) { return rect_make(124, row_y(r), 124, 24); }
static Rect back_rect(void) { return rect_make(72, 160, 112, 26); }

static void draw_volume(Surface *s, int r, const char *label, int value)
{
    text_draw(s, 10, row_y(r) + 7, label, COL_INK, 1);
    ui_button(s, minus_rect(r), "-", value > 0 ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    ui_button(s, plus_rect(r), "+", value < 4 ? BTN_STYLE_NORMAL : BTN_STYLE_DISABLED);
    for (int i = 0; i < 4; i++)
        draw_round_rect(s, 158 + i * 15, row_y(r) + 5, 12, 14, i < value ? COL_PINK : COL_CREAM_DARK,
                        COL_PINK_DARK);
}

static void draw_options(Surface *s)
{
    const Progress *p = progress();
    decor_polka(s, COL_CREAM, COL_CREAM_DARK, g_frame);
    draw_rect(s, 0, 0, SCREEN_W, 20, COL_LAVENDER);
    text_draw_aligned(s, 0, 5, SCREEN_W, "Options", COL_INK, ALIGN_CENTER);
    draw_volume(s, OPT_MUSIC, "Musique", p->music_volume);
    draw_volume(s, OPT_SFX, "Effets sonores", p->sfx_volume);
    text_draw(s, 10, row_y(OPT_LEFTY) + 7, "Mode gaucher", COL_INK, 1);
    ui_button(s, toggle_rect(OPT_LEFTY), p->lefty ? "Oui (bacs à droite)" : "Non",
              p->lefty ? BTN_STYLE_PRIMARY : BTN_STYLE_NORMAL);
    text_draw(s, 10, row_y(OPT_ERASE) + 7, "Sauvegarde", COL_INK, 1);
    ui_button(s, toggle_rect(OPT_ERASE),
              g_erased ? "Effacée" : g_confirm_erase ? "Sûr ? Touche encore" : "Tout effacer",
              g_erased ? BTN_STYLE_DISABLED : g_confirm_erase ? BTN_STYLE_PRESSED : BTN_STYLE_NORMAL);
    ui_button(s, back_rect(), "Retour", BTN_STYLE_PRIMARY);
}

static void draw_info(Surface *s)
{
    (void)s;
}

static void options_enter(void)
{
    g_confirm_erase = 0;
    g_erased = false;
    Surface *top = screen_background(SCREEN_TOP);
    ui_polka_background(top, COL_LAVENDER, color_mix(COL_LAVENDER, COL_WHITE, 120));
    text_draw_shadow(top, (SCREEN_W - text_width("Options", 2)) / 2, 30, "Options", COL_LAV_DARK,
                     COL_WHITE, 2);
    text_draw_wrapped(top, 24, 70, 208,
                      "Le mode gaucher inverse les bacs de garnitures et les boutons d'action "
                      "sur l'écran tactile.\n\nSELECT + L en jeu : affiche la charge du processeur.",
                      COL_INK, ALIGN_CENTER);
    ui_polka_background(screen_background(SCREEN_BOTTOM), COL_CREAM, COL_CREAM_DARK);
    screen_set_dynamic(SCREEN_TOP, draw_info);
    screen_set_dynamic(SCREEN_BOTTOM, draw_options);
    watch_reset(&w_opts);
}

static bool tapped(Rect r) { return input_tapped_in(r.x, r.y, r.w, r.h); }

static void change_volume(uint8_t *v, int delta)
{
    int n = *v + delta;
    if (n < 0 || n > 4)
        return;
    *v = (uint8_t)n;
    plat_set_volumes(progress()->music_volume, progress()->sfx_volume);
    plat_sfx(SFX_CLICK);
}

static void options_update(void)
{
    const Input *in = input_get();
    Progress *p = progress();
    g_frame++;
    if ((g_frame & 3) == 0)
        screen_dirty_all(SCREEN_BOTTOM);
    if (g_confirm_erase)
        g_confirm_erase--;
    if (scene_transitioning())
        return;

    if (tapped(minus_rect(OPT_MUSIC))) change_volume(&p->music_volume, -1);
    if (tapped(plus_rect(OPT_MUSIC)))  change_volume(&p->music_volume, +1);
    if (tapped(minus_rect(OPT_SFX)))   change_volume(&p->sfx_volume, -1);
    if (tapped(plus_rect(OPT_SFX)))    change_volume(&p->sfx_volume, +1);
    if (tapped(toggle_rect(OPT_LEFTY))) {
        p->lefty = !p->lefty;
        plat_sfx(SFX_CLICK);
    }
    if (tapped(toggle_rect(OPT_ERASE)) && !g_erased) {
        if (g_confirm_erase) {
            progress_erase();
            plat_set_volumes(p->music_volume, p->sfx_volume);
            g_erased = true;
            plat_sfx(SFX_TRASH);
        } else {
            g_confirm_erase = 150;
            plat_sfx(SFX_CLICK);
        }
    }
    if (tapped(back_rect()) || (in->down & (BTN_B | BTN_START))) {
        if (!g_erased)
            progress_save();
        scene_change(&SCENE_TITLE);
    }

    uint32_t h = hash_add(hash_begin(), p->music_volume);
    h = hash_add(h, p->sfx_volume);
    h = hash_add(h, p->lefty);
    h = hash_add(h, g_confirm_erase > 0);
    h = hash_add(h, g_erased);
    watch(SCREEN_BOTTOM, &w_opts, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
}

const Scene SCENE_OPTIONS = {
    .name = "options",
    .enter = options_enter,
    .update = options_update,
    .exit = NULL,
};
