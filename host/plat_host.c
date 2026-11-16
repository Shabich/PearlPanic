// Implémentation PC de la couche plateforme (tests, captures d'écran)
#include "plat_host.h"

#include <stdio.h>
#include <string.h>

static uint16_t g_px[2][SCREEN_W * SCREEN_H];
static Surface g_screens[2];
static RawInput g_input;
static int g_last_sfx = -1;
static uint8_t g_save[256];
static uint32_t g_save_size;

void plat_init(void)
{
    for (int i = 0; i < 2; i++)
        surface_init(&g_screens[i], g_px[i], SCREEN_W, SCREEN_H, SCREEN_W);
}

Surface *plat_screen(int screen)
{
    if (!g_screens[0].px)
        plat_init();
    return &g_screens[screen & 1];
}

void plat_poll(RawInput *in) { *in = g_input; }
void plat_wait_vblank(void) {}
uint32_t plat_entropy(void) { return 0xC0FFEEu; }

void plat_sfx(SfxId id) { g_last_sfx = id; }
void plat_pour_loop(bool on) { (void)on; }
void plat_music(bool on) { (void)on; }
void plat_set_volumes(int music, int sfx) { (void)music; (void)sfx; }
void plat_voice(int pitch_permille) { (void)pitch_permille; }
void plat_brightness(int level) { (void)level; }
int plat_frame_load(void) { return 0; }
void plat_music_tempo(int permille) { (void)permille; }

bool plat_save_read(void *buf, uint32_t size)
{
    if (g_save_size != size)
        return false;
    memcpy(buf, g_save, size);
    return true;
}

bool plat_save_write(const void *buf, uint32_t size)
{
    if (size > sizeof(g_save))
        return false;
    memcpy(g_save, buf, size);
    g_save_size = size;
    return true;
}

void host_set_input(const RawInput *in) { g_input = *in; }
int host_last_sfx(void) { return g_last_sfx; }

bool host_write_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;
    fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H * 2);
    for (int s = 0; s < 2; s++) {
        for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
            uint16_t c = g_px[s][i];
            unsigned char rgb[3] = {
                (unsigned char)((c & 31) * 255 / 31),
                (unsigned char)(((c >> 5) & 31) * 255 / 31),
                (unsigned char)(((c >> 10) & 31) * 255 / 31),
            };
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return true;
}
