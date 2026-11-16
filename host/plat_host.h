// Plateforme PC pour les tests et les captures : entrées scriptées
#ifndef PP_PLAT_HOST_H
#define PP_PLAT_HOST_H

#include "plat/plat.h"

// Entrée brute renvoyée par le prochain plat_poll()
void host_set_input(const RawInput *in);
// Écrit les deux écrans empilés (256x384) en PPM
bool host_write_ppm(const char *path);
// Dernier effet sonore joué (pour les tests), -1 si aucun
int host_last_sfx(void);

#endif
