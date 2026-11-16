// Joue un scénario scripté sur PC et écrit des captures des deux écrans.
//   make snap   ->  build/snap/*.png
#include <stdio.h>
#include <string.h>

#include "core/app.h"
#include "game/progress.h"
#include "plat_host.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/shop_layout.h"

static const char *g_dir;
static RawInput g_in;

static void frames(int n)
{
    for (int i = 0; i < n; i++) {
        host_set_input(&g_in);
        app_frame();
    }
}

static void snap(const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.ppm", g_dir, name);
    host_write_ppm(path);
}

static void touch(int x, int y) { g_in.touching = true; g_in.tx = x; g_in.ty = y; }
static void release(void) { g_in.touching = false; }

static void tap(int x, int y)
{
    touch(x, y);
    frames(3);
    release();
    frames(2);
}

static void hold(int x, int y, int n)
{
    touch(x, y);
    frames(n);
    release();
    frames(1);
}

static void drag(int x0, int y0, int x1, int y1)
{
    touch(x0, y0);
    frames(2);
    for (int k = 1; k <= 8; k++) {
        touch(x0 + (x1 - x0) * k / 8, y0 + (y1 - y0) * k / 8);
        frames(1);
    }
    release();
    frames(2);
}

static void press(uint32_t keys)
{
    g_in.keys = keys;
    frames(1);
    g_in.keys = 0;
    frames(1);
}

static void center(Rect r, int *x, int *y) { *x = r.x + r.w / 2; *y = r.y + r.h / 2; }

int main(int argc, char **argv)
{
    g_dir = argc > 1 ? argv[1] : ".";
    plat_init();
    app_init();
    int x, y;

    frames(30);
    snap("01_title");

    press(BTN_START);
    frames(FPS + ARRIVE_FRAMES + 10);
    snap("02_day1_first_customer");

    // Thé au lait perles : thé noir 70, lait 30, perles x2
    center(tap_rect(0), &x, &y);
    touch(x, y);
    frames(40);
    snap("03_pouring");
    frames(30);
    release();
    frames(1);
    center(tap_rect(4), &x, &y);
    hold(x, y, 30);
    center(bin_rect(0), &x, &y);
    touch(x, y);
    frames(2);
    touch(CUP_CX + 30, 130);
    frames(2);
    snap("04_dragging_pearl");
    release();
    frames(2);
    drag(x, y, CUP_CX, 140);
    snap("05_assembled");
    center(action_rect(ACT_SEAL), &x, &y);
    tap(x, y);
    snap("06_sealed");
    center(action_rect(ACT_SERVE), &x, &y);
    tap(x, y);
    frames(10);
    snap("07_served");

    // On laisse les clients attendre : stress, puis départs fâchés
    frames(30 * FPS);
    snap("08_waiting");
    frames(25 * FPS);
    snap("09_stress");
    for (int i = 0; i < 200 * FPS && shop_session()->in_progress; i++)
        frames(1);
    frames(5);
    snap("10_day_end");

    // Jour suivant (ou réessai), pause, livre de recettes
    press(BTN_START);
    frames(FPS * 3);
    snap("11_day_next");
    press(BTN_START);
    frames(2);
    snap("12_pause");
    { Rect r = pause_button_rect(1); tap(r.x + r.w / 2, r.y + r.h / 2); }
    frames(2);
    snap("13_book");
    printf("captures écrites dans %s\n", g_dir);
    return 0;
}
