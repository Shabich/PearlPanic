#include "game/world.h"

#include "game/customer.h"

#define SL(i, l) { i, l }

const World WORLDS[WORLD_COUNT] = {
    [WORLD_QUARTIER] = {
        "Le Petit Bar", "Quartier des Lanternes",
        { SL(ING_TEA_BLACK, 0), SL(ING_TEA_GREEN, 1), SL(ING_TEA_JASMINE, 3), SL(ING_TEA_MATCHA, 2),
          SL(ING_MILK, 0), SL(ING_MILK_OAT, 3) },
        { SL(ING_SYRUP_BROWN, 0), SL(ING_SYRUP_TARO, 2), SL(ING_SYRUP_STRAWBERRY, 2), SL(ING_SYRUP_MANGO, 1) },
        { SL(ING_PEARLS, 0), SL(ING_POPPING, 1), SL(ING_JELLY, 1), SL(ING_ICE, 0), SL(ING_CREAM, 3) },
        { CT_LAPIN, CT_CHAT, CT_OURS, CT_GRENOUILLE, CT_POUSSIN, CT_PANDA },
    },
    [WORLD_PLAGE] = {
        "La Paillote des Vagues", "Plage des Coquillages",
        { SL(ING_TEA_GREEN, 0), SL(ING_TEA_BLUE, 0), SL(ING_TEA_JASMINE, 1), SL(ING_COCONUT, 0),
          SL(ING_MILK, 2), SL(ING_TEA_BLACK, 3) },
        { SL(ING_SYRUP_MANGO, 0), SL(ING_SYRUP_PINEAPPLE, 0), SL(ING_SYRUP_PASSION, 1), SL(ING_SYRUP_STRAWBERRY, 2) },
        { SL(ING_PEARLS, 0), SL(ING_POPPING, 0), SL(ING_ICE, 0), SL(ING_NATA, 1), SL(ING_SEA_CREAM, 2) },
        { CT_PHOQUE, CT_CRABE, CT_PINGOUIN, CT_POULPE, CT_TORTUE, CT_DAUPHIN },
    },
    [WORLD_MONTAGNE] = {
        "Le Chalet Cocon", "Col des Flocons",
        { SL(ING_TEA_BLACK, 0), SL(ING_CHAI, 0), SL(ING_CHOCOLATE, 0), SL(ING_MILK, 0),
          SL(ING_MILK_OAT, 1), SL(ING_TEA_MATCHA, 2) },
        { SL(ING_SYRUP_BROWN, 0), SL(ING_SYRUP_CINNAMON, 0), SL(ING_SYRUP_CARAMEL, 1), SL(ING_SYRUP_VANILLA, 2) },
        { SL(ING_PEARLS, 0), SL(ING_MARSHMALLOW, 0), SL(ING_CREAM, 1), SL(ING_PUDDING, 2), SL(ING_JELLY, 3) },
        { CT_RENARD, CT_HIBOU, CT_ECUREUIL, CT_MARMOTTE, CT_BICHE, CT_YETI },
    },
    [WORLD_VILLE] = {
        "Le Kiosque Néon", "Avenue des Étoiles",
        { SL(ING_SODA, 0), SL(ING_TEA_OOLONG, 0), SL(ING_TEA_BLUE, 1), SL(ING_TEA_GREEN, 0),
          SL(ING_MILK, 1), SL(ING_TEA_JASMINE, 2) },
        { SL(ING_SYRUP_YUZU, 0), SL(ING_SYRUP_LYCHEE, 0), SL(ING_SYRUP_GRAPE, 1), SL(ING_SYRUP_STRAWBERRY, 2) },
        { SL(ING_POPPING, 0), SL(ING_ICE, 0), SL(ING_GLOW_JELLY, 1), SL(ING_PEARLS, 0), SL(ING_CREAM, 2) },
        { CT_RATON, CT_PIGEON, CT_CHAUVESOURIS, CT_SOURIS, CT_CARLIN, CT_CHAT_NOIR },
    },
    [WORLD_FORET] = {
        "L'Arbre aux Thés", "Bois des Lucioles",
        { SL(ING_TEA_HIBISCUS, 0), SL(ING_TEA_ROSE, 0), SL(ING_TEA_JASMINE, 0), SL(ING_MILK, 1),
          SL(ING_MILK_OAT, 1), SL(ING_TEA_MATCHA, 2) },
        { SL(ING_SYRUP_HONEY, 0), SL(ING_SYRUP_BERRY, 0), SL(ING_SYRUP_LAVENDER, 1), SL(ING_SYRUP_STRAWBERRY, 2) },
        { SL(ING_PEARLS, 0), SL(ING_PETALS, 0), SL(ING_JELLY, 1), SL(ING_CREAM, 2), SL(ING_POPPING, 2) },
        { CT_CHAMPIGNON, CT_LUCIOLE, CT_ESCARGOT, CT_PAPILLON, CT_HERISSON, CT_DRAGON },
    },
};

int world_ing_level(int world, Ingredient ing)
{
    if (world < 0 || world >= WORLD_COUNT)
        return -1;
    const World *w = &WORLDS[world];
    for (int i = 0; i < WORLD_TAPS; i++)
        if (w->taps[i].ing == ing)
            return w->taps[i].level;
    for (int i = 0; i < WORLD_PUMPS; i++)
        if (w->pumps[i].ing == ing)
            return w->pumps[i].level;
    for (int i = 0; i < WORLD_BINS; i++)
        if (w->bins[i].ing == ing)
            return w->bins[i].level;
    return -1;
}

bool level_has_ing(int level, Ingredient ing)
{
    int l = world_ing_level(level_world(level), ing);
    return l >= 0 && l <= level_index(level);
}
