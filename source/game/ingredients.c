#include "game/ingredients.h"

#include "gfx/canvas.h"

#define L(name, s, r, g, b)            { name, s, KIND_LIQUID, RGB888(r, g, b), TOP_NONE, 0, false }
#define HOT(name, s, r, g, b)          { name, s, KIND_LIQUID, RGB888(r, g, b), TOP_NONE, 0, true }
#define S(name, s, r, g, b)            { name, s, KIND_SYRUP, RGB888(r, g, b), TOP_NONE, 0, false }
#define T(name, s, r, g, b, st, n)     { name, s, KIND_TOPPING, RGB888(r, g, b), st, n, false }

static const IngredientInfo INFO[ING_COUNT] = {
    [ING_NONE]             = L("rien", "", 0, 0, 0),
    // Liquides
    [ING_TEA_BLACK]        = L("thé noir", "Noir", 150, 85, 50),
    [ING_TEA_GREEN]        = L("thé vert", "Vert", 190, 200, 110),
    [ING_TEA_JASMINE]      = L("thé jasmin", "Jasmin", 235, 215, 140),
    [ING_TEA_MATCHA]       = L("matcha", "Matcha", 120, 175, 90),
    [ING_MILK]             = L("lait", "Lait", 250, 248, 240),
    [ING_MILK_OAT]         = L("lait d'avoine", "Avoine", 235, 220, 190),
    [ING_TEA_BLUE]         = L("thé bleu", "Bleu", 90, 130, 230),
    [ING_COCONUT]          = L("lait de coco", "Coco", 245, 240, 228),
    [ING_CHAI]             = HOT("chaï", "Chaï", 200, 140, 90),
    [ING_CHOCOLATE]        = HOT("chocolat chaud", "Choco", 110, 66, 50),
    [ING_SODA]             = L("eau pétillante", "Soda", 215, 240, 250),
    [ING_TEA_OOLONG]       = L("thé oolong", "Oolong", 200, 130, 60),
    [ING_TEA_HIBISCUS]     = L("thé hibiscus", "Hibisc", 200, 50, 90),
    [ING_TEA_ROSE]         = L("thé à la rose", "Rose", 245, 170, 185),
    // Sirops
    [ING_SYRUP_BROWN]      = S("sucre brun", "Brun", 120, 70, 40),
    [ING_SYRUP_TARO]       = S("sirop de taro", "Taro", 185, 150, 225),
    [ING_SYRUP_STRAWBERRY] = S("sirop de fraise", "Fraise", 250, 120, 150),
    [ING_SYRUP_MANGO]      = S("sirop de mangue", "Mangue", 255, 190, 70),
    [ING_SYRUP_PINEAPPLE]  = S("sirop d'ananas", "Ananas", 250, 220, 90),
    [ING_SYRUP_PASSION]    = S("fruit de la passion", "Passio", 240, 150, 60),
    [ING_SYRUP_CINNAMON]   = S("cannelle", "Canne.", 170, 95, 55),
    [ING_SYRUP_CARAMEL]    = S("caramel", "Caram.", 215, 140, 60),
    [ING_SYRUP_VANILLA]    = S("vanille", "Vanil.", 250, 236, 190),
    [ING_SYRUP_YUZU]       = S("sirop de yuzu", "Yuzu", 240, 230, 80),
    [ING_SYRUP_LYCHEE]     = S("sirop de litchi", "Litchi", 250, 205, 215),
    [ING_SYRUP_GRAPE]      = S("sirop de raisin", "Raisin", 150, 80, 170),
    [ING_SYRUP_HONEY]      = S("miel", "Miel", 245, 180, 50),
    [ING_SYRUP_BERRY]      = S("sirop de baies", "Baies", 160, 50, 110),
    [ING_SYRUP_LAVENDER]   = S("sirop de lavande", "Lavand", 180, 160, 230),
    // Garnitures
    [ING_PEARLS]           = T("perles", "Perles", 74, 44, 42, TOP_SINK_BALL, 6),
    [ING_POPPING]          = T("popping boba", "Pop", 255, 160, 70, TOP_SINK_BALL, 4),
    [ING_JELLY]            = T("gelée", "Gelée", 150, 220, 120, TOP_SINK_CUBE, 4),
    [ING_ICE]              = T("glaçons", "Glace", 215, 240, 255, TOP_FLOAT_CUBE, 2),
    [ING_CREAM]            = T("crème", "Crème", 255, 252, 240, TOP_LAYER, 1),
    [ING_SEA_CREAM]        = T("crème salée", "Salée", 225, 240, 245, TOP_LAYER, 1),
    [ING_NATA]             = T("nata de coco", "Nata", 240, 245, 235, TOP_SINK_CUBE, 4),
    [ING_MARSHMALLOW]      = T("guimauves", "Guim.", 255, 220, 230, TOP_FLOAT_CUBE, 3),
    [ING_PUDDING]          = T("pudding", "Puddin", 250, 210, 110, TOP_SINK_CUBE, 2),
    [ING_GLOW_JELLY]       = T("gelée fluo", "Fluo", 120, 255, 170, TOP_SINK_CUBE, 4),
    [ING_PETALS]           = T("pétales", "Pétale", 255, 160, 190, TOP_FLOAT_FLAT, 5),
};

const IngredientInfo *ing_info(Ingredient i)
{
    if (i <= ING_NONE || i >= ING_COUNT)
        return &INFO[ING_NONE];
    return &INFO[i];
}
