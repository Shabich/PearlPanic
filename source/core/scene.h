// Machine à états de scènes (tâche 1.2)
#ifndef PP_SCENE_H
#define PP_SCENE_H

#include <stdbool.h>

typedef struct {
    const char *name;
    void (*enter)(void);
    void (*update)(void);   // logique + préparation de l'affichage, 1 fois par frame
    void (*exit)(void);
} Scene;

// Demande un changement de scène : fondu au blanc, puis bascule.
void scene_change(const Scene *next);

// Applique un changement en attente puis met à jour la scène courante.
void scene_update(void);

const Scene *scene_current(void);

// Vrai pendant le fondu sortant (la scène courante n'est plus mise à jour)
bool scene_transitioning(void);

#endif
