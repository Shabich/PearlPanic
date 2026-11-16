#include "scenes/cup_anim.h"

#include <string.h>

#include "core/rng.h"
#include "gfx/palette.h"
#include "scenes/shop_layout.h"

#define SUB         16
#define LIQUID_PX   (CUP_BOT_Y - CUP_TOP_Y)
#define SPAWN_Y     (CUP_TOP_Y - 18)


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

static int style_of(int ing) { return ing_info(ing)->style; }

// Rang d'un objet parmi tous ceux qui partagent le même comportement
// (les billes et cubes s'empilent ensemble au fond, les flotteurs ensemble en surface)
static int rank(const Drink *d, int ing, int index)
{
    int style = style_of(ing);
    bool sinks = style == TOP_SINK_BALL || style == TOP_SINK_CUBE;
    int k = 0;
    for (int j = TOPPING_FIRST; j < ing; j++) {
        int sj = style_of(j);
        bool same = sinks ? (sj == TOP_SINK_BALL || sj == TOP_SINK_CUBE) : sj == style;
        if (same)
            k += d->qty[j] * ing_info(j)->per_scoop;
    }
    return k + index;
}

static void rest_pos(const Drink *d, int ing, int index, int *x, int *y)
{
    int bot = CUP_BOT_Y;
    int surf = cup_surface_y(d, 0);
    int k = rank(d, ing, index);
    switch (style_of(ing)) {
    case TOP_SINK_BALL:
    case TOP_SINK_CUBE:
        *x = -CUP_BOT_HW + 5 + (k * 7) % SPAN;
        *y = bot - 3 - ((k * 7) / SPAN) * 4 - (k & 1);
        break;
    case TOP_FLOAT_FLAT:
        *x = -20 + (k * 11) % 40;
        *y = d->liquid_total ? surf - 1 : bot - 3;
        break;
    default:   // flotteurs (glaçons, guimauves)
        *x = -18 + (k * 9) % 32;
        *y = d->liquid_total ? surf + 2 + ((k * 9) / 32) * 6 : bot - 8 - ((k * 9) / 32) * 6;
        break;
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
            if (a->items[i].ing == ITEM_BUBBLE)
                return &a->items[i];
        return NULL;
    }
    return &a->items[a->count++];
}

static void spawn_toppings(CupAnim *a, const Drink *d, Ingredient ing)
{
    const IngredientInfo *info = ing_info(ing);
    if (info->style == TOP_LAYER || info->style == TOP_NONE)
        return;
    int per = info->per_scoop;
    int target = d->qty[ing] * per;
    while (a->spawned[ing] < target) {
        CupItem *it = add_item(a);
        if (!it)
            return;
        int rx, ry;
        rest_pos(d, ing, a->spawned[ing], &rx, &ry);
        it->ing = (uint8_t)ing;
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

    for (int ing = TOPPING_FIRST; ing <= TOPPING_LAST; ing++)
        spawn_toppings(a, d, ing);

    int surf = cup_surface_y(d, 0) * SUB;
    bool has_liquid = d->liquid_total > 2;

    // Bulles d'air sous le filet
    if (pouring && has_liquid && (frame % 3) == 0) {
        CupItem *b = add_item(a);
        if (b) {
            b->ing = ITEM_BUBBLE;
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
        if (it->ing == ITEM_BUBBLE) {
            it->y += it->vy;
            it->x += (frame + i) & 4 ? 0 : (i & 1 ? 1 : -1) * ((frame & 7) == 0);
            if (--it->life == 0 || it->y <= surf)
                continue;   // éclate à la surface
            a->items[n++] = *it;
            continue;
        }
        rest_pos(d, it->ing, it->index, &rx, &ry);
        int rest = ry * SUB;
        bool in_liquid = has_liquid && it->y > surf;
        int style = style_of(it->ing);

        if ((style == TOP_FLOAT_CUBE || style == TOP_FLOAT_FLAT) && has_liquid) {
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
            int drift = sin64((int)(frame / 2 + it->index * 17)) / (style == TOP_FLOAT_FLAT ? 12 : 22);
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
                if (it->ing == ING_POPPING && it->vy > 10) {
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

    // Garnitures animées
    if (a) {
        for (int i = 0; i < a->count; i++) {
            const CupItem *it = &a->items[i];
            int x = cx + it->x, y = it->y / SUB + dy;
            if (it->ing == ITEM_BUBBLE) {
                draw_pixel(s, x, y, color_mix(top_color, COL_WHITE, 200));
                if (it->life & 8)
                    draw_pixel(s, x + 1, y, color_mix(top_color, COL_WHITE, 140));
                continue;
            }
            // Tourbillon juste après avoir secoué
            if (d->shaken && a->mix < 32)
                x += sin64((int)frame * 4 + it->index * 9) * (32 - a->mix) / (64 * 4);
            const IngredientInfo *info = ing_info(it->ing);
            uint16_t col = info->color;
            uint16_t light = color_mix(col, COL_WHITE, 150);
            switch (info->style) {
            case TOP_SINK_BALL:
                draw_disc(s, x, y, 2, col);
                draw_pixel(s, x - 1, y - 1, it->ing == ING_PEARLS ? RGB888(160, 120, 110) : COL_WHITE);
                break;
            case TOP_SINK_CUBE: {
                // Tremblote ; la gelée fluo clignote doucement
                bool squish = ((frame / 10) + it->index) & 1;
                if (it->ing == ING_GLOW_JELLY && ((frame / 6 + it->index) & 3) == 0)
                    col = light;
                if (it->ing == ING_PUDDING) {
                    draw_rect(s, x - 1, y - 1 + squish, 6, 4, col);
                    draw_hline(s, x - 1, y - 1 + squish, 6, RGB888(170, 100, 50));
                } else {
                    draw_rect(s, x, y + squish, squish ? 5 : 4, squish ? 3 : 4, col);
                }
                draw_pixel(s, x, y + squish, COL_WHITE);
                break;
            }
            case TOP_FLOAT_CUBE: {
                if (it->ing == ING_ICE) {
                    uint16_t under = d->liquid_total ? top_color : COL_GLASS;
                    uint16_t body = color_mix(under, COL_WHITE, 170);
                    draw_rect(s, x, y, 7, 6, body);
                    draw_rect_outline(s, x, y, 7, 6, color_mix(body, RGB888(120, 170, 220), 140));
                    draw_hline(s, x + 1, y + 1, 2, COL_WHITE);
                    draw_pixel(s, x + 1, y + 2, COL_WHITE);
                } else {   // guimauves : petits cylindres moelleux
                    draw_rect(s, x, y, 6, 5, col);
                    draw_hline(s, x, y, 6, COL_WHITE);
                    draw_vline(s, x + 5, y + 1, 4, color_darken(col, 4));
                }
                break;
            }
            case TOP_FLOAT_FLAT: {
                // Pétales qui tournent doucement en dérivant
                int r = ((frame / 12) + it->index) & 3;
                draw_hline(s, x, y, r & 1 ? 2 : 3, col);
                draw_hline(s, x + (r >> 1), y - 1, 2, light);
                break;
            }
            default:
                break;
            }
        }
    }

    // Crèmes : petit nuage qui suit les vagues (couleur de la crème choisie)
    int cream_ing = 0, cream_qty = 0;
    for (int i = TOPPING_FIRST; i <= TOPPING_LAST; i++)
        if (ing_info(i)->style == TOP_LAYER && d->qty[i]) {
            cream_ing = i;
            cream_qty += d->qty[i];
        }
    // Crème : petit nuage qui suit les vagues
    if (cream_qty) {
        uint16_t cream = ing_info(cream_ing)->color;
        int cy = (d->liquid_total ? surface : inner_bot) - 1;
        int hw = halfwidth(cy, rim, bottom) - 3;
        int thick = 2 + cream_qty * 2;
        for (int k = -hw; k <= hw; k += 5) {
            int wave = (sin64(k * 5 + (int)frame * 3) * slosh) / (64 * SUB);
            draw_disc(s, cx + k, cy - thick / 2 + wave, thick / 2 + 1, cream);
        }
        draw_pixel(s, cx - hw / 2, cy - thick, COL_WHITE);
        if (cream_ing == ING_SEA_CREAM)   // grains de sel
            for (int k = -hw + 2; k < hw; k += 7)
                draw_pixel(s, cx + k, cy - thick / 2 - 1, COL_WHITE);
    }

    // Vapeur au-dessus des boissons chaudes
    bool hot = false;
    for (int i = LIQUID_FIRST; i <= LIQUID_LAST; i++)
        hot |= d->qty[i] && ing_info(i)->hot;
    if (hot && a) {
        uint16_t steam = RGB888(240, 236, 244);
        for (int k = 0; k < 3; k++) {
            int base = cx - 10 + k * 10;
            for (int j = 0; j < 12; j++) {
                int t = (int)frame + k * 20;
                int yy = rim - 4 - ((j * 2 + t / 2) % 24);
                int xx = base + sin64(yy * 6 + t * 2) / 32;
                if ((j + k) % 3 != 0)
                    draw_pixel(s, xx, yy, steam);
            }
        }
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
