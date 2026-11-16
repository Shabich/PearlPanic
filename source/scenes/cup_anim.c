#include "scenes/cup_anim.h"

#include <string.h>

#include "core/rng.h"
#include "gfx/palette.h"
#include "scenes/shop_layout.h"

#define SUB         16
#define LIQUID_PX   (CUP_BOT_Y - CUP_TOP_Y)
#define SPAWN_Y     (CUP_TOP_Y - 18)

// Nombre d'objets dessinés par cuillerée
#define PER_PEARL   6
#define PER_POP     4
#define PER_JELLY   4
#define PER_ICE     2

static Rng g_rng = { 0xA5A5F00Du };

// Sinus sur 64 pas, amplitude 64
static const int8_t SIN64[64] = {
    0, 6, 12, 19, 24, 30, 35, 40, 45, 49, 53, 56, 59, 61, 62, 64,
    64, 64, 62, 61, 59, 56, 53, 49, 45, 40, 35, 30, 24, 19, 12, 6,
    0, -6, -12, -19, -24, -30, -35, -40, -45, -49, -53, -56, -59, -61, -62, -64,
    -64, -64, -62, -61, -59, -56, -53, -49, -45, -40, -35, -30, -24, -19, -12, -6,
};
static inline int sin64(int i) { return SIN64[i & 63]; }

static int halfwidth(int y, int rim, int bottom)
{
    return CUP_TOP_HW + (CUP_BOT_HW - CUP_TOP_HW) * (y - rim) / (bottom - rim);
}

int cup_surface_y(const Drink *d, int dy)
{
    return CUP_BOT_Y + dy - d->liquid_total * LIQUID_PX / CUP_CAPACITY;
}

// ------------------------------------------------------------ Positions de repos

#define SPAN (2 * CUP_BOT_HW - 8)

static int pearl_rows(const Drink *d)
{
    int n = d->qty[ING_PEARLS] * PER_PEARL;
    return n ? (n * 7) / SPAN + 1 : 0;
}

static void rest_pos(const Drink *d, int type, int k, int *x, int *y)
{
    int bot = CUP_BOT_Y;
    int base = bot - 4 - pearl_rows(d) * 4;
    switch (type) {
    case ITEM_PEARL:
        *x = -CUP_BOT_HW + 5 + (k * 7) % SPAN;
        *y = bot - 3 - ((k * 7) / SPAN) * 4 - (k & 1);
        break;
    case ITEM_POP:
        *x = -CUP_BOT_HW + 7 + (k * 11) % (SPAN - 2);
        *y = base - ((k * 11) / (SPAN - 2)) * 5 - (k % 3);
        break;
    case ITEM_JELLY:
        *x = -CUP_BOT_HW + 6 + (k * 13) % (SPAN - 4);
        *y = base - 8 - ((k * 13) / (SPAN - 4)) * 6 - (k & 1) * 2;
        break;
    default: {  // glaçons : flottent sous la surface, ou au fond sans liquide
        int surf = cup_surface_y(d, 0);
        *x = -18 + (k * 9) % 32;
        *y = d->liquid_total ? surf + 2 + ((k * 9) / 32) * 6 : bot - 8 - ((k * 9) / 32) * 6;
        break;
    }
    }
}

// ------------------------------------------------------------------ Mise à jour

void cup_anim_reset(CupAnim *a)
{
    memset(a, 0, sizeof(*a));
    a->slosh = 16;
}

static CupItem *add_item(CupAnim *a)
{
    if (a->count >= CUP_ITEMS) {
        // On recycle une bulle si possible
        for (int i = 0; i < a->count; i++)
            if (a->items[i].type == ITEM_BUBBLE)
                return &a->items[i];
        return NULL;
    }
    return &a->items[a->count++];
}

static void spawn_toppings(CupAnim *a, const Drink *d, Ingredient ing, int type, int per)
{
    int target = d->qty[ing] * per;
    while (a->spawned[ing] < target) {
        CupItem *it = add_item(a);
        if (!it)
            return;
        int rx, ry;
        rest_pos(d, type, a->spawned[ing], &rx, &ry);
        it->type = (uint8_t)type;
        it->index = a->spawned[ing];
        it->resting = false;
        it->x = (int16_t)(rx + rng_range(&g_rng, 5) - 2);
        // Les objets d'une même cuillerée tombent légèrement décalés
        it->y = (SPAWN_Y - (a->spawned[ing] % per) * 5) * SUB;
        it->vy = (int16_t)rng_range(&g_rng, 8);
        it->life = 0;
        a->spawned[ing]++;
    }
}

void cup_anim_update(CupAnim *a, const Drink *d, bool pouring, int wobble, uint32_t frame)
{
    if (drink_is_empty(d) && (a->count || a->seal_t)) {
        cup_anim_reset(a);
        return;
    }

    spawn_toppings(a, d, ING_PEARLS, ITEM_PEARL, PER_PEARL);
    spawn_toppings(a, d, ING_POPPING, ITEM_POP, PER_POP);
    spawn_toppings(a, d, ING_JELLY, ITEM_JELLY, PER_JELLY);
    spawn_toppings(a, d, ING_ICE, ITEM_ICE, PER_ICE);

    int surf = cup_surface_y(d, 0) * SUB;
    bool has_liquid = d->liquid_total > 2;

    // Bulles d'air sous le filet
    if (pouring && has_liquid && (frame % 3) == 0) {
        CupItem *b = add_item(a);
        if (b) {
            b->type = ITEM_BUBBLE;
            b->x = (int16_t)(rng_range(&g_rng, 13) - 6);
            int depth = d->liquid_total * LIQUID_PX / CUP_CAPACITY;
            b->y = surf + (2 + rng_range(&g_rng, depth > 4 ? depth - 2 : 2)) * SUB;
            b->vy = (int16_t)(-6 - rng_range(&g_rng, 6));
            b->life = 40;
            b->resting = false;
        }
    }

    // Vagues : le versement et les secousses agitent la surface
    int dv = wobble - a->prev_wobble;
    a->prev_wobble = wobble;
    if (pouring && a->slosh < 34)
        a->slosh = 34;
    a->slosh += (dv < 0 ? -dv : dv) * 6;
    if (a->slosh > 64)
        a->slosh = 64;
    if (a->slosh > 16)
        a->slosh = (int16_t)(16 + (a->slosh - 16) * 15 / 16);
    int target_tilt = -wobble;
    a->tilt = (int8_t)(a->tilt + (target_tilt - a->tilt) / 3);

    // Physique des objets
    int n = 0;
    for (int i = 0; i < a->count; i++) {
        CupItem *it = &a->items[i];
        int rx, ry;
        if (it->type == ITEM_BUBBLE) {
            it->y += it->vy;
            it->x += (frame + i) & 4 ? 0 : (i & 1 ? 1 : -1) * ((frame & 7) == 0);
            if (--it->life == 0 || it->y <= surf)
                continue;   // éclate à la surface
            a->items[n++] = *it;
            continue;
        }
        rest_pos(d, it->type, it->index, &rx, &ry);
        int rest = ry * SUB;
        bool in_liquid = has_liquid && it->y > surf;

        if (it->type == ITEM_ICE && has_liquid) {
            // Flotte : rejoint doucement sa ligne de flottaison et y danse
            int bob = sin64((int)(frame * 2 + it->index * 23)) / 40;
            int target = rest + bob * SUB;
            if (!in_liquid) {
                it->vy += 5;
                it->y += it->vy;
                if (it->y > surf && it->vy > 0)
                    it->vy = (int16_t)(-it->vy / 3);   // plouf
            } else {
                it->vy = (int16_t)(it->vy * 3 / 4 + (target - it->y) / 6);
                it->y += it->vy;
            }
            int drift = sin64((int)(frame / 2 + it->index * 17)) / 22;
            it->x = (int16_t)(rx + drift);
        } else if (!it->resting) {
            if (in_liquid) {
                // Coule lentement dans le liquide
                it->vy = (int16_t)(it->vy > 12 ? it->vy - 4 : it->vy + 1);
            } else {
                it->vy += 6;
                if (it->vy > 72)
                    it->vy = 72;
            }
            it->y += it->vy;
            if (it->y >= rest) {
                it->y = rest;
                if (it->type == ITEM_POP && it->vy > 10) {
                    it->vy = (int16_t)(-it->vy / 2);   // rebondit
                } else {
                    it->resting = true;
                    it->vy = 0;
                }
            }
            it->x += (it->x < rx) - (it->x > rx);
        } else {
            it->y = rest;   // suit les changements de pile
            it->x = (int16_t)rx;
        }
        a->items[n++] = *it;
    }
    a->count = n;

    // Mélange après avoir secoué, couvercle, coulures
    if (d->shaken && !a->was_shaken)
        a->mix = 0;
    if (d->shaken && a->mix < 32)
        a->mix++;
    a->was_shaken = d->shaken;
    if (d->sealed) {
        if (!a->was_sealed)
            a->seal_t = 0;
        if (a->seal_t < 60)
            a->seal_t++;
    } else {
        a->seal_t = 0;
    }
    a->was_sealed = d->sealed;
    if (d->overflowed)
        a->drip_t++;
}

bool cup_anim_busy(const CupAnim *a, const Drink *d)
{
    if (a->slosh > 17 || a->tilt || (d->shaken && a->mix < 32) || (d->sealed && a->seal_t < 40))
        return true;
    for (int i = 0; i < a->count; i++)
        if (!a->items[i].resting)
            return true;
    return false;
}

// ------------------------------------------------------------------- Dessin

static uint16_t layer_color_at(const Drink *d, int units, uint16_t mixed, int mix)
{
    int layer = 0, acc = d->layer_count ? d->layers[0].amount : 0;
    while (layer + 1 < d->layer_count && units >= acc)
        acc += d->layers[++layer].amount;
    uint16_t c = ing_info(d->layers[layer].ing)->color;
    if (d->shaken)
        c = color_mix(c, mixed, mix * 8);
    return c;
}

void draw_cup(Surface *s, int cx, int dy, const Drink *d, const CupAnim *a, uint32_t frame,
              int wobble, int pour_ing)
{
    cx += wobble;
    int rim = CUP_TOP_Y - 6 + dy, bottom = CUP_BOT_Y + 2 + dy, inner_bot = CUP_BOT_Y + dy;
    int level = d->liquid_total * LIQUID_PX / CUP_CAPACITY;
    int surface = inner_bot - level;
    uint16_t mixed = drink_mixed_color(d);
    int mix = a ? a->mix : 32;
    int slosh = a ? a->slosh : 0;
    int tilt = a ? a->tilt : 0;
    int seal_t = a ? a->seal_t : 60;
    uint16_t top_color = d->layer_count ? ing_info(d->layers[d->layer_count - 1].ing)->color : mixed;
    if (d->shaken)
        top_color = color_mix(top_color, mixed, mix * 8);

    // Flaque et coulures si ça a débordé
    if (d->overflowed) {
        draw_rect(s, cx - CUP_BOT_HW - 8, bottom + 1, 2 * CUP_BOT_HW + 17, 3, top_color);
        draw_rect(s, cx - CUP_BOT_HW - 4, bottom + 4, 2 * CUP_BOT_HW + 9, 2, top_color);
        int t = a ? a->drip_t : 0;
        for (int side = -1; side <= 1; side += 2) {
            int len = (t + (side > 0 ? 17 : 0)) % 48;
            for (int y = rim; y < rim + len && y < bottom; y++) {
                int hw = halfwidth(y, rim, bottom) + 1;
                draw_pixel(s, cx + side * hw, y, top_color);
            }
            if (len > 4)
                draw_rect(s, cx + side * (halfwidth(rim + len, rim, bottom) + 1) - 1, rim + len, 2, 2, top_color);
        }
    }

    // Paille (derrière), qui glisse dans le gobelet après la fermeture
    if (d->sealed && seal_t > 6) {
        int slide = seal_t < 22 ? (22 - seal_t) * 3 : 0;
        int top = rim - 22 - slide;
        draw_rect(s, cx + 6, top, 5, 30, COL_PINK_DARK);
        for (int y = top + 2; y < top + 28; y += 6)
            draw_rect(s, cx + 6, y, 5, 2, COL_WHITE);
    }

    // Verre
    for (int y = rim; y <= bottom; y++) {
        int hw = halfwidth(y, rim, bottom);
        draw_hline(s, cx - hw, y, 2 * hw + 1, COL_GLASS);
    }

    // Liquide colonne par colonne : surface ondulée et inclinée
    if (d->liquid_total) {
        static uint16_t row_color[CUP_BOT_Y - CUP_TOP_Y + 8];
        for (int y = 0; y <= level + 4 && y < (int)(sizeof(row_color) / 2); y++) {
            int units = y * CUP_CAPACITY / LIQUID_PX;
            row_color[y] = layer_color_at(d, units, mixed, mix);
        }
        uint16_t foam = color_mix(top_color, COL_WHITE, d->shaken ? 150 : 90);
        for (int x = cx - CUP_TOP_HW + 1; x <= cx + CUP_TOP_HW - 1; x++) {
            int rel = x - cx;
            int wave = (sin64(rel * 5 + (int)frame * 3) * slosh) / (64 * SUB)
                     + (sin64(rel * 3 - (int)frame * 2) * slosh) / (64 * SUB * 2);
            int sy = surface + wave + rel * tilt / 24;
            if (sy < rim + 1)
                sy = rim + 1;
            for (int y = sy; y <= inner_bot; y++) {
                int hw = halfwidth(y, rim, bottom) - 1;
                if (rel < -hw || rel > hw)
                    continue;
                int from_bottom = inner_bot - y;
                if (from_bottom < 0)
                    from_bottom = 0;
                if (from_bottom >= (int)(sizeof(row_color) / 2))
                    from_bottom = sizeof(row_color) / 2 - 1;
                uint16_t c = row_color[from_bottom];
                if (y == sy || (d->shaken && y <= sy + 2))
                    c = foam;
                else if (rel <= -hw + 1)
                    c = color_mix(c, COL_WHITE, 110);
                draw_pixel(s, x, y, c);
            }
        }
    }

    // Sirops : bande colorée au fond, qui disparaît avec le mélange
    int pumps = 0, sr = 0, sg = 0, sb = 0;
    for (int i = SYRUP_FIRST; i <= SYRUP_LAST; i++) {
        uint16_t c = ing_info(i)->color;
        pumps += d->qty[i];
        sr += (c & 31) * d->qty[i];
        sg += ((c >> 5) & 31) * d->qty[i];
        sb += ((c >> 10) & 31) * d->qty[i];
    }
    if (pumps && (!d->shaken || mix < 32)) {
        uint16_t syrup = RGB(sr / pumps, sg / pumps, sb / pumps);
        int band = pumps * 3 > 18 ? 18 : pumps * 3;
        if (d->shaken)
            band = band * (32 - mix) / 32;
        for (int y = inner_bot; y > inner_bot - band; y--) {
            int hw = halfwidth(y, rim, bottom) - 1;
            // Le sirop « coule » le long des parois en vaguelettes
            int wave = ((y * 5 + (int)frame / 8) & 7) < 3 ? 2 : 0;
            draw_hline(s, cx - hw, y, 3 + wave, syrup);
            draw_hline(s, cx + hw - 2 - wave, y, 3 + wave, syrup);
            if (y > inner_bot - band / 2 || y <= surface)
                draw_hline(s, cx - hw, y, 2 * hw + 1, syrup);
        }
    }

    // Garnitures animées (ou positions de repos sans état d'animation)
    if (a) {
        for (int i = 0; i < a->count; i++) {
            const CupItem *it = &a->items[i];
            int x = cx + it->x, y = it->y / SUB + dy;
            // Tourbillon juste après avoir secoué
            if (d->shaken && a->mix < 32 && it->type != ITEM_BUBBLE)
                x += sin64((int)frame * 4 + it->index * 9) * (32 - a->mix) / (64 * 4);
            switch (it->type) {
            case ITEM_PEARL:
                draw_disc(s, x, y, 2, RGB888(74, 44, 42));
                draw_pixel(s, x - 1, y - 1, RGB888(160, 120, 110));
                break;
            case ITEM_POP:
                draw_disc(s, x, y, 2, RGB888(255, 160, 70));
                draw_pixel(s, x - 1, y - 1, COL_WHITE);
                break;
            case ITEM_JELLY: {
                // Gelée qui tremblote
                bool squish = ((frame / 10) + it->index) & 1;
                draw_rect(s, x, y + squish, squish ? 5 : 4, squish ? 3 : 4, RGB888(150, 220, 120));
                draw_pixel(s, x, y + squish, COL_WHITE);
                break;
            }
            case ITEM_ICE: {
                uint16_t under = d->liquid_total ? top_color : COL_GLASS;
                uint16_t body = color_mix(under, COL_WHITE, 170);
                draw_rect(s, x, y, 7, 6, body);
                draw_rect_outline(s, x, y, 7, 6, color_mix(body, RGB888(120, 170, 220), 140));
                draw_hline(s, x + 1, y + 1, 2, COL_WHITE);
                draw_pixel(s, x + 1, y + 2, COL_WHITE);
                break;
            }
            case ITEM_BUBBLE:
                draw_pixel(s, x, y, color_mix(top_color, COL_WHITE, 200));
                if (it->life & 8)
                    draw_pixel(s, x + 1, y, color_mix(top_color, COL_WHITE, 140));
                break;
            }
        }
    }

    // Crème : petit nuage qui suit les vagues
    if (d->qty[ING_CREAM]) {
        int cy = (d->liquid_total ? surface : inner_bot) - 1;
        int hw = halfwidth(cy, rim, bottom) - 3;
        int thick = 2 + d->qty[ING_CREAM] * 2;
        for (int k = -hw; k <= hw; k += 5) {
            int wave = (sin64(k * 5 + (int)frame * 3) * slosh) / (64 * SUB);
            draw_disc(s, cx + k, cy - thick / 2 + wave, thick / 2 + 1, RGB888(255, 252, 240));
        }
        draw_pixel(s, cx - hw / 2, cy - thick, COL_WHITE);
    }

    // Bords du verre, reflet, graduations
    for (int y = rim; y <= bottom; y++) {
        int hw = halfwidth(y, rim, bottom);
        draw_pixel(s, cx - hw, y, COL_GLASS_EDGE);
        draw_pixel(s, cx + hw, y, COL_GLASS_EDGE);
        if (y > rim + 4 && y < bottom - 12 && ((y / 3) & 3) != 3)
            draw_pixel(s, cx + hw - 4, y, color_mix(COL_GLASS, COL_WHITE, 200));
    }
    draw_hline(s, cx - CUP_BOT_HW, bottom, 2 * CUP_BOT_HW + 1, COL_GLASS_EDGE);
    for (int u = 10; u <= 100; u += 10) {
        int y = inner_bot - u * LIQUID_PX / CUP_CAPACITY;
        int hw = halfwidth(y, rim, bottom);
        int len = (u % 50) ? 3 : 6;
        draw_hline(s, cx + hw - len, y, len, COL_GLASS_EDGE);
    }

    // Couvercle qui descend se poser, puis un éclat de lumière le parcourt
    if (d->sealed) {
        int drop = seal_t < 8 ? (8 - seal_t) * 2 : 0;
        int ly = rim - 1 - drop;
        draw_hline(s, cx - CUP_TOP_HW - 1, ly, 2 * CUP_TOP_HW + 3, COL_PINK_DARK);
        draw_hline(s, cx - CUP_TOP_HW, ly + 1, 2 * CUP_TOP_HW + 1, COL_PINK);
        draw_hline(s, cx - CUP_TOP_HW, ly + 2, 2 * CUP_TOP_HW + 1, COL_PINK_PALE);
        if (seal_t >= 22 && seal_t < 40) {
            int sx = cx - CUP_TOP_HW + (seal_t - 22) * (2 * CUP_TOP_HW) / 18;
            draw_rect(s, sx, ly + 1, 3, 2, COL_WHITE);
        }
    }

    // Filet de liquide : ondule, des reflets descendent, éclaboussures en surface
    if (pour_ing > 0) {
        uint16_t c = ing_info(pour_ing)->color;
        uint16_t light = color_mix(c, COL_WHITE, 120);
        int sx = cx - 1 + (sin64((int)frame * 5) > 20 ? 1 : 0);
        int top = CUP_AREA_Y + dy;
        int end = d->sealed ? rim - 3 : surface;
        for (int y = top; y < end; y++) {
            int w = y < top + 6 ? 2 : 3;   // s'élargit en tombant
            draw_hline(s, sx - (w > 2), y, w, c);
            if (((y - (int)frame * 3) % 7 + 7) % 7 == 0)
                draw_pixel(s, sx, y, light);
        }
        if (!d->sealed && d->liquid_total) {
            int f = (int)frame & 3;
            draw_pixel(s, sx - 3 - f / 2, surface - 1 - (f & 1), light);
            draw_pixel(s, sx + 3 + f / 2, surface - 1 - ((f + 1) & 1), light);
            draw_hline(s, sx - 2, surface, 5, light);
        }
    }
}
