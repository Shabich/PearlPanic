// Générateur pseudo-aléatoire xorshift32 (tâche 1.11)
#ifndef PP_RNG_H
#define PP_RNG_H

#include <stdint.h>

typedef struct {
    uint32_t state;
} Rng;

void rng_seed(Rng *r, uint32_t seed);
uint32_t rng_next(Rng *r);
// Entier dans [0, n)
int rng_range(Rng *r, int n);

#endif
