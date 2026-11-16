// Ingrédients (tâche 2.6 / 3.1)
#ifndef PP_INGREDIENTS_H
#define PP_INGREDIENTS_H

#include <stdint.h>

typedef enum {
    ING_NONE = 0,
    // Liquides versés au stylet (unités, gobelet = 100)
    ING_TEA_BLACK,
    ING_TEA_GREEN,
    ING_TEA_JASMINE,
    ING_TEA_MATCHA,
    ING_MILK,
    ING_MILK_OAT,
    // Sirops (pressions de pompe)
    ING_SYRUP_BROWN,
    ING_SYRUP_TARO,
    ING_SYRUP_STRAWBERRY,
    ING_SYRUP_MANGO,
    // Garnitures (cuillerées)
    ING_PEARLS,
    ING_POPPING,
    ING_JELLY,
    ING_ICE,
    ING_CREAM,
    ING_COUNT
} Ingredient;

typedef enum {
    KIND_LIQUID,
    KIND_SYRUP,
    KIND_TOPPING,
} IngredientKind;

typedef struct {
    const char *name;        // « thé noir »
    const char *short_name;  // ≤ 6 caractères pour les boutons
    IngredientKind kind;
    uint16_t color;          // RGB15
    uint8_t unlock_day;      // jour où l'ingrédient arrive en boutique
} IngredientInfo;

const IngredientInfo *ing_info(Ingredient i);

#define LIQUID_FIRST ING_TEA_BLACK
#define LIQUID_LAST  ING_MILK_OAT
#define SYRUP_FIRST  ING_SYRUP_BROWN
#define SYRUP_LAST   ING_SYRUP_MANGO
#define TOPPING_FIRST ING_PEARLS
#define TOPPING_LAST  ING_CREAM

#endif
