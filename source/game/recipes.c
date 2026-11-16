#include "game/recipes.h"

#include <stdio.h>
#include <stdlib.h>

#define Q(ing, n) [ing] = n

// Gobelet : 100 unités de liquide. Sirops en pressions, garnitures en cuillerées.
#include "game/world.h"

const Recipe RECIPES[] = {
    // ===================================================== Le Petit Bar (monde 0)
    // nom, énigme, quantités, secouée, secrète, monde, niveau, prix
    { "Thé au lait perles", "Le grand classique, avec des perles qui roulent !",
      { Q(ING_TEA_BLACK, 70), Q(ING_MILK, 30), Q(ING_PEARLS, 2) }, false, false, 0, 0, 6 },
    { "Thé noir glacé", "Juste du thé bien sombre et très frais. Brrr !",
      { Q(ING_TEA_BLACK, 100), Q(ING_ICE, 2) }, false, false, 0, 0, 4 },
    { "Brown sugar boba", "Du lait tout doux, du sucre caramel et des perles !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_BROWN, 2), Q(ING_PEARLS, 2) }, false, false, 0, 0, 7 },
    { "Tempête de perles", "Du thé noir… et des perles, plein, plein, PLEIN !",
      { Q(ING_TEA_BLACK, 100), Q(ING_PEARLS, 5) }, false, true, 0, 0, 8 },
    { "Thé vert mangue", "Du vert, du soleil et des billes qui éclatent !",
      { Q(ING_TEA_GREEN, 100), Q(ING_SYRUP_MANGO, 2), Q(ING_POPPING, 2) }, false, false, 0, 1, 8 },
    { "Thé vert au lait", "Un thé vert tout câlin, avec un peu de lait.",
      { Q(ING_TEA_GREEN, 70), Q(ING_MILK, 30) }, false, false, 0, 1, 5 },
    { "Thé vert glacé gelée", "Vert, frais, et quelque chose qui tremblote !",
      { Q(ING_TEA_GREEN, 100), Q(ING_ICE, 2), Q(ING_JELLY, 2) }, false, false, 0, 1, 7 },
    { "Taro latte", "Quelque chose de violet et rebondissant !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_TARO, 2), Q(ING_PEARLS, 2) }, false, false, 0, 2, 9 },
    { "Lait fraise", "Tout rose comme mes joues, avec de la gelée !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_STRAWBERRY, 2), Q(ING_JELLY, 1) }, false, false, 0, 2, 8 },
    { "Arc-en-ciel", "Une boisson de toutes les couleurs, sur du lait !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_TARO, 1), Q(ING_SYRUP_STRAWBERRY, 1),
        Q(ING_SYRUP_MANGO, 1), Q(ING_JELLY, 1) }, false, true, 0, 2, 12 },
    { "Matcha latte", "Vert profond et lait, bien secoué s'il vous plaît !",
      { Q(ING_TEA_MATCHA, 40), Q(ING_MILK, 60) }, true, false, 0, 2, 9 },
    { "Fraise matcha", "Rose en bas, vert en haut… et on secoue !",
      { Q(ING_TEA_MATCHA, 30), Q(ING_MILK, 70), Q(ING_SYRUP_STRAWBERRY, 1) }, true, false, 0, 2, 10 },
    { "Crème brûlée boba", "Thé au lait caramel avec des perles, rien que ça.",
      { Q(ING_TEA_BLACK, 30), Q(ING_MILK, 70), Q(ING_SYRUP_BROWN, 2), Q(ING_PEARLS, 2) },
      false, false, 0, 2, 10 },
    { "Jasmin glacé", "Une fleur blanche dans un verre très frais.",
      { Q(ING_TEA_JASMINE, 100), Q(ING_ICE, 1) }, false, false, 0, 3, 6 },
    { "Jasmin mangue pop", "Floral, ensoleillé et plein de petites bulles !",
      { Q(ING_TEA_JASMINE, 70), Q(ING_TEA_GREEN, 30), Q(ING_SYRUP_MANGO, 2), Q(ING_POPPING, 3) },
      false, false, 0, 3, 11 },
    { "Taro avoine", "Violet tout doux, mais sans lait de vache !",
      { Q(ING_MILK_OAT, 100), Q(ING_SYRUP_TARO, 2) }, false, false, 0, 3, 8 },
    { "Nuage de matcha", "Un matcha secoué avec un petit nuage sur le dessus.",
      { Q(ING_TEA_MATCHA, 70), Q(ING_MILK_OAT, 30), Q(ING_CREAM, 1) }, true, false, 0, 3, 12 },
    { "Thé jasmin au lait", "Une fleur, de l'avoine, et quelques perles.",
      { Q(ING_TEA_JASMINE, 70), Q(ING_MILK_OAT, 30), Q(ING_PEARLS, 1) }, false, false, 0, 3, 9 },
    { "Le Câlin Total", "Tout doux, tout sucré, avec un nuage ET des perles…",
      { Q(ING_MILK, 100), Q(ING_SYRUP_BROWN, 1), Q(ING_SYRUP_TARO, 1), Q(ING_PEARLS, 2),
        Q(ING_CREAM, 1) }, false, true, 0, 3, 15 },
    // ===================================================== La Paillote des Vagues (monde 1)
    { "Lagon bleu", "Bleu comme le lagon, doux comme la noix de coco !",
      { Q(ING_TEA_BLUE, 70), Q(ING_COCONUT, 30), Q(ING_ICE, 2) }, false, false, 1, 0, 8 },
    { "Mangue coco", "Une noix de coco qui aurait croqué une mangue !",
      { Q(ING_COCONUT, 100), Q(ING_SYRUP_MANGO, 2), Q(ING_POPPING, 2) }, false, false, 1, 0, 9 },
    { "Ananas glacé", "Du vert, de l'ananas et des glaçons qui cliquettent.",
      { Q(ING_TEA_GREEN, 100), Q(ING_SYRUP_PINEAPPLE, 2), Q(ING_ICE, 2) }, false, false, 1, 0, 8 },
    { "Perles des mers", "Un océan bleu, avec des perles au fond…",
      { Q(ING_TEA_BLUE, 100), Q(ING_PEARLS, 3) }, false, false, 1, 0, 8 },
    { "Passion jasmin", "Une fleur, un fruit de la passion et des petits cubes blancs.",
      { Q(ING_TEA_JASMINE, 100), Q(ING_SYRUP_PASSION, 2), Q(ING_NATA, 2) }, false, false, 1, 1, 10 },
    { "Coco nata", "Coco et thé vert, avec des cubes tout blancs !",
      { Q(ING_COCONUT, 70), Q(ING_TEA_GREEN, 30), Q(ING_NATA, 2) }, false, false, 1, 1, 9 },
    { "Écume salée", "Un thé vert au lait, avec de l'écume de mer dessus.",
      { Q(ING_TEA_GREEN, 70), Q(ING_MILK, 30), Q(ING_SEA_CREAM, 1) }, false, false, 1, 2, 10 },
    { "Coucher de soleil", "Les couleurs du coucher de soleil sur la plage !",
      { Q(ING_COCONUT, 60), Q(ING_MILK, 40), Q(ING_SYRUP_MANGO, 1), Q(ING_SYRUP_STRAWBERRY, 1),
        Q(ING_ICE, 1) }, false, false, 1, 2, 11 },
    { "Tempête tropicale", "Le ciel gronde : du noir, du coco et une pluie de billes fruitées !",
      { Q(ING_TEA_BLACK, 70), Q(ING_COCONUT, 30), Q(ING_SYRUP_PASSION, 1), Q(ING_SYRUP_PINEAPPLE, 1),
        Q(ING_POPPING, 3) }, false, true, 1, 3, 15 },
    { "Phare au lait", "Thé noir au lait, crème salée et perles, comme au port.",
      { Q(ING_TEA_BLACK, 60), Q(ING_MILK, 40), Q(ING_SEA_CREAM, 1), Q(ING_PEARLS, 2) },
      false, false, 1, 3, 12 },
    // ===================================================== Le Chalet Cocon (monde 2)
    { "Chocolat chaud", "Quelque chose de chaud et chocolaté, avec des petits nuages roses !",
      { Q(ING_CHOCOLATE, 100), Q(ING_MARSHMALLOW, 2) }, false, false, 2, 0, 9 },
    { "Chaï du chalet", "Épicé, chaud et laiteux, avec un soupçon de cannelle.",
      { Q(ING_CHAI, 70), Q(ING_MILK, 30), Q(ING_SYRUP_CINNAMON, 1) }, false, false, 2, 0, 9 },
    { "Thé noir sucré", "Un thé noir bien sucré avec des perles, pour se réchauffer.",
      { Q(ING_TEA_BLACK, 100), Q(ING_SYRUP_BROWN, 2), Q(ING_PEARLS, 2) }, false, false, 2, 0, 8 },
    { "Caramel d'avoine", "Caramel fondant, sans lait de vache, avec des perles.",
      { Q(ING_MILK_OAT, 100), Q(ING_SYRUP_CARAMEL, 2), Q(ING_PEARLS, 2) }, false, false, 2, 1, 10 },
    { "Chaï crémeux", "Un chaï avec un chapeau de crème et de la cannelle.",
      { Q(ING_CHAI, 100), Q(ING_CREAM, 1), Q(ING_SYRUP_CINNAMON, 1) }, false, false, 2, 1, 10 },
    { "Matcha des cimes", "Vert, doux, vanillé… et bien secoué !",
      { Q(ING_TEA_MATCHA, 50), Q(ING_MILK_OAT, 50), Q(ING_SYRUP_VANILLA, 1) }, true, false, 2, 2, 11 },
    { "Pudding vanille", "Du lait vanillé avec du pudding au fond, miam !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_VANILLA, 2), Q(ING_PUDDING, 2) }, false, false, 2, 2, 10 },
    { "Choco caramel", "Chocolat, lait, caramel et crème : l'hiver parfait.",
      { Q(ING_CHOCOLATE, 70), Q(ING_MILK, 30), Q(ING_SYRUP_CARAMEL, 1), Q(ING_CREAM, 1) },
      false, false, 2, 2, 11 },
    { "Avalanche", "Une avalanche de guimauves sur un chocolat crémeux !",
      { Q(ING_CHOCOLATE, 60), Q(ING_MILK, 40), Q(ING_MARSHMALLOW, 3), Q(ING_CREAM, 1) },
      false, true, 2, 3, 15 },
    { "Gelée des neiges", "Thé au lait vanillé, avec une gelée qui tremble de froid.",
      { Q(ING_TEA_BLACK, 70), Q(ING_MILK, 30), Q(ING_JELLY, 2), Q(ING_SYRUP_VANILLA, 1) },
      false, false, 2, 3, 11 },
    // ===================================================== Le Kiosque Néon (monde 3)
    { "Yuzu pétillant", "Ça pétille, c'est jaune et ça pique un peu !",
      { Q(ING_SODA, 100), Q(ING_SYRUP_YUZU, 2), Q(ING_ICE, 2) }, false, false, 3, 0, 9 },
    { "Litchi oolong", "Un oolong parfumé au litchi, avec des billes qui éclatent.",
      { Q(ING_TEA_OOLONG, 100), Q(ING_SYRUP_LYCHEE, 2), Q(ING_POPPING, 2) }, false, false, 3, 0, 10 },
    { "Thé vert pop", "Vert, acidulé, avec des billes.",
      { Q(ING_TEA_GREEN, 100), Q(ING_SYRUP_YUZU, 1), Q(ING_POPPING, 2) }, false, false, 3, 0, 8 },
    { "Néon bleu", "Un soda bleu qui brille dans le noir !",
      { Q(ING_SODA, 70), Q(ING_TEA_BLUE, 30), Q(ING_GLOW_JELLY, 2) }, false, false, 3, 1, 11 },
    { "Raisin au lait", "Un thé au lait couleur raisin.",
      { Q(ING_TEA_OOLONG, 70), Q(ING_MILK, 30), Q(ING_SYRUP_GRAPE, 2) }, false, false, 3, 1, 10 },
    { "Jasmin litchi", "Une fleur blanche et un litchi, bien frais.",
      { Q(ING_TEA_JASMINE, 100), Q(ING_SYRUP_LYCHEE, 1), Q(ING_ICE, 2) }, false, false, 3, 2, 9 },
    { "Fraise pétillante", "Des bulles roses et une gelée qui brille.",
      { Q(ING_SODA, 100), Q(ING_SYRUP_STRAWBERRY, 2), Q(ING_GLOW_JELLY, 1) }, false, false, 3, 2, 11 },
    { "Oolong crème", "Un oolong avec de la crème et des perles.",
      { Q(ING_TEA_OOLONG, 100), Q(ING_CREAM, 1), Q(ING_PEARLS, 2) }, false, false, 3, 2, 11 },
    { "Nuit étoilée", "Une nuit bleue et violette, pleine d'étoiles qui brillent…",
      { Q(ING_TEA_BLUE, 70), Q(ING_MILK, 30), Q(ING_SYRUP_GRAPE, 1), Q(ING_GLOW_JELLY, 2),
        Q(ING_POPPING, 1) }, false, true, 3, 3, 16 },
    { "Minuit au lait", "Pour les couche-tard : vert, lait, yuzu et perles.",
      { Q(ING_TEA_GREEN, 50), Q(ING_MILK, 50), Q(ING_SYRUP_YUZU, 1), Q(ING_PEARLS, 2) },
      false, false, 3, 3, 12 },
    // ===================================================== L'Arbre aux Thés (monde 4)
    { "Hibiscus miel", "Rouge comme une fleur, doux comme le miel.",
      { Q(ING_TEA_HIBISCUS, 100), Q(ING_SYRUP_HONEY, 2) }, false, false, 4, 0, 9 },
    { "Thé de rose", "Rose, rose, et encore des pétales !",
      { Q(ING_TEA_ROSE, 100), Q(ING_PETALS, 2) }, false, false, 4, 0, 9 },
    { "Jasmin des bois", "Une fleur blanche, des baies et des perles.",
      { Q(ING_TEA_JASMINE, 100), Q(ING_SYRUP_BERRY, 1), Q(ING_PEARLS, 2) }, false, false, 4, 0, 10 },
    { "Lavande au lait", "Un lait parfumé à la lavande… on dirait un rêve.",
      { Q(ING_MILK, 70), Q(ING_TEA_JASMINE, 30), Q(ING_SYRUP_LAVENDER, 2) }, false, false, 4, 1, 11 },
    { "Baies d'avoine", "Avoine, baies des bois et gelée qui tremblote.",
      { Q(ING_MILK_OAT, 100), Q(ING_SYRUP_BERRY, 2), Q(ING_JELLY, 2) }, false, false, 4, 1, 11 },
    { "Matcha fleuri", "Un matcha secoué, du miel et des pétales, comme un jardin.",
      { Q(ING_TEA_MATCHA, 40), Q(ING_MILK_OAT, 60), Q(ING_SYRUP_HONEY, 1), Q(ING_PETALS, 2) },
      true, false, 4, 2, 13 },
    { "Rose crème", "Une rose au lait avec un nuage.",
      { Q(ING_TEA_ROSE, 70), Q(ING_MILK, 30), Q(ING_CREAM, 1) }, false, false, 4, 2, 12 },
    { "Potion des fées", "Une potion rouge et fleurie, avec des billes magiques !",
      { Q(ING_TEA_HIBISCUS, 70), Q(ING_TEA_JASMINE, 30), Q(ING_SYRUP_LAVENDER, 1), Q(ING_POPPING, 2) },
      false, false, 4, 2, 13 },
    { "Enchantement", "Tout le bois dans un verre : fleurs, miel, baies et pétales…",
      { Q(ING_TEA_ROSE, 50), Q(ING_TEA_HIBISCUS, 50), Q(ING_SYRUP_HONEY, 1), Q(ING_SYRUP_BERRY, 1),
        Q(ING_PETALS, 3) }, false, true, 4, 3, 18 },
    { "Grand Final", "La boisson de la grande soirée : matcha, lavande, crème et perles !",
      { Q(ING_TEA_MATCHA, 30), Q(ING_MILK, 70), Q(ING_SYRUP_LAVENDER, 1), Q(ING_CREAM, 1),
        Q(ING_PEARLS, 2) }, true, false, 4, 3, 16 },
};

const int RECIPE_COUNT = sizeof(RECIPES) / sizeof(RECIPES[0]);

// Calculs en centièmes de point
#define WEIGHT_LIQUID 200
#define WEIGHT_COUNT  100
#define PENALTY_NOT_SEALED 20
#define PENALTY_OVERFLOW   15
#define PENALTY_NOT_SHAKEN 25

static int liquid_credit(int target, int actual)
{
    int err = abs(target - actual);
    if (err <= LIQUID_TOLERANCE)
        return WEIGHT_LIQUID;
    if (err >= LIQUID_MAX_ERROR)
        return 0;
    return WEIGHT_LIQUID * (LIQUID_MAX_ERROR - err) / (LIQUID_MAX_ERROR - LIQUID_TOLERANCE);
}

int recipe_score(const Recipe *r, const Drink *d)
{
    int total_weight = 0, credit = 0, penalty = 0;

    for (int i = 1; i < ING_COUNT; i++) {
        int t = r->qty[i], a = d->qty[i];
        bool liquid = ing_info(i)->kind == KIND_LIQUID;
        if (t > 0) {
            if (liquid) {
                total_weight += WEIGHT_LIQUID;
                credit += liquid_credit(t, a);
            } else {
                total_weight += WEIGHT_COUNT;
                int err = abs(t - a);
                credit += err == 0 ? WEIGHT_COUNT : err == 1 ? WEIGHT_COUNT / 2 : 0;
            }
        } else if (a > 0) {
            // Ingrédient non demandé
            if (liquid)
                penalty += WEIGHT_LIQUID * (a < LIQUID_MAX_ERROR ? a : LIQUID_MAX_ERROR) /
                           LIQUID_MAX_ERROR;
            else
                penalty += WEIGHT_COUNT;
        }
    }
    if (total_weight == 0)
        return 0;

    int score = 100 * (credit - penalty) / total_weight;
    if (!d->sealed)
        score -= PENALTY_NOT_SEALED;
    if (d->overflowed)
        score -= PENALTY_OVERFLOW;
    if (r->shaken && !d->shaken)
        score -= PENALTY_NOT_SHAKEN;
    if (score < 0)
        score = 0;
    if (score > 100)
        score = 100;
    return score;
}

int recipe_best_match(const Drink *d, int world, int *score)
{
    int best = 0, best_score = -1;
    for (int i = 0; i < RECIPE_COUNT; i++) {
        if (RECIPES[i].world != world)
            continue;
        int s = recipe_score(&RECIPES[i], d);
        if (s > best_score) {
            best_score = s;
            best = i;
        }
    }
    if (score)
        *score = best_score;
    return best;
}

const char *recipe_hint(const Recipe *r, const Drink *d, char *buf, int n)
{
    // On cherche l'écart le plus coûteux
    int worst_ing = 0, worst_cost = 0, worst_dir = 0;
    for (int i = 1; i < ING_COUNT; i++) {
        int t = r->qty[i], a = d->qty[i];
        bool liquid = ing_info(i)->kind == KIND_LIQUID;
        int cost;
        if (liquid)
            cost = abs(t - a) <= LIQUID_TOLERANCE ? 0 : WEIGHT_LIQUID - (t ? liquid_credit(t, a) : 0);
        else
            cost = t == a ? 0 : WEIGHT_COUNT;
        if (cost > worst_cost) {
            worst_cost = cost;
            worst_ing = i;
            worst_dir = a < t ? -1 : 1;
        }
    }
    if (worst_ing) {
        const IngredientInfo *info = ing_info(worst_ing);
        int t = r->qty[worst_ing], a = d->qty[worst_ing];
        if (t == 0)
            snprintf(buf, n, "Pas de %s, merci !", info->name);
        else if (a == 0)
            snprintf(buf, n, "Il manque : %s !", info->name);
        else if (worst_dir < 0)
            snprintf(buf, n, "Plus de %s !", info->name);
        else
            snprintf(buf, n, "Moins de %s !", info->name);
        return buf;
    }
    if (d->overflowed)
        return "Ça déborde partout !";
    if (r->shaken && !d->shaken)
        return "Il fallait la secouer !";
    if (!d->sealed)
        return "Le couvercle, svp !";
    return NULL;
}

int recipe_min_level(const Recipe *r)
{
    int level = 0;
    for (int i = 1; i < ING_COUNT; i++) {
        if (!r->qty[i])
            continue;
        int l = world_ing_level(r->world, i);
        if (l < 0)
            return -1;
        if (l > level)
            level = l;
    }
    return level;
}

int recipe_level(int recipe)
{
    return level_id(RECIPES[recipe].world, RECIPES[recipe].level);
}

bool recipe_available(int recipe, int level)
{
    const Recipe *r = &RECIPES[recipe];
    return r->world == level_world(level) && r->level <= level_index(level);
}
