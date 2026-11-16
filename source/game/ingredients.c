#include "game/ingredients.h"

#include "gfx/canvas.h"

static const IngredientInfo INFO[ING_COUNT] = {
    [ING_NONE]             = { "rien",         "",       KIND_LIQUID,  0,                  0 },
    [ING_TEA_BLACK]        = { "thé noir",     "Noir",   KIND_LIQUID,  RGB888(150, 85, 50),  1 },
    [ING_TEA_GREEN]        = { "thé vert",     "Vert",   KIND_LIQUID,  RGB888(190, 200, 110), 2 },
    [ING_TEA_JASMINE]      = { "thé jasmin",   "Jasmin", KIND_LIQUID,  RGB888(235, 215, 140), 5 },
    [ING_TEA_MATCHA]       = { "matcha",       "Matcha", KIND_LIQUID,  RGB888(120, 175, 90), 4 },
    [ING_MILK]             = { "lait",         "Lait",   KIND_LIQUID,  RGB888(250, 248, 240), 1 },
    [ING_MILK_OAT]         = { "lait d'avoine", "Avoine", KIND_LIQUID, RGB888(235, 220, 190), 5 },
    [ING_SYRUP_BROWN]      = { "sucre brun",   "Brun",   KIND_SYRUP,   RGB888(120, 70, 40),  1 },
    [ING_SYRUP_TARO]       = { "sirop de taro", "Taro",  KIND_SYRUP,   RGB888(185, 150, 225), 3 },
    [ING_SYRUP_STRAWBERRY] = { "sirop de fraise", "Fraise", KIND_SYRUP, RGB888(250, 120, 150), 3 },
    [ING_SYRUP_MANGO]      = { "sirop de mangue", "Mangue", KIND_SYRUP, RGB888(255, 190, 70), 2 },
    [ING_PEARLS]           = { "perles",       "Perles", KIND_TOPPING, RGB888(74, 44, 42),   1 },
    [ING_POPPING]          = { "popping boba", "Pop",    KIND_TOPPING, RGB888(255, 160, 70), 2 },
    [ING_JELLY]            = { "gelée",        "Gelée",  KIND_TOPPING, RGB888(150, 220, 120), 2 },
    [ING_ICE]              = { "glaçons",      "Glace",  KIND_TOPPING, RGB888(215, 240, 255), 1 },
    [ING_CREAM]            = { "crème",        "Crème",  KIND_TOPPING, RGB888(255, 252, 240), 5 },
};

const IngredientInfo *ing_info(Ingredient i)
{
    if (i <= ING_NONE || i >= ING_COUNT)
        return &INFO[ING_NONE];
    return &INFO[i];
}
