// Habitués avec une mini-histoire (tâche 4.10) : un ou deux par bar
//
// Chaque habitué revient à des niveaux précis de son monde. Une visite réussie
// (client content) fait avancer son histoire ; une visite ratée sera retentée
// au prochain niveau joué dans ce monde.
#ifndef PP_SPECIALS_H
#define PP_SPECIALS_H

#include <stdint.h>

#include "game/customer.h"

#define SPECIAL_VISITS     3
#define SPECIALS_PER_LEVEL 2

typedef enum {
    ACC_BOW_GLASSES,   // nœud + lunettes rondes
    ACC_CHEF_HAT,      // toque
    ACC_BERET,         // béret d'artiste
    ACC_SAILOR,        // casquette de marin
    ACC_HEADPHONES,    // casque de DJ
    ACC_PRESS_HAT,     // grand chapeau de critique
} Accessory;

typedef struct {
    uint8_t level;          // niveau global où la visite peut avoir lieu (0 = pas de visite)
    const char *recipe;     // nom de la recette commandée
    const char *hello;      // réplique à l'arrivée
    const char *thanks;     // réplique si la visite est réussie
} SpecialVisit;

typedef struct {
    const char *name;
    uint8_t type;           // espèce (image de base)
    uint8_t accessory;
    uint8_t visit_count;
    uint8_t tip_bonus;      // pourboire en plus à chaque visite réussie
    uint8_t final_bonus;    // cadeau en pièces à la fin de l'histoire
    SpecialVisit visits[SPECIAL_VISITS];
} SpecialCustomer;

typedef enum {
    SP_MAMIE, SP_CAPITAINE, SP_CHEF, SP_DJ, SP_POETE, SP_PLUME, SPECIAL_COUNT_ENUM
} SpecialId;

extern const SpecialCustomer SPECIALS[];
extern const int SPECIAL_COUNT;
#define SPECIAL_MAX 8      // taille du tableau d'avancement dans la sauvegarde

// Habitués attendus à ce niveau (au plus SPECIALS_PER_LEVEL), selon leur avancement.
// Renvoie le nombre trouvé.
int specials_due(int level, const uint8_t steps[], int out[SPECIALS_PER_LEVEL]);

// Recette (index) de la visite `step`, -1 si introuvable
int special_recipe(int special, int step);

// Monde de l'habitué
int special_world(int special);

#endif
