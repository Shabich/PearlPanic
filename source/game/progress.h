// Progression persistante et sauvegarde (tâches 6.3, 6.5)
#ifndef PP_PROGRESS_H
#define PP_PROGRESS_H

#include <stdbool.h>
#include <stdint.h>

#include "game/specials.h"
#include "game/world.h"

typedef enum {
    UPG_FAST_TAP,     // tireuses plus rapides
    UPG_CUSHIONS,     // coussins douillets : +25 % de patience
    UPG_GARLANDS,     // guirlandes : l'ambiance baisse 2x moins vite
    UPG_TIP_JAR,      // pot à pourboires : +1 pièce de pourboire
    UPG_SECOND_CUP,   // deuxième gobelet sur le poste (tâche 3.12)
    UPG_COUNT
} Upgrade;

typedef struct {
    const char *name;
    const char *desc;
    uint16_t cost;
} UpgradeInfo;

// Étapes de l'histoire déjà vues (cinématiques)
enum {
    STORY_INTRO = 1 << 0,          // ouverture du premier bar
    STORY_WORLD_FIRST = 1 << 1,    // + monde : arrivée dans chaque nouveau bar (bits 1..5)
    STORY_ENDING = 1 << 6,         // cinématique de fin
};

typedef struct {
    uint8_t stars[LEVEL_COUNT];    // meilleures étoiles par niveau (0 = pas terminé)
    uint8_t level;                 // niveau sélectionné sur la carte (1..20)
    uint32_t coins;
    uint64_t known_recipes;        // bit i = recette i découverte
    uint8_t upgrades;              // bit i = amélioration i achetée
    uint32_t total_served;
    uint32_t best_day_coins;
    uint32_t best_rush;            // meilleur score du mode Rush (tâche 6.7)
    uint8_t special_steps[SPECIAL_MAX];   // avancement de chaque habitué (tâche 4.10)
    uint8_t story;                 // STORY_*
    uint8_t journal_read;          // articles déjà lus dans le Journal
    // Réglages (tâche 8.5), conservés quand on recommence une partie
    uint8_t music_volume;          // 0..4
    uint8_t sfx_volume;            // 0..4
    bool lefty;                    // poste d'assemblage en miroir
} Progress;

Progress *progress(void);
const UpgradeInfo *upgrade_info(Upgrade u);

void progress_new(void);          // remise à zéro complète (réglages par défaut)
void progress_new_game(void);     // nouvelle partie : garde réglages et record Rush
void progress_erase(void);        // efface la sauvegarde (réglages compris)
bool progress_load(void);         // false = aucune sauvegarde valide (nouvelle partie)
void progress_save(void);
bool progress_has_save(void);

bool progress_recipe_known(int recipe);
void progress_learn_recipe(int recipe);
int  progress_known_count(void);
int  progress_known_in_world(int world, int *total);

bool progress_has_upgrade(Upgrade u);
bool progress_buy_upgrade(Upgrade u);   // false si déjà acheté ou trop cher

// Niveaux et mondes
bool progress_level_unlocked(int level);     // niveau 1 toujours ; sinon le précédent est terminé
bool progress_world_unlocked(int world);
int  progress_furthest_level(void);          // plus haut niveau débloqué
int  progress_total_stars(void);
int  progress_world_stars(int world);
// Enregistre le résultat d'un niveau ; renvoie true si c'est la 1re réussite
bool progress_record_level(int level, int stars);
bool progress_game_finished(void);           // dernier niveau terminé

// Sérialisation (testable sans plateforme)
#define SAVE_SIZE 96
void progress_serialize(const Progress *p, uint8_t out[SAVE_SIZE]);
bool progress_deserialize(Progress *p, const uint8_t in[SAVE_SIZE]);

#endif
