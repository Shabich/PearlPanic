// Clients spéciaux récurrents avec une mini-histoire (tâche 4.10)
//
// Chaque habitué revient plusieurs fois. Une visite réussie (client content)
// fait avancer son histoire ; une visite ratée sera retentée un autre jour.
#ifndef PP_SPECIALS_H
#define PP_SPECIALS_H

#include <stdint.h>

#include "game/customer.h"

#define SPECIAL_VISITS 3

typedef enum {
    ACC_BOW_GLASSES,   // nœud + lunettes rondes
    ACC_CHEF_HAT,      // toque
    ACC_BERET,         // béret d'artiste
} Accessory;

typedef struct {
    const char *recipe;     // nom de la recette commandée
    const char *hello;      // réplique à l'arrivée
    const char *thanks;     // réplique si la visite est réussie
} SpecialVisit;

typedef struct {
    const char *name;
    uint8_t type;           // espèce (image de base)
    uint8_t accessory;
    uint8_t first_day;      // jour de la première visite
    uint8_t gap;            // jours entre deux visites
    uint8_t tip_bonus;      // pourboire en plus à chaque visite réussie
    uint8_t final_bonus;    // cadeau en pièces à la fin de l'histoire
    SpecialVisit visits[SPECIAL_VISITS];
} SpecialCustomer;

extern const SpecialCustomer SPECIALS[];
extern const int SPECIAL_COUNT;

// Jour à partir duquel la visite `step` d'un habitué peut avoir lieu
int special_visit_day(int special, int step);

// Habitué attendu ce jour-là compte tenu de son avancement (-1 si aucun)
int special_due(int day, const uint8_t steps[]);

// Recette (index) de la visite courante, -1 si introuvable
int special_recipe(int special, int step);

#endif
