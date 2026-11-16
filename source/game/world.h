// Mondes et niveaux : chaque monde est un nouveau bar (lieu, clients, ingrédients)
#ifndef PP_WORLD_H
#define PP_WORLD_H

#include <stdbool.h>
#include <stdint.h>

#include "game/ingredients.h"

#define WORLD_COUNT       5
#define LEVELS_PER_WORLD  4
#define LEVEL_COUNT       (WORLD_COUNT * LEVELS_PER_WORLD)
#define WORLD_TAPS        6
#define WORLD_PUMPS       4
#define WORLD_BINS        5
#define WORLD_CUSTOMERS   6

typedef enum {
    WORLD_QUARTIER,   // le petit bar du quartier
    WORLD_PLAGE,      // la paillote au bord de la mer
    WORLD_MONTAGNE,   // le chalet enneigé
    WORLD_VILLE,      // le kiosque néon en ville, la nuit
    WORLD_FORET,      // l'arbre aux thés de la forêt enchantée
} WorldId;

typedef struct {
    Ingredient ing;
    uint8_t level;    // niveau du monde (0..3) où l'ingrédient arrive
} StationSlot;

typedef struct {
    const char *name;          // nom du bar
    const char *place;         // lieu
    StationSlot taps[WORLD_TAPS];
    StationSlot pumps[WORLD_PUMPS];
    StationSlot bins[WORLD_BINS];
    uint8_t customers[WORLD_CUSTOMERS];   // espèces (CustomerTypeId)
} World;

extern const World WORLDS[WORLD_COUNT];

// Niveau global 1..20 <-> monde 0..4 + niveau dans le monde 0..3
static inline int level_world(int level) { return (level - 1) / LEVELS_PER_WORLD; }
static inline int level_index(int level) { return (level - 1) % LEVELS_PER_WORLD; }
static inline int level_id(int world, int index) { return world * LEVELS_PER_WORLD + index + 1; }

// Niveau du monde où l'ingrédient arrive au poste ; -1 s'il n'existe pas dans ce bar
int world_ing_level(int world, Ingredient ing);
// Disponible au poste pour ce niveau global ?
bool level_has_ing(int level, Ingredient ing);

#endif
