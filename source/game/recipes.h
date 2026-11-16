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
    uint8_t day;                   // premier jour où elle peut être commandée
    uint8_t price;                 // pièces
} Recipe;

extern const Recipe RECIPES[];
extern const int RECIPE_COUNT;

// Tolérance sur les liquides : écart (en unités) encore payé plein pot
#define LIQUID_TOLERANCE 8
// Écart au-delà duquel un liquide ne rapporte plus rien
#define LIQUID_MAX_ERROR 30

// Score 0..100 d'une boisson par rapport à une recette
int recipe_score(const Recipe *r, const Drink *d);

// Recette la plus proche (index) ; *score reçoit son score
int recipe_best_match(const Drink *d, int *score);

// Plus grosse erreur sous forme de phrase courte (« Il manque du thé noir »),
// ou NULL si la boisson est parfaite. `buf` doit faire au moins 48 octets.
const char *recipe_hint(const Recipe *r, const Drink *d, char *buf, int buf_size);

// Jour où tous les ingrédients de la recette sont disponibles
int recipe_min_day(const Recipe *r);

#endif
