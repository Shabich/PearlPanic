// Joue un scénario scripté sur PC et écrit des captures des deux écrans.
//   make snap   ->  build/snap/*.png
#include <stdio.h>
#include <string.h>

#include "core/app.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/world.h"
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

// Séquence de captures (pour vérifier les animations) : seq_prefix_00, _01…
__attribute__((unused)) static void snap_seq(const char *prefix, int count, int interval)
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

// Parcourt une cinématique : une capture au milieu de chaque plan
static void play_cutscene(const char *prefix, int shots)
{
    char name[64];
    for (int i = 0; i < shots; i++) {
        frames(70);
        snprintf(name, sizeof(name), "%s_%d", prefix, i + 1);
        snap(name);
        press(BTN_A);          // termine la phrase
        frames(2);
        press(BTN_A);          // plan suivant
    }
    frames(30);
}

// Termine les niveaux jusqu'à `level` (exclu) avec 3 étoiles
static void unlock_until(int level)
{
    for (int l = 1; l < level; l++)
        progress_record_level(l, 3);
}

int main(int argc, char **argv)
{
    g_dir = argc > 1 ? argv[1] : ".";
    plat_init();
    app_init();
    int x, y;

    frames(30);
    snap("01_title");

    // Nouvelle partie -> cinématique d'ouverture -> carte
    press(BTN_START);
    frames(20);
    play_cutscene("02_intro", 7);
    frames(40);
    snap("03_map_w1");

    // Niveau 1-1
    press(BTN_A);
    frames(FPS + ARRIVE_FRAMES + 30);
    snap("04_level_1");
    center(tap_rect(0), &x, &y);
    hold(x, y, 70);
    center(tap_rect(4), &x, &y);
    hold(x, y, 30);
    center(bin_rect(0), &x, &y);
    drag(x, y, CUP_CX, 140);
    drag(x, y, CUP_CX, 140);
    center(action_rect(ACT_SEAL), &x, &y);
    tap(x, y);
    frames(30);
    snap("05_sealed");
    center(action_rect(ACT_SERVE), &x, &y);
    tap(x, y);
    frames(12);
    snap("06_served");
    for (int i = 0; i < 200 * FPS && shop_session()->in_progress; i++)
        frames(1);
    frames(100);
    snap("07_results");

    // Retour à la carte : Perle avance
    press(BTN_START);
    frames(30);
    snap("08_map_walk");
    frames(40);

    // Journal et livre
    press(BTN_Y);
    frames(20);
    snap("09_journal");
    press(BTN_B);
    frames(20);
    press(BTN_X);
    frames(30);
    snap("10_book");
    press(BTN_B);
    frames(20);

    // Chaque nouveau bar : cinématique d'arrivée, carte, poste de travail
    static const char *const NAMES[WORLD_COUNT] = { "quartier", "plage", "montagne", "ville", "foret" };
    for (int w = 1; w < WORLD_COUNT; w++) {
        unlock_until(level_id(w, 0));
        progress()->level = (uint8_t)level_id(w, 0);
        press(BTN_B);              // menu
        frames(20);
        press(BTN_START);          // continuer -> carte -> cinématique d'arrivée
        frames(30);
        char name[64];
        snprintf(name, sizeof(name), "20_world%d_intro", w + 1);
        play_cutscene(name, 4);
        frames(40);
        snprintf(name, sizeof(name), "21_map_%s", NAMES[w]);
        snap(name);
        press(BTN_A);              // jouer le premier niveau du bar
        frames(FPS * 18);
        center(tap_rect(0), &x, &y);
        hold(x, y, 40);
        center(bin_rect(1), &x, &y);
        tap(x, y);
        tap(x, y);
        frames(10);
        snprintf(name, sizeof(name), "22_shop_%s", NAMES[w]);
        snap(name);
        press(BTN_START);          // pause -> quitter vers la carte
        { Rect r = pause_button_rect(2); tap(r.x + r.w / 2, r.y + r.h / 2); }
        frames(30);
    }

    // Cinématique de fin
    unlock_until(LEVEL_COUNT + 1);
    cutscene_play(CUT_ENDING, &SCENE_MAP);
    scene_change(&SCENE_CUTSCENE);
    frames(20);
    play_cutscene("30_ending", 7);
    snap("31_after_ending");

    printf("captures écrites dans %s\n", g_dir);
    return 0;
}
