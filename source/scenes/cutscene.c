// Cinématiques : ouverture du premier bar, arrivée dans chaque nouveau bar, fin.
//
// Une cinématique est une suite de plans. Chaque plan dessine l'écran du haut
// (décor + personnages animés) et affiche une réplique en bas, lettre par lettre.
// A / toucher : finir la phrase puis passer au plan suivant. START : passer tout.
#include <stdio.h>

#include "core/input.h"
#include "core/tween.h"
#include "game/customer.h"
#include "game/progress.h"
#include "game/specials.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/decor.h"
#include "scenes/fx.h"
#include "scenes/scenes.h"
#include "scenes/shop_draw.h"
#include "scenes/ui.h"

typedef enum {
    SPK_NONE, SPK_PERLE, SPK_MAMIE, SPK_CAPITAINE, SPK_CHEF, SPK_DJ, SPK_POETE, SPK_PLUME,
    SPK_REPORTER, SPK_PHOTO, SPK_FOULE,
} Speaker;

typedef void (*ShotDraw)(Surface *s, int t);

typedef struct {
    ShotDraw draw;
    Speaker speaker;
    const char *text;
} Shot;

#define GROUND_Y 184     // pieds des personnages sur les décors extérieurs

static const Shot *g_shots;
static int g_count, g_index;
static int g_t;             // frames depuis le début du plan
static int g_reveal;        // caractères affichés
static const Scene *g_next;
static CutsceneId g_id;
static uint32_t g_frame;

// ================================================================ Acteurs

static const Image *perle_image(const char *pose)
{
    if (pose[0] == 'h') return &IMG_heroine_perle_happy;
    if (pose[0] == 't') return &IMG_heroine_perle_talk;
    if (pose[0] == 's') return &IMG_heroine_perle_surprised;
    if (pose[0] == 'b') return &IMG_heroine_perle_blink;
    if (pose[0] == '1') return &IMG_heroine_perle_walk1;
    if (pose[0] == '2') return &IMG_heroine_perle_walk2;
    return &IMG_heroine_perle_normal;
}

// Perle à la position x (pieds au sol), qui parle si `talking`
static void perle(Surface *s, int x, bool talking, const char *pose)
{
    const char *p = pose;
    if (talking && ((g_frame / 6) & 1))
        p = "talk";
    else if (!talking && ((g_frame + 37) % 150) < 6 && pose[0] == 'n')
        p = "blink";
    draw_image(s, perle_image(p), x - 20, GROUND_Y - 48 + (((g_frame / 20) & 1) ? -1 : 0));
}

// Perle qui marche de x0 à x1 pendant `dur` frames (avec sa valise si `bag`)
static int perle_walk(Surface *s, int t, int x0, int x1, int dur, bool bag)
{
    int p = tw_progress(t, dur);
    int x = tw_lerp(x0, x1, ease_out_quad(p));
    bool moving = p < 256;
    const char *pose = moving ? (((g_frame / 8) & 1) ? "1" : "2") : "normal";
    int hop = moving && ((g_frame / 4) & 1) ? -1 : 0;
    if (bag) {   // valise rose à roulettes
        draw_rect(s, x + 14, GROUND_Y - 16 + hop, 12, 14, COL_PINK_DARK);
        draw_rect(s, x + 15, GROUND_Y - 15 + hop, 10, 4, COL_PINK);
        draw_rect(s, x + 18, GROUND_Y - 20 + hop, 4, 4, COL_INK_SOFT);
        draw_pixel(s, x + 16, GROUND_Y - 1, COL_INK);
        draw_pixel(s, x + 24, GROUND_Y - 1, COL_INK);
    }
    draw_image(s, perle_image(pose), x - 20, GROUND_Y - 48 + hop);
    return x;
}

static void actor(Surface *s, int type, int mood, int x, bool talking, bool flip)
{
    int pose = talking && ((g_frame / 6) & 1) ? POSE_TALK
             : ((g_frame + x * 7) % 140) < 6 ? POSE_BLINK : POSE_NORMAL;
    int bob = ((g_frame / 24 + x) & 1) ? -1 : 0;
    draw_image_flip(s, customer_image_ex(type, mood, pose), x - 20, GROUND_Y - 48 + bob, flip);
}

static void actor_walk(Surface *s, int type, int t, int x0, int x1, int dur, bool flip)
{
    int p = tw_progress(t, dur);
    int x = tw_lerp(x0, x1, ease_out_quad(p));
    int hop = p < 256 && ((g_frame / 4) & 1) ? -2 : 0;
    draw_image_flip(s, customer_image_ex(type, 0, POSE_NORMAL), x - 20, GROUND_Y - 48 + hop, flip);
}

// Habitué (avec son accessoire), immobile ou qui arrive en marchant
static void special_at(Surface *s, int sp, int x, bool talking, int hop)
{
    int pose = talking && ((g_frame / 6) & 1) ? POSE_TALK
             : ((g_frame + x * 7) % 140) < 6 ? POSE_BLINK : POSE_NORMAL;
    int y = GROUND_Y - 48 + hop + (((g_frame / 24 + x) & 1) ? -1 : 0);
    draw_image(s, customer_image_ex(SPECIALS[sp].type, 0, pose), x - 20, y);
    draw_accessory(s, SPECIALS[sp].accessory, x - 20, y);
}

static void special_walk(Surface *s, int sp, int t, int x0, int x1, int dur, bool talking)
{
    int p = tw_progress(t, dur);
    int x = tw_lerp(x0, x1, ease_out_quad(p));
    special_at(s, sp, x, talking && p >= 256, p < 256 && ((g_frame / 4) & 1) ? -2 : 0);
}

// Accessoires de journaliste
static void prop_notepad(Surface *s, int x, int y)
{
    draw_rect(s, x, y, 8, 10, COL_WHITE);
    draw_rect_outline(s, x, y, 8, 10, COL_GREY_DARK);
    for (int k = 0; k < 3; k++)
        draw_hline(s, x + 2, y + 3 + k * 2, 4, COL_GREY);
    draw_line(s, x + 9, y - 2, x + 5, y + 6, COL_YELLOW);
}

static void prop_camera(Surface *s, int x, int y, bool flash)
{
    draw_rect(s, x, y, 14, 9, COL_INK);
    draw_rect(s, x + 2, y - 2, 4, 2, COL_INK);
    draw_disc(s, x + 8, y + 4, 3, COL_GREY_DARK);
    draw_disc(s, x + 8, y + 4, 1, COL_LAVENDER);
    if (flash) {
        draw_disc(s, x + 3, y - 4, 5, COL_WHITE);
        for (int k = 0; k < 8; k++)
            draw_line(s, x + 3, y - 4, x + 3 + (k % 3 - 1) * 10, y - 4 + (k / 3 - 1) * 10, COL_YELLOW);
    }
}

static void prop_mic(Surface *s, int x, int y)
{
    draw_line(s, x, y, x + 8, y - 8, COL_GREY_DARK);
    draw_disc(s, x + 9, y - 9, 3, COL_INK);
    draw_pixel(s, x + 8, y - 10, COL_WHITE);
}

// Grand flash d'appareil photo sur tout un plan
static void flashes(Surface *s, int t, int every, int seed)
{
    int k = (t + seed) % every;
    if (k < 2) {
        int x = 30 + ((t / every) * 67 + seed * 31) % 200;
        int y = 100 + ((t / every) * 29) % 50;
        draw_disc(s, x, y, 6 - k * 2, COL_WHITE);
        draw_line(s, x - 12, y, x + 12, y, COL_YELLOW);
        draw_line(s, x, y - 12, x, y + 12, COL_YELLOW);
    }
}

// ================================================================ Décors

static void bg_petit_bar(Surface *s) { draw_image(s, &IMG_bg_title, 0, 0); }

// La boutique avant l'ouverture : pas d'auvent, panneau « À louer »
static void bg_a_louer(Surface *s, int awning_progress)
{
    bg_petit_bar(s);
    int cover = 236 - 236 * awning_progress / 256;   // l'auvent se déroule de gauche à droite
    if (cover > 0) {
        draw_rect(s, 246 - cover, 88, cover, 12, RGB888(250, 240, 250));
        draw_rect(s, 246 - cover, 100, cover, 12, RGB888(255, 240, 222));
        for (int y = 100; y < 112; y += 6)
            draw_hline(s, 246 - cover, y, cover, RGB888(240, 218, 196));
    }
    if (awning_progress == 0) {
        // Panneau sur la porte
        draw_rect(s, 112, 128, 32, 18, COL_WHITE);
        draw_rect_outline(s, 112, 128, 32, 18, COL_BAD);
        text_draw(s, 116, 132, "À LOUER", COL_BAD, 1);
        // Vitrines poussiéreuses
        for (int x = 30; x < 92; x += 7)
            draw_pixel(s, x, 130 + (x % 13), COL_GREY);
    }
}

static void sign_pearl_panic(Surface *s, int t)
{
    int drop = t < 20 ? (20 - t) * 3 : 0;
    int y = 70 - drop;
    draw_round_rect(s, 70, y, 116, 18, COL_PINK, COL_PINK_DARK);
    text_draw_shadow(s, 128 - text_width("Pearl Panic", 1) / 2, y + 4, "Pearl Panic", COL_WHITE, COL_PINK_DARK, 1);
    draw_vline(s, 80, y + 18, 88 - y - 18, COL_INK_SOFT);
    draw_vline(s, 176, y + 18, 88 - y - 18, COL_INK_SOFT);
}

static const Image *EXTERIORS[WORLD_COUNT] = {
    &IMG_bg_title, &IMG_bg_ext_plage, &IMG_bg_ext_montagne, &IMG_bg_ext_ville, &IMG_bg_ext_foret,
};

static void sign_world(Surface *s, int world, int y)
{
    char buf[40];
    snprintf(buf, sizeof(buf), "Pearl Panic - %s", WORLDS[world].name);
    int w = text_width(buf, 1) + 12;
    draw_round_rect(s, 128 - w / 2, y, w, 15, COL_PINK, COL_PINK_DARK);
    text_draw(s, 128 - w / 2 + 6, y + 2, buf, COL_WHITE, 1);
}

// ================================================================ Ouverture

static void intro_1(Surface *s, int t) { bg_a_louer(s, 0); perle_walk(s, t, -30, 96, 120, true); }
static void intro_2(Surface *s, int t)
{
    bg_a_louer(s, 0);
    perle_walk(s, 999, 96, 96, 1, true);
    if (t > 10 && t < 40 && (t % 8) == 0)
        fx_emit(SCREEN_TOP, FX_HEART, 96, 130, 1, 0);
}
static void intro_3(Surface *s, int t)
{
    int p = tw_progress(t - 20, 90);
    bg_a_louer(s, p);
    if ((t % 6) == 0 && p < 256)
        fx_emit(SCREEN_TOP, FX_SPARKLE, 246 - 236 * (256 - p) / 256, 96, 2, 0);
    perle(s, 96, false, "happy");
}
static void intro_4(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, t);
    if (t == 22)
        fx_emit(SCREEN_TOP, FX_STAR, 128, 80, 10, 0);
    perle(s, 96, g_reveal >= 0, "happy");
}
static void intro_5(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    perle(s, 90, false, "normal");
    special_walk(s, SP_MAMIE, t, 290, 170, 80, g_reveal >= 0);
}
static void intro_6(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    perle(s, 90, g_reveal >= 0, "surprised");
    special_at(s, SP_MAMIE, 170, false, 0);
}
static void intro_7(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    perle(s, 90, false, "happy");
    special_at(s, SP_MAMIE, 170, false, ((g_frame / 10) & 1) ? -2 : 0);
    if (t % 20 == 0)
        fx_emit(SCREEN_TOP, FX_HEART, 128, 120, 2, 0);
}

static const Shot INTRO[] = {
    { intro_1, SPK_NONE, "Quartier des Lanternes, un matin de printemps. Une petite loutre arrive avec une "
                         "grosse valise rose…" },
    { intro_2, SPK_PERLE, "C'est ici ! La boutique est à moi… Mon tout premier bar à bubble tea !" },
    { intro_3, SPK_NONE, "Un coup de chiffon, un auvent tout rose…" },
    { intro_4, SPK_PERLE, "… et l'enseigne ! Pearl Panic est ouvert !" },
    { intro_5, SPK_MAMIE, "Oh ! Un bar à bubble tea ? Je serai ta toute première cliente, ma petite !" },
    { intro_6, SPK_PERLE, "Bienvenue ! Alors, euh… comment on fait un bubble tea, déjà ?" },
    { intro_7, SPK_NONE, "Ainsi commence l'aventure de Pearl Panic !" },
};

// ================================================================ Arrivée dans un nouveau bar

static int g_world_cut;   // monde de la cinématique d'arrivée

static void world_bg(Surface *s) { draw_image(s, EXTERIORS[g_world_cut], 0, 0); }

static void world_1(Surface *s, int t)
{
    world_bg(s);
    sign_world(s, g_world_cut, 4);
    perle_walk(s, t, -30, 100, 120, true);
}
static void world_2(Surface *s, int t)
{
    world_bg(s);
    sign_world(s, g_world_cut, 4);
    perle(s, 100, g_reveal >= 0, "happy");
    if (t % 25 == 0)
        fx_emit(SCREEN_TOP, FX_SPARKLE, 128, 110, 4, 0);
}
static int world_local(void)
{
    static const int SP[WORLD_COUNT] = { SP_MAMIE, SP_CAPITAINE, SP_CHEF, SP_DJ, SP_POETE };
    return SP[g_world_cut];
}
static void world_3(Surface *s, int t)
{
    world_bg(s);
    sign_world(s, g_world_cut, 4);
    perle(s, 92, false, "normal");
    special_walk(s, world_local(), t, 290, 176, 70, g_reveal >= 0);
}
static void world_4(Surface *s, int t)
{
    world_bg(s);
    sign_world(s, g_world_cut, 4);
    perle(s, 92, g_reveal >= 0, "happy");
    special_at(s, world_local(), 176, false, 0);
    if (t % 18 == 0)
        fx_emit(SCREEN_TOP, FX_HEART, 132, 130, 1, 0);
}

static const char *const WORLD_NARRATION[WORLD_COUNT] = {
    "",
    "Grâce au succès du Petit Bar, Perle ouvre une paillote sur la Plage des Coquillages !",
    "Après la plage, cap sur la montagne : le Chalet Cocon ouvre au Col des Flocons.",
    "Perle descend en ville : un kiosque néon s'allume sur l'Avenue des Étoiles.",
    "Dernière étape : un bar dans un arbre géant, au cœur du Bois des Lucioles.",
};
static const char *const WORLD_PERLE[WORLD_COUNT] = {
    "",
    "Du sable, des vagues… et des clients qui ont très soif !",
    "Brrr ! Il va falloir des boissons bien chaudes ici.",
    "Waouh, tout brille ! Ici, on sert jusqu'au bout de la nuit !",
    "C'est magique… On dirait que les arbres chuchotent.",
};
static const char *const WORLD_LOCAL[WORLD_COUNT] = {
    "",
    "Ohé, moussaillonne ! Une paillote à bubble tea ? Je serai ton meilleur client !",
    "Hmm. Un bar à bubble tea à la montagne. Montre-moi ce que tu sais faire.",
    "Yo ! Enfin un bar ouvert après minuit ! Ma soirée est sauvée !",
    "Une loutre dans les bois… Voilà qui m'inspire un poème.",
};
static const Speaker WORLD_SPEAKER[WORLD_COUNT] = { SPK_MAMIE, SPK_CAPITAINE, SPK_CHEF, SPK_DJ, SPK_POETE };

static Shot g_world_shots[4];

// ================================================================ Fin

static void ending_1(Surface *s, int t)
{
    // La une du Grand Journal
    draw_fill(s, RGB888(246, 240, 226));
    draw_rect(s, 0, 0, SCREEN_W, 24, RGB888(40, 34, 44));
    text_draw_shadow(s, 128 - text_width("LE GRAND JOURNAL", 2) / 2, 4, "LE GRAND JOURNAL", COL_WHITE, COL_INK, 2);
    draw_hline(s, 0, 26, SCREEN_W, COL_INK);
    int p = ease_out_back(tw_progress(t, 30));
    int scale = p > 200 ? 2 : 1;
    text_draw_aligned(s, 0, 36, SCREEN_W, "PEARL PANIC :", COL_INK, ALIGN_CENTER);
    if (t > 10)
        text_draw_shadow(s, 128 - text_width("5 ÉTOILES !", scale) / 2, 50, "5 ÉTOILES !", COL_PINK_DARK, COL_INK, scale);
    draw_round_rect(s, 16, 80, 90, 96, COL_WHITE, COL_GREY);
    draw_image(s, &IMG_heroine_perle_happy, 41, 104);
    text_draw_wrapped(s, 114, 84, 132, "« Cinq bars, une seule passion. Le meilleur bubble tea du pays. » - Plume",
                      COL_INK_SOFT, ALIGN_LEFT);
    for (int k = 0; k < 5; k++)
        if (t > 20 + k * 6)
            draw_disc(s, 124 + k * 22, 166, 5, COL_YELLOW);
}

// Le Petit Bar, le lendemain matin : les journalistes arrivent de plus en plus nombreux
static void ending_2(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    perle(s, 70, false, "surprised");
    actor_walk(s, CT_SOURIS, t, 300, 170, 70, true);
    if (t > 70) {
        actor(s, CT_SOURIS, 0, 170, g_reveal >= 0, true);
        prop_notepad(s, 150, 150);
    }
}
static void ending_3(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    perle(s, 64, false, "surprised");
    actor(s, CT_SOURIS, 0, 150, false, true);
    prop_notepad(s, 130, 150);
    actor_walk(s, CT_PIGEON, t, 300, 196, 50, true);
    actor_walk(s, CT_RATON, t - 20, 320, 236, 60, true);
    if (t > 50)
        prop_camera(s, 176, 150, (t % 30) < 3);
    if (t > 80)
        prop_mic(s, 212, 156);
    flashes(s, t, 30, 0);
}
static void ending_4(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    // Une foule de journalistes : ça se bouscule, ça mitraille
    static const uint8_t CROWD[] = { CT_SOURIS, CT_PIGEON, CT_RATON, CT_CHAT_NOIR, CT_CARLIN, CT_HIBOU,
                                     CT_RENARD, CT_PINGOUIN, CT_DAUPHIN };
    for (int k = 0; k < 9; k++) {
        int x = 116 + (k % 5) * 30 + ((k / 5) ? 14 : 0);
        int arrive = k * 12;
        if (t < arrive)
            continue;
        int jitter = ((g_frame / 5 + k) & 1) ? -1 : 1;
        int y_off = (k / 5) ? -10 : 0;
        int dx = tw_lerp(160, 0, ease_out_quad(tw_progress(t - arrive, 30)));
        draw_image_flip(s, customer_image_ex(CROWD[k], 0, ((g_frame / 6 + k) & 1) ? POSE_TALK : POSE_NORMAL),
                        x - 20 + dx + jitter, GROUND_Y - 48 + y_off, true);
        if (k % 3 == 1)
            prop_camera(s, x - 22 + dx, GROUND_Y - 30 + y_off, ((t + k * 7) % 24) < 2);
        else if (k % 3 == 2)
            prop_mic(s, x - 24 + dx, GROUND_Y - 22 + y_off);
    }
    perle(s, 58, false, "surprised");
    flashes(s, t, 12, 3);
    flashes(s, t, 17, 9);
}
static void ending_5(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    for (int k = 0; k < 6; k++)
        draw_image_flip(s, customer_image_ex((CT_SOURIS + k) % CT_COUNT, 0, POSE_NORMAL),
                        130 + k * 20, GROUND_Y - 48, true);
    perle(s, 70, g_reveal >= 0, "happy");
    if (t % 10 == 0)
        fx_emit(SCREEN_TOP, FX_HEART, 70, 130, 1, 0);
    flashes(s, t, 25, 5);
}
static void ending_6(Surface *s, int t)
{
    bg_petit_bar(s);
    sign_pearl_panic(s, 99);
    // Tous les habitués sont venus féliciter Perle
    static const int CAST[] = { SP_MAMIE, SP_CAPITAINE, SP_CHEF, SP_DJ, SP_POETE, SP_PLUME };
    for (int k = 0; k < 6; k++) {
        int x = 22 + k * 30 + (k >= 3 ? 34 : 0);
        special_at(s, CAST[k], x, false, ((g_frame / 8 + k) & 3) == 0 ? -3 : 0);
    }
    perle(s, 128, false, "happy");
    if (t % 8 == 0)
        fx_emit(SCREEN_TOP, (t / 8) & 1 ? FX_STAR : FX_HEART, 20 + (t * 37) % 216, 100, 2, 0);
}
static void ending_7(Surface *s, int t)
{
    // Générique qui défile
    draw_fill(s, RGB888(40, 34, 60));
    static const char *const LINES[] = {
        "PEARL PANIC", "", "Une aventure de Perle la loutre", "",
        "Avec", "Mamie Lapine", "Capitaine Phoque", "Chef Panda", "DJ Raton", "Grenouille poète",
        "Mlle Plume", "", "et les 30 clients les plus", "impatients du pays !", "",
        "Merci d'avoir joué !", "", "Le jeu continue : améliore", "tes étoiles dans chaque bar !",
    };
    int n = sizeof(LINES) / sizeof(LINES[0]);
    int scroll = t / 2;
    for (int i = 0; i < n; i++) {
        int y = SCREEN_H - scroll + i * 14;
        if (y < -12 || y > SCREEN_H)
            continue;
        int scale = i == 0 ? 2 : 1;
        text_draw_shadow(s, 128 - text_width(LINES[i], scale) / 2, y, LINES[i],
                         i == 0 ? COL_PINK : COL_CREAM, COL_INK, scale);
    }
    if (t % 6 == 0)
        fx_emit(SCREEN_TOP, FX_SPARKLE, (t * 53) % 256, (t * 29) % 192, 1, 0);
}

static const Shot ENDING[] = {
    { ending_1, SPK_NONE, "Le lendemain matin, l'article de Mlle Plume fait la une du Grand Journal…" },
    { ending_2, SPK_REPORTER, "Bonjour ! Le Petit Écho ! Je peux vous poser une question ?" },
    { ending_3, SPK_PHOTO, "Une photo pour le Grand Journal ! Souriez, mademoiselle Perle !" },
    { ending_4, SPK_FOULE, "Une interview ! Votre secret ?! Par ici ! Une photo ! Encore une !" },
    { ending_5, SPK_PERLE, "Mon secret ? Des perles, beaucoup d'amour… et un tout petit peu de panique !" },
    { ending_6, SPK_NONE, "Pearl Panic est devenu le bar à bubble tea le plus célèbre du pays." },
    { ending_7, SPK_NONE, "Merci d'avoir joué !" },
};

// ================================================================ Boîte de dialogue (écran bas)

static const char *speaker_name(Speaker k)
{
    switch (k) {
    case SPK_PERLE:     return "Perle";
    case SPK_MAMIE:     return SPECIALS[SP_MAMIE].name;
    case SPK_CAPITAINE: return SPECIALS[SP_CAPITAINE].name;
    case SPK_CHEF:      return SPECIALS[SP_CHEF].name;
    case SPK_DJ:        return SPECIALS[SP_DJ].name;
    case SPK_POETE:     return SPECIALS[SP_POETE].name;
    case SPK_PLUME:     return SPECIALS[SP_PLUME].name;
    case SPK_REPORTER:  return "Reporter du Petit Écho";
    case SPK_PHOTO:     return "Photographe";
    case SPK_FOULE:     return "Les journalistes";
    default:            return NULL;
    }
}

static const Image *speaker_portrait(Speaker k, bool talk)
{
    int pose = talk ? POSE_TALK : POSE_NORMAL;
    switch (k) {
    case SPK_PERLE:     return talk ? &IMG_heroine_perle_talk : &IMG_heroine_perle_happy;
    case SPK_MAMIE:     return customer_image_ex(SPECIALS[SP_MAMIE].type, 0, pose);
    case SPK_CAPITAINE: return customer_image_ex(SPECIALS[SP_CAPITAINE].type, 0, pose);
    case SPK_CHEF:      return customer_image_ex(SPECIALS[SP_CHEF].type, 0, pose);
    case SPK_DJ:        return customer_image_ex(SPECIALS[SP_DJ].type, 0, pose);
    case SPK_POETE:     return customer_image_ex(SPECIALS[SP_POETE].type, 0, pose);
    case SPK_PLUME:     return customer_image_ex(SPECIALS[SP_PLUME].type, 0, pose);
    case SPK_REPORTER:  return customer_image_ex(CT_SOURIS, 0, pose);
    case SPK_PHOTO:     return customer_image_ex(CT_PIGEON, 0, pose);
    case SPK_FOULE:     return customer_image_ex(CT_RATON, 0, pose);
    default:            return NULL;
    }
}

static Rect skip_rect(void) { return rect_make(196, 4, 56, 20); }

static void draw_dialogue(Surface *s)
{
    decor_polka(s, COL_CREAM, COL_CREAM_DARK, g_frame);
    ui_button(s, skip_rect(), "Passer", BTN_STYLE_NORMAL);
    const Shot *sh = &g_shots[g_index];
    bool typing = g_reveal >= 0 && g_reveal < 400;
    const char *name = speaker_name(sh->speaker);
    int box_x = 8, box_w = 240;
    draw_round_rect(s, 8, 60, 240, 104, COL_WHITE, COL_LAV_DARK);
    if (name) {
        const Image *portrait = speaker_portrait(sh->speaker, typing && ((g_frame / 6) & 1));
        draw_round_rect(s, 14, 70, 46, 54, COL_PINK_PALE, COL_PINK_DARK);
        if (portrait)
            draw_image(s, portrait, 17, 73);
        draw_round_rect(s, 64, 50, text_width(name, 1) + 12, 16, COL_PINK, COL_PINK_DARK);
        text_draw(s, 70, 53, name, COL_WHITE, 1);
        box_x = 66;
        box_w = 176;
    }
    text_set_reveal(g_reveal);
    text_draw_wrapped(s, box_x, 72, box_w, sh->text, name ? COL_INK : COL_INK_SOFT, ALIGN_LEFT);
    text_set_reveal(-1);
    if (!typing && ((g_frame / 15) & 1))
        draw_image(s, &IMG_icons_heart, 228, 144);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d/%d", g_index + 1, g_count);
    text_draw(s, 12, 176, buf, COL_INK_SOFT, 1);
}

static void draw_top(Surface *s)
{
    g_shots[g_index].draw(s, g_t);
    fx_draw(SCREEN_TOP, s);
}

// ================================================================ Scène

void cutscene_play(CutsceneId id, const Scene *next)
{
    g_id = id;
    g_next = next;
}

static void cutscene_enter(void)
{
    if (g_id == CUT_INTRO) {
        g_shots = INTRO;
        g_count = sizeof(INTRO) / sizeof(INTRO[0]);
    } else if (g_id == CUT_ENDING) {
        g_shots = ENDING;
        g_count = sizeof(ENDING) / sizeof(ENDING[0]);
    } else {
        g_world_cut = 1 + (g_id - CUT_WORLD_2);
        g_world_shots[0] = (Shot){ world_1, SPK_NONE, WORLD_NARRATION[g_world_cut] };
        g_world_shots[1] = (Shot){ world_2, SPK_PERLE, WORLD_PERLE[g_world_cut] };
        g_world_shots[2] = (Shot){ world_3, WORLD_SPEAKER[g_world_cut], WORLD_LOCAL[g_world_cut] };
        g_world_shots[3] = (Shot){ world_4, SPK_PERLE, "Bienvenue chez Pearl Panic ! On ouvre !" };
        g_shots = g_world_shots;
        g_count = 4;
    }
    g_index = 0;
    g_t = 0;
    g_reveal = 0;
    g_frame = 0;
    fx_clear();
    plat_music(true);
    screen_set_dynamic(SCREEN_TOP, draw_top);
    screen_set_dynamic(SCREEN_BOTTOM, draw_dialogue);
}

static void finish(void)
{
    // Mémorise la cinématique vue
    if (g_id == CUT_INTRO)
        progress()->story |= STORY_INTRO;
    else if (g_id == CUT_ENDING)
        progress()->story |= STORY_ENDING;
    else
        progress()->story |= (uint8_t)(STORY_WORLD_FIRST << (g_id - CUT_WORLD_2 + 1));
    progress_save();
    scene_change(g_next ? g_next : &SCENE_MAP);
}

static void cutscene_update(void)
{
    const Input *in = input_get();
    g_frame++;
    g_t++;
    if (g_reveal >= 0) {
        g_reveal += 1 + (g_frame & 1);
        if (g_reveal > 400)
            g_reveal = -1;
        else if ((g_frame % 4) == 0)
            plat_voice(1100 + (int)(g_frame % 5) * 40);   // petit babillage pendant le texte
    }
    fx_update();
    screen_dirty_all(SCREEN_TOP);
    if ((g_frame & 1) == 0)
        screen_dirty_all(SCREEN_BOTTOM);
    if (scene_transitioning())
        return;

    Rect sk = skip_rect();
    if (input_tapped_in(sk.x, sk.y, sk.w, sk.h) || (in->down & BTN_START)) {
        finish();
        return;
    }
    if (in->tapped || (in->down & (BTN_A | BTN_B))) {
        if (g_reveal >= 0) {
            g_reveal = -1;            // affiche toute la phrase
        } else if (g_index + 1 < g_count) {
            g_index++;
            g_t = 0;
            g_reveal = 0;
            plat_sfx(SFX_CLICK);
        } else {
            finish();
        }
    }
}

const Scene SCENE_CUTSCENE = {
    .name = "cutscene",
    .enter = cutscene_enter,
    .update = cutscene_update,
    .exit = NULL,
};
