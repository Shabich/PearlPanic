#include "core/app.h"

#include <stdio.h>

#include "core/input.h"
#include "core/scene.h"
#include "game/progress.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"

// Overlay de debug (tâche 1.12) : SELECT + L
static bool g_debug;
static int g_load_avg;

static void draw_debug(Surface *s)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "CPU %2d%%", g_load_avg);
    draw_rect(s, 0, 0, 48, 12, COL_INK);
    text_draw(s, 2, 1, buf, g_load_avg > 85 ? COL_BAD : COL_GOOD, 1);
}

void app_init(void)
{
    screen_init();
    progress_load();
    plat_set_volumes(progress()->music_volume, progress()->sfx_volume);
    scene_change(&SCENE_TITLE);
}

void app_frame(void)
{
    RawInput raw;
    plat_poll(&raw);
    input_feed(&raw);

    const Input *in = input_get();
    if ((in->held & BTN_SELECT) && (in->down & BTN_L)) {
        g_debug = !g_debug;
        screen_set_overlay(SCREEN_TOP, g_debug ? draw_debug : NULL);
    }
    if (g_debug) {
        g_load_avg = (g_load_avg * 7 + plat_frame_load()) / 8;
        screen_dirty(SCREEN_TOP, rect_make(0, 0, 48, 12));
    }

    scene_update();
    plat_wait_vblank();
    screen_present();
}
