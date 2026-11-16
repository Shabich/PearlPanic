// Configuration vidéo des deux écrans (tâche 1.4)
//
// Écran du haut  = moteur principal (main) : comptoir et clients.
// Écran du bas   = moteur secondaire (sub), tactile : poste d'assemblage.
//
// Banques VRAM :
//   A (128 Ko) fonds main   B (128 Ko) sprites main
//   C (128 Ko) fonds sub    D (128 Ko) sprites sub
//   E–I libres (palettes étendues, son, etc. plus tard)
#ifndef PP_VIDEO_H
#define PP_VIDEO_H

#include <nds.h>

typedef enum { SCREEN_TOP = 0, SCREEN_BOTTOM = 1 } Screen;

void video_init(void);

// Couleur de fond (backdrop) d'un écran
void video_set_backdrop(Screen s, u16 color);

// Console texte de debug sur BG0 de chaque écran (remplacée par la tâche 1.8)
PrintConsole *video_console(Screen s);

// Moteur OAM associé à un écran
OamState *video_oam(Screen s);

#endif
