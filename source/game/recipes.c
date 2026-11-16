#include "game/recipes.h"

#include <stdio.h>
#include <stdlib.h>

#define Q(ing, n) [ing] = n

// Gobelet : 100 unités de liquide. Sirops en pressions, garnitures en cuillerées.
const Recipe RECIPES[] = {
    // --- Jour 1
    { "Thé au lait perles", "Le grand classique, avec des perles qui roulent !",
      { Q(ING_TEA_BLACK, 70), Q(ING_MILK, 30), Q(ING_PEARLS, 2) }, false, false, 1, 6 },
    { "Thé noir glacé", "Juste du thé bien sombre et très frais. Brrr !",
      { Q(ING_TEA_BLACK, 100), Q(ING_ICE, 2) }, false, false, 1, 4 },
    { "Brown sugar boba", "Du lait tout doux, du sucre caramel et des perles !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_BROWN, 2), Q(ING_PEARLS, 2) }, false, false, 1, 7 },
    { "Tempête de perles", "Du thé noir… et des perles, plein, plein, PLEIN !",
      { Q(ING_TEA_BLACK, 100), Q(ING_PEARLS, 5) }, false, true, 1, 8 },
    // --- Jour 2
    { "Thé vert mangue", "Du vert, du soleil et des billes qui éclatent !",
      { Q(ING_TEA_GREEN, 100), Q(ING_SYRUP_MANGO, 2), Q(ING_POPPING, 2) }, false, false, 2, 8 },
    { "Thé vert au lait", "Un thé vert tout câlin, avec un peu de lait.",
      { Q(ING_TEA_GREEN, 70), Q(ING_MILK, 30) }, false, false, 2, 5 },
    { "Thé vert glacé gelée", "Vert, frais, et quelque chose qui tremblote !",
      { Q(ING_TEA_GREEN, 100), Q(ING_ICE, 2), Q(ING_JELLY, 2) }, false, false, 2, 7 },
    // --- Jour 3
    { "Taro latte", "Quelque chose de violet et rebondissant !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_TARO, 2), Q(ING_PEARLS, 2) }, false, false, 3, 9 },
    { "Lait fraise", "Tout rose comme mes joues, avec de la gelée !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_STRAWBERRY, 2), Q(ING_JELLY, 1) }, false, false, 3, 8 },
    { "Arc-en-ciel", "Une boisson de toutes les couleurs, sur du lait !",
      { Q(ING_MILK, 100), Q(ING_SYRUP_TARO, 1), Q(ING_SYRUP_STRAWBERRY, 1),
        Q(ING_SYRUP_MANGO, 1), Q(ING_JELLY, 1) }, false, true, 3, 12 },
    // --- Jour 4
    { "Matcha latte", "Vert profond et lait, bien secoué s'il vous plaît !",
      { Q(ING_TEA_MATCHA, 40), Q(ING_MILK, 60) }, true, false, 4, 9 },
    { "Fraise matcha", "Rose en bas, vert en haut… et on secoue !",
      { Q(ING_TEA_MATCHA, 30), Q(ING_MILK, 70), Q(ING_SYRUP_STRAWBERRY, 1) }, true, false, 4, 10 },
    { "Crème brûlée boba", "Thé au lait caramel avec des perles, rien que ça.",
      { Q(ING_TEA_BLACK, 30), Q(ING_MILK, 70), Q(ING_SYRUP_BROWN, 2), Q(ING_PEARLS, 2) },
      false, false, 4, 10 },
    // --- Jour 5
    { "Jasmin glacé", "Une fleur blanche dans un verre très frais.",
      { Q(ING_TEA_JASMINE, 100), Q(ING_ICE, 1) }, false, false, 5, 6 },
    { "Jasmin mangue pop", "Floral, ensoleillé et plein de petites bulles !",
      { Q(ING_TEA_JASMINE, 70), Q(ING_TEA_GREEN, 30), Q(ING_SYRUP_MANGO, 2), Q(ING_POPPING, 3) },
      false, false, 5, 11 },
    { "Taro avoine", "Violet tout doux, mais sans lait de vache !",
      { Q(ING_MILK_OAT, 100), Q(ING_SYRUP_TARO, 2) }, false, false, 5, 8 },
    { "Nuage de matcha", "Un matcha secoué avec un petit nuage sur le dessus.",
      { Q(ING_TEA_MATCHA, 70), Q(ING_MILK_OAT, 30), Q(ING_CREAM, 1) }, true, false, 5, 12 },
    { "Thé jasmin au lait", "Une fleur, de l'avoine, et quelques perles.",
      { Q(ING_TEA_JASMINE, 70), Q(ING_MILK_OAT, 30), Q(ING_PEARLS, 1) }, false, false, 5, 9 },
    { "Le Câlin Total", "Tout doux, tout sucré, avec un nuage ET des perles…",
      { Q(ING_MILK, 100), Q(ING_SYRUP_BROWN, 1), Q(ING_SYRUP_TARO, 1), Q(ING_PEARLS, 2),
        Q(ING_CREAM, 1) }, false, true, 5, 15 },
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

int recipe_best_match(const Drink *d, int *score)
{
    int best = 0, best_score = -1;
    for (int i = 0; i < RECIPE_COUNT; i++) {
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

int recipe_min_day(const Recipe *r)
{
    int day = 1;
    for (int i = 1; i < ING_COUNT; i++)
        if (r->qty[i] && ing_info(i)->unlock_day > day)
            day = ing_info(i)->unlock_day;
    return day;
}
