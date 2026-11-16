// Implémentation Nintendo DS de la couche plateforme (libnds + maxmod)
//
// Vidéo : les deux moteurs en MODE_5_2D avec un fond bitmap 16 bits (BG3)
//   VRAM A -> fond bitmap de l'écran du haut (128 Ko)
//   VRAM C -> fond bitmap de l'écran du bas  (128 Ko)
// Son   : maxmod, banque générée par mmutil depuis audio/
// Sauvegarde : fichier sur la carte SD (DLDI / SD du DSi) via libfat
#include <stdio.h>
#include <time.h>

#include <fat.h>
#include <maxmod9.h>
#include <nds.h>

#include "core/app.h"
#include "plat/plat.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SAVE_PATH "pearlpanic.sav"
#define POUR_SOUND_PERIOD 12

static Surface g_screens[2];
static bool g_fat_ok;
static bool g_pour_on;
static int g_pour_timer;
static bool g_music_on;
static uint32_t g_frame;
static int g_music_volume = 4;
static int g_music_tempo = -1;
static int g_load;

static const mm_word SFX_MAP[SFX_COUNT] = {
    [SFX_POUR]   = SFX_SND_POUR,
    [SFX_PLOP]   = SFX_SND_PLOP,
    [SFX_PUMP]   = SFX_SND_PUMP,
    [SFX_SEAL]   = SFX_SND_SEAL,
    [SFX_SHAKE]  = SFX_SND_SHAKE,
    [SFX_TRASH]  = SFX_SND_TRASH,
    [SFX_CASH]   = SFX_SND_CASH,
    [SFX_HAPPY]  = SFX_SND_HAPPY,
    [SFX_ANGRY]  = SFX_SND_ANGRY,
    [SFX_UNLOCK] = SFX_SND_UNLOCK,
    [SFX_BELL]   = SFX_SND_BELL,
    [SFX_CLICK]  = SFX_SND_CLICK,
};

void plat_init(void)
{
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);
    vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_LCD, VRAM_C_SUB_BG, VRAM_D_LCD);

    int top = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bottom = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    surface_init(&g_screens[0], bgGetGfxPtr(top), SCREEN_W, SCREEN_H, 256);
    surface_init(&g_screens[1], bgGetGfxPtr(bottom), SCREEN_W, SCREEN_H, 256);

    // Écran tactile en bas
    lcdMainOnTop();

    mmInitDefaultMem((mm_addr)soundbank_bin);
    for (int i = 0; i < SFX_COUNT; i++)
        mmLoadEffect(SFX_MAP[i]);
    mmLoadEffect(SFX_SND_BLIP);
    mmLoad(MOD_MUSIC_SHOP);

    g_fat_ok = fatInitDefault();
}

Surface *plat_screen(int screen)
{
    return &g_screens[screen & 1];
}

void plat_poll(RawInput *in)
{
    scanKeys();
    uint32_t keys = keysHeld();
    in->keys = keys & 0x0FFF;
    in->touching = (keys & KEY_TOUCH) != 0;
    if (in->touching) {
        touchPosition tp;
        touchRead(&tp);
        in->tx = tp.px;
        in->ty = tp.py;
    } else {
        in->tx = in->ty = 0;
    }
}

void plat_wait_vblank(void)
{
    // Le calcul de la frame commence au début du VBlank (ligne 192)
    int line = REG_VCOUNT;
    g_load = ((line - 192 + 263) % 263) * 100 / 263;
    swiWaitForVBlank();
    g_frame++;
    if (g_pour_on && --g_pour_timer <= 0) {
        mmEffect(SFX_SND_POUR);
        g_pour_timer = POUR_SOUND_PERIOD;
    }
}

uint32_t plat_entropy(void)
{
    return (uint32_t)time(NULL) ^ (g_frame * 2654435761u) ^ ((uint32_t)REG_VCOUNT << 16);
}

void plat_sfx(SfxId id)
{
    if ((unsigned)id < SFX_COUNT)
        mmEffect(SFX_MAP[id]);
}

void plat_pour_loop(bool on)
{
    if (on && !g_pour_on)
        g_pour_timer = 0;
    g_pour_on = on;
}

void plat_music(bool on)
{
    if (on == g_music_on)
        return;
    g_music_on = on;
    if (on) {
        mmStart(MOD_MUSIC_SHOP, MM_PLAY_LOOP);
        mmSetModuleVolume(g_music_volume * 180);
    } else {
        mmStop();
    }
}

void plat_music_tempo(int permille)
{
    // maxmod : 1024 = tempo normal (plage 512..2048)
    int t = 1024 * permille / 1000;
    t = t < 512 ? 512 : t > 2048 ? 2048 : t;
    if (t == g_music_tempo)
        return;
    g_music_tempo = t;
    mmSetModuleTempo(t);
}

void plat_set_volumes(int music, int sfx)
{
    g_music_volume = music;
    mmSetModuleVolume(music * 180);
    mmSetEffectsVolume(sfx * 256);
}

void plat_voice(int pitch_permille)
{
    mm_sound_effect fx = {
        .id = SFX_SND_BLIP,
        .rate = (mm_hword)(1024 * pitch_permille / 1000),
        .handle = 0,
        .volume = 200,
        .panning = 128,
    };
    mmEffectEx(&fx);
}

void plat_brightness(int level)
{
    setBrightness(3, level);
}

int plat_frame_load(void)
{
    return g_load;
}

bool plat_save_read(void *buf, uint32_t size)
{
    if (!g_fat_ok)
        return false;
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f)
        return false;
    bool ok = fread(buf, 1, size, f) == size;
    fclose(f);
    return ok;
}

bool plat_save_write(const void *buf, uint32_t size)
{
    if (!g_fat_ok)
        return false;
    FILE *f = fopen(SAVE_PATH, "wb");
    if (!f)
        return false;
    bool ok = fwrite(buf, 1, size, f) == size;
    fclose(f);
    return ok;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    plat_init();
    app_init();
    while (1)
        app_frame();
    return 0;
}
