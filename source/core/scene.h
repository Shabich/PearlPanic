// Machine à états de scènes (tâche 1.2)
#ifndef PP_SCENE_H
#define PP_SCENE_H

typedef struct {
    const char *name;
    void (*enter)(void);
    void (*update)(void);   // logique + préparation de l'affichage, 1 fois par frame
    void (*exit)(void);
} Scene;

// Demande un changement de scène ; appliqué au début de la frame suivante.
void scene_change(const Scene *next);

// Applique un changement en attente puis met à jour la scène courante.
void scene_update(void);

const Scene *scene_current(void);

#endif
