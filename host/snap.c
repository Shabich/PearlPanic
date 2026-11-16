// Joue un scénario scripté sur PC et écrit des captures des deux écrans.
//   make snap   ->  build/snap/*.png
#include <stdio.h>
#include <string.h>

#include "core/app.h"
#include "game/progress.h"
#include "game/recipes.h"
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

// Séquence de captures (pour vérifier les animations) : prefix_00, prefix_01…
static void snap_seq(const char *prefix, int count, int interval)
{
    char name[64];
    for (int i = 0; i < count; i++) {
        snprintf(name, sizeof(name), "seq_%s_%02d", prefix, i);
        snap(name);
        frames(interval);
    }
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
    frames(FPS + 12);
    snap_seq("arrive", 8, 4);
    frames(4);
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
    snap_seq("leave", 10, 4);
    snap("07_served");

    // On laisse les clients attendre : stress, puis départs fâchés
    frames(30 * FPS);
    snap("08_waiting");
    frames(25 * FPS);
    snap("09_stress");
    for (int i = 0; i < 200 * FPS && shop_session()->in_progress; i++)
        frames(1);
    frames(20);
    snap("10_day_end");

    // Jour suivant (ou réessai), pause, livre de recettes
    press(BTN_START);
    frames(FPS * 3);
    snap("11_day_next");
    press(BTN_START);
    frames(2);
    snap("12_pause");
    { Rect r = pause_button_rect(1); tap(r.x + r.w / 2, r.y + r.h / 2); }
    frames(20);
    snap("13_book");

    // Retour en boutique, quitter la journée, options
    press(BTN_B);
    frames(20);
    press(BTN_START);
    { Rect r = pause_button_rect(2); tap(r.x + r.w / 2, r.y + r.h / 2); }
    frames(20);
    snap("14_title_with_save");
    tap(128, 40 + 3 * 32 + 13);           // Options
    frames(20);
    tap(186, 30 + 2 * 30 + 12);           // Mode gaucher
    tap(234, 30 + 0 * 30 + 12);           // Musique +
    frames(2);
    snap("15_options");
    press(BTN_B);
    frames(20);

    // Mode Rush (débloqué après le jour 1)
    progress()->day = 2;
    progress_learn_recipe(1);
    progress_learn_recipe(2);
    tap(128, 40 + 2 * 32 + 13);
    frames(20 * FPS);
    snap("16_rush_lefty");
    for (int i = 0; i < 30 * 60 * FPS && shop_session()->in_progress; i++)
        frames(1);
    frames(20);
    snap("17_rush_end");

    // Deux gobelets (amélioration) + habitué du jour 3
    press(BTN_A);
    frames(20);
    progress()->upgrades |= 1u << UPG_SECOND_CUP;
    progress()->day = 3;
    press(BTN_START);                       // Continuer
    frames(16 * FPS + ARRIVE_FRAMES);
    center(tap_rect(0), &x, &y);
    hold(x, y, 60);
    center(bin_rect(0), &x, &y);
    drag(x, y, cup_cx_of(0, 2), 140);
    tap(cup_cx_of(1, 2), 150);              // sélectionne le 2e gobelet
    center(tap_rect(4), &x, &y);
    hold(x, y, 80);
    center(pump_rect(1), &x, &y);
    tap(x, y);
    frames(4);
    snap("18_two_cups_special");

    // Séquences d'animation sur le gobelet 1 : versement, garnitures, glaçons, secousse
    tap(cup_cx_of(0, 2), 150);
    center(action_rect(ACT_TRASH), &x, &y);
    tap(x, y);
    center(tap_rect(0), &x, &y);
    touch(x, y);
    frames(30);
    snap_seq("pour", 6, 3);
    frames(20);
    release();
    frames(1);
    center(bin_rect(3), &x, &y);            // glaçons
    tap(x, y);
    tap(x, y);
    center(bin_rect(0), &x, &y);            // perles
    tap(x, y);
    snap_seq("drop", 8, 3);
    frames(40);
    snap_seq("float", 6, 6);
    center(action_rect(ACT_SEAL), &x, &y);
    tap(x, y);
    snap_seq("seal", 6, 4);
    // secousse : va-et-vient sur le gobelet
    touch(cup_cx_of(0, 2), 130);
    frames(1);
    for (int k = 0; k < 8; k++) {
        touch(cup_cx_of(0, 2) + ((k & 1) ? -10 : 10), 130);
        frames(3);
    }
    release();
    frames(1);
    snap_seq("mix", 6, 5);
    printf("captures écrites dans %s\n", g_dir);
    return 0;
}
