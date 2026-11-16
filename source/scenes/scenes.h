#ifndef PP_SCENES_H
#define PP_SCENES_H

#include "core/scene.h"

extern const Scene SCENE_TITLE;
extern const Scene SCENE_SHOP;
extern const Scene SCENE_DAY_END;
extern const Scene SCENE_BOOK;
extern const Scene SCENE_OPTIONS;

// État partagé entre la boutique et ses écrans annexes (livre, fin de journée)
#include "game/day.h"
#include "game/drink.h"

typedef struct {
    bool in_progress;   // une journée est en cours (retour du livre de recettes)
    Day day;
    Drink drink;
} ShopSession;

ShopSession *shop_session(void);

// Démarre une nouvelle journée au prochain passage dans SCENE_SHOP
void shop_begin_day(void);
// Démarre une partie en mode Rush (tâche 6.7)
void shop_begin_rush(void);

#endif
