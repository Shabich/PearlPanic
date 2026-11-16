#include "game/specials.h"

#include <string.h>

#include "game/recipes.h"

const SpecialCustomer SPECIALS[] = {
    { "Mamie Lapine", CT_LAPIN, ACC_BOW_GLASSES, 1, 2, 3, 15, {
        { "Thé au lait perles",
          "Bonjour mon petit ! Comme au bon vieux temps : un thé au lait perles.",
          "Exactement comme celui de mon mari… Merci !" },
        { "Brown sugar boba",
          "Mon petit-fils adore le sucre brun. Tu m'en prépares un pour lui ?",
          "Il va être ravi ! Tiens, un petit quelque chose." },
        { "Le Câlin Total",
          "C'est mon anniversaire ! Je veux un lait tout doux, sucré, avec un nuage ET des perles.",
          "Le plus beau des anniversaires ! Je reviendrai toujours ici." },
    } },
    { "Chef Panda", CT_PANDA, ACC_CHEF_HAT, 2, 2, 4, 20, {
        { "Thé vert mangue",
          "Je suis chef pâtissier. Montre-moi ton thé vert mangue, apprenti.",
          "Équilibré. Pas mal du tout." },
        { "Matcha latte",
          "Un matcha latte. Bien secoué. Je regarde.",
          "Mousse parfaite. Je suis impressionné." },
        { "Arc-en-ciel",
          "Pour mon nouveau dessert, il me faut une boisson de toutes les couleurs, sur du lait.",
          "Magnifique ! Ta boutique sera sur ma carte !" },
    } },
    { "Grenouille poète", CT_GRENOUILLE, ACC_BERET, 3, 3, 3, 20, {
        { "Taro latte",
          "Je cherche l'inspiration… Quelque chose de violet, avec des perles ?",
          "Ah ! Un poème me vient… Merci !" },
        { "Jasmin glacé",
          "Pour mon haïku, il me faut une fleur blanche dans un verre glacé.",
          "« Thé de jasmin, glaçons qui tintent… le cœur se repose. »" },
        { "Tempête de perles",
          "Mon recueil s'appelle « Tempête ». Un thé noir… avec une tempête de perles !",
          "Mon recueil est fini. Il t'est dédié !" },
    } },
};

const int SPECIAL_COUNT = sizeof(SPECIALS) / sizeof(SPECIALS[0]);

int special_visit_day(int s, int step)
{
    return SPECIALS[s].first_day + step * SPECIALS[s].gap;
}

int special_due(int day, const uint8_t steps[])
{
    for (int s = 0; s < SPECIAL_COUNT; s++) {
        int step = steps[s];
        if (step < SPECIAL_VISITS && day >= special_visit_day(s, step))
            return s;
    }
    return -1;
}

int special_recipe(int s, int step)
{
    if (s < 0 || s >= SPECIAL_COUNT || step < 0 || step >= SPECIAL_VISITS)
        return -1;
    const char *name = SPECIALS[s].visits[step].recipe;
    for (int i = 0; i < RECIPE_COUNT; i++)
        if (strcmp(RECIPES[i].name, name) == 0)
            return i;
    return -1;
}
