// La boutique : poste d'assemblage (écran bas) et comptoir (écran haut)
// Tâches 3.x (assemblage), 4.x (clients), 5.3–5.5, 8.2 (pause), 8.3 (HUD), 8.4 (tutoriel)
#include <stdio.h>
#include <string.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
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
#include "scenes/shop_layout.h"
#include "scenes/ui.h"

#define SHAKE_REVERSALS   6
#define TOAST_FRAMES      150
#define END_DELAY_FRAMES  90

static ShopSession g_session;
static bool g_begin_requested = true;
static bool g_rush_requested;
static ShopView g_view;

// Watches de l'écran haut
static Watch w_hud, w_panel, w_slots[DAY_SLOTS];
// Watches de l'écran bas
static Watch w_taps, w_pumps, w_sel, w_bins, w_actions, w_cup, w_pause;
static Tracked t_held;

ShopSession *shop_session(void) { return &g_session; }

void shop_begin_day(void)
{
    g_begin_requested = true;
    g_rush_requested = false;
}

void shop_begin_rush(void)
{
    g_begin_requested = true;
    g_rush_requested = true;
}

// Hauteur de la surface du liquide du gobelet actif (pour les éclaboussures)
static int active_surface_y(void) { return cup_surface_y(session_drink(&g_session), 0); }

static int customer_head_x(int slot) { return slot_cx(slot); }

// Centre du gobelet actif
static int active_cx(void) { return cup_cx_of(g_session.active_cup, g_session.cup_count); }

// Gobelet sous le stylet (-1 si aucun)
static int cup_at(int x, int y)
{
    for (int i = 0; i < g_session.cup_count; i++)
        if (rect_contains(cup_hit_rect(i, g_session.cup_count), x, y))
            return i;
    return -1;
}
#define CUSTOMER_HEAD_Y (CUSTOMER_Y + 14)

static bool ing_unlocked(Ingredient ing)
{
    return ing_info(ing)->unlock_day <= g_session.day.number;
}

static void toast(const char *line1, const char *line2, uint16_t color)
{
    snprintf(g_view.toast1, sizeof(g_view.toast1), "%s", line1 ? line1 : "");
    snprintf(g_view.toast2, sizeof(g_view.toast2), "%s", line2 ? line2 : "");
    g_view.toast_color = color;
    g_view.toast_timer = TOAST_FRAMES;
    g_view.toast_id++;
}

static void reset_watches(void)
{
    watch_reset(&w_hud);
    watch_reset(&w_panel);
    for (int i = 0; i < DAY_SLOTS; i++)
        watch_reset(&w_slots[i]);
    watch_reset(&w_taps);
    watch_reset(&w_pumps);
    watch_reset(&w_sel);
    watch_reset(&w_bins);
    watch_reset(&w_actions);
    watch_reset(&w_cup);
    watch_reset(&w_pause);
    t_held.shown = false;
}

static void shop_enter(void)
{
    if (g_begin_requested || !g_session.in_progress) {
        if (g_rush_requested)
            day_start_rush(&g_session.day, progress()->day, plat_entropy());
        else
            day_start(&g_session.day, progress()->day, plat_entropy());
        drink_reset(&g_session.cups[0]);
        drink_reset(&g_session.cups[1]);
        g_session.active_cup = 0;
        g_session.in_progress = true;
        g_begin_requested = false;
        memset(&g_view, 0, sizeof(g_view));
        g_view.pouring = -1;
        g_view.held = ING_NONE;
        fx_clear();
        cup_anim_reset(&g_view.cup_anim[0]);
        cup_anim_reset(&g_view.cup_anim[1]);
        char l1[48], l2[96];
        if (g_session.day.endless) {
            toast("Mode Rush !", "Tiens le plus longtemps possible : ça va accélérer…", COL_BAD);
        } else {
            objective_describe(&g_session.day.objective, l1, sizeof(l1));
            snprintf(l2, sizeof(l2), "Objectif du jour : %s (+%d)", l1, g_session.day.objective.reward);
            if (g_session.day.number == 1)
                snprintf(l1, sizeof(l1), "Jour 1 : bienvenue !");
            else
                snprintf(l1, sizeof(l1), "Jour %d : c'est parti !", g_session.day.number);
            toast(l1, l2, COL_PINK_DARK);
        }
    }
    layout_set_lefty(progress()->lefty);
    g_session.cup_count = progress_has_upgrade(UPG_SECOND_CUP) ? 2 : 1;
    if (g_session.active_cup >= g_session.cup_count)
        g_session.active_cup = 0;
    plat_music(true);
    g_view.paused = false;
    g_view.pouring = -1;
    g_view.held = ING_NONE;
    g_view.session = &g_session;

    shop_draw_backgrounds(&g_view);
    screen_set_dynamic(SCREEN_TOP, shop_draw_top);
    screen_set_dynamic(SCREEN_BOTTOM, shop_draw_bottom);
    reset_watches();
}

static void shop_exit(void)
{
    plat_pour_loop(false);
}

// ----------------------------------------------------------------- Service

static void handle_events(int first);

static void serve(void)
{
    Day *d = &g_session.day;
    Drink *drink = session_drink(&g_session);
    if (drink_is_empty(drink))
        return;
    int slot = d->active;
    int first_event = d->event_count;
    ServeResult r;
    if (!day_serve(d, drink, &r))
        return;

    plat_sfx(SFX_CASH);
    plat_sfx(r.happy ? SFX_HAPPY : SFX_ANGRY);
    const Customer *c = &d->slots[slot];
    plat_voice(customer_type(c->type)->voice * (r.happy ? 11 : 8) / 10);
    int hx = customer_head_x(slot);
    if (r.happy)
        fx_emit(SCREEN_TOP, FX_HEART, hx, CUSTOMER_HEAD_Y, r.score >= 90 ? 6 : 3, 0);
    else
        fx_emit(SCREEN_TOP, FX_SMOKE, hx, CUSTOMER_HEAD_Y - 6, 3, 0);
    if (r.score >= 90)
        fx_emit(SCREEN_TOP, FX_STAR, hx, CUSTOMER_HEAD_Y, 5, 0);
    if (r.coins + r.tip > 0)
        fx_emit(SCREEN_TOP, FX_COIN, hx, CUSTOMER_HEAD_Y, r.coins + r.tip > 6 ? 4 : 2, 0);
    if (r.unlocked >= 0)
        fx_emit(SCREEN_TOP, FX_SPARKLE, SCREEN_W / 2, PANEL_Y + PANEL_H / 2, 14, 0);
    g_view.coin_popup = r.coins + r.tip;
    g_view.coin_popup_timer = 60;
    g_view.coin_popup_slot = slot;

    handle_events(first_event);   // annonces de l'habitué, déblocages
    char l1[48], l2[64];
    if (d->slots[slot].special >= 0 && d->special_result == 1) {
        // le toast de l'habitué reste affiché
    } else if (r.unlocked >= 0) {
        snprintf(l1, sizeof(l1), "Nouvelle recette !");
        snprintf(l2, sizeof(l2), "« %s » ajoutée au livre.", RECIPES[r.unlocked].name);
        toast(l1, l2, COL_LAV_DARK);
    } else {
        snprintf(l1, sizeof(l1), "%d %% · +%d pièce%s%s", r.score, r.coins + r.tip,
                 r.coins + r.tip > 1 ? "s" : "", r.tip ? " (pourboire !)" : "");
        toast(l1, r.hint, r.happy ? COL_MINT_DARK : COL_BAD);
    }
    drink_reset(drink);
}

// ---------------------------------------------------------------- Entrées

static void update_shake(const Input *in)
{
    Drink *drink = session_drink(&g_session);
    if (!drink->sealed || drink->shaken || g_view.held != ING_NONE) {
        g_view.shaking = false;
        g_view.wobble = 0;
        return;
    }
    if (in->touch_down && rect_contains(cup_hit_rect(g_session.active_cup, g_session.cup_count), in->x, in->y)) {
        g_view.shaking = true;
        g_view.shake_dir = 0;
        g_view.shake_reversals = 0;
    }
    if (!in->touching) {
        g_view.shaking = false;
        g_view.wobble = 0;
        return;
    }
    if (!g_view.shaking)
        return;
    int w = in->x - in->start_x;
    g_view.wobble = w < -8 ? -8 : w > 8 ? 8 : w;
    int dir = in->dx >= 2 ? 1 : in->dx <= -2 ? -1 : 0;
    if (dir && dir != g_view.shake_dir) {
        if (g_view.shake_dir)
            g_view.shake_reversals++;
        g_view.shake_dir = dir;
        plat_sfx(SFX_SHAKE);
    }
    if (g_view.shake_reversals >= SHAKE_REVERSALS) {
        drink_shake(drink);
        fx_emit(SCREEN_BOTTOM, FX_STAR, active_cx(), CUP_TOP_Y, 6, 0);
        g_view.shaking = false;
        g_view.wobble = 0;
        toast("Bien secoué !", NULL, COL_MINT_DARK);
    }
}

static void update_station(const Input *in)
{
    Day *d = &g_session.day;
    Drink *drink = session_drink(&g_session);

    // Choix du client
    if (in->down & (BTN_L | BTN_LEFT))
        day_select(d, -1);
    if (in->down & (BTN_R | BTN_RIGHT))
        day_select(d, +1);
    Rect rp = sel_prev_rect(), rn = sel_next_rect();
    if (input_pressed_in(rp.x, rp.y, rp.w, rp.h)) {
        day_select(d, -1);
        plat_sfx(SFX_CLICK);
    }
    if (input_pressed_in(rn.x, rn.y, rn.w, rn.h)) {
        day_select(d, +1);
        plat_sfx(SFX_CLICK);
    }

    // Deux gobelets : toucher un gobelet le rend actif (tâche 3.12)
    if (g_session.cup_count == 2 && in->touch_down && g_view.held == ING_NONE) {
        int c = cup_at(in->x, in->y);
        if (c >= 0 && c != g_session.active_cup) {
            g_session.active_cup = c;
            drink = session_drink(&g_session);
            plat_sfx(SFX_CLICK);
        }
    }

    // Tireuses : maintenir pour verser
    int pouring = -1;
    if (in->touching && g_view.held == ING_NONE && !g_view.shaking) {
        for (int i = 0; i < TAP_COUNT; i++) {
            Rect r = tap_rect(i);
            if (rect_contains(r, in->x, in->y) && ing_unlocked(TAP_INGS[i])) {
                pouring = i;
                break;
            }
        }
    }
    if (pouring >= 0 && !drink->sealed) {
        int speed = progress_has_upgrade(UPG_FAST_TAP) ? 2 : 1;
        bool was_full = drink->overflowed;
        drink_pour(drink, TAP_INGS[pouring], speed);
        if (drink->overflowed && !was_full)
            toast("Oups, ça déborde !", "Relâche le stylet plus tôt.", COL_BAD);
        uint16_t col = ing_info(TAP_INGS[pouring])->color;
        if (drink->overflowed && (g_view.frame & 3) == 0)
            fx_emit(SCREEN_BOTTOM, FX_SPLASH, active_cx() + ((g_view.frame & 4) ? 30 : -30), CUP_TOP_Y - 6, 1, col);
        else if ((g_view.frame & 7) == 0)
            fx_emit(SCREEN_BOTTOM, FX_SPLASH, active_cx(), active_surface_y(), 1, col);
    } else {
        pouring = -1;
    }
    if ((pouring >= 0) != (g_view.pouring >= 0))
        plat_pour_loop(pouring >= 0);
    g_view.pouring = pouring;

    // Pompes à sirop : une pression par appui
    for (int i = 0; i < PUMP_COUNT; i++) {
        Rect r = pump_rect(i);
        if (g_view.pump_flash[i])
            g_view.pump_flash[i]--;
        if (input_pressed_in(r.x, r.y, r.w, r.h) && ing_unlocked(PUMP_INGS[i])) {
            if (drink_add(drink, PUMP_INGS[i])) {
                g_view.pump_flash[i] = 8;
                plat_sfx(SFX_PUMP);
                fx_emit(SCREEN_BOTTOM, FX_SPLASH, active_cx(), active_surface_y(), 3, ing_info(PUMP_INGS[i])->color);
            }
        }
    }

    // Bacs de garnitures : glisser jusqu'au gobelet (ou toucher pour ajouter)
    for (int i = 0; i < BIN_COUNT; i++) {
        Rect r = bin_rect(i);
        bool any_open = !g_session.cups[0].sealed || (g_session.cup_count == 2 && !g_session.cups[1].sealed);
        if (input_pressed_in(r.x, r.y, r.w, r.h) && ing_unlocked(BIN_INGS[i]) && any_open) {
            g_view.held = BIN_INGS[i];
            g_view.held_from = i;
        }
    }
    if (in->touch_up && g_view.held != ING_NONE) {
        Rect from = bin_rect(g_view.held_from);
        int target = cup_at(in->x, in->y);
        bool tapped = in->tapped && rect_contains(from, in->start_x, in->start_y);
        if (target >= 0) {
            g_session.active_cup = target;
            drink = session_drink(&g_session);
        }
        if ((target >= 0 || tapped) && drink_add(drink, g_view.held)) {
            plat_sfx(SFX_PLOP);
            uint16_t col = drink->liquid_total ? drink_mixed_color(drink) : ing_info(g_view.held)->color;
            fx_emit(SCREEN_BOTTOM, FX_SPLASH, active_cx(), active_surface_y(), 4, col);
        }
        g_view.held = ING_NONE;
    }

    // Actions
    for (int a = 0; a < ACT_COUNT; a++) {
        Rect r = action_rect(a);
        if (!input_tapped_in(r.x, r.y, r.w, r.h))
            continue;
        if (a == ACT_SEAL && drink_seal(drink)) {
            plat_sfx(SFX_SEAL);
            fx_emit(SCREEN_BOTTOM, FX_SPARKLE, active_cx(), CUP_TOP_Y - 8, 4, 0);
        }
        else if (a == ACT_TRASH && !drink_is_empty(drink)) {
            drink_reset(drink);
            plat_sfx(SFX_TRASH);
        } else if (a == ACT_SERVE)
            serve();
    }
    if (in->down & BTN_A)
        serve();

    update_shake(in);
}

static void update_pause(const Input *in)
{
    if (in->down & (BTN_START | BTN_B)) {
        g_view.paused = false;
        return;
    }
    for (int i = 0; i < PAUSE_BUTTONS; i++) {
        Rect r = pause_button_rect(i);
        if (!input_tapped_in(r.x, r.y, r.w, r.h))
            continue;
        plat_sfx(SFX_CLICK);
        if (i == 0) {
            g_view.paused = false;
        } else if (i == 1) {
            scene_change(&SCENE_BOOK);
        } else {
            g_session.in_progress = false;
            scene_change(&SCENE_TITLE);
        }
    }
}

static void handle_events(int first)
{
    Day *d = &g_session.day;
    for (int i = first; i < d->event_count; i++) {
        const DayEvent *e = &d->events[i];
        switch (e->type) {
        case EV_ARRIVE: {
            const Customer *c = &d->slots[e->slot];
            plat_sfx(SFX_BELL);
            plat_voice(customer_type(c->type)->voice);
            if (c->special >= 0) {
                char l1[48];
                snprintf(l1, sizeof(l1), "%s est là !", SPECIALS[c->special].name);
                toast(l1, "Un habitué : soigne sa commande !", COL_WOOD_DARK);
                fx_emit(SCREEN_TOP, FX_SPARKLE, customer_head_x(e->slot), CUSTOMER_HEAD_Y, 8, 0);
            }
            break;
        }
        case EV_SPECIAL_DONE: {
            const Customer *c = &d->slots[e->slot];
            char l1[48];
            if (e->value >= SPECIAL_VISITS)
                snprintf(l1, sizeof(l1), "Histoire terminée : %s !", SPECIALS[c->special].name);
            else
                snprintf(l1, sizeof(l1), "%s reviendra bientôt…", SPECIALS[c->special].name);
            toast(l1, c->says, COL_WOOD_DARK);
            plat_sfx(SFX_UNLOCK);
            fx_emit(SCREEN_TOP, FX_HEART, customer_head_x(e->slot), CUSTOMER_HEAD_Y, 8, 0);
            break;
        }
        case EV_STRESS_UP:
            g_view.jolt[e->slot] = 8;
            if (e->value >= 2)
                fx_emit(SCREEN_TOP, FX_SWEAT, customer_head_x(e->slot) + 14, CUSTOMER_HEAD_Y - 10, 2, 0);
            if (e->value == 3)
                plat_voice(customer_type(d->slots[e->slot].type)->voice * 7 / 10);
            break;
        case EV_LEAVE_ANGRY:
            plat_sfx(SFX_ANGRY);
            fx_emit(SCREEN_TOP, FX_SMOKE, customer_head_x(e->slot), CUSTOMER_HEAD_Y - 8, 6, 0);
            toast("Un client est parti fâché…", "L'ambiance de la boutique en prend un coup.", COL_BAD);
            break;
        case EV_UNLOCK:
            plat_sfx(SFX_UNLOCK);
            break;
        case EV_DAY_END:
            if (d->endless)
                toast("Fin du Rush !", "L'ambiance n'a pas tenu le choc…", COL_BAD);
            else if (d->collapsed)
                toast("L'ambiance est au plus bas…", "La boutique ferme plus tôt.", COL_BAD);
            else
                toast("Fermeture !", "Bravo pour cette journée.", COL_MINT_DARK);
            g_view.end_timer = END_DELAY_FRAMES;
            plat_pour_loop(false);
            break;
        default:
            break;
        }
    }
}

static void shop_update(void)
{
    const Input *in = input_get();
    Day *d = &g_session.day;
    g_view.frame++;

    if (g_view.paused) {
        update_pause(in);
    } else if (d->ended) {
        g_view.pouring = -1;
        g_view.held = ING_NONE;
        if (g_view.end_timer > 0 && --g_view.end_timer == 0) {
            g_session.in_progress = false;
            scene_change(&SCENE_DAY_END);
            return;
        }
    } else {
        if (in->down & BTN_START) {
            g_view.paused = true;
            g_view.pouring = -1;
            g_view.held = ING_NONE;
            plat_pour_loop(false);
        } else {
            day_update(d);
            handle_events(0);
            if (!d->ended)
                update_station(in);
        }
    }

    // La musique s'emballe quand l'ambiance chute (tâche 7.2)
    plat_music_tempo(d->ambiance < AMBIANCE_MAX * 3 / 10 ? 1150 : 1000);

    decor_shop_dirty(g_view.frame);
    if (!g_view.paused) {
        fx_update();
        for (int i = 0; i < DAY_SLOTS; i++)
            if (g_view.jolt[i])
                g_view.jolt[i]--;
        // Clients qui marchent : toute la bande du comptoir est redessinée
        // (et encore 2 frames après, pour effacer la dernière position)
        static int band_timer;
        for (int i = 0; i < DAY_SLOTS; i++) {
            CustomerState st = d->slots[i].state;
            if (st == CS_ARRIVING || (st == CS_LEAVING && d->slots[i].anim >= 16))
                band_timer = 3;
        }
        if (band_timer > 0) {
            band_timer--;
            screen_dirty(SCREEN_TOP, rect_make(0, CUSTOMER_Y - 16, SCREEN_W, COUNTER_Y - CUSTOMER_Y + 16));
        }
        for (int i = 0; i < g_session.cup_count; i++) {
            bool pouring = g_view.pouring >= 0 && i == g_session.active_cup;
            cup_anim_update(&g_view.cup_anim[i], &g_session.cups[i], pouring,
                            i == g_session.active_cup ? g_view.wobble : 0, g_view.frame);
        }
    }
    if (g_view.toast_timer > 0)
        g_view.toast_timer--;
    if (g_view.coin_popup_timer > 0)
        g_view.coin_popup_timer--;

    // Le compteur de pièces défile jusqu'au total
    int total = d->coins + d->tips;
    if (g_view.coins_shown != total && (g_view.frame & 1) == 0)
        g_view.coins_shown += g_view.coins_shown < total ? 1 : -1;

    // Machine à écrire du ticket : repart de zéro quand le contenu change
    uint32_t key = g_view.toast_timer > 0 ? 0x80000000u | (uint32_t)g_view.toast_id
                 : d->active >= 0 ? 0x40000000u | d->slots[d->active].seq : 0;
    if (key != g_view.panel_key) {
        g_view.panel_key = key;
        g_view.panel_since = g_view.frame;
    }
    int reveal = (int)(g_view.frame - g_view.panel_since) * 2;
    g_view.panel_reveal = reveal > 200 ? -1 : reveal;

    // ---- Zones à redessiner (écran haut)
    uint32_t h = hash_begin();
    h = hash_add(h, (int32_t)(d->endless ? d->frame / FPS : day_time_left(d) / FPS));
    h = hash_add(h, g_view.coins_shown);
    h = hash_add(h, d->ambiance / 10);
    h = hash_add(h, (int32_t)(g_view.frame / 10));   // cœur qui bat, horloge qui pulse
    watch(SCREEN_TOP, &w_hud, rect_make(0, 0, SCREEN_W, HUD_H), h);

    for (int i = 0; i < DAY_SLOTS; i++)
        watch(SCREEN_TOP, &w_slots[i], slot_rect(i), shop_slot_hash(&g_view, i));

    h = hash_begin();
    h = hash_add(h, d->active);
    h = hash_add(h, d->active >= 0 ? d->slots[d->active].recipe : -1);
    h = hash_add(h, d->active >= 0 ? d->slots[d->active].seq : -1);
    h = hash_add(h, g_view.toast_timer > 0 ? g_view.toast_id : -1);
    h = hash_add(h, (int32_t)shop_tutorial_hash(&g_view));
    h = hash_add(h, g_view.panel_reveal);
    h = hash_add(h, d->ended);
    watch(SCREEN_TOP, &w_panel, rect_make(0, COUNTER_Y, SCREEN_W, SCREEN_H - COUNTER_Y), h);

    // ---- Zones à redessiner (écran bas)
    h = hash_add(hash_begin(), g_view.pouring);
    h = hash_add(h, g_view.pouring >= 0 ? (int32_t)(g_view.frame / 2) : 0);
    watch(SCREEN_BOTTOM, &w_taps, rect_make(0, 0, SCREEN_W, PUMP_Y), h);
    h = hash_begin();
    for (int i = 0; i < PUMP_COUNT; i++)
        h = hash_add(h, g_view.pump_flash[i]);
    watch(SCREEN_BOTTOM, &w_pumps, rect_make(0, PUMP_Y, SEL_X, PUMP_H + 2), h);

    h = hash_add(hash_begin(), d->active);
    h = hash_add(h, d->active >= 0 ? d->slots[d->active].type : -1);
    h = hash_add(h, (int32_t)(g_view.frame / 15));
    watch(SCREEN_BOTTOM, &w_sel, rect_make(SEL_X, SEL_Y, SEL_W, PUMP_H + 2), h);

    h = hash_add(hash_begin(), g_view.held);
    h = hash_add(h, session_drink(&g_session)->sealed);
    h = hash_add(h, g_view.held != ING_NONE ? (int32_t)(g_view.frame / 4) : 0);
    watch(SCREEN_BOTTOM, &w_bins, rect_make(col_left_x() - 4, ROW_Y0, COL_W + 8, SCREEN_H - ROW_Y0), h);

    h = hash_add(hash_begin(), shop_actions_hash(&g_view));
    watch(SCREEN_BOTTOM, &w_actions, rect_make(col_right_x() - 4, ROW_Y0, COL_W + 8, SCREEN_H - ROW_Y0), h);

    watch(SCREEN_BOTTOM, &w_cup, cup_rect(), shop_cup_hash(&g_view));

    Rect held_r = rect_make(in->x - 9, in->y - 9, 18, 18);
    screen_track(SCREEN_BOTTOM, &t_held, held_r, g_view.held != ING_NONE, false);

    watch(SCREEN_BOTTOM, &w_pause, rect_make(0, 0, SCREEN_W, SCREEN_H),
          hash_add(hash_begin(), g_view.paused));
}

const Scene SCENE_SHOP = {
    .name = "shop",
    .enter = shop_enter,
    .update = shop_update,
    .exit = shop_exit,
};
