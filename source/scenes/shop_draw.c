// Rendu de la boutique : comptoir et clients (haut), poste d'assemblage (bas)
#include "scenes/shop_draw.h"

#include <stdio.h>
#include <string.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/specials.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "core/tween.h"
#include "scenes/cup_anim.h"
#include "scenes/decor.h"
#include "scenes/fx.h"
#include "scenes/shop_layout.h"
#include "scenes/ui.h"

static const ShopView *V;
int g_station_world;
#define BUBBLE_POP 10
bool g_layout_lefty;

static bool visible(const Surface *s, Rect r)
{
    return !rect_empty(rect_intersect(s->clip, r));
}

// ============================================================ Données d'aide

const Image *customer_image_ex(int type, int mood, int pose)
{
    // [espèce][humeur][pose] : pose 0 = normal, 1 = yeux fermés, 2 = parle
    // Ordre identique à CustomerTypeId (et à ORDER dans tools/art/customers.py)
    static const Image *const IMGS[CT_COUNT][STRESS_LEVELS][CUSTOMER_POSES] = {
        {
            { &IMG_customers_lapin_0, &IMG_customers_lapin_0b, &IMG_customers_lapin_0t },
            { &IMG_customers_lapin_1, &IMG_customers_lapin_1b, &IMG_customers_lapin_1t },
            { &IMG_customers_lapin_2, &IMG_customers_lapin_2b, &IMG_customers_lapin_2t },
            { &IMG_customers_lapin_3, &IMG_customers_lapin_3b, &IMG_customers_lapin_3t },
        },
        {
            { &IMG_customers_chat_0, &IMG_customers_chat_0b, &IMG_customers_chat_0t },
            { &IMG_customers_chat_1, &IMG_customers_chat_1b, &IMG_customers_chat_1t },
            { &IMG_customers_chat_2, &IMG_customers_chat_2b, &IMG_customers_chat_2t },
            { &IMG_customers_chat_3, &IMG_customers_chat_3b, &IMG_customers_chat_3t },
        },
        {
            { &IMG_customers_ours_0, &IMG_customers_ours_0b, &IMG_customers_ours_0t },
            { &IMG_customers_ours_1, &IMG_customers_ours_1b, &IMG_customers_ours_1t },
            { &IMG_customers_ours_2, &IMG_customers_ours_2b, &IMG_customers_ours_2t },
            { &IMG_customers_ours_3, &IMG_customers_ours_3b, &IMG_customers_ours_3t },
        },
        {
            { &IMG_customers_grenouille_0, &IMG_customers_grenouille_0b, &IMG_customers_grenouille_0t },
            { &IMG_customers_grenouille_1, &IMG_customers_grenouille_1b, &IMG_customers_grenouille_1t },
            { &IMG_customers_grenouille_2, &IMG_customers_grenouille_2b, &IMG_customers_grenouille_2t },
            { &IMG_customers_grenouille_3, &IMG_customers_grenouille_3b, &IMG_customers_grenouille_3t },
        },
        {
            { &IMG_customers_poussin_0, &IMG_customers_poussin_0b, &IMG_customers_poussin_0t },
            { &IMG_customers_poussin_1, &IMG_customers_poussin_1b, &IMG_customers_poussin_1t },
            { &IMG_customers_poussin_2, &IMG_customers_poussin_2b, &IMG_customers_poussin_2t },
            { &IMG_customers_poussin_3, &IMG_customers_poussin_3b, &IMG_customers_poussin_3t },
        },
        {
            { &IMG_customers_panda_0, &IMG_customers_panda_0b, &IMG_customers_panda_0t },
            { &IMG_customers_panda_1, &IMG_customers_panda_1b, &IMG_customers_panda_1t },
            { &IMG_customers_panda_2, &IMG_customers_panda_2b, &IMG_customers_panda_2t },
            { &IMG_customers_panda_3, &IMG_customers_panda_3b, &IMG_customers_panda_3t },
        },
        {
            { &IMG_customers_phoque_0, &IMG_customers_phoque_0b, &IMG_customers_phoque_0t },
            { &IMG_customers_phoque_1, &IMG_customers_phoque_1b, &IMG_customers_phoque_1t },
            { &IMG_customers_phoque_2, &IMG_customers_phoque_2b, &IMG_customers_phoque_2t },
            { &IMG_customers_phoque_3, &IMG_customers_phoque_3b, &IMG_customers_phoque_3t },
        },
        {
            { &IMG_customers_crabe_0, &IMG_customers_crabe_0b, &IMG_customers_crabe_0t },
            { &IMG_customers_crabe_1, &IMG_customers_crabe_1b, &IMG_customers_crabe_1t },
            { &IMG_customers_crabe_2, &IMG_customers_crabe_2b, &IMG_customers_crabe_2t },
            { &IMG_customers_crabe_3, &IMG_customers_crabe_3b, &IMG_customers_crabe_3t },
        },
        {
            { &IMG_customers_pingouin_0, &IMG_customers_pingouin_0b, &IMG_customers_pingouin_0t },
            { &IMG_customers_pingouin_1, &IMG_customers_pingouin_1b, &IMG_customers_pingouin_1t },
            { &IMG_customers_pingouin_2, &IMG_customers_pingouin_2b, &IMG_customers_pingouin_2t },
            { &IMG_customers_pingouin_3, &IMG_customers_pingouin_3b, &IMG_customers_pingouin_3t },
        },
        {
            { &IMG_customers_poulpe_0, &IMG_customers_poulpe_0b, &IMG_customers_poulpe_0t },
            { &IMG_customers_poulpe_1, &IMG_customers_poulpe_1b, &IMG_customers_poulpe_1t },
            { &IMG_customers_poulpe_2, &IMG_customers_poulpe_2b, &IMG_customers_poulpe_2t },
            { &IMG_customers_poulpe_3, &IMG_customers_poulpe_3b, &IMG_customers_poulpe_3t },
        },
        {
            { &IMG_customers_tortue_0, &IMG_customers_tortue_0b, &IMG_customers_tortue_0t },
            { &IMG_customers_tortue_1, &IMG_customers_tortue_1b, &IMG_customers_tortue_1t },
            { &IMG_customers_tortue_2, &IMG_customers_tortue_2b, &IMG_customers_tortue_2t },
            { &IMG_customers_tortue_3, &IMG_customers_tortue_3b, &IMG_customers_tortue_3t },
        },
        {
            { &IMG_customers_dauphin_0, &IMG_customers_dauphin_0b, &IMG_customers_dauphin_0t },
            { &IMG_customers_dauphin_1, &IMG_customers_dauphin_1b, &IMG_customers_dauphin_1t },
            { &IMG_customers_dauphin_2, &IMG_customers_dauphin_2b, &IMG_customers_dauphin_2t },
            { &IMG_customers_dauphin_3, &IMG_customers_dauphin_3b, &IMG_customers_dauphin_3t },
        },
        {
            { &IMG_customers_renard_0, &IMG_customers_renard_0b, &IMG_customers_renard_0t },
            { &IMG_customers_renard_1, &IMG_customers_renard_1b, &IMG_customers_renard_1t },
            { &IMG_customers_renard_2, &IMG_customers_renard_2b, &IMG_customers_renard_2t },
            { &IMG_customers_renard_3, &IMG_customers_renard_3b, &IMG_customers_renard_3t },
        },
        {
            { &IMG_customers_hibou_0, &IMG_customers_hibou_0b, &IMG_customers_hibou_0t },
            { &IMG_customers_hibou_1, &IMG_customers_hibou_1b, &IMG_customers_hibou_1t },
            { &IMG_customers_hibou_2, &IMG_customers_hibou_2b, &IMG_customers_hibou_2t },
            { &IMG_customers_hibou_3, &IMG_customers_hibou_3b, &IMG_customers_hibou_3t },
        },
        {
            { &IMG_customers_ecureuil_0, &IMG_customers_ecureuil_0b, &IMG_customers_ecureuil_0t },
            { &IMG_customers_ecureuil_1, &IMG_customers_ecureuil_1b, &IMG_customers_ecureuil_1t },
            { &IMG_customers_ecureuil_2, &IMG_customers_ecureuil_2b, &IMG_customers_ecureuil_2t },
            { &IMG_customers_ecureuil_3, &IMG_customers_ecureuil_3b, &IMG_customers_ecureuil_3t },
        },
        {
            { &IMG_customers_marmotte_0, &IMG_customers_marmotte_0b, &IMG_customers_marmotte_0t },
            { &IMG_customers_marmotte_1, &IMG_customers_marmotte_1b, &IMG_customers_marmotte_1t },
            { &IMG_customers_marmotte_2, &IMG_customers_marmotte_2b, &IMG_customers_marmotte_2t },
            { &IMG_customers_marmotte_3, &IMG_customers_marmotte_3b, &IMG_customers_marmotte_3t },
        },
        {
            { &IMG_customers_biche_0, &IMG_customers_biche_0b, &IMG_customers_biche_0t },
            { &IMG_customers_biche_1, &IMG_customers_biche_1b, &IMG_customers_biche_1t },
            { &IMG_customers_biche_2, &IMG_customers_biche_2b, &IMG_customers_biche_2t },
            { &IMG_customers_biche_3, &IMG_customers_biche_3b, &IMG_customers_biche_3t },
        },
        {
            { &IMG_customers_yeti_0, &IMG_customers_yeti_0b, &IMG_customers_yeti_0t },
            { &IMG_customers_yeti_1, &IMG_customers_yeti_1b, &IMG_customers_yeti_1t },
            { &IMG_customers_yeti_2, &IMG_customers_yeti_2b, &IMG_customers_yeti_2t },
            { &IMG_customers_yeti_3, &IMG_customers_yeti_3b, &IMG_customers_yeti_3t },
        },
        {
            { &IMG_customers_raton_0, &IMG_customers_raton_0b, &IMG_customers_raton_0t },
            { &IMG_customers_raton_1, &IMG_customers_raton_1b, &IMG_customers_raton_1t },
            { &IMG_customers_raton_2, &IMG_customers_raton_2b, &IMG_customers_raton_2t },
            { &IMG_customers_raton_3, &IMG_customers_raton_3b, &IMG_customers_raton_3t },
        },
        {
            { &IMG_customers_pigeon_0, &IMG_customers_pigeon_0b, &IMG_customers_pigeon_0t },
            { &IMG_customers_pigeon_1, &IMG_customers_pigeon_1b, &IMG_customers_pigeon_1t },
            { &IMG_customers_pigeon_2, &IMG_customers_pigeon_2b, &IMG_customers_pigeon_2t },
            { &IMG_customers_pigeon_3, &IMG_customers_pigeon_3b, &IMG_customers_pigeon_3t },
        },
        {
            { &IMG_customers_chauvesouris_0, &IMG_customers_chauvesouris_0b, &IMG_customers_chauvesouris_0t },
            { &IMG_customers_chauvesouris_1, &IMG_customers_chauvesouris_1b, &IMG_customers_chauvesouris_1t },
            { &IMG_customers_chauvesouris_2, &IMG_customers_chauvesouris_2b, &IMG_customers_chauvesouris_2t },
            { &IMG_customers_chauvesouris_3, &IMG_customers_chauvesouris_3b, &IMG_customers_chauvesouris_3t },
        },
        {
            { &IMG_customers_souris_0, &IMG_customers_souris_0b, &IMG_customers_souris_0t },
            { &IMG_customers_souris_1, &IMG_customers_souris_1b, &IMG_customers_souris_1t },
            { &IMG_customers_souris_2, &IMG_customers_souris_2b, &IMG_customers_souris_2t },
            { &IMG_customers_souris_3, &IMG_customers_souris_3b, &IMG_customers_souris_3t },
        },
        {
            { &IMG_customers_carlin_0, &IMG_customers_carlin_0b, &IMG_customers_carlin_0t },
            { &IMG_customers_carlin_1, &IMG_customers_carlin_1b, &IMG_customers_carlin_1t },
            { &IMG_customers_carlin_2, &IMG_customers_carlin_2b, &IMG_customers_carlin_2t },
            { &IMG_customers_carlin_3, &IMG_customers_carlin_3b, &IMG_customers_carlin_3t },
        },
        {
            { &IMG_customers_chat_noir_0, &IMG_customers_chat_noir_0b, &IMG_customers_chat_noir_0t },
            { &IMG_customers_chat_noir_1, &IMG_customers_chat_noir_1b, &IMG_customers_chat_noir_1t },
            { &IMG_customers_chat_noir_2, &IMG_customers_chat_noir_2b, &IMG_customers_chat_noir_2t },
            { &IMG_customers_chat_noir_3, &IMG_customers_chat_noir_3b, &IMG_customers_chat_noir_3t },
        },
        {
            { &IMG_customers_champignon_0, &IMG_customers_champignon_0b, &IMG_customers_champignon_0t },
            { &IMG_customers_champignon_1, &IMG_customers_champignon_1b, &IMG_customers_champignon_1t },
            { &IMG_customers_champignon_2, &IMG_customers_champignon_2b, &IMG_customers_champignon_2t },
            { &IMG_customers_champignon_3, &IMG_customers_champignon_3b, &IMG_customers_champignon_3t },
        },
        {
            { &IMG_customers_luciole_0, &IMG_customers_luciole_0b, &IMG_customers_luciole_0t },
            { &IMG_customers_luciole_1, &IMG_customers_luciole_1b, &IMG_customers_luciole_1t },
            { &IMG_customers_luciole_2, &IMG_customers_luciole_2b, &IMG_customers_luciole_2t },
            { &IMG_customers_luciole_3, &IMG_customers_luciole_3b, &IMG_customers_luciole_3t },
        },
        {
            { &IMG_customers_escargot_0, &IMG_customers_escargot_0b, &IMG_customers_escargot_0t },
            { &IMG_customers_escargot_1, &IMG_customers_escargot_1b, &IMG_customers_escargot_1t },
            { &IMG_customers_escargot_2, &IMG_customers_escargot_2b, &IMG_customers_escargot_2t },
            { &IMG_customers_escargot_3, &IMG_customers_escargot_3b, &IMG_customers_escargot_3t },
        },
        {
            { &IMG_customers_papillon_0, &IMG_customers_papillon_0b, &IMG_customers_papillon_0t },
            { &IMG_customers_papillon_1, &IMG_customers_papillon_1b, &IMG_customers_papillon_1t },
            { &IMG_customers_papillon_2, &IMG_customers_papillon_2b, &IMG_customers_papillon_2t },
            { &IMG_customers_papillon_3, &IMG_customers_papillon_3b, &IMG_customers_papillon_3t },
        },
        {
            { &IMG_customers_herisson_0, &IMG_customers_herisson_0b, &IMG_customers_herisson_0t },
            { &IMG_customers_herisson_1, &IMG_customers_herisson_1b, &IMG_customers_herisson_1t },
            { &IMG_customers_herisson_2, &IMG_customers_herisson_2b, &IMG_customers_herisson_2t },
            { &IMG_customers_herisson_3, &IMG_customers_herisson_3b, &IMG_customers_herisson_3t },
        },
        {
            { &IMG_customers_dragon_0, &IMG_customers_dragon_0b, &IMG_customers_dragon_0t },
            { &IMG_customers_dragon_1, &IMG_customers_dragon_1b, &IMG_customers_dragon_1t },
            { &IMG_customers_dragon_2, &IMG_customers_dragon_2b, &IMG_customers_dragon_2t },
            { &IMG_customers_dragon_3, &IMG_customers_dragon_3b, &IMG_customers_dragon_3t },
        },
    };
    if (type < 0 || type >= CT_COUNT)
        type = 0;
    if (mood < 0)
        mood = 0;
    if (mood >= STRESS_LEVELS)
        mood = STRESS_LEVELS - 1;
    if (pose < 0 || pose >= CUSTOMER_POSES)
        pose = 0;
    return IMGS[type][mood][pose];
}

const Image *customer_image(int type, int mood)
{
    return customer_image_ex(type, mood, POSE_NORMAL);
}

const Image *topping_icon(int ing)
{
    switch (ing) {
    case ING_PEARLS:  return &IMG_icons_pearl;
    case ING_POPPING: return &IMG_icons_popping;
    case ING_JELLY:   return &IMG_icons_jelly;
    case ING_ICE:     return &IMG_icons_ice;
    case ING_CREAM:   return &IMG_icons_cream;
    case ING_SEA_CREAM:   return &IMG_icons_sea_cream;
    case ING_NATA:        return &IMG_icons_nata;
    case ING_MARSHMALLOW: return &IMG_icons_marshmallow;
    case ING_PUDDING:     return &IMG_icons_pudding;
    case ING_GLOW_JELLY:  return &IMG_icons_glow;
    case ING_PETALS:      return &IMG_icons_petals;
    default:          return &IMG_icons_lock;
    }
}

void recipe_describe(int recipe, char *buf, int size)
{
    const Recipe *r = &RECIPES[recipe];
    int n = 0;
    buf[0] = 0;
    for (int i = 1; i < ING_COUNT && n < size; i++) {
        if (!r->qty[i])
            continue;
        const IngredientInfo *info = ing_info(i);
        const char *sep = n ? ", " : "";
        if (info->kind == KIND_LIQUID)
            n += snprintf(buf + n, size - n, "%s%s %d %%", sep, info->name, r->qty[i]);
        else
            n += snprintf(buf + n, size - n, "%s%s ×%d", sep, info->name, r->qty[i]);
    }
    if (r->shaken && n < size)
        snprintf(buf + n, size - n, ", secouée");
}

// Étape suivante du tutoriel (jour 1, commande en clair). NULL si rien à dire.
static const char *tutorial_step(const ShopView *v, int *step_id)
{
    const Day *d = &v->session->day;
    const Drink *k = session_drink(v->session);
    static char buf[64];
    *step_id = 0;
    if (d->number != 1 || d->active < 0)
        return NULL;
    const Customer *c = &d->slots[d->active];
    if (c->riddle)
        return NULL;
    const Recipe *r = &RECIPES[c->recipe];

    for (int i = 1; i < ING_COUNT; i++) {
        const IngredientInfo *info = ing_info(i);
        int t = r->qty[i], a = k->qty[i];
        *step_id = i * 4;
        if (t == 0 && a > 0) {
            snprintf(buf, sizeof(buf), "Pas de %s ici : touche « Jeter » !", info->name);
            return buf;
        }
        if (info->kind == KIND_LIQUID) {
            if (a > t + LIQUID_TOLERANCE) {
                snprintf(buf, sizeof(buf), "Trop de %s : jette et recommence !", info->name);
                return buf;
            }
            if (a < t - LIQUID_TOLERANCE) {
                snprintf(buf, sizeof(buf), "Maintiens « %s » jusqu'au trait %d.",
                         info->short_name, (k->liquid_total - a + t) / 10);
                return buf;
            }
        } else if (a < t) {
            *step_id += 1;
            if (info->kind == KIND_SYRUP)
                snprintf(buf, sizeof(buf), "Appuie sur la pompe « %s » (×%d).", info->short_name, t);
            else
                snprintf(buf, sizeof(buf), "Glisse « %s » dans le gobelet (×%d).", info->short_name, t);
            return buf;
        } else if (a > t) {
            *step_id += 2;
            snprintf(buf, sizeof(buf), "Trop de %s : jette et recommence !", info->name);
            return buf;
        }
    }
    if (!k->sealed) {
        *step_id = 200;
        return "Touche « Fermer » pour sceller.";
    }
    if (r->shaken && !k->shaken) {
        *step_id = 201;
        return "Frotte le gobelet de gauche à droite !";
    }
    *step_id = 202;
    return "Parfait ! Touche « Servir » (ou A).";
}

uint32_t shop_tutorial_hash(const ShopView *v)
{
    int id;
    tutorial_step(v, &id);
    return (uint32_t)id;
}

uint32_t shop_cup_hash(const ShopView *v)
{
    uint32_t h = hash_begin();
    for (int c = 0; c < v->session->cup_count; c++) {
        const Drink *d = &v->session->cups[c];
        for (int i = 0; i < ING_COUNT; i++)
            h = hash_add(h, d->qty[i]);
        h = hash_add(h, d->sealed | (d->shaken << 1) | (d->overflowed << 2));
    }
    h = hash_add(h, v->session->active_cup);
    // Les vagues bougent en permanence : on redessine toutes les 2 frames
    h = hash_add(h, (int32_t)(v->frame / 2));
    h = hash_add(h, v->wobble);
    h = hash_add(h, v->pouring);
    h = hash_add(h, v->paused);
    return h;
}

// ================================================================ Écran bas

// Texte lisible sur une couleur donnée
static uint16_t ink_on(uint16_t bg)
{
    int lum = (bg & 31) * 3 + ((bg >> 5) & 31) * 6 + ((bg >> 10) & 31);
    return lum < 160 ? COL_WHITE : COL_INK;
}

static void draw_tap(Surface *s, int i, bool pressed)
{
    Rect r = tap_rect(i);
    if (!visible(s, r))
        return;
    Ingredient ing = TAP_ING(i);
    const IngredientInfo *info = ing_info(ing);
    bool locked = !level_has_ing(V->session->day.number, ing);
    draw_mask(s, &IMG_station_tap_mask, r.x, r.y, locked ? COL_GREY : info->color);
    if (pressed) {
        // Glouglou : des bulles remontent dans le réservoir pendant le versement
        uint16_t b = color_mix(info->color, COL_WHITE, 170);
        for (int k = 0; k < 4; k++) {
            int by = r.y + 15 - (int)((V->frame / 2 + k * 4) % 12);
            int bx = r.x + 9 + (k * 5) % 14 + (((V->frame / 4) + k) & 1);
            draw_pixel(s, bx, by, b);
            if (k & 1)
                draw_pixel(s, bx + 1, by, b);
        }
    }
    draw_image(s, pressed ? &IMG_station_tap_on : &IMG_station_tap, r.x, r.y);
    if (locked)
        draw_image(s, &IMG_icons_lock, r.x + 8, r.y + 3);
    text_draw_aligned(s, r.x, r.y + 23, r.w, info->short_name,
                      locked ? COL_GREY_DARK : pressed ? COL_PINK_DARK : COL_INK, ALIGN_CENTER);
}

static void draw_pump(Surface *s, int i, bool flash)
{
    Rect r = pump_rect(i);
    if (!visible(s, r))
        return;
    Ingredient ing = PUMP_ING(i);
    const IngredientInfo *info = ing_info(ing);
    bool locked = !level_has_ing(V->session->day.number, ing);
    uint16_t body = locked ? COL_GREY : flash ? color_mix(info->color, COL_WHITE, 110) : info->color;
    draw_mask(s, &IMG_station_pump_mask, r.x, r.y, body);
    draw_image(s, flash ? &IMG_station_pump_on : &IMG_station_pump, r.x, r.y);
    // Une goutte de sirop tombe du bec après une pression
    int t = V->pump_flash[i];
    if (t > 0 && !locked) {
        int gy = r.y + 4 + (8 - t);
        draw_rect(s, r.x + 27, gy, 2, 2, info->color);
        draw_pixel(s, r.x + 27, gy, color_mix(info->color, COL_WHITE, 140));
    }
    if (locked)
        draw_image(s, &IMG_icons_lock, r.x + 11, r.y + 7);
    else
        text_draw_aligned(s, r.x + 2, r.y + 10, r.w - 4, info->short_name, ink_on(body), ALIGN_CENTER);
}

static void draw_selector(Surface *s)
{
    Rect r = rect_make(SEL_X, SEL_Y, SEL_W, SEL_H);
    if (!visible(s, r))
        return;
    const Day *d = &V->session->day;
    draw_round_rect(s, r.x, r.y, r.w, r.h, COL_LAVENDER, COL_LAV_DARK);
    int waiting = 0;
    for (int k = 0; k < DAY_SLOTS; k++)
        waiting += d->slots[k].state == CS_WAITING;
    int nudge = waiting > 1 && ((V->frame / 15) & 1) ? 1 : 0;
    text_draw(s, r.x + 3 - nudge, r.y + 7, "<", COL_INK, 1);
    text_draw(s, r.x + r.w - 9 + nudge, r.y + 7, ">", COL_INK, 1);
    const char *full = d->active >= 0 ? customer_type(d->slots[d->active].type)->name : "-";
    char name[12];
    snprintf(name, sizeof(name), "%s", full);
    if (text_width(full, 1) > SEL_W - 20) {   // « Chauve-so. »
        int chars = 0, i = 0;
        for (; full[i] && chars < 8; i++)
            if ((full[i] & 0xC0) != 0x80)
                chars++;
        snprintf(name, sizeof(name), "%.*s.", i, full);
    }
    text_draw_aligned(s, r.x + 10, r.y + 2, r.w - 20, "Pour :", COL_INK_SOFT, ALIGN_CENTER);
    text_draw_aligned(s, r.x + 10, r.y + 12, r.w - 20, name, COL_INK, ALIGN_CENTER);
}

static void draw_bin(Surface *s, int i)
{
    Rect r = bin_rect(i);
    if (!visible(s, r))
        return;
    Ingredient ing = BIN_ING(i);
    const IngredientInfo *info = ing_info(ing);
    bool locked = !level_has_ing(V->session->day.number, ing);
    bool holding = V->held == (int)ing;
    draw_image(s, holding ? &IMG_station_bin_on : &IMG_station_bin, r.x, r.y);
    int jiggle = holding ? (((V->frame / 4) & 1) ? -1 : 1) : 0;
    draw_image(s, locked ? &IMG_icons_lock : topping_icon(ing), r.x + (r.w - 16) / 2 + jiggle, r.y + 1);
    text_draw_aligned(s, r.x, r.y + 16, r.w, info->short_name, locked ? COL_GREY_DARK : COL_INK,
                      ALIGN_CENTER);
}

uint32_t shop_actions_hash(const ShopView *v)
{
    const Drink *d = session_drink(v->session);
    uint32_t h = hash_add(hash_begin(), drink_is_empty(d));
    h = hash_add(h, d->sealed);
    h = hash_add(h, v->session->day.active >= 0);
    h = hash_add(h, d->sealed && v->session->day.active >= 0 ? (int)(v->frame / 12) & 1 : 0);
    return h;
}

static void draw_actions(Surface *s)
{
    const Drink *d = session_drink(V->session);
    bool empty = drink_is_empty(d);
    draw_bin(s, 4);   // crème, en haut de la colonne de droite
    ui_button(s, action_rect(ACT_SEAL), d->sealed ? "Fermé" : "Fermer",
              empty || d->sealed ? BTN_STYLE_DISABLED : BTN_STYLE_NORMAL);
    ui_button(s, action_rect(ACT_TRASH), "Jeter", empty ? BTN_STYLE_DISABLED : BTN_STYLE_NORMAL);
    bool can_serve = !empty && V->session->day.active >= 0;
    Rect sr = action_rect(ACT_SERVE);
    // Boisson fermée et client qui attend : le bouton « Servir » pulse
    bool ready = can_serve && d->sealed;
    if (ready && ((V->frame / 12) & 1))
        sr.y -= 1;
    ui_button(s, sr, "Servir", can_serve ? (ready && ((V->frame / 12) & 1) ? BTN_STYLE_PRESSED : BTN_STYLE_PRIMARY)
                                         : BTN_STYLE_DISABLED);
}

Rect pause_button_rect(int i)
{
    return rect_make(64, 66 + i * 27, 128, 24);
}

static void draw_pause(Surface *s)
{
    draw_round_rect(s, 40, 36, 176, 128, COL_CREAM, COL_PINK_DARK);
    draw_round_rect(s, 42, 38, 172, 124, COL_CREAM, COL_PINK);
    text_draw_shadow(s, 128 - text_width("Pause", 2) / 2, 44, "Pause", COL_PINK_DARK, COL_PINK_PALE, 2);
    static const char *const LABELS[PAUSE_BUTTONS] = { "Reprendre", "Livre de recettes", "Quitter la journée" };
    for (int i = 0; i < PAUSE_BUTTONS; i++)
        ui_button(s, pause_button_rect(i), LABELS[i], i == 0 ? BTN_STYLE_PRIMARY : BTN_STYLE_NORMAL);
    const Day *d = &V->session->day;
    if (d->objective.type != OBJ_NONE) {
        char obj[48], line[64];
        objective_describe(&d->objective, obj, sizeof(obj));
        snprintf(line, sizeof(line), "%s (%d/%d)", obj, objective_value(&d->objective, d),
                 d->objective.type == OBJ_NO_LOSS ? 0 : d->objective.target);
        text_draw_aligned(s, 42, 149, 172, line, COL_INK_SOFT, ALIGN_CENTER);
    }
}

void shop_draw_bottom(Surface *s)
{
    for (int i = 0; i < TAP_COUNT; i++)
        draw_tap(s, i, V->pouring == i);
    for (int i = 0; i < PUMP_COUNT; i++)
        draw_pump(s, i, V->pump_flash[i] > 0);
    draw_selector(s);
    for (int i = 0; i < 4; i++)
        draw_bin(s, i);
    draw_actions(s);

    if (visible(s, cup_rect())) {
        const ShopSession *ss = V->session;
        int pour = V->pouring >= 0 ? (int)TAP_ING(V->pouring) : 0;
        for (int i = 0; i < ss->cup_count; i++) {
            bool active = i == ss->active_cup;
            int cx = cup_cx_of(i, ss->cup_count);
            // Sous-verre : rose pour le gobelet actif quand il y en a deux
            uint16_t coaster = ss->cup_count == 2 && active ? COL_PINK_DARK : COL_MINT_DARK;
            draw_rect(s, cx - 31, CUP_BOT_Y + 3, 63, 6, coaster);
            draw_rect(s, cx - 27, CUP_BOT_Y + 9, 55, 2, color_darken(coaster, 4));
            draw_cup(s, cx, 0, &ss->cups[i], &V->cup_anim[i], V->frame, active ? V->wobble : 0,
                     active ? pour : 0);
        }
    }

    if (V->held != ING_NONE) {
        const Input *in = input_get();
        draw_image(s, topping_icon(V->held), in->x - 8, in->y - 8);
    }
    fx_draw(SCREEN_BOTTOM, s);
    if (V->paused)
        draw_pause(s);
}

// ================================================================ Écran haut

// Accessoire dessiné par-dessus l'image d'un habitué (x, y = coin de l'image 40x48)
void draw_accessory(Surface *s, int acc, int x, int y)
{
    switch (acc) {
    case ACC_BOW_GLASSES:
        draw_disc(s, x + 15, y + 12, 3, COL_PINK_DARK);
        draw_disc(s, x + 25, y + 12, 3, COL_PINK_DARK);
        draw_rect(s, x + 19, y + 11, 3, 3, COL_PINK);
        draw_rect_outline(s, x + 9, y + 23, 8, 7, COL_INK);
        draw_rect_outline(s, x + 22, y + 23, 8, 7, COL_INK);
        draw_hline(s, x + 17, y + 25, 5, COL_INK);
        break;
    case ACC_CHEF_HAT:
        draw_rect(s, x + 10, y + 9, 21, 6, COL_WHITE);
        draw_hline(s, x + 10, y + 14, 21, COL_GREY);
        draw_disc(s, x + 14, y + 6, 5, COL_WHITE);
        draw_disc(s, x + 20, y + 3, 6, COL_WHITE);
        draw_disc(s, x + 26, y + 6, 5, COL_WHITE);
        break;
    case ACC_BERET:
        draw_disc(s, x + 18, y + 6, 7, COL_LAV_DARK);
        draw_rect(s, x + 9, y + 7, 20, 3, COL_LAV_DARK);
        draw_rect(s, x + 18, y - 2, 2, 3, COL_LAV_DARK);
        draw_disc(s, x + 15, y + 4, 2, COL_LAVENDER);
        break;
    case ACC_SAILOR:   // casquette de marin
        draw_rect(s, x + 9, y + 9, 22, 5, COL_WHITE);
        draw_rect(s, x + 7, y + 13, 26, 2, RGB888(40, 60, 110));
        draw_rect(s, x + 17, y + 10, 6, 3, RGB888(250, 200, 80));
        draw_hline(s, x + 9, y + 9, 22, COL_GREY);
        break;
    case ACC_HEADPHONES:   // casque de DJ
        for (int k = 0; k < 18; k++)
            draw_pixel(s, x + 11 + k, y + 10 - (k < 9 ? k / 3 : (17 - k) / 3), COL_INK);
        draw_rect(s, x + 4, y + 20, 6, 10, COL_PINK_DARK);
        draw_rect(s, x + 30, y + 20, 6, 10, COL_PINK_DARK);
        draw_vline(s, x + 5, y + 21, 7, COL_PINK);
        draw_vline(s, x + 31, y + 21, 7, COL_PINK);
        break;
    case ACC_PRESS_HAT:   // grand chapeau de critique + plume
        draw_ellipse(s, x + 20, y + 13, 19, 3, RGB888(60, 50, 70));
        draw_rect(s, x + 12, y + 2, 16, 11, RGB888(60, 50, 70));
        draw_hline(s, x + 12, y + 10, 16, COL_PINK_DARK);
        draw_line(s, x + 27, y + 2, x + 34, y - 6, COL_WHITE);
        draw_line(s, x + 28, y + 3, x + 35, y - 5, COL_GREY);
        break;
    }
}

static int mood_of(const Customer *c)
{
    if (c->state == CS_LEAVING) {
        if (c->last_score < 0)
            return 3;
        return c->happy ? 0 : 2;
    }
    if (c->state == CS_ARRIVING)
        return 0;
    return day_stress_level(c);
}

// Petits sauts : |sin| sur `hops` sauts pendant la progression t (0..256)
static int hop(int t256, int hops, int height)
{
    static const uint8_t ARC[16] = { 0, 3, 6, 8, 10, 11, 12, 12, 12, 11, 10, 8, 6, 3, 0, 0 };
    int phase = (t256 * hops * 16 / 256) & 15;
    return -ARC[phase] * height / 12;
}

// Les clients des deux premières places entrent par la gauche, la 3e par la droite
static int entry_side(int slot) { return slot == DAY_SLOTS - 1 ? 1 : -1; }

// Décalage de l'animation du client (arrivée, attente, départ)
static void customer_offset(const ShopView *v, int slot, int *dx, int *dy)
{
    const Customer *c = &v->session->day.slots[slot];
    int side = entry_side(slot);
    int far = side < 0 ? -(slot_cx(slot) + 24) : SCREEN_W - slot_cx(slot) + 24;
    *dx = 0;
    *dy = 0;
    switch (c->state) {
    case CS_ARRIVING: {
        // Arrive en sautillant depuis le bord (tâche 1.9)
        int t = tw_progress(c->anim, ARRIVE_FRAMES);
        *dx = tw_lerp(far, 0, ease_out_quad(t));
        *dy = hop(t, 4, 5);
        break;
    }
    case CS_WAITING: {
        int level = day_stress_level(c);
        // Respiration lente, qui s'accélère avec le stress
        int period = level >= 3 ? 4 : level == 2 ? 12 : 30;
        *dy = ((v->frame / period + slot) & 1) ? -1 : 0;
        if (level >= 3)
            *dx = ((v->frame / 3) & 1) ? 1 : -1;
        // Petit sursaut quand le stress monte d'un cran
        if (v->jolt[slot])
            *dy -= v->jolt[slot] > 4 ? 3 : 1;
        // Rebond de joie à l'arrivée au comptoir
        if (c->anim < 10)
            *dy -= (10 - c->anim) / 3;
        break;
    }
    case CS_LEAVING:
        if (c->anim < 16) {
            // Réaction sur place : saut de joie ou trépignement
            int t = c->anim < 8 ? c->anim : 16 - c->anim;
            if (c->happy)
                *dy = -t;
            else
                *dx = (c->anim & 2) ? 2 : -2;
        } else {
            // Repart en marchant vers la sortie (côté opposé si content)
            int t = tw_progress(c->anim - 16, LEAVE_FRAMES - 16);
            int out = c->happy ? -far : far;
            *dx = tw_lerp(0, out, ease_in_quad(t));
            *dy = c->happy ? hop(t, 3, 6) : hop(t, 5, 1);
        }
        break;
    default:
        break;
    }
}

// Pose du client : clignement régulier, bouche qui bouge quand il parle
static int customer_pose(const ShopView *v, int slot)
{
    const Customer *c = &v->session->day.slots[slot];
    bool talking = (c->state == CS_WAITING && c->anim < 45) ||
                   (c->state == CS_LEAVING && c->anim < 28 && c->says);
    if (talking)
        return ((v->frame / 6) & 1) ? POSE_TALK : POSE_NORMAL;
    if (((v->frame + c->seq * 53 + slot * 29) % 160) < 6)
        return POSE_BLINK;
    return POSE_NORMAL;
}

uint32_t shop_slot_hash(const ShopView *v, int slot)
{
    const Day *d = &v->session->day;
    const Customer *c = &d->slots[slot];
    int dx, dy;
    customer_offset(v, slot, &dx, &dy);
    uint32_t h = hash_add(hash_begin(), c->state);
    h = hash_add(h, c->type);
    h = hash_add(h, c->recipe);
    h = hash_add(h, c->seq);
    h = hash_add(h, mood_of(c));
    h = hash_add(h, dx * 100 + dy);
    h = hash_add(h, customer_pose(v, slot));
    h = hash_add(h, d->active == slot);
    h = hash_add(h, c->state == CS_WAITING ? 40 * c->patience / (c->patience_max ? c->patience_max : 1) : -1);
    h = hash_add(h, c->state == CS_WAITING && c->anim < BUBBLE_POP ? c->anim : -1);
    if (c->state == CS_WAITING) {
        int ratio = 256 * c->patience / (c->patience_max ? c->patience_max : 1);
        h = hash_add(h, ratio < 64 ? (int)(v->frame / 8) & 1 : 2);
        h = hash_add(h, d->active == slot ? (int)(v->frame / 24) & 1 : 0);
    }
    h = hash_add(h, v->coin_popup_slot == slot && v->coin_popup_timer > 0 ? v->coin_popup_timer / 3 : -1);
    return h;
}

static void draw_slot(Surface *s, int i)
{
    const Day *d = &V->session->day;
    const Customer *c = &d->slots[i];
    if (c->state == CS_EMPTY)
        return;
    int cx = slot_cx(i);
    int dx, dy;
    customer_offset(V, i, &dx, &dy);

    // Client, caché sous le comptoir
    Rect body = rect_make(cx - 20 + dx, CUSTOMER_Y + dy - 4, 40, 52);
    if (visible(s, body)) {
        Rect saved = s->clip;
        s->clip = rect_intersect(s->clip, rect_make(0, HUD_H, SCREEN_W, COUNTER_Y - HUD_H));
        draw_image(s, customer_image_ex(c->type, mood_of(c), customer_pose(V, i)), cx - 20 + dx,
                   CUSTOMER_Y + dy);
        if (c->special >= 0)
            draw_accessory(s, SPECIALS[c->special].accessory, cx - 20 + dx, CUSTOMER_Y + dy);
        s->clip = saved;
    }
    if (!visible(s, slot_rect(i)))
        return;

    bool active = d->active == i;
    Rect b = rect_make(cx - BUBBLE_W / 2, BUBBLE_Y, BUBBLE_W, BUBBLE_H);

    if (c->state == CS_WAITING && c->anim < BUBBLE_POP) {
        // La bulle grossit avec un léger dépassement avant d'afficher la commande
        int k = ease_out_back(tw_progress(c->anim, BUBBLE_POP));
        int bw = BUBBLE_W * k / 256, bh = BUBBLE_H * k / 256;
        if (bw > 8 && bh > 8)
            ui_bubble(s, rect_make(cx - bw / 2, BUBBLE_Y + (BUBBLE_H - bh) / 2, bw, bh), COL_WHITE,
                      c->special >= 0 ? COL_WARN : COL_INK_SOFT);
    } else if (c->state == CS_WAITING) {
        uint16_t border = c->special >= 0 ? COL_WARN : active ? COL_PINK_DARK : COL_INK_SOFT;
        // Bulle du client sélectionné : flotte doucement
        if (active && ((V->frame / 24) & 1))
            b.y -= 1;
        ui_bubble(s, b, active ? COL_PINK_PALE : COL_WHITE, border);
        if (active)
            draw_round_rect(s, b.x + 1, b.y + 1, b.w - 2, b.h - 2, COL_PINK_PALE, COL_PINK);
        if (c->riddle) {
            text_draw_shadow(s, cx - text_width("???", 2) / 2, b.y + 6, "???", COL_LAV_DARK, COL_LAVENDER, 2);
            text_draw_aligned(s, b.x, b.y + 30, b.w, "Énigme", COL_INK_SOFT, ALIGN_CENTER);
        } else {
            const char *name = RECIPES[c->recipe].name;
            int lines = text_count_lines(b.w - 4, name);
            text_draw_wrapped(s, b.x + 2, b.y + (b.h - lines * FONT_LINE) / 2, b.w - 4, name,
                              COL_INK, ALIGN_CENTER);
        }
        if (c->state == CS_WAITING) {
            int ratio = 256 * c->patience / (c->patience_max ? c->patience_max : 1);
            uint16_t col = ui_patience_color(ratio);
            if (ratio < 64 && ((V->frame / 8) & 1))
                col = COL_WHITE;   // clignote quand il ne reste presque plus de patience
            ui_gauge(s, cx - 20, GAUGE_Y, 40, 6, c->patience, c->patience_max, col);
        }
    } else if (c->state == CS_LEAVING && c->says && c->anim < LEAVE_FRAMES - 4) {
        uint16_t fill = c->happy ? COL_MINT : COL_PINK_PALE;
        ui_bubble(s, b, fill, c->happy ? COL_MINT_DARK : COL_BAD);
        int lines = text_count_lines(b.w - 4, c->says);
        text_draw_wrapped(s, b.x + 2, b.y + (b.h - lines * FONT_LINE) / 2, b.w - 4, c->says,
                          COL_INK, ALIGN_CENTER);
    }

    if (V->coin_popup_slot == i && V->coin_popup_timer > 0 && V->coin_popup > 0) {
        char txt[12];
        snprintf(txt, sizeof(txt), "+%d", V->coin_popup);
        int y = CUSTOMER_Y + 4 - tw_lerp(0, 22, ease_out_quad(tw_progress(60 - V->coin_popup_timer, 40)));
        draw_image(s, &IMG_icons_coin, cx + 6, y);
        text_draw_shadow(s, cx + 23, y + 3, txt, COL_YELLOW, COL_INK, 1);
    }
}

static void draw_hud(Surface *s)
{
    Rect r = rect_make(0, 0, SCREEN_W, HUD_H);
    if (!visible(s, r))
        return;
    const Day *d = &V->session->day;
    char buf[40];
    draw_rect(s, 0, 0, SCREEN_W, HUD_H, COL_PINK);
    draw_hline(s, 0, HUD_H - 1, SCREEN_W, COL_PINK_DARK);
    if (d->endless)
        snprintf(buf, sizeof(buf), "RUSH");
    else
        snprintf(buf, sizeof(buf), "Niv. %d-%d", d->world + 1, d->level + 1);
    text_draw(s, 4, 3, buf, d->endless ? COL_BAD : COL_INK, 1);

    int amb = 256 * d->ambiance / AMBIANCE_MAX;
    // Le cœur bat, de plus en plus vite quand l'ambiance chute
    int beat_period = amb < 77 ? 10 : amb < 140 ? 20 : 40;
    int beat = ((V->frame / beat_period) & 1) ? -1 : 0;
    draw_image(s, &IMG_icons_heart, 54, beat);
    ui_gauge(s, 72, 4, 70, 8, d->ambiance, AMBIANCE_MAX, ui_patience_color(amb));

    uint32_t left = d->endless ? d->frame / FPS : (day_time_left(d) + FPS - 1) / FPS;
    snprintf(buf, sizeof(buf), "%lu:%02lu", (unsigned long)(left / 60), (unsigned long)(left % 60));
    bool hurry = !d->endless && left <= 15;
    text_draw(s, 154, 3 - (hurry && ((V->frame / 15) & 1)), buf, hurry ? COL_BAD : COL_INK, 1);

    draw_image(s, &IMG_icons_coin, 200, 0);
    snprintf(buf, sizeof(buf), "%d", V->coins_shown);
    bool counting = V->coins_shown != d->coins + d->tips;
    text_draw(s, 218, 3 - counting, buf, counting ? COL_PINK_DARK : COL_INK, 1);
}

static void draw_panel(Surface *s)
{
    Rect r = rect_make(PANEL_X, PANEL_Y, PANEL_W, PANEL_H);
    if (!visible(s, r))
        return;
    const Day *d = &V->session->day;
    draw_round_rect(s, r.x, r.y, r.w, r.h, COL_CREAM, COL_WOOD_DARK);
    int x = r.x + 4, w = r.w - 8, y = r.y + 3;
    text_set_reveal(V->panel_reveal);

    if (V->toast_timer > 0) {
        text_draw_shadow(s, x + (w - text_width(V->toast1, 1)) / 2, y + 6, V->toast1,
                         V->toast_color, COL_CREAM_DARK, 1);
        if (V->toast2[0])
            text_draw_wrapped(s, x, y + 22, w, V->toast2, COL_INK, ALIGN_CENTER);
        text_set_reveal(-1);
        return;
    }
    if (d->active < 0) {
        const char *msg = d->ended ? "La boutique ferme…"
                          : day_time_left(d) == 0 ? "Plus de clients, on range !"
                                                  : "En attente de clients…";
        text_draw_aligned(s, x, y + 14, w, msg, COL_INK_SOFT, ALIGN_CENTER);
        if (d->objective.type != OBJ_NONE) {
            char obj[48], line2[72];
            objective_describe(&d->objective, obj, sizeof(obj));
            snprintf(line2, sizeof(line2), "Objectif : %s", obj);
            text_draw_aligned(s, x, y + 30, w, line2, COL_PINK_DARK, ALIGN_CENTER);
        }
        text_set_reveal(-1);
        return;
    }

    const Customer *c = &d->slots[d->active];
    char line[128];
    if (c->special >= 0) {
        // Habitué : son nom, sa réplique, puis la commande si elle est connue
        const SpecialCustomer *sp = &SPECIALS[c->special];
        int step = progress()->special_steps[c->special];
        if (step >= SPECIAL_VISITS)
            step = SPECIAL_VISITS - 1;
        snprintf(line, sizeof(line), "%s (visite %d/%d)", sp->name, step + 1, SPECIAL_VISITS);
        text_draw(s, x, y, line, COL_WOOD_DARK, 1);
        snprintf(line, sizeof(line), "« %s »", sp->visits[step].hello);
        int lines = text_draw_wrapped(s, x, y + FONT_LINE + 2, w, line, COL_INK, ALIGN_LEFT);
        if (!c->riddle && lines < 3) {
            recipe_describe(c->recipe, line, sizeof(line));
            text_draw_wrapped(s, x, y + FONT_LINE * (lines + 1) + 3, w, line, COL_INK_SOFT, ALIGN_LEFT);
        }
        text_set_reveal(-1);
        return;
    }
    if (c->riddle) {
        snprintf(line, sizeof(line), "%s demande… ???", customer_type(c->type)->name);
        text_draw(s, x, y, line, COL_LAV_DARK, 1);
        snprintf(line, sizeof(line), "« %s »", RECIPES[c->recipe].riddle);
        text_draw_wrapped(s, x, y + FONT_LINE + 2, w, line, COL_INK, ALIGN_LEFT);
    } else {
        snprintf(line, sizeof(line), "%s : %s", customer_type(c->type)->name, RECIPES[c->recipe].name);
        text_draw(s, x, y, line, COL_PINK_DARK, 1);
        recipe_describe(c->recipe, line, sizeof(line));
        text_draw_wrapped(s, x, y + FONT_LINE + 2, w, line, COL_INK, ALIGN_LEFT);
    }
    text_set_reveal(-1);
    int id;
    const char *tip = tutorial_step(V, &id);
    if (tip) {
        int lines = text_count_lines(w, tip);
        text_draw_wrapped(s, x, r.y + r.h - lines * FONT_LINE - 3, w, tip, COL_MINT_DARK, ALIGN_LEFT);
    }
}

void shop_draw_top(Surface *s)
{
    decor_shop_draw(s, V->frame);
    draw_hud(s);
    for (int i = 0; i < DAY_SLOTS; i++)
        draw_slot(s, i);
    draw_panel(s);
    fx_draw(SCREEN_TOP, s);
}

// ================================================================ Fonds statiques

// Décors dessinés dans gfx/bg/ (tools/art/backgrounds.py)
static const Image *const SHOP_TOP[WORLD_COUNT] = {
    &IMG_bg_shop_top, &IMG_bg_shop_plage, &IMG_bg_shop_montagne, &IMG_bg_shop_ville, &IMG_bg_shop_foret,
};
static const Image *const STATION[WORLD_COUNT] = {
    &IMG_bg_station, &IMG_bg_station_plage, &IMG_bg_station_montagne, &IMG_bg_station_ville,
    &IMG_bg_station_foret,
};

static void draw_top_background(Surface *s)
{
    draw_image(s, SHOP_TOP[V->session->day.world], 0, 0);
    decor_set_world(V->session->day.world);
}

static void draw_bottom_background(Surface *s)
{
    draw_image(s, STATION[V->session->day.world], 0, 0);
}

void shop_draw_backgrounds(const ShopView *v)
{
    V = v;
    draw_top_background(screen_background(SCREEN_TOP));
    draw_bottom_background(screen_background(SCREEN_BOTTOM));
    screen_dirty_all(SCREEN_TOP);
    screen_dirty_all(SCREEN_BOTTOM);
}
