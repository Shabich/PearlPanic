#include "gfx/video.h"

static PrintConsole g_console[2];

void video_init(void)
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_SPRITE,
                        VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);

    // Le moteur principal affiche en haut, le tactile reste en bas
    lcdMainOnTop();

    consoleInit(&g_console[SCREEN_TOP], 0, BgType_Text4bpp, BgSize_T_256x256,
                31, 0, true, true);
    consoleInit(&g_console[SCREEN_BOTTOM], 0, BgType_Text4bpp, BgSize_T_256x256,
                31, 0, false, true);
}

void video_set_backdrop(Screen s, u16 color)
{
    if (s == SCREEN_TOP)
        BG_PALETTE[0] = color;
    else
        BG_PALETTE_SUB[0] = color;
}

PrintConsole *video_console(Screen s)
{
    return &g_console[s];
}

OamState *video_oam(Screen s)
{
    return s == SCREEN_TOP ? &oamMain : &oamSub;
}
