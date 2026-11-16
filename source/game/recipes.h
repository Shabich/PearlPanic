// Recettes, évaluation et indices (tâches 5.1, 5.2, 5.5, 5.7)
#ifndef PP_RECIPES_H
#define PP_RECIPES_H

#include <stdbool.h>
#include <stdint.h>

#include "game/drink.h"

typedef struct {
    const char *name;
    const char *riddle;            // commande énigmatique (découverte)
    uint8_t qty[ING_COUNT];        // cible : unités (liquides) / nombre (sirops, garnitures)
    bool shaken;                   // doit être secouée
    bool secret;                   // jamais commandée explicitement, à découvrir seul
    uint8_t world;                 // bar où on la sert
    uint8_t level;                 // niveau du monde (0..3) à partir duquel on la commande
    uint8_t price;                 // pièces
} Recipe;

extern const Recipe RECIPES[];
extern const int RECIPE_COUNT;
#define RECIPE_MAX 64   // tient dans le masque de sauvegarde (64 bits)

// Tolérance sur les liquides : écart (en unités) encore payé plein pot
#define LIQUID_TOLERANCE 8
// Écart au-delà duquel un liquide ne rapporte plus rien
#define LIQUID_MAX_ERROR 30

// Score 0..100 d'une boisson par rapport à une recette
int recipe_score(const Recipe *r, const Drink *d);

// Recette la plus proche (index) parmi celles d'un monde ; *score reçoit son score
int recipe_best_match(const Drink *d, int world, int *score);

// La recette peut-elle être commandée à ce niveau global ?
bool recipe_available(int recipe, int level);
// Niveau global où la recette devient commandable
int recipe_level(int recipe);

// Plus grosse erreur sous forme de phrase courte (« Il manque du thé noir »),
// ou NULL si la boisson est parfaite. `buf` doit faire au moins 48 octets.
const char *recipe_hint(const Recipe *r, const Drink *d, char *buf, int buf_size);

// Niveau du monde (0..3) où tous les ingrédients sont au poste, -1 si impossible dans ce bar
int recipe_min_level(const Recipe *r);

#endif
