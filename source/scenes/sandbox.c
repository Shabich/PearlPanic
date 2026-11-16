// Bac à sable du jalon M1 : vérifie les deux écrans, les sprites et le stylet.
//
// Écran du haut : trois clients au comptoir + infos de debug.
// Écran du bas  : un bac de perles à gauche et un gobelet au centre.
//                 Glisser une perle du bac jusqu'au gobelet l'ajoute.
// START : retour à l'écran titre.
#include <stdio.h>

#include "core/input.h"
#include "gfx/palette.h"
#include "gfx/sprites.h"
#include "scenes/scenes.h"

#include "sprites/cup.h"
#include "sprites/customer.h"
#include "sprites/pearl.h"

// Zones tactiles de l'écran du bas
#define BIN_X 16
#define BIN_Y 120
#define BIN_W 48
#define BIN_H 48

#define CUP_X 112
#define CUP_Y 104
#define CUP_SIZE 32

#define CUSTOMER_COUNT 3

static SpriteGfx g_customer_gfx, g_cup_gfx, g_pearl_gfx;
static int g_customer_ids[CUSTOMER_COUNT];
static int g_cup_id, g_held_pearl_id, g_bin_pearl_id;
static bool g_holding_pearl;
static int g_pearls_in_cup;
static int g_taps;
static int g_frame;

static void draw_bottom_labels(void)
{
    consoleSelect(video_console(SCREEN_BOTTOM));
    printf("\x1b[13;2HPerles");
    printf("\x1b[21;12HGobelet : %2d perle(s)", g_pearls_in_cup);
}

static void sandbox_enter(void)
{
    g_frame = 0;
    g_holding_pearl = false;
    g_pearls_in_cup = 0;
    g_taps = 0;
    sprites_reset();

    video_set_backdrop(SCREEN_TOP, COL_CREAM);
    video_set_backdrop(SCREEN_BOTTOM, COL_MINT);

    consoleSelect(video_console(SCREEN_TOP));
    consoleClear();
    consoleSetColor(video_console(SCREEN_TOP), CONSOLE_BLACK);
    printf("\x1b[1;1HComptoir (ecran du haut)");
    printf("\x1b[17;0H________________________________");

    consoleSelect(video_console(SCREEN_BOTTOM));
    consoleClear();
    consoleSetColor(video_console(SCREEN_BOTTOM), CONSOLE_BLACK);
    printf("\x1b[0;1HPoste d'assemblage (tactile)");
    printf("\x1b[23;1HSTART : titre");
    draw_bottom_labels();

    g_customer_gfx = sprite_gfx_load(SCREEN_TOP, SpriteSize_32x32,
                                     customerTiles, customerTilesLen,
                                     customerPal, customerPalLen);
    g_cup_gfx = sprite_gfx_load(SCREEN_BOTTOM, SpriteSize_32x32,
                                cupTiles, cupTilesLen, cupPal, cupPalLen);
    g_pearl_gfx = sprite_gfx_load(SCREEN_BOTTOM, SpriteSize_16x16,
                                  pearlTiles, pearlTilesLen, pearlPal, pearlPalLen);

    for (int i = 0; i < CUSTOMER_COUNT; i++)
        g_customer_ids[i] = sprite_alloc(SCREEN_TOP);
    g_held_pearl_id = sprite_alloc(SCREEN_BOTTOM);  // id le plus bas = devant
    g_cup_id = sprite_alloc(SCREEN_BOTTOM);
    g_bin_pearl_id = sprite_alloc(SCREEN_BOTTOM);
}

static void sandbox_update(void)
{
    const Input *in = input_get();
    g_frame++;

    // --- Écran du haut : clients qui sautillent, décalés dans le temps
    for (int i = 0; i < CUSTOMER_COUNT; i++) {
        int t = (g_frame + i * 11) & 31;
        int bob = (t < 16 ? t : 31 - t) / 4;
        sprite_show(g_customer_ids[i], &g_customer_gfx,
                    40 + i * 72, 104 - bob, 1, i == 2);
    }

    // --- Écran du bas : glisser-déposer une perle
    if (in->touch_down &&
        in->x >= BIN_X && in->x < BIN_X + BIN_W &&
        in->y >= BIN_Y && in->y < BIN_Y + BIN_H)
        g_holding_pearl = true;

    if (in->touch_up && g_holding_pearl) {
        g_holding_pearl = false;
        if (in->x >= CUP_X && in->x < CUP_X + CUP_SIZE &&
            in->y >= CUP_Y - 16 && in->y < CUP_Y + CUP_SIZE) {
            if (g_pearls_in_cup < 99)
                g_pearls_in_cup++;
            draw_bottom_labels();
        }
    }
    if (in->tapped)
        g_taps++;

    sprite_show(g_bin_pearl_id, &g_pearl_gfx, BIN_X + 16, BIN_Y + 16, 1, false);
    sprite_show(g_cup_id, &g_cup_gfx, CUP_X, CUP_Y, 1, false);
    if (g_holding_pearl)
        sprite_show(g_held_pearl_id, &g_pearl_gfx, in->x - 8, in->y - 8, 0, false);
    else
        sprite_hide(SCREEN_BOTTOM, g_held_pearl_id);

    // --- Debug (écran du haut)
    consoleSelect(video_console(SCREEN_TOP));
    printf("\x1b[19;1HStylet  x:%3d y:%3d  %s   ", in->x, in->y,
           in->dragging ? "drag" : in->touching ? "pose" : "leve");
    printf("\x1b[20;1HTaps : %d   Frame : %d   ", g_taps, g_frame);
    printf("\x1b[21;1HBoutons : %08lx", (unsigned long)in->held);

    if (in->down & KEY_START)
        scene_change(&SCENE_TITLE);
}

const Scene SCENE_SANDBOX = {
    .name = "sandbox",
    .enter = sandbox_enter,
    .update = sandbox_update,
    .exit = NULL,
};
