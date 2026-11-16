// Objectif du jour (tâche 6.6)
#ifndef PP_OBJECTIVES_H
#define PP_OBJECTIVES_H

#include <stdbool.h>
#include <stdint.h>

struct Day;

typedef enum {
    OBJ_NONE,
    OBJ_SERVE,         // servir N clients
    OBJ_PERFECT,       // N boissons parfaites (≥ 90 %)
    OBJ_NO_LOSS,       // aucun client perdu
    OBJ_DISCOVER,      // découvrir une recette
    OBJ_AMBIANCE,      // finir avec l'ambiance ≥ N %
    OBJ_COINS,         // gagner N pièces
    OBJ_COUNT
} ObjectiveType;

typedef struct {
    uint8_t type;
    int16_t target;
    uint8_t reward;    // pièces bonus
} Objective;

// Objectif déterministe pour un jour donné. `can_discover` : il reste une
// recette (non secrète) à découvrir ce jour-là.
Objective objective_for_day(int day, bool can_discover);

// Avancement actuel (même unité que target) ; pour NO_LOSS : clients perdus
int objective_value(const Objective *o, const struct Day *d);
// Atteint ? (à appeler en fin de journée pour NO_LOSS / AMBIANCE)
bool objective_done(const Objective *o, const struct Day *d);

void objective_describe(const Objective *o, char *buf, int size);

#endif
