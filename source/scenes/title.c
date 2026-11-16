// Écran titre provisoire (la vraie version = tâche 8.1)
#include <stdio.h>

#include "core/input.h"
#include "gfx/palette.h"
#include "gfx/sprites.h"
#include "scenes/scenes.h"

#include "sprites/customer.h"

static SpriteGfx g_customer_gfx;
static int g_customer_id;
static int g_frame;

static void title_enter(void)
{
    g_frame = 0;
    sprites_reset();

    video_set_backdrop(SCREEN_TOP, COL_PINK);
    video_set_backdrop(SCREEN_BOTTOM, COL_CREAM);

    consoleSelect(video_console(SCREEN_TOP));
    consoleClear();
    consoleSetColor(video_console(SCREEN_TOP), CONSOLE_BLACK);
    printf("\x1b[6;10HPEARL PANIC");
    printf("\x1b[8;8Hbubble tea rush");

    consoleSelect(video_console(SCREEN_BOTTOM));
    consoleClear();
    consoleSetColor(video_console(SCREEN_BOTTOM), CONSOLE_BLACK);
    printf("\x1b[11;5HTouche l'ecran pour");
    printf("\x1b[12;10Hcommencer");

    g_customer_gfx = sprite_gfx_load(SCREEN_TOP, SpriteSize_32x32,
                                     customerTiles, customerTilesLen,
                                     customerPal, customerPalLen);
    g_customer_id = sprite_alloc(SCREEN_TOP);
}

static void title_update(void)
{
    const Input *in = input_get();
    g_frame++;

    // Petit rebond : onde triangle de période 32 frames
    int t = g_frame & 31;
    int bob = (t < 16 ? t : 31 - t) / 4;
    sprite_show(g_customer_id, &g_customer_gfx, 112, 104 - bob, 0, false);

    if (in->tapped || (in->down & (KEY_START | KEY_A)))
        scene_change(&SCENE_SANDBOX);
}

const Scene SCENE_TITLE = {
    .name = "title",
    .enter = title_enter,
    .update = title_update,
    .exit = NULL,
};
