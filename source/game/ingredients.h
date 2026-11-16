// Ingrédients de tous les bars (tâches 2.6 / 3.1)
#ifndef PP_INGREDIENTS_H
#define PP_INGREDIENTS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ING_NONE = 0,
    // --- Liquides versés au stylet (unités, gobelet = 100)
    ING_TEA_BLACK,
    ING_TEA_GREEN,
    ING_TEA_JASMINE,
    ING_TEA_MATCHA,
    ING_MILK,
    ING_MILK_OAT,
    ING_TEA_BLUE,        // thé bleu (pois papillon)
    ING_COCONUT,         // lait de coco
    ING_CHAI,
    ING_CHOCOLATE,       // chocolat chaud
    ING_SODA,            // eau pétillante
    ING_TEA_OOLONG,
    ING_TEA_HIBISCUS,
    ING_TEA_ROSE,
    // --- Sirops (pressions de pompe)
    ING_SYRUP_BROWN,
    ING_SYRUP_TARO,
    ING_SYRUP_STRAWBERRY,
    ING_SYRUP_MANGO,
    ING_SYRUP_PINEAPPLE,
    ING_SYRUP_PASSION,
    ING_SYRUP_CINNAMON,
    ING_SYRUP_CARAMEL,
    ING_SYRUP_VANILLA,
    ING_SYRUP_YUZU,
    ING_SYRUP_LYCHEE,
    ING_SYRUP_GRAPE,
    ING_SYRUP_HONEY,
    ING_SYRUP_BERRY,
    ING_SYRUP_LAVENDER,
    // --- Garnitures (cuillerées)
    ING_PEARLS,
    ING_POPPING,
    ING_JELLY,
    ING_ICE,
    ING_CREAM,
    ING_SEA_CREAM,       // crème salée
    ING_NATA,            // nata de coco
    ING_MARSHMALLOW,
    ING_PUDDING,
    ING_GLOW_JELLY,      // gelée fluo
    ING_PETALS,          // pétales de fleurs
    ING_COUNT
} Ingredient;

typedef enum {
    KIND_LIQUID,
    KIND_SYRUP,
    KIND_TOPPING,
} IngredientKind;

// Comportement d'une garniture dans le gobelet (animation)
typedef enum {
    TOP_NONE,
    TOP_SINK_BALL,    // billes qui coulent (perles, popping)
    TOP_SINK_CUBE,    // cubes qui coulent et tremblotent (gelées, nata, pudding)
    TOP_FLOAT_CUBE,   // cubes qui flottent (glaçons, guimauves)
    TOP_FLOAT_FLAT,   // petites choses plates qui dérivent en surface (pétales)
    TOP_LAYER,        // couche sur le dessus (crèmes)
} ToppingStyle;

typedef struct {
    const char *name;        // « thé noir »
    const char *short_name;  // ≤ 6 caractères pour les boutons
    IngredientKind kind;
    uint16_t color;          // RGB15
    uint8_t style;           // ToppingStyle (garnitures)
    uint8_t per_scoop;       // objets dessinés par cuillerée
    bool hot;                // boisson chaude : de la vapeur s'en échappe
} IngredientInfo;

const IngredientInfo *ing_info(Ingredient i);

#define LIQUID_FIRST ING_TEA_BLACK
#define LIQUID_LAST  ING_TEA_ROSE
#define SYRUP_FIRST  ING_SYRUP_BROWN
#define SYRUP_LAST   ING_SYRUP_LAVENDER
#define TOPPING_FIRST ING_PEARLS
#define TOPPING_LAST  ING_PETALS

#endif
