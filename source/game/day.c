#include "game/day.h"

#include <stdio.h>
#include <string.h>

#include "game/progress.h"
#include "game/recipes.h"
#include "game/specials.h"
#include "game/world.h"

// --- Difficulté (tâche 6.4)

// n = niveau global 1..20 : la difficulté monte doucement sur tout le jeu,
// avec un petit répit au début de chaque nouveau bar (nouveaux ingrédients)
int day_max_customers(int n) { return n <= 1 ? 2 : 3; }

static int difficulty(int n)
{
    int d = n - 1 - (level_index(n) == 0 && n > 1 ? 1 : 0);
    return d < 0 ? 0 : d;
}

int day_spawn_interval(int n)
{
    int s = 15 - difficulty(n) / 2;     // secondes
    return (s < 6 ? 6 : s) * FPS;
}

int day_base_patience(int n)
{
    int s = 44 - difficulty(n) * 6 / 5; // secondes
    return (s < 20 ? 20 : s) * FPS;
}

uint32_t day_time_left(const Day *d)
{
    if (d->endless)
        return DAY_DURATION;
    return d->frame >= DAY_DURATION ? 0 : DAY_DURATION - d->frame;
}

int rush_spawn_interval(uint32_t frame)
{
    int s = 11 - (int)(frame / (30 * FPS));   // -1 s toutes les 30 s
    return (s < 4 ? 4 : s) * FPS;
}

int rush_patience(int number, uint32_t frame)
{
    int p = day_base_patience(number) - (int)(frame / (30 * FPS)) * 2 * FPS;
    return p < 14 * FPS ? 14 * FPS : p;
}

static void push_event(Day *d, DayEventType t, int slot, int value)
{
    if (d->event_count < MAX_EVENTS)
        d->events[d->event_count++] = (DayEvent){ (uint8_t)t, (int8_t)slot, (int16_t)value };
}

void day_start(Day *d, int number, uint32_t seed)
{
    memset(d, 0, sizeof(*d));
    if (number < 1)
        number = 1;
    if (number > LEVEL_COUNT)
        number = LEVEL_COUNT;
    d->number = number;
    d->world = level_world(number);
    d->level = level_index(number);
    d->ambiance = AMBIANCE_MAX;
    d->active = -1;
    d->spawn_timer = FPS;   // premier client après 1 s
    rng_seed(&d->rng, seed);
    for (int i = 0; i < DAY_SLOTS; i++)
        d->slots[i].last_score = -1;

    // En arrivant dans un nouveau bar, on connaît déjà sa première recette
    int first = -1;
    bool known_here = false;
    for (int i = 0; i < RECIPE_COUNT; i++) {
        if (RECIPES[i].world != d->world)
            continue;
        if (first < 0 && !RECIPES[i].secret)
            first = i;
        known_here |= progress_recipe_known(i);
    }
    if (!known_here && first >= 0)
        progress_learn_recipe(first);

    bool can_discover = false;
    for (int i = 0; i < RECIPE_COUNT; i++)
        if (!RECIPES[i].secret && recipe_available(i, d->number) && !progress_recipe_known(i))
            can_discover = true;
    d->objective = objective_for_day(d->number, can_discover);

    int due[SPECIALS_PER_LEVEL];
    int n = specials_due(d->number, progress()->special_steps, due);
    for (int i = 0; i < SPECIALS_PER_LEVEL; i++) {
        d->specials[i] = (int8_t)(i < n ? due[i] : -1);
        d->special_result[i] = -1;
    }
}

int day_compute_stars(const Day *d)
{
    if (d->collapsed)
        return 0;
    int stars = 1;
    if (d->objective_met)
        stars++;
    if (d->lost == 0 && d->ambiance * 100 / AMBIANCE_MAX >= 60)
        stars++;
    return stars;
}

void day_start_rush(Day *d, int number, uint32_t seed)
{
    day_start(d, number, seed);
    d->endless = true;
    d->objective.type = OBJ_NONE;
    for (int i = 0; i < SPECIALS_PER_LEVEL; i++)
        d->specials[i] = -1;
}

int day_stress_level(const Customer *c)
{
    if (c->patience_max <= 0)
        return 0;
    int pct = 100 * c->patience / c->patience_max;
    if (pct > 70) return 0;
    if (pct > 40) return 1;
    if (pct > 15) return 2;
    return 3;
}

// --- Choix de la commande (tâche 4.5)

static int pick_recipe(Day *d, bool *riddle)
{
    int known[RECIPE_MAX], unknown[RECIPE_MAX], secret[RECIPE_MAX];
    int nk = 0, nu = 0, ns = 0;
    for (int i = 0; i < RECIPE_COUNT && i < RECIPE_MAX; i++) {
        const Recipe *r = &RECIPES[i];
        if (!recipe_available(i, d->number))
            continue;
        if (progress_recipe_known(i))
            known[nk++] = i;
        else if (r->secret)
            secret[ns++] = i;
        else
            unknown[nu++] = i;
    }
    // Mode Rush : uniquement des recettes connues, en clair
    if (d->endless) {
        *riddle = false;
        return nk ? known[rng_range(&d->rng, nk)] : 0;
    }
    // Premier client d'un niveau : une recette connue, en clair (tutoriel au niveau 1)
    if (d->next_seq == 0 && nk) {
        *riddle = false;
        return d->number == 1 ? 0 : known[rng_range(&d->rng, nk)];
    }
    int roll = rng_range(&d->rng, 100);
    if (nu && (roll < 40 || nk == 0)) {
        *riddle = true;
        return unknown[rng_range(&d->rng, nu)];
    }
    if (ns && roll >= 92) {
        *riddle = true;
        return secret[rng_range(&d->rng, ns)];
    }
    *riddle = false;
    return nk ? known[rng_range(&d->rng, nk)] : 0;
}

static int recipe_complexity(int recipe)
{
    int n = 0;
    for (int i = 1; i < ING_COUNT; i++)
        n += RECIPES[recipe].qty[i] != 0;
    return n + RECIPES[recipe].shaken;
}

static void spawn(Day *d)
{
    int count = 0, free_slot = -1;
    for (int i = 0; i < DAY_SLOTS; i++) {
        if (d->slots[i].state != CS_EMPTY)
            count++;
        else if (free_slot < 0 || rng_range(&d->rng, 2))
            free_slot = i;
    }
    if (free_slot < 0 || count >= (d->endless ? DAY_SLOTS : day_max_customers(d->number)))
        return;

    Customer *c = &d->slots[free_slot];
    memset(c, 0, sizeof(*c));
    c->state = CS_ARRIVING;
    c->type = WORLDS[d->world].customers[rng_range(&d->rng, WORLD_CUSTOMERS)];
    c->recipe = (int8_t)pick_recipe(d, &c->riddle);
    c->seq = d->next_seq++;
    c->last_score = -1;
    c->special = -1;

    // Un habitué passe à ce niveau ?
    for (int k = 0; k < SPECIALS_PER_LEVEL; k++) {
        uint32_t when = k == 0 ? SPECIAL_ARRIVAL : SPECIAL_ARRIVAL2;
        if (d->specials[k] < 0 || d->special_spawned[k] || d->frame < when)
            continue;
        int s = d->specials[k];
        int recipe = special_recipe(s, progress()->special_steps[s]);
        if (recipe >= 0) {
            c->special = (int8_t)s;
            c->type = SPECIALS[s].type;
            c->recipe = (int8_t)recipe;
            c->riddle = !progress_recipe_known(recipe);
        }
        d->special_spawned[k] = true;
        break;
    }

    int32_t p = d->endless ? rush_patience(d->number, d->frame) : day_base_patience(d->number);
    p += recipe_complexity(c->recipe) * 2 * FPS;
    if (c->riddle)
        p += 10 * FPS;
    p = p * customer_type(c->type)->patience_pct / 100;
    if (c->special >= 0)
        p = p * 3 / 2;   // les habitués sont plus indulgents
    if (progress_has_upgrade(UPG_CUSHIONS))
        p = p * 125 / 100;
    c->patience = c->patience_max = p;
    push_event(d, EV_ARRIVE, free_slot, c->type);
}

static void set_special_result(Day *d, int special, int result)
{
    for (int k = 0; k < SPECIALS_PER_LEVEL; k++)
        if (d->specials[k] == special)
            d->special_result[k] = (int8_t)result;
}

static bool is_waiting(const Day *d, int i)
{
    return i >= 0 && i < DAY_SLOTS && d->slots[i].state == CS_WAITING;
}

static void ensure_active(Day *d)
{
    if (is_waiting(d, d->active))
        return;
    d->active = -1;
    int best_seq = 0x7FFFFFFF;
    for (int i = 0; i < DAY_SLOTS; i++) {
        if (is_waiting(d, i) && d->slots[i].seq < best_seq) {
            best_seq = d->slots[i].seq;
            d->active = i;
        }
    }
}

void day_select(Day *d, int dir)
{
    for (int k = 1; k <= DAY_SLOTS; k++) {
        int i = ((d->active < 0 ? 0 : d->active) + dir * k + DAY_SLOTS * 2) % DAY_SLOTS;
        if (is_waiting(d, i)) {
            d->active = i;
            return;
        }
    }
}

void day_select_slot(Day *d, int slot)
{
    if (is_waiting(d, slot))
        d->active = slot;
}

static void change_ambiance(Day *d, int delta)
{
    d->ambiance += delta;
    if (d->ambiance > AMBIANCE_MAX)
        d->ambiance = AMBIANCE_MAX;
    if (d->ambiance < 0)
        d->ambiance = 0;
}

static bool anyone_present(const Day *d)
{
    for (int i = 0; i < DAY_SLOTS; i++)
        if (d->slots[i].state != CS_EMPTY)
            return true;
    return false;
}

void day_update(Day *d)
{
    d->event_count = 0;
    if (d->ended)
        return;
    d->frame++;

    // Arrivées
    if (day_time_left(d) > 15 * FPS) {
        if (--d->spawn_timer <= 0) {
            spawn(d);
            int base = d->endless ? rush_spawn_interval(d->frame) : day_spawn_interval(d->number);
            d->spawn_timer = base - base / 4 + rng_range(&d->rng, base / 2 + 1);
        }
    }

    bool garlands = progress_has_upgrade(UPG_GARLANDS);
    bool anyone_stressed = false;

    for (int i = 0; i < DAY_SLOTS; i++) {
        Customer *c = &d->slots[i];
        c->anim++;
        switch (c->state) {
        case CS_EMPTY:
            break;
        case CS_ARRIVING:
            if (c->anim >= ARRIVE_FRAMES) {
                c->state = CS_WAITING;
                c->anim = 0;
            }
            break;
        case CS_WAITING: {
            int before = day_stress_level(c);
            c->patience--;
            int level = day_stress_level(c);
            if (level > before)
                push_event(d, EV_STRESS_UP, i, level);
            if (level >= 2) {
                anyone_stressed = true;
                // L'ambiance de la boutique souffre (tâche 4.9) :
                // -1 toutes les 8 frames (inquiet) ou 4 frames (furieux)
                int period = level == 2 ? 8 : 4;
                if (garlands)
                    period *= 2;
                if (d->frame % period == 0)
                    change_ambiance(d, -1);
            }
            if (c->patience <= 0) {
                c->state = CS_LEAVING;
                c->anim = 0;
                c->happy = false;
                c->says = c->special >= 0 ? "Je repasserai un autre jour…" : "Trop long ! Je m'en vais !";
                if (c->special >= 0)
                    set_special_result(d, c->special, 0);
                d->lost++;
                change_ambiance(d, -AMBIANCE_ANGRY_LEAVE);
                push_event(d, EV_LEAVE_ANGRY, i, 0);
            }
            break;
        }
        case CS_LEAVING:
            if (c->anim >= LEAVE_FRAMES) {
                c->state = CS_EMPTY;
                c->anim = 0;
            }
            break;
        }
    }

    if (!anyone_stressed && (d->frame % 30) == 0)
        change_ambiance(d, 3);

    ensure_active(d);

    if (d->ambiance <= 0) {
        d->ended = true;
        d->collapsed = true;
    } else if (!d->endless && day_time_left(d) == 0 && !anyone_present(d)) {
        d->ended = true;
    }
    if (d->ended) {
        d->objective_met = objective_done(&d->objective, d);
        d->stars = day_compute_stars(d);
        push_event(d, EV_DAY_END, -1, d->collapsed);
    }
}

static void unlock(Day *d, int recipe)
{
    progress_learn_recipe(recipe);
    if (d->unlocked_count < MAX_UNLOCKS)
        d->unlocked[d->unlocked_count++] = (int8_t)recipe;
    push_event(d, EV_UNLOCK, -1, recipe);
}

bool day_serve(Day *d, const Drink *drink, ServeResult *out)
{
    ensure_active(d);
    if (!is_waiting(d, d->active))
        return false;

    int slot = d->active;
    Customer *c = &d->slots[slot];
    const Recipe *r = &RECIPES[c->recipe];
    const CustomerType *t = customer_type(c->type);

    memset(out, 0, sizeof(*out));
    out->unlocked = -1;
    out->score = recipe_score(r, drink);
    out->happy = out->score >= t->picky;
    out->coins = out->score >= 30 ? (r->price * out->score + 50) / 100 : 0;
    if (out->score >= 90) {
        int stress = day_stress_level(c);
        out->tip = t->tip + (stress == 0 ? 2 : stress == 1 ? 1 : 0);
        if (progress_has_upgrade(UPG_TIP_JAR))
            out->tip++;
    }
    out->hint = out->score >= 100 ? NULL : recipe_hint(r, drink, d->hint_buf, sizeof(d->hint_buf));

    // Découverte (tâche 5.4) : la recette demandée…
    if (out->score >= 70 && !progress_recipe_known(c->recipe)) {
        unlock(d, c->recipe);
        out->unlocked = c->recipe;
    }
    // … ou n'importe quelle recette reproduite presque parfaitement
    int match_score;
    int match = recipe_best_match(drink, d->world, &match_score);
    if (match_score >= 85 && !progress_recipe_known(match)) {
        unlock(d, match);
        if (out->unlocked < 0)
            out->unlocked = match;
    }

    d->served++;
    if (out->score >= 90)
        d->perfect++;
    d->coins += out->coins;
    d->tips += out->tip;
    change_ambiance(d, out->happy ? 60 : -80);

    c->state = CS_LEAVING;
    c->anim = 0;
    c->happy = out->happy;
    c->last_score = (int8_t)out->score;
    if (out->score >= 90)
        c->says = "Parfait ! Merci !!";
    else if (out->happy)
        c->says = "Miam, merci !";
    else {
        snprintf(c->says_buf, sizeof(c->says_buf), "%s", out->hint ? out->hint : "Bof…");
        c->says = c->says_buf;
    }
    // Habitué : l'histoire avance si la visite est réussie
    if (c->special >= 0) {
        const SpecialCustomer *sp = &SPECIALS[c->special];
        uint8_t *step = &progress()->special_steps[c->special];
        if (out->happy && *step < sp->visit_count) {
            c->says = sp->visits[*step].thanks;
            (*step)++;
            int bonus = sp->tip_bonus + (*step == sp->visit_count ? sp->final_bonus : 0);
            out->tip += bonus;
            d->tips += bonus;
            set_special_result(d, c->special, 1);
            push_event(d, EV_SPECIAL_DONE, slot, *step);
        } else {
            c->says = "Pas tout à fait… Je repasserai !";
            set_special_result(d, c->special, 0);
        }
    }
    push_event(d, EV_SERVED, slot, out->score);
    ensure_active(d);
    return true;
}
