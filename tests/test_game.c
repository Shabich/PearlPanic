// Tests unitaires de la logique de jeu, compilés pour PC (tâche 9.1)
//   make test
#include <stdio.h>
#include <string.h>

#include "core/input.h"
#include "core/rng.h"
#include "core/tween.h"
#include "game/objectives.h"
#include "scenes/fx.h"
#include "game/day.h"
#include "game/drink.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/specials.h"
#include "game/world.h"
#include "gfx/canvas.h"
#include "gfx/font.h"

static int g_failed, g_checks;

#define CHECK(cond)                                                          \
    do {                                                                     \
        g_checks++;                                                          \
        if (!(cond)) {                                                       \
            g_failed++;                                                      \
            printf("  ÉCHEC %s:%d : %s\n", __FILE__, __LINE__, #cond);       \
        }                                                                    \
    } while (0)

// Prépare une boisson conforme à une recette
static void make_perfect(Drink *d, const Recipe *r)
{
    drink_reset(d);
    for (int i = 1; i < ING_COUNT; i++) {
        if (ing_info(i)->kind == KIND_LIQUID)
            drink_pour(d, i, r->qty[i]);
        else
            for (int k = 0; k < r->qty[i]; k++)
                drink_add(d, i);
    }
    drink_seal(d);
    if (r->shaken)
        drink_shake(d);
}

static void test_drink(void)
{
    Drink d;
    drink_reset(&d);
    CHECK(drink_is_empty(&d));
    CHECK(drink_pour(&d, ING_TEA_BLACK, 60) == 60);
    CHECK(drink_pour(&d, ING_TEA_BLACK, 10) == 10);
    CHECK(d.layer_count == 1 && d.layers[0].amount == 70);
    CHECK(drink_pour(&d, ING_MILK, 50) == 30);
    CHECK(d.overflowed);
    CHECK(d.liquid_total == CUP_CAPACITY);
    CHECK(drink_pour(&d, ING_PEARLS, 5) == 0);     // pas un liquide
    CHECK(drink_add(&d, ING_PEARLS));
    CHECK(!drink_add(&d, ING_MILK));               // pas une garniture
    CHECK(!drink_shake(&d));                       // pas encore scellé
    CHECK(drink_seal(&d));
    CHECK(!drink_add(&d, ING_PEARLS));             // scellé
    CHECK(drink_pour(&d, ING_MILK, 1) == 0);
    CHECK(drink_shake(&d));

    Drink e;
    drink_reset(&e);
    CHECK(!drink_seal(&e));                        // gobelet vide
}

static void test_recipes(void)
{
    Drink d;
    for (int i = 0; i < RECIPE_COUNT; i++) {
        const Recipe *r = &RECIPES[i];
        make_perfect(&d, r);
        int s = recipe_score(r, &d);
        if (s != 100)
            printf("  recette %d (%s) : score %d\n", i, r->name, s);
        CHECK(s == 100);
        // La recette doit être réalisable le jour où elle apparaît
        // La recette est faisable dans son bar, au niveau où elle apparaît
        int ml = recipe_min_level(r);
        if (ml < 0 || ml > r->level)
            printf("  recette %d (%s) : ingrédients pas au poste (niveau %d)\n", i, r->name, ml);
        CHECK(ml >= 0 && ml <= r->level);
        CHECK(r->world < WORLD_COUNT && r->level < LEVELS_PER_WORLD);
        // Le liquide doit remplir le gobelet
        int liquid = 0;
        for (int k = LIQUID_FIRST; k <= LIQUID_LAST; k++)
            liquid += r->qty[k];
        CHECK(liquid == CUP_CAPACITY);
        // La meilleure correspondance d'une boisson parfaite est elle-même
        int ms;
        CHECK(recipe_best_match(&d, r->world, &ms) == i && ms == 100);
        char buf[48];
        CHECK(recipe_hint(r, &d, buf, sizeof(buf)) == NULL);
    }
    CHECK(RECIPE_COUNT <= RECIPE_MAX);   // tient dans le masque de sauvegarde
    // Chaque bar a de quoi faire : au moins 8 recettes, dont une au niveau 0
    for (int w = 0; w < WORLD_COUNT; w++) {
        int n = 0, first = 0;
        for (int i = 0; i < RECIPE_COUNT; i++)
            if (RECIPES[i].world == w) {
                n++;
                first += RECIPES[i].level == 0 && !RECIPES[i].secret;
            }
        CHECK(n >= 8 && first >= 2);
    }

    // Recettes non ambiguës dans un même bar : aucune boisson parfaite n'en valide une autre à ≥ 85
    for (int i = 0; i < RECIPE_COUNT; i++) {
        make_perfect(&d, &RECIPES[i]);
        for (int j = 0; j < RECIPE_COUNT; j++) {
            if (i == j || RECIPES[i].world != RECIPES[j].world)
                continue;
            int s = recipe_score(&RECIPES[j], &d);
            if (s >= 85)
                printf("  ambigu : %s ~ %s (%d)\n", RECIPES[i].name, RECIPES[j].name, s);
            CHECK(s < 85);
        }
    }

    const Recipe *classic = &RECIPES[0];
    char buf[48];

    // Petite erreur de dosage tolérée
    drink_reset(&d);
    drink_pour(&d, ING_TEA_BLACK, 75);
    drink_pour(&d, ING_MILK, 25);
    drink_add(&d, ING_PEARLS);
    drink_add(&d, ING_PEARLS);
    drink_seal(&d);
    CHECK(recipe_score(classic, &d) == 100);

    // Oubli du couvercle
    d.sealed = false;
    CHECK(recipe_score(classic, &d) == 80);
    CHECK(strcmp(recipe_hint(classic, &d, buf, sizeof(buf)), "Le couvercle, svp !") == 0);

    // Ingrédient manquant
    drink_reset(&d);
    drink_pour(&d, ING_TEA_BLACK, 85);
    drink_seal(&d);
    int s = recipe_score(classic, &d);
    CHECK(s > 0 && s < 60);
    CHECK(strstr(recipe_hint(classic, &d, buf, sizeof(buf)), "lait") != NULL);

    // Ingrédient non demandé
    make_perfect(&d, classic);
    d.sealed = false;
    drink_add(&d, ING_JELLY);
    d.sealed = true;
    CHECK(recipe_score(classic, &d) < 100);
    CHECK(strstr(recipe_hint(classic, &d, buf, sizeof(buf)), "Pas de gelée") != NULL);

    // Matcha non secoué
    const Recipe *matcha = NULL;
    for (int i = 0; i < RECIPE_COUNT; i++)
        if (RECIPES[i].shaken) { matcha = &RECIPES[i]; break; }
    CHECK(matcha != NULL);
    make_perfect(&d, matcha);
    d.shaken = false;
    CHECK(recipe_score(matcha, &d) == 75);

    // Gobelet vide = 0
    drink_reset(&d);
    CHECK(recipe_score(classic, &d) == 0);
}

static void test_progress(void)
{
    progress_new();
    CHECK(progress()->level == 1);
    CHECK(progress_level_unlocked(1) && !progress_level_unlocked(2));
    CHECK(progress_record_level(1, 2));          // 1re réussite
    CHECK(!progress_record_level(1, 1));         // on garde le meilleur
    CHECK(progress()->stars[0] == 2 && progress_level_unlocked(2) && !progress_level_unlocked(3));
    CHECK(progress_furthest_level() == 2 && progress_total_stars() == 2);
    CHECK(progress_world_unlocked(0) && !progress_world_unlocked(1));
    for (int l = 1; l <= 4; l++)
        progress_record_level(l, 3);
    CHECK(progress_world_unlocked(1) && progress_world_stars(0) == 12);
    CHECK(!progress_game_finished());
    progress_learn_recipe(40);
    CHECK(progress_recipe_known(40));             // au-delà de 32 recettes
    progress()->story = STORY_INTRO | STORY_ENDING;
    CHECK(progress_recipe_known(0));
    CHECK(!progress_recipe_known(1));
    progress_learn_recipe(5);
    progress()->coins = 100;
    CHECK(progress_buy_upgrade(UPG_TIP_JAR));
    CHECK(!progress_buy_upgrade(UPG_TIP_JAR));
    CHECK(progress()->coins == 100 - upgrade_info(UPG_TIP_JAR)->cost);

    uint8_t buf[SAVE_SIZE];
    Progress copy;
    progress_serialize(progress(), buf);
    CHECK(progress_deserialize(&copy, buf));
    CHECK(copy.coins == progress()->coins && copy.known_recipes == progress()->known_recipes);
    CHECK(copy.upgrades == progress()->upgrades && copy.level == 1);
    CHECK(copy.stars[3] == 3 && copy.story == progress()->story && progress_recipe_known(40));
    CHECK((copy.known_recipes >> 40) & 1);
    buf[10] ^= 0xFF;   // corruption
    CHECK(!progress_deserialize(&copy, buf));

    // Réglages : sauvegardés, et conservés par une nouvelle partie
    progress()->music_volume = 1;
    progress()->sfx_volume = 2;
    progress()->lefty = true;
    progress()->best_rush = 77;
    progress_serialize(progress(), buf);
    CHECK(progress_deserialize(&copy, buf));
    CHECK(copy.music_volume == 1 && copy.sfx_volume == 2 && copy.lefty && copy.best_rush == 77);
    progress_new_game();
    CHECK(progress()->music_volume == 1 && progress()->lefty && progress()->best_rush == 77);
    CHECK(progress()->coins == 0 && progress()->level == 1 && progress()->upgrades == 0);
    CHECK(progress()->stars[0] == 0 && progress()->story == 0);
    progress_erase();
    CHECK(!progress_has_save() && progress()->best_rush == 0 && !progress()->lefty);
}

static void test_objectives(void)
{
    progress_new();
    for (int day = 1; day <= 12; day++) {
        Objective o = objective_for_day(day, true);
        CHECK(o.type > OBJ_NONE && o.type < OBJ_COUNT && o.reward > 0);
        char buf[48];
        objective_describe(&o, buf, sizeof(buf));
        CHECK(buf[0] && buf[0] != '-');
    }
    // Sans recette à découvrir, pas d'objectif de découverte
    for (int day = 1; day <= 12; day++)
        CHECK(objective_for_day(day, false).type != OBJ_DISCOVER);

    Day d;
    day_start(&d, 1, 5);
    CHECK(d.objective.type == OBJ_SERVE);
    d.served = d.objective.target - 1;
    CHECK(!objective_done(&d.objective, &d));
    d.served++;
    CHECK(objective_done(&d.objective, &d));

    Objective nl = { OBJ_NO_LOSS, 0, 5 };
    d.lost = 0;
    CHECK(objective_done(&nl, &d));
    d.lost = 1;
    CHECK(!objective_done(&nl, &d));

    // L'objectif est évalué à la fin de la journée
    progress_new();
    day_start(&d, 1, 5);
    while (!d.ended)
        day_update(&d);
    CHECK(d.objective_met == objective_done(&d.objective, &d));
}

static void test_rush(void)
{
    progress_new();
    progress_learn_recipe(1);
    progress_learn_recipe(2);
    Day d;
    day_start_rush(&d, 3, 11);
    CHECK(d.endless && d.objective.type == OBJ_NONE);
    int arrivals = 0, riddles = 0;
    uint32_t limit = 30 * 60 * FPS;
    while (!d.ended && d.frame < limit) {
        day_update(&d);
        for (int e = 0; e < d.event_count; e++) {
            if (d.events[e].type == EV_ARRIVE) {
                arrivals++;
                const Customer *c = &d.slots[d.events[e].slot];
                riddles += c->riddle;
                CHECK(progress_recipe_known(c->recipe));
            }
        }
    }
    // Sans personne pour servir, le Rush finit par l'effondrement de l'ambiance
    CHECK(d.ended && d.collapsed);
    CHECK(d.frame > DAY_DURATION / 4);
    CHECK(arrivals > 3 && riddles == 0);
    // Ça accélère avec le temps
    CHECK(rush_spawn_interval(0) > rush_spawn_interval(5 * 60 * FPS));
    CHECK(rush_patience(1, 0) > rush_patience(1, 5 * 60 * FPS));
    CHECK(rush_spawn_interval(60 * 60 * FPS) >= 4 * FPS);
}

static void test_specials(void)
{
    // Chaque visite : recette existante, du bon bar, faisable au niveau de la visite
    for (int sp = 0; sp < SPECIAL_COUNT; sp++) {
        CHECK(SPECIALS[sp].visit_count >= 1 && SPECIALS[sp].visit_count <= SPECIAL_VISITS);
        for (int v = 0; v < SPECIALS[sp].visit_count; v++) {
            const SpecialVisit *vis = &SPECIALS[sp].visits[v];
            int r = special_recipe(sp, v);
            if (r < 0)
                printf("  recette introuvable : %s\n", vis->recipe);
            CHECK(r >= 0);
            CHECK(vis->level >= 1 && vis->level <= LEVEL_COUNT);
            CHECK(level_world(vis->level) == special_world(sp));
            if (v > 0)
                CHECK(vis->level >= SPECIALS[sp].visits[v - 1].level);
            if (r >= 0) {
                if (!recipe_available(r, vis->level))
                    printf("  %s : « %s » pas dispo au niveau %d\n", SPECIALS[sp].name, vis->recipe, vis->level);
                CHECK(recipe_available(r, vis->level));
            }
            CHECK(text_width(vis->thanks, 1) > 0);
        }
    }
    CHECK(SPECIAL_COUNT <= SPECIAL_MAX);

    // Au niveau 2, Mamie Lapine passe et commande le classique
    progress_new();
    Day day;
    day_start(&day, 2, 3);
    CHECK(day.specials[0] == SP_MAMIE && day.specials[1] == -1);
    int slot = -1;
    for (int f = 0; f < DAY_DURATION && slot < 0; f++) {
        day_update(&day);
        for (int e = 0; e < day.event_count; e++)
            if (day.events[e].type == EV_ARRIVE && day.slots[day.events[e].slot].special >= 0)
                slot = day.events[e].slot;
        // On sert parfaitement tout le monde pour libérer les places
        if (day.active >= 0 && day.slots[day.active].state == CS_WAITING &&
            day.slots[day.active].special < 0) {
            Drink d;
            make_perfect(&d, &RECIPES[day.slots[day.active].recipe]);
            ServeResult r;
            day_serve(&day, &d, &r);
        }
    }
    CHECK(slot >= 0);
    if (slot >= 0) {
        Customer *c = &day.slots[slot];
        CHECK(c->type == SPECIALS[SP_MAMIE].type && c->recipe == special_recipe(SP_MAMIE, 0) && !c->riddle);
        while (c->state != CS_WAITING)
            day_update(&day);
        day_select_slot(&day, slot);
        Drink d;
        make_perfect(&d, &RECIPES[c->recipe]);
        ServeResult r;
        int tips_before = day.tips;
        CHECK(day_serve(&day, &d, &r));
        CHECK(progress()->special_steps[SP_MAMIE] == 1);
        CHECK(day.special_result[0] == 1);
        CHECK(day.tips - tips_before == r.tip && r.tip >= SPECIALS[SP_MAMIE].tip_bonus);
        CHECK(c->says == SPECIALS[SP_MAMIE].visits[0].thanks);
    }

    int due[SPECIALS_PER_LEVEL];
    uint8_t steps[SPECIAL_MAX] = { 0 };
    CHECK(specials_due(1, steps, due) == 0);
    CHECK(specials_due(2, steps, due) == 1 && due[0] == SP_MAMIE);
    CHECK(specials_due(4, steps, due) == 1);      // rattrapage dans le même bar
    CHECK(specials_due(5, steps, due) == 0);      // pas dans un autre bar
    steps[SP_MAMIE] = 1;
    CHECK(specials_due(2, steps, due) == 0);      // prochaine visite au niveau 3
    // Niveau 20 : la grenouille poète et la critique, deux habitués le même jour
    uint8_t late[SPECIAL_MAX] = { 3, 3, 3, 3, 2, 1, 0, 0 };
    CHECK(specials_due(20, late, due) == 2);
    // Pas d'habitué en mode Rush
    day_start_rush(&day, 6, 1);
    CHECK(day.specials[0] == -1 && day.specials[1] == -1);
}

static void test_worlds(void)
{
    for (int w = 0; w < WORLD_COUNT; w++) {
        const World *wd = &WORLDS[w];
        int at0 = 0;
        // Pas de doublon au poste ; types d'ingrédients cohérents avec les emplacements
        for (int i = 0; i < WORLD_TAPS; i++) {
            CHECK(ing_info(wd->taps[i].ing)->kind == KIND_LIQUID);
            at0 += wd->taps[i].level == 0;
            for (int j = 0; j < i; j++)
                CHECK(wd->taps[i].ing != wd->taps[j].ing);
        }
        for (int i = 0; i < WORLD_PUMPS; i++)
            CHECK(ing_info(wd->pumps[i].ing)->kind == KIND_SYRUP);
        for (int i = 0; i < WORLD_BINS; i++) {
            CHECK(ing_info(wd->bins[i].ing)->kind == KIND_TOPPING);
            CHECK(ing_info(wd->bins[i].ing)->style != TOP_NONE && ing_info(wd->bins[i].ing)->per_scoop > 0);
        }
        CHECK(at0 >= 2);
        for (int i = 0; i < WORLD_CUSTOMERS; i++)
            CHECK(wd->customers[i] < CT_COUNT);
    }
    CHECK(level_world(1) == 0 && level_index(1) == 0);
    CHECK(level_world(20) == 4 && level_index(20) == 3);
    CHECK(level_id(2, 1) == 10);

    // Un niveau d'un nouveau bar : clients du bar, 1re recette connue, commande en clair
    progress_new();
    Day d;
    day_start(&d, level_id(WORLD_MONTAGNE, 0), 77);
    CHECK(d.world == WORLD_MONTAGNE && d.level == 0);
    for (int f = 0; f < 3 * FPS; f++)
        day_update(&d);
    CHECK(d.active >= 0);
    if (d.active >= 0) {
        const Customer *c = &d.slots[d.active];
        bool local = false;
        for (int i = 0; i < WORLD_CUSTOMERS; i++)
            local |= WORLDS[WORLD_MONTAGNE].customers[i] == c->type;
        CHECK(local && !c->riddle && RECIPES[c->recipe].world == WORLD_MONTAGNE);
    }
    // Étoiles
    Day e;
    memset(&e, 0, sizeof(e));
    e.ambiance = AMBIANCE_MAX;
    CHECK(day_compute_stars(&e) == 2);            // pas d'objectif, aucun client perdu
    e.objective_met = true;
    CHECK(day_compute_stars(&e) == 3);
    e.lost = 1;
    CHECK(day_compute_stars(&e) == 2);
    e.collapsed = true;
    CHECK(day_compute_stars(&e) == 0);
}

static void test_tween_fx(void)
{
    CHECK(ease_in_quad(0) == 0 && ease_in_quad(256) == 256);
    CHECK(ease_out_quad(0) == 0 && ease_out_quad(256) == 256);
    CHECK(ease_out_back(0) == 0 && ease_out_back(256) == 256);
    CHECK(ease_out_bounce(0) == 0 && ease_out_bounce(256) == 256);
    int peak = 0;
    for (int t = 0; t <= 256; t++) {
        int v = ease_out_back(t);
        if (v > peak)
            peak = v;
        CHECK(ease_out_bounce(t) >= 0 && ease_out_bounce(t) <= 256);
    }
    CHECK(peak > 256 && peak < 300);   // léger dépassement
    CHECK(tw_progress(5, 10) == 128 && tw_progress(20, 10) == 256);

    fx_clear();
    fx_emit(SCREEN_TOP, FX_HEART, 100, 100, 5, 0);
    CHECK(fx_count() == 5);
    for (int i = 0; i < 100; i++)
        fx_update();
    CHECK(fx_count() == 0);
    for (int i = 0; i < 200; i++)
        fx_emit(SCREEN_BOTTOM, FX_SPLASH, 10, 10, 1, 0);
    CHECK(fx_count() == 64);   // plafonné
    fx_clear();
}

static void test_day(void)
{
    progress_new();
    Day day;
    day_start(&day, 1, 1234);

    // Le premier client arrive après ~1 s et commande le classique en clair
    for (int i = 0; i < FPS + ARRIVE_FRAMES + 2; i++)
        day_update(&day);
    CHECK(day.active >= 0);
    Customer *c = &day.slots[day.active];
    CHECK(c->state == CS_WAITING);
    CHECK(c->recipe == 0 && !c->riddle);

    // Service parfait
    Drink d;
    make_perfect(&d, &RECIPES[0]);
    ServeResult res;
    CHECK(day_serve(&day, &d, &res));
    CHECK(res.score == 100 && res.happy && res.coins == RECIPES[0].price && res.tip > 0);
    CHECK(day.served == 1 && day.perfect == 1);
    CHECK(c->state == CS_LEAVING);

    // Un client ignoré finit par partir furieux et fait baisser l'ambiance
    day_start(&day, 3, 99);
    int lost_seen = 0;
    for (int f = 0; f < DAY_DURATION + 120 * FPS && !day.ended; f++) {
        day_update(&day);
        for (int e = 0; e < day.event_count; e++)
            lost_seen += day.events[e].type == EV_LEAVE_ANGRY;
    }
    CHECK(lost_seen > 0);
    CHECK(day.lost == lost_seen);
    CHECK(day.ended);
    CHECK(day.ambiance < AMBIANCE_MAX);

    // Découverte : servir une recette inconnue la débloque
    progress_new();
    day_start(&day, 2, 7);
    int found = -1;
    for (int f = 0; f < DAY_DURATION && found < 0; f++) {
        day_update(&day);
        if (day.active >= 0 && day.slots[day.active].riddle)
            found = day.slots[day.active].recipe;
    }
    CHECK(found > 0);
    if (found > 0) {
        CHECK(!progress_recipe_known(found));
        make_perfect(&d, &RECIPES[found]);
        CHECK(day_serve(&day, &d, &res));
        CHECK(res.unlocked == found);
        CHECK(progress_recipe_known(found));
    }

    // Difficulté croissante
    CHECK(day_base_patience(1) > day_base_patience(5));
    CHECK(day_spawn_interval(1) > day_spawn_interval(5));
    CHECK(day_base_patience(50) >= 20 * FPS);
}

static void test_input(void)
{
    RawInput raw = { 0 };
    input_feed(&raw);
    raw.touching = true; raw.tx = 100; raw.ty = 50;
    input_feed(&raw);
    CHECK(input_get()->touch_down && input_pressed_in(90, 40, 20, 20));
    input_feed(&raw);
    CHECK(!input_get()->touch_down && input_get()->touching);
    raw.touching = false;
    input_feed(&raw);
    CHECK(input_get()->tapped && input_tapped_in(90, 40, 20, 20));

    // Glisser : pas un tap
    raw.touching = true; raw.tx = 10; raw.ty = 10;
    input_feed(&raw);
    raw.tx = 40;
    input_feed(&raw);
    CHECK(input_get()->dragging && input_get()->dx == 30);
    raw.touching = false;
    input_feed(&raw);
    CHECK(!input_get()->tapped && input_get()->touch_up);

    raw.keys = BTN_A;
    input_feed(&raw);
    CHECK(input_get()->down & BTN_A);
    input_feed(&raw);
    CHECK(!(input_get()->down & BTN_A) && (input_get()->held & BTN_A));
}

static void test_canvas(void)
{
    static uint16_t px[32 * 16];
    Surface s;
    surface_init(&s, px, 32, 16, 32);
    draw_fill(&s, RGB(1, 2, 3));
    CHECK(px[0] == RGB(1, 2, 3) && px[32 * 16 - 1] == RGB(1, 2, 3));
    s.clip = rect_make(4, 4, 4, 4);
    draw_fill(&s, RGB(31, 0, 0));
    CHECK(px[4 * 32 + 4] == RGB(31, 0, 0));
    CHECK(px[3 * 32 + 4] == RGB(1, 2, 3));
    CHECK(px[4 * 32 + 8] == RGB(1, 2, 3));
    surface_reset_clip(&s);
    CHECK(text_width("Thé", 1) == 3 * FONT_W);
    CHECK(text_count_lines(6 * 10, "Bonjour tout le monde") == 3);
    CHECK(color_mix(RGB(0, 0, 0), RGB(31, 31, 31), 256) == RGB(31, 31, 31));
}

int main(void)
{
    test_drink();
    test_recipes();
    test_progress();
    test_day();
    test_input();
    test_canvas();
    test_objectives();
    test_rush();
    test_tween_fx();
    test_specials();
    test_worlds();
    printf("%d vérifications, %d échec(s)\n", g_checks, g_failed);
    return g_failed ? 1 : 0;
}
