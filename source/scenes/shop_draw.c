// Rendu de la boutique : comptoir et clients (haut), poste d'assemblage (bas)
#include "scenes/shop_draw.h"

#include <stdio.h>
#include <string.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "core/tween.h"
#include "scenes/fx.h"
#include "scenes/shop_layout.h"
#include "scenes/ui.h"

static const ShopView *V;
bool g_layout_lefty;

static bool visible(const Surface *s, Rect r)
{
    return !rect_empty(rect_intersect(s->clip, r));
}

// ============================================================ Données d'aide

const Image *customer_image(int type, int mood)
{
    static const Image *const IMGS[CT_COUNT][STRESS_LEVELS] = {
        [CT_LAPIN]      = { &IMG_customers_lapin_0, &IMG_customers_lapin_1, &IMG_customers_lapin_2, &IMG_customers_lapin_3 },
        [CT_CHAT]       = { &IMG_customers_chat_0, &IMG_customers_chat_1, &IMG_customers_chat_2, &IMG_customers_chat_3 },
        [CT_OURS]       = { &IMG_customers_ours_0, &IMG_customers_ours_1, &IMG_customers_ours_2, &IMG_customers_ours_3 },
        [CT_GRENOUILLE] = { &IMG_customers_grenouille_0, &IMG_customers_grenouille_1, &IMG_customers_grenouille_2, &IMG_customers_grenouille_3 },
        [CT_POUSSIN]    = { &IMG_customers_poussin_0, &IMG_customers_poussin_1, &IMG_customers_poussin_2, &IMG_customers_poussin_3 },
        [CT_PANDA]      = { &IMG_customers_panda_0, &IMG_customers_panda_1, &IMG_customers_panda_2, &IMG_customers_panda_3 },
    };
    if (type < 0 || type >= CT_COUNT)
        type = 0;
    if (mood < 0)
        mood = 0;
    if (mood >= STRESS_LEVELS)
        mood = STRESS_LEVELS - 1;
    return IMGS[type][mood];
}

static const Image *topping_icon(int ing)
{
    switch (ing) {
    case ING_PEARLS:  return &IMG_icons_pearl;
    case ING_POPPING: return &IMG_icons_popping;
    case ING_JELLY:   return &IMG_icons_jelly;
    case ING_ICE:     return &IMG_icons_ice;
    case ING_CREAM:   return &IMG_icons_cream;
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
    const Drink *k = &v->session->drink;
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

// ================================================================ Gobelet

static int cup_halfwidth(int y, int rim, int bottom)
{
    return CUP_TOP_HW + (CUP_BOT_HW - CUP_TOP_HW) * (y - rim) / (bottom - rim);
}

#define LIQUID_PX (CUP_BOT_Y - CUP_TOP_Y)

void draw_cup(Surface *s, int cx, int dy, const Drink *d, int wobble, int pouring_ing)
{
    cx += wobble;
    int rim = CUP_TOP_Y - 6 + dy, bottom = CUP_BOT_Y + 2 + dy, inner_bot = CUP_BOT_Y + dy;
    int level = d->liquid_total * LIQUID_PX / CUP_CAPACITY;
    int surface_y = inner_bot - level;
    uint16_t mixed = drink_mixed_color(d);

    // Flaque si ça a débordé
    if (d->overflowed) {
        uint16_t c = d->layer_count ? ing_info(d->layers[d->layer_count - 1].ing)->color : mixed;
        draw_rect(s, cx - CUP_BOT_HW - 8, bottom + 1, 2 * CUP_BOT_HW + 17, 3, c);
        draw_rect(s, cx - CUP_BOT_HW - 4, bottom + 4, 2 * CUP_BOT_HW + 9, 2, c);
    }

    // Paille (derrière le couvercle)
    if (d->sealed) {
        draw_rect(s, cx + 6, rim - 22, 5, 30, COL_PINK_DARK);
        for (int y = rim - 20; y < rim + 6; y += 6)
            draw_rect(s, cx + 6, y, 5, 2, COL_WHITE);
    }

    // Verre
    for (int y = rim; y <= bottom; y++) {
        int hw = cup_halfwidth(y, rim, bottom);
        draw_hline(s, cx - hw, y, 2 * hw + 1, COL_GLASS);
        draw_pixel(s, cx - hw, y, COL_GLASS_EDGE);
        draw_pixel(s, cx + hw, y, COL_GLASS_EDGE);
    }
    draw_hline(s, cx - CUP_BOT_HW, bottom, 2 * CUP_BOT_HW + 1, COL_GLASS_EDGE);

    // Liquide, couche par couche depuis le fond
    int layer = 0, acc = d->layer_count ? d->layers[0].amount : 0;
    for (int y = inner_bot; y > surface_y; y--) {
        int units = (inner_bot - y) * CUP_CAPACITY / LIQUID_PX;
        while (layer + 1 < d->layer_count && units >= acc)
            acc += d->layers[++layer].amount;
        uint16_t c = d->shaken ? mixed : ing_info(d->layers[layer].ing)->color;
        int hw = cup_halfwidth(y, rim, bottom) - 1;
        draw_hline(s, cx - hw, y, 2 * hw + 1, c);
        draw_hline(s, cx - hw + 1, y, 2, color_mix(c, COL_WHITE, 120));
    }

    // Sirops : bande colorée au fond (avant mélange)
    int pumps = 0, sr = 0, sg = 0, sb = 0;
    for (int i = SYRUP_FIRST; i <= SYRUP_LAST; i++) {
        uint16_t c = ing_info(i)->color;
        pumps += d->qty[i];
        sr += (c & 31) * d->qty[i];
        sg += ((c >> 5) & 31) * d->qty[i];
        sb += ((c >> 10) & 31) * d->qty[i];
    }
    if (pumps && !d->shaken) {
        uint16_t syrup = RGB(sr / pumps, sg / pumps, sb / pumps);
        int band = pumps * 3 > 18 ? 18 : pumps * 3;
        for (int y = inner_bot; y > inner_bot - band; y--) {
            int hw = cup_halfwidth(y, rim, bottom) - 1;
            uint16_t c = syrup;
            // vagues de sirop sur les parois
            int wave = ((y * 5) & 7) < 3 ? 2 : 0;
            draw_hline(s, cx - hw, y, 3 + wave, c);
            draw_hline(s, cx + hw - 2 - wave, y, 3 + wave, c);
            if (y > inner_bot - band / 2 || y <= surface_y)
                draw_hline(s, cx - hw, y, 2 * hw + 1, c);
        }
    }

    // Garnitures
    int span = 2 * CUP_BOT_HW - 8;
    int pearls = d->qty[ING_PEARLS] * 6;
    for (int k = 0; k < pearls; k++) {
        int x = cx - CUP_BOT_HW + 5 + (k * 7) % span;
        int y = inner_bot - 3 - ((k * 7) / span) * 4 - (k & 1);
        draw_disc(s, x, y, 2, RGB888(74, 44, 42));
        draw_pixel(s, x - 1, y - 1, RGB888(160, 120, 110));
    }
    int base = inner_bot - 4 - (pearls * 7 / span + 1) * 4;
    int pops = d->qty[ING_POPPING] * 4;
    for (int k = 0; k < pops; k++) {
        int x = cx - CUP_BOT_HW + 7 + (k * 11) % (span - 2);
        int y = base - ((k * 11) / (span - 2)) * 5 - (k % 3);
        draw_disc(s, x, y, 2, RGB888(255, 160, 70));
        draw_pixel(s, x - 1, y - 1, COL_WHITE);
    }
    int jelly = d->qty[ING_JELLY] * 4;
    for (int k = 0; k < jelly; k++) {
        int x = cx - CUP_BOT_HW + 6 + (k * 13) % (span - 4);
        int y = base - 8 - ((k * 13) / (span - 4)) * 6 - (k & 1) * 2;
        draw_rect(s, x, y, 4, 4, RGB888(150, 220, 120));
        draw_pixel(s, x, y, COL_WHITE);
    }
    int ice = d->qty[ING_ICE] * 2;
    int ice_y = d->liquid_total ? surface_y + 2 : inner_bot - 8;
    for (int k = 0; k < ice; k++) {
        int x = cx - 18 + (k * 9) % 32;
        int y = ice_y + ((k * 9) / 32) * 7;
        draw_rect(s, x, y, 7, 6, RGB888(215, 240, 255));
        draw_rect_outline(s, x, y, 7, 6, RGB888(150, 195, 230));
        draw_hline(s, x + 1, y + 1, 2, COL_WHITE);
    }
    if (d->qty[ING_CREAM]) {
        int cy = (d->liquid_total ? surface_y : inner_bot) - 1;
        int hw = cup_halfwidth(cy, rim, bottom) - 3;
        int thick = 2 + d->qty[ING_CREAM] * 2;
        for (int k = -hw; k <= hw; k += 5)
            draw_disc(s, cx + k, cy - thick / 2, thick / 2 + 1, RGB888(255, 252, 240));
    }

    // Graduations (tous les 10 %)
    for (int u = 10; u <= 100; u += 10) {
        int y = inner_bot - u * LIQUID_PX / CUP_CAPACITY;
        int hw = cup_halfwidth(y, rim, bottom);
        int len = (u % 50) ? 3 : 6;
        draw_hline(s, cx + hw - len, y, len, COL_GLASS_EDGE);
    }

    // Couvercle scellé
    if (d->sealed) {
        draw_hline(s, cx - CUP_TOP_HW - 1, rim - 1, 2 * CUP_TOP_HW + 3, COL_PINK_DARK);
        draw_hline(s, cx - CUP_TOP_HW, rim, 2 * CUP_TOP_HW + 1, COL_PINK);
        draw_hline(s, cx - CUP_TOP_HW, rim + 1, 2 * CUP_TOP_HW + 1, COL_PINK_PALE);
    }

    // Filet de liquide en cours de versement
    if (pouring_ing > 0) {
        uint16_t c = ing_info(pouring_ing)->color;
        draw_rect(s, cx - 2, CUP_AREA_Y + dy, 4, surface_y - CUP_AREA_Y - dy, c);
        draw_vline(s, cx - 1, CUP_AREA_Y + dy, surface_y - CUP_AREA_Y - dy, color_mix(c, COL_WHITE, 100));
    }
}

uint32_t shop_cup_hash(const ShopView *v)
{
    const Drink *d = &v->session->drink;
    uint32_t h = hash_begin();
    for (int i = 0; i < ING_COUNT; i++)
        h = hash_add(h, d->qty[i]);
    h = hash_add(h, d->sealed | (d->shaken << 1) | (d->overflowed << 2));
    h = hash_add(h, v->wobble);
    h = hash_add(h, v->pouring);
    h = hash_add(h, v->paused);
    return h;
}

// ================================================================ Écran bas

static void draw_tap(Surface *s, int i, bool pressed)
{
    Rect r = tap_rect(i);
    if (!visible(s, r))
        return;
    Ingredient ing = TAP_INGS[i];
    const IngredientInfo *info = ing_info(ing);
    bool locked = info->unlock_day > V->session->day.number;

    uint16_t frame = locked ? COL_GREY_DARK : pressed ? COL_INK : COL_WOOD_DARK;
    draw_round_rect(s, r.x, r.y, r.w, r.h, locked ? COL_GREY : COL_CREAM, frame);
    // Réservoir
    uint16_t tank = locked ? COL_GREY : info->color;
    draw_round_rect(s, r.x + 4, r.y + 3, r.w - 8, 16, tank, color_darken(tank, 6));
    draw_rect(s, r.x + 6, r.y + 5, 2, 10, color_mix(tank, COL_WHITE, 140));
    // Bec
    draw_rect(s, r.x + r.w / 2 - 2, r.y + 19, 4, 3, COL_GREY_DARK);
    if (locked)
        draw_image(s, &IMG_icons_lock, r.x + r.w / 2 - 8, r.y + 3);
    text_draw_aligned(s, r.x, r.y + 22, r.w, info->short_name,
                      locked ? COL_GREY_DARK : pressed ? COL_PINK_DARK : COL_INK, ALIGN_CENTER);
}

static void draw_pump(Surface *s, int i, bool flash)
{
    Rect r = pump_rect(i);
    if (!visible(s, r))
        return;
    const IngredientInfo *info = ing_info(PUMP_INGS[i]);
    bool locked = info->unlock_day > V->session->day.number;
    uint16_t body = locked ? COL_GREY : flash ? color_mix(info->color, COL_WHITE, 120) : info->color;
    // Tête de pompe
    draw_rect(s, r.x + r.w / 2 - 6, r.y + (flash ? 2 : 0), 12, 3, COL_GREY_DARK);
    draw_rect(s, r.x + r.w / 2 - 1, r.y + 3, 2, 3, COL_GREY_DARK);
    draw_round_rect(s, r.x + 2, r.y + 6, r.w - 4, r.h - 6, body, color_darken(body, 8));
    if (locked)
        draw_image(s, &IMG_icons_lock, r.x + r.w / 2 - 8, r.y + 7);
    else
        text_draw_aligned(s, r.x + 2, r.y + 10, r.w - 4, info->short_name,
                          info->color == ing_info(ING_SYRUP_BROWN)->color ? COL_WHITE : COL_INK,
                          ALIGN_CENTER);
}

static void draw_selector(Surface *s)
{
    Rect r = rect_make(SEL_X, SEL_Y, SEL_W, SEL_H);
    if (!visible(s, r))
        return;
    const Day *d = &V->session->day;
    draw_round_rect(s, r.x, r.y, r.w, r.h, COL_LAVENDER, COL_LAV_DARK);
    text_draw(s, r.x + 3, r.y + 7, "<", COL_INK, 1);
    text_draw(s, r.x + r.w - 9, r.y + 7, ">", COL_INK, 1);
    const char *name = d->active >= 0 ? customer_type(d->slots[d->active].type)->name : "-";
    text_draw_aligned(s, r.x + 10, r.y + 2, r.w - 20, "Pour :", COL_INK_SOFT, ALIGN_CENTER);
    text_draw_aligned(s, r.x + 10, r.y + 12, r.w - 20, name, COL_INK, ALIGN_CENTER);
}

static void draw_bin(Surface *s, int i)
{
    Rect r = bin_rect(i);
    if (!visible(s, r))
        return;
    Ingredient ing = BIN_INGS[i];
    const IngredientInfo *info = ing_info(ing);
    bool locked = info->unlock_day > V->session->day.number;
    bool holding = V->held == (int)ing;
    draw_round_rect(s, r.x, r.y, r.w, r.h, locked ? COL_GREY : holding ? COL_PEACH : COL_WOOD_LIGHT,
                    locked ? COL_GREY_DARK : COL_WOOD_DARK);
    draw_image(s, locked ? &IMG_icons_lock : topping_icon(ing), r.x + (r.w - 16) / 2, r.y + 1);
    text_draw_aligned(s, r.x, r.y + 16, r.w, info->short_name, locked ? COL_GREY_DARK : COL_INK,
                      ALIGN_CENTER);
}

uint32_t shop_actions_hash(const ShopView *v)
{
    const Drink *d = &v->session->drink;
    uint32_t h = hash_add(hash_begin(), drink_is_empty(d));
    h = hash_add(h, d->sealed);
    h = hash_add(h, v->session->day.active >= 0);
    return h;
}

static void draw_actions(Surface *s)
{
    const Drink *d = &V->session->drink;
    bool empty = drink_is_empty(d);
    draw_bin(s, 4);   // crème, en haut de la colonne de droite
    ui_button(s, action_rect(ACT_SEAL), d->sealed ? "Fermé" : "Fermer",
              empty || d->sealed ? BTN_STYLE_DISABLED : BTN_STYLE_NORMAL);
    ui_button(s, action_rect(ACT_TRASH), "Jeter", empty ? BTN_STYLE_DISABLED : BTN_STYLE_NORMAL);
    bool can_serve = !empty && V->session->day.active >= 0;
    ui_button(s, action_rect(ACT_SERVE), "Servir", can_serve ? BTN_STYLE_PRIMARY : BTN_STYLE_DISABLED);
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
        int pour = V->pouring >= 0 ? (int)TAP_INGS[V->pouring] : 0;
        draw_cup(s, CUP_CX, 0, &V->session->drink, V->wobble, pour);
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

// Décalage vertical / horizontal de l'animation du client
static void customer_offset(const ShopView *v, int slot, int *dx, int *dy)
{
    const Customer *c = &v->session->day.slots[slot];
    *dx = 0;
    *dy = 0;
    switch (c->state) {
    case CS_ARRIVING:
        // Surgit de derrière le comptoir avec un petit dépassement (tâche 1.9)
        *dy = tw_lerp(48, 0, ease_out_back(tw_progress(c->anim, ARRIVE_FRAMES)));
        break;
    case CS_WAITING: {
        int level = day_stress_level(c);
        int period = level >= 3 ? 4 : level == 2 ? 10 : 24;
        *dy = ((v->frame / period + slot) & 1) ? -1 : 0;
        if (level >= 3)
            *dx = ((v->frame / 3) & 1) ? 1 : -1;
        break;
    }
    case CS_LEAVING:
        if (c->anim < 16) {
            int t = c->anim < 8 ? c->anim : 16 - c->anim;
            if (c->happy)
                *dy = -t / 2;
            else
                *dx = (c->anim & 2) ? 2 : -2;
        } else {
            *dy = tw_lerp(0, 52, ease_in_quad(tw_progress(c->anim - 16, LEAVE_FRAMES - 16)));
        }
        break;
    default:
        break;
    }
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
    h = hash_add(h, d->active == slot);
    h = hash_add(h, c->state == CS_WAITING ? 40 * c->patience / (c->patience_max ? c->patience_max : 1) : -1);
    h = hash_add(h, c->state == CS_ARRIVING ? c->anim > ARRIVE_FRAMES / 2 : 0);
    h = hash_add(h, v->coin_popup_slot == slot && v->coin_popup_timer > 0 ? v->coin_popup_timer / 3 : -1);
    return h;
}

static void draw_slot(Surface *s, int i)
{
    Rect area = slot_rect(i);
    if (!visible(s, area))
        return;
    const Day *d = &V->session->day;
    const Customer *c = &d->slots[i];
    if (c->state == CS_EMPTY)
        return;
    int cx = slot_cx(i);
    int dx, dy;
    customer_offset(V, i, &dx, &dy);

    // Client, caché sous le comptoir
    Rect saved = s->clip;
    s->clip = rect_intersect(s->clip, rect_make(0, HUD_H, SCREEN_W, COUNTER_Y - HUD_H));
    draw_image(s, customer_image(c->type, mood_of(c)), cx - 20 + dx, CUSTOMER_Y + dy);
    s->clip = saved;

    bool active = d->active == i;
    Rect b = rect_make(cx - BUBBLE_W / 2, BUBBLE_Y, BUBBLE_W, BUBBLE_H);

    if (c->state == CS_WAITING || (c->state == CS_ARRIVING && c->anim > ARRIVE_FRAMES / 2)) {
        ui_bubble(s, b, active ? COL_PINK_PALE : COL_WHITE, active ? COL_PINK_DARK : COL_INK_SOFT);
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
            ui_gauge(s, cx - 20, GAUGE_Y, 40, 6, c->patience, c->patience_max, ui_patience_color(ratio));
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
    char buf[24];
    draw_rect(s, 0, 0, SCREEN_W, HUD_H, COL_PINK);
    draw_hline(s, 0, HUD_H - 1, SCREEN_W, COL_PINK_DARK);
    if (d->endless)
        snprintf(buf, sizeof(buf), "RUSH");
    else
        snprintf(buf, sizeof(buf), "Jour %d", d->number);
    text_draw(s, 4, 3, buf, d->endless ? COL_BAD : COL_INK, 1);

    draw_image(s, &IMG_icons_heart, 54, 0);
    int amb = 256 * d->ambiance / AMBIANCE_MAX;
    ui_gauge(s, 72, 4, 70, 8, d->ambiance, AMBIANCE_MAX, ui_patience_color(amb));

    uint32_t left = d->endless ? d->frame / FPS : (day_time_left(d) + FPS - 1) / FPS;
    snprintf(buf, sizeof(buf), "%lu:%02lu", (unsigned long)(left / 60), (unsigned long)(left % 60));
    text_draw(s, 154, 3, buf, !d->endless && left <= 15 ? COL_BAD : COL_INK, 1);

    draw_image(s, &IMG_icons_coin, 200, 0);
    snprintf(buf, sizeof(buf), "%d", d->coins + d->tips);
    text_draw(s, 218, 3, buf, COL_INK, 1);
}

static void draw_panel(Surface *s)
{
    Rect r = rect_make(PANEL_X, PANEL_Y, PANEL_W, PANEL_H);
    if (!visible(s, r))
        return;
    const Day *d = &V->session->day;
    draw_round_rect(s, r.x, r.y, r.w, r.h, COL_CREAM, COL_WOOD_DARK);
    int x = r.x + 4, w = r.w - 8, y = r.y + 3;

    if (V->toast_timer > 0) {
        text_draw_shadow(s, x + (w - text_width(V->toast1, 1)) / 2, y + 6, V->toast1,
                         V->toast_color, COL_CREAM_DARK, 1);
        if (V->toast2[0])
            text_draw_wrapped(s, x, y + 22, w, V->toast2, COL_INK, ALIGN_CENTER);
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
        return;
    }

    const Customer *c = &d->slots[d->active];
    char line[96];
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
    int id;
    const char *tip = tutorial_step(V, &id);
    if (tip) {
        int lines = text_count_lines(w, tip);
        text_draw_wrapped(s, x, r.y + r.h - lines * FONT_LINE - 3, w, tip, COL_MINT_DARK, ALIGN_LEFT);
    }
}

void shop_draw_top(Surface *s)
{
    draw_hud(s);
    for (int i = 0; i < DAY_SLOTS; i++)
        draw_slot(s, i);
    draw_panel(s);
    fx_draw(SCREEN_TOP, s);
}

// ================================================================ Fonds statiques

static void draw_top_background(Surface *s)
{
    // Mur rayé
    draw_fill(s, COL_WALL);
    for (int x = 0; x < SCREEN_W; x += 16)
        draw_rect(s, x, HUD_H, 8, COUNTER_Y - HUD_H, COL_WALL_LINE);
    // Étagère avec bocaux
    draw_rect(s, 8, 40, 240, 3, COL_WOOD_DARK);
    static const uint16_t JARS[] = { COL_PINK, COL_MINT, COL_YELLOW, COL_LAVENDER, COL_PEACH };
    for (int i = 0; i < 12; i++) {
        int x = 14 + i * 20;
        uint16_t c = JARS[i % 5];
        draw_round_rect(s, x, 28, 12, 12, COL_GLASS, COL_GLASS_EDGE);
        draw_rect(s, x + 2, 32, 8, 7, c);
        draw_rect(s, x + 1, 26, 10, 3, COL_WOOD_DARK);
    }
    // Plantes
    for (int k = 0; k < 2; k++) {
        int x = k ? 236 : 12;
        draw_rect(s, x - 6, 104, 13, 20, COL_PEACH);
        draw_rect(s, x - 7, 104, 15, 3, color_darken(COL_PEACH, 4));
        for (int l = 0; l < 5; l++)
            draw_disc(s, x - 6 + l * 3, 96 - (l & 1) * 6, 4, COL_MINT_DARK);
    }
    // Guirlande
    for (int x = 4; x < SCREEN_W; x += 12) {
        int y = 20 + ((x / 12) & 1) * 2;
        draw_disc(s, x, y, 2, JARS[(x / 12) % 5]);
    }
    // Comptoir
    draw_rect(s, 0, COUNTER_Y, SCREEN_W, SCREEN_H - COUNTER_Y, COL_WOOD);
    draw_rect(s, 0, COUNTER_Y, SCREEN_W, 4, COL_WOOD_LIGHT);
    draw_hline(s, 0, COUNTER_Y + 4, SCREEN_W, COL_WOOD_DARK);
    for (int y = COUNTER_Y + 12; y < SCREEN_H; y += 10)
        draw_hline(s, 0, y, SCREEN_W, color_mix(COL_WOOD, COL_WOOD_DARK, 80));
}

static void draw_bottom_background(Surface *s)
{
    ui_polka_background(s, COL_MINT, color_mix(COL_MINT, COL_WHITE, 120));
    // Panneau des tireuses
    draw_rect(s, 0, 0, SCREEN_W, PUMP_Y + PUMP_H + 3, COL_WOOD_LIGHT);
    draw_hline(s, 0, PUMP_Y + PUMP_H + 3, SCREEN_W, COL_WOOD_DARK);
    draw_hline(s, 0, PUMP_Y - 2, SCREEN_W, color_mix(COL_WOOD_LIGHT, COL_WOOD_DARK, 100));
    // Sous-verre du gobelet
    draw_rect(s, CUP_CX - 34, CUP_BOT_Y + 3, 69, 6, COL_MINT_DARK);
    draw_rect(s, CUP_CX - 30, CUP_BOT_Y + 9, 61, 2, color_darken(COL_MINT_DARK, 4));
}

void shop_draw_backgrounds(const ShopView *v)
{
    V = v;
    draw_top_background(screen_background(SCREEN_TOP));
    draw_bottom_background(screen_background(SCREEN_BOTTOM));
    screen_dirty_all(SCREEN_TOP);
    screen_dirty_all(SCREEN_BOTTOM);
}
