// Progression persistante et sauvegarde (tâches 6.3, 6.5)
#ifndef PP_PROGRESS_H
#define PP_PROGRESS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    UPG_FAST_TAP,     // tireuses plus rapides
    UPG_CUSHIONS,     // coussins doux : +25 % de patience
    UPG_GARLANDS,     // guirlandes : l'ambiance baisse 2x moins vite
    UPG_TIP_JAR,      // pot à pourboires : +1 pièce de pourboire
    UPG_COUNT
} Upgrade;

typedef struct {
    const char *name;
    const char *desc;
    uint16_t cost;
} UpgradeInfo;

typedef struct {
    uint16_t day;              // prochain jour à jouer (1 = début)
    uint32_t coins;
    uint32_t known_recipes;    // bit i = recette i découverte
    uint8_t upgrades;          // bit i = amélioration i achetée
    uint32_t total_served;
    uint32_t best_day_coins;
} Progress;

Progress *progress(void);
const UpgradeInfo *upgrade_info(Upgrade u);

void progress_new(void);
bool progress_load(void);      // false = aucune sauvegarde valide (nouvelle partie)
void progress_save(void);
bool progress_has_save(void);

bool progress_recipe_known(int recipe);
void progress_learn_recipe(int recipe);
int  progress_known_count(void);

bool progress_has_upgrade(Upgrade u);
bool progress_buy_upgrade(Upgrade u);   // false si déjà acheté ou trop cher

// Sérialisation (testable sans plateforme)
#define SAVE_SIZE 32
void progress_serialize(const Progress *p, uint8_t out[SAVE_SIZE]);
bool progress_deserialize(Progress *p, const uint8_t in[SAVE_SIZE]);

#endif
