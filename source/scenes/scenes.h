#ifndef PP_SCENES_H
#define PP_SCENES_H

#include "core/scene.h"

extern const Scene SCENE_TITLE;
extern const Scene SCENE_SHOP;
extern const Scene SCENE_DAY_END;
extern const Scene SCENE_BOOK;
extern const Scene SCENE_OPTIONS;
extern const Scene SCENE_MAP;
extern const Scene SCENE_JOURNAL;
extern const Scene SCENE_CUTSCENE;

// Ouvre le Journal / le Livre en retenant l'écran où revenir
void journal_open_from(const Scene *back);
void book_open_from(const Scene *back, int world);

// Cinématiques
typedef enum {
    CUT_INTRO,         // ouverture du tout premier bar
    CUT_WORLD_2,       // arrivée dans chaque nouveau bar
    CUT_WORLD_3,
    CUT_WORLD_4,
    CUT_WORLD_5,
    CUT_ENDING,        // le succès, les journalistes
    CUT_COUNT
} CutsceneId;
void cutscene_play(CutsceneId id, const Scene *next);

// État partagé entre la boutique et ses écrans annexes (livre, fin de journée)
#include "game/day.h"
#include "game/drink.h"

typedef struct {
    bool in_progress;   // une journée est en cours (retour du livre de recettes)
    Day day;
    Drink cups[2];      // deuxième gobelet avec l'amélioration (tâche 3.12)
    int active_cup;
    int cup_count;
} ShopSession;

static inline Drink *session_drink(ShopSession *s) { return &s->cups[s->active_cup]; }

ShopSession *shop_session(void);

// Démarre une nouvelle journée au prochain passage dans SCENE_SHOP
void shop_begin_day(void);
// Démarre une partie en mode Rush (tâche 6.7) dans le bar du niveau donné
void shop_begin_rush(int level);

#endif
