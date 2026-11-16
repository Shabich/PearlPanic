#include "game/drink.h"

#include <string.h>

#include "gfx/canvas.h"

void drink_reset(Drink *d)
{
    memset(d, 0, sizeof(*d));
}

bool drink_is_empty(const Drink *d)
{
    for (int i = 1; i < ING_COUNT; i++)
        if (d->qty[i])
            return false;
    return true;
}

int drink_pour(Drink *d, Ingredient ing, int units)
{
    if (d->sealed || units <= 0 || ing_info(ing)->kind != KIND_LIQUID)
        return 0;
    int room = CUP_CAPACITY - d->liquid_total;
    int added = units < room ? units : room;
    if (units > room)
        d->overflowed = true;
    if (added <= 0)
        return 0;

    d->qty[ing] += added;
    d->liquid_total += added;

    Layer *top = d->layer_count ? &d->layers[d->layer_count - 1] : NULL;
    if (top && top->ing == ing) {
        top->amount += added;
    } else if (d->layer_count < DRINK_MAX_LAYERS) {
        d->layers[d->layer_count].ing = ing;
        d->layers[d->layer_count].amount = added;
        d->layer_count++;
    } else {
        d->layers[DRINK_MAX_LAYERS - 1].amount += added;
    }
    return added;
}

bool drink_add(Drink *d, Ingredient ing)
{
    IngredientKind k = ing_info(ing)->kind;
    if (d->sealed || (k != KIND_SYRUP && k != KIND_TOPPING))
        return false;
    if (d->qty[ing] >= DRINK_MAX_COUNT)
        return false;
    d->qty[ing]++;
    return true;
}

bool drink_seal(Drink *d)
{
    if (d->sealed || drink_is_empty(d))
        return false;
    d->sealed = true;
    return true;
}

bool drink_shake(Drink *d)
{
    if (!d->sealed || d->shaken)
        return false;
    d->shaken = true;
    return true;
}

uint16_t drink_mixed_color(const Drink *d)
{
    int r = 0, g = 0, b = 0, w = 0;
    for (int i = LIQUID_FIRST; i <= SYRUP_LAST; i++) {
        int weight = d->qty[i];
        if (ing_info(i)->kind == KIND_SYRUP)
            weight *= 8;   // une pression colore fortement
        if (!weight)
            continue;
        uint16_t c = ing_info(i)->color;
        r += (c & 31) * weight;
        g += ((c >> 5) & 31) * weight;
        b += ((c >> 10) & 31) * weight;
        w += weight;
    }
    if (!w)
        return RGB(28, 28, 28);
    return RGB(r / w, g / w, b / w);
}
