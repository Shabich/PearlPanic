// Boisson en cours d'assemblage (tâche 3.1)
#ifndef PP_DRINK_H
#define PP_DRINK_H

#include <stdbool.h>
#include <stdint.h>

#include "game/ingredients.h"

#define CUP_CAPACITY      100
#define DRINK_MAX_LAYERS  8
#define DRINK_MAX_COUNT   9    // pressions / cuillerées max par ingrédient

typedef struct {
    uint8_t ing;
    uint8_t amount;
} Layer;

typedef struct {
    // Liquides : unités versées par ingrédient ; sirops/garnitures : nombre
    uint8_t qty[ING_COUNT];
    // Couches de liquide dans l'ordre de versement (pour l'affichage)
    Layer layers[DRINK_MAX_LAYERS];
    uint8_t layer_count;
    uint8_t liquid_total;
    bool overflowed;    // a débordé au moins une fois
    bool shaken;
    bool sealed;
} Drink;

void drink_reset(Drink *d);
bool drink_is_empty(const Drink *d);

// Verse `units` d'un liquide. Ce qui dépasse le gobelet est perdu et marque
// le débordement. Renvoie le nombre d'unités réellement ajoutées.
int drink_pour(Drink *d, Ingredient ing, int units);

// Ajoute une pression de sirop ou une cuillerée de garniture
bool drink_add(Drink *d, Ingredient ing);

bool drink_seal(Drink *d);
// Secouer n'est possible qu'une fois le gobelet scellé
bool drink_shake(Drink *d);

// Couleur moyenne du liquide (sirops compris), pour l'affichage mélangé
uint16_t drink_mixed_color(const Drink *d);

#endif
