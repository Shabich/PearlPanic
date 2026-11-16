#include "game/objectives.h"

#include <stdio.h>

#include "game/day.h"

Objective objective_for_day(int day, bool can_discover)
{
    Objective o = { OBJ_NONE, 0, 0 };
    if (day < 1)
        day = 1;
    // Rotation fixe : chaque jour un défi différent, de plus en plus exigeant
    static const uint8_t ROTATION[] = { OBJ_SERVE, OBJ_DISCOVER, OBJ_PERFECT, OBJ_COINS,
                                        OBJ_NO_LOSS, OBJ_AMBIANCE };
    o.type = ROTATION[(day - 1) % (int)sizeof(ROTATION)];
    if (o.type == OBJ_DISCOVER && !can_discover)
        o.type = OBJ_PERFECT;
    switch (o.type) {
    case OBJ_SERVE:    o.target = (int16_t)(4 + day);          o.reward = 8;  break;
    case OBJ_PERFECT:  o.target = (int16_t)(2 + day / 2);      o.reward = 10; break;
    case OBJ_NO_LOSS:  o.target = 0;                           o.reward = 12; break;
    case OBJ_DISCOVER: o.target = 1;                           o.reward = 10; break;
    case OBJ_AMBIANCE: o.target = (int16_t)(day < 8 ? 50 + day * 3 : 75); o.reward = 12; break;
    case OBJ_COINS:    o.target = (int16_t)(20 + 8 * day);     o.reward = 10; break;
    default: break;
    }
    return o;
}

int objective_value(const Objective *o, const Day *d)
{
    switch (o->type) {
    case OBJ_SERVE:    return d->served;
    case OBJ_PERFECT:  return d->perfect;
    case OBJ_NO_LOSS:  return d->lost;
    case OBJ_DISCOVER: return d->unlocked_count;
    case OBJ_AMBIANCE: return d->ambiance * 100 / AMBIANCE_MAX;
    case OBJ_COINS:    return d->coins + d->tips;
    default:           return 0;
    }
}

bool objective_done(const Objective *o, const Day *d)
{
    int v = objective_value(o, d);
    switch (o->type) {
    case OBJ_NONE:     return false;
    case OBJ_NO_LOSS:  return v == 0 && !d->collapsed;
    case OBJ_AMBIANCE: return v >= o->target && !d->collapsed;
    default:           return v >= o->target;
    }
}

void objective_describe(const Objective *o, char *buf, int n)
{
    switch (o->type) {
    case OBJ_SERVE:    snprintf(buf, n, "Servir %d clients", o->target); break;
    case OBJ_PERFECT:  snprintf(buf, n, "%d boissons parfaites", o->target); break;
    case OBJ_NO_LOSS:  snprintf(buf, n, "Ne perdre aucun client"); break;
    case OBJ_DISCOVER: snprintf(buf, n, "Découvrir une recette"); break;
    case OBJ_AMBIANCE: snprintf(buf, n, "Finir avec %d %% d'ambiance", o->target); break;
    case OBJ_COINS:    snprintf(buf, n, "Gagner %d pièces", o->target); break;
    default:           snprintf(buf, n, "-"); break;
    }
}
