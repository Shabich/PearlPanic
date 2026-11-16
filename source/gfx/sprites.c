#include "gfx/sprites.h"

#include <string.h>

#define OAM_COUNT 128

static bool g_used[2][OAM_COUNT];
static u8   g_next_palette[2];

void sprites_init(void)
{
    sprites_reset();
}

SpriteGfx sprite_gfx_load(Screen s, SpriteSize size,
                          const void *tiles, u32 tiles_len,
                          const void *pal, u32 pal_len)
{
    SpriteGfx g = { .screen = s, .size = size, .gfx = NULL, .palette = 0 };

    g.gfx = oamAllocateGfx(video_oam(s), size, SpriteColorFormat_16Color);
    if (!g.gfx)
        return g;
    dmaCopy(tiles, g.gfx, tiles_len);

    g.palette = g_next_palette[s]++ & 15;
    u16 *pal_dst = (s == SCREEN_TOP ? SPRITE_PALETTE : SPRITE_PALETTE_SUB) + g.palette * 16;
    dmaCopy(pal, pal_dst, pal_len > 32 ? 32 : pal_len);
    return g;
}

void sprite_gfx_free(SpriteGfx *g)
{
    if (g->gfx)
        oamFreeGfx(video_oam(g->screen), g->gfx);
    g->gfx = NULL;
}

int sprite_alloc(Screen s)
{
    for (int i = 0; i < OAM_COUNT; i++) {
        if (!g_used[s][i]) {
            g_used[s][i] = true;
            return i;
        }
    }
    return -1;
}

void sprite_free(Screen s, int id)
{
    if (id < 0 || id >= OAM_COUNT)
        return;
    g_used[s][id] = false;
    sprite_hide(s, id);
}

void sprite_show(int id, const SpriteGfx *g, int x, int y, int priority, bool hflip)
{
    if (id < 0 || !g->gfx)
        return;
    oamSet(video_oam(g->screen), id, x, y, priority, g->palette, g->size,
           SpriteColorFormat_16Color, g->gfx, -1, false, false, hflip, false, false);
}

void sprite_hide(Screen s, int id)
{
    if (id < 0 || id >= OAM_COUNT)
        return;
    oamSetHidden(video_oam(s), id, true);
}

void sprites_reset(void)
{
    memset(g_used, 0, sizeof(g_used));
    memset(g_next_palette, 0, sizeof(g_next_palette));
    oamClear(&oamMain, 0, OAM_COUNT);
    oamClear(&oamSub, 0, OAM_COUNT);
    // Réinitialise l'allocateur de VRAM sprites
    oamInit(&oamMain, SpriteMapping_1D_32, false);
    oamInit(&oamSub, SpriteMapping_1D_32, false);
}

void sprites_commit(void)
{
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}
