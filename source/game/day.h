// Une journée en boutique : file de clients, stress, ambiance, service
// (tâches 4.2–4.5, 4.9, 5.3, 5.4, 6.1, 6.4)
#ifndef PP_DAY_H
#define PP_DAY_H

#include <stdbool.h>
#include <stdint.h>

#include "core/rng.h"
#include "game/customer.h"
#include "game/drink.h"
#include "game/objectives.h"
#include "game/specials.h"

#define FPS              60
#define DAY_SLOTS        3
#define DAY_DURATION     (150 * FPS)   // 2 min 30
#define AMBIANCE_MAX     1000
#define AMBIANCE_ANGRY_LEAVE 100
#define ARRIVE_FRAMES    30
#define LEAVE_FRAMES     40
#define MAX_EVENTS       8
#define MAX_UNLOCKS      8
#define SPECIAL_ARRIVAL  (15 * FPS)   // le 1er habitué arrive après 15 s
#define SPECIAL_ARRIVAL2 (60 * FPS)   // le 2e (s'il y en a un) après 1 min

typedef enum {
    CS_EMPTY,
    CS_ARRIVING,
    CS_WAITING,
    CS_LEAVING,
} CustomerState;

typedef struct {
    CustomerState state;
    uint8_t type;
    int8_t recipe;
    bool riddle;          // commande énigmatique (recette inconnue)
    bool happy;           // en partant
    int32_t patience;     // frames restantes
    int32_t patience_max;
    int16_t anim;         // frames passées dans l'état courant
    uint16_t seq;         // ordre d'arrivée
    int8_t last_score;    // -1 = pas servi
    int8_t special;       // habitué (index SPECIALS), -1 sinon
    const char *says;     // réplique en partant (peut être NULL)
    char says_buf[48];
} Customer;

typedef enum {
    EV_ARRIVE,
    EV_STRESS_UP,
    EV_LEAVE_ANGRY,
    EV_SERVED,
    EV_UNLOCK,
    EV_DAY_END,
    EV_SPECIAL_DONE,      // visite d'un habitué réussie (value = étape atteinte)
} DayEventType;

typedef struct {
    uint8_t type;
    int8_t slot;
    int16_t value;
} DayEvent;

typedef struct {
    int score;
    int coins;
    int tip;
    int unlocked;         // recette découverte, -1 sinon
    bool happy;
    const char *hint;     // pourquoi ce n'était pas parfait (NULL si parfait)
} ServeResult;

typedef struct Day {
    int number;           // niveau global 1..20
    int world;            // bar (0..4)
    int level;            // niveau dans le monde (0..3)
    bool endless;         // mode Rush : pas d'horloge, finit quand l'ambiance tombe à 0
    Objective objective;  // objectif du niveau (OBJ_NONE en mode Rush)
    bool objective_met;   // calculé en fin de journée
    int stars;            // 0..3, calculé en fin de journée
    int8_t specials[SPECIALS_PER_LEVEL];       // habitués attendus (-1 = aucun)
    bool special_spawned[SPECIALS_PER_LEVEL];
    int8_t special_result[SPECIALS_PER_LEVEL]; // -1 pas encore venu, 0 raté, 1 réussi
    uint32_t frame;
    int32_t spawn_timer;
    int ambiance;
    Customer slots[DAY_SLOTS];
    int active;           // client sélectionné (-1 = aucun)
    uint16_t next_seq;
    int served, perfect, lost;
    int coins, tips;
    int8_t unlocked[MAX_UNLOCKS];
    int unlocked_count;
    bool ended;
    bool collapsed;       // fin anticipée : ambiance à zéro
    Rng rng;
    DayEvent events[MAX_EVENTS];
    int event_count;
    char hint_buf[48];
} Day;

// Démarre le niveau global `level` (1..20)
void day_start(Day *d, int level, uint32_t seed);
// Mode Rush (tâche 6.7) dans le bar du niveau donné : recettes connues, cadence qui accélère
void day_start_rush(Day *d, int level, uint32_t seed);

// Étoiles d'un niveau terminé : 1 = terminé, 2 = objectif atteint, 3 = aucun client
// perdu et ambiance ≥ 60 %. 0 si l'ambiance s'est effondrée.
int day_compute_stars(const Day *d);

// Avance d'une frame. Les événements de la frame sont dans d->events.
void day_update(Day *d);

// Sert la boisson au client actif. false si aucun client n'attend.
bool day_serve(Day *d, const Drink *drink, ServeResult *out);

// Change de client actif (dir = -1 / +1), parmi ceux qui attendent
void day_select(Day *d, int dir);
void day_select_slot(Day *d, int slot);

// Palier de stress 0..3 d'un client
int day_stress_level(const Customer *c);

// Nombre de clients max en même temps et intervalle d'arrivée (difficulté)
int day_max_customers(int number);
int day_spawn_interval(int number);
int day_base_patience(int number);

uint32_t day_time_left(const Day *d);

// Mode Rush : intervalle d'arrivée et patience selon le temps écoulé
int rush_spawn_interval(uint32_t frame);
int rush_patience(int number, uint32_t frame);

#endif
