// Types de clients (tâches 4.1, 4.7)
#ifndef PP_CUSTOMER_H
#define PP_CUSTOMER_H

#include <stdint.h>

typedef struct {
    const char *name;
    uint8_t patience_pct;    // 100 = normal
    uint8_t tip;             // pourboire de base si boisson excellente
    uint8_t picky;           // score minimal pour être content
} CustomerType;

typedef enum {
    CT_LAPIN,
    CT_CHAT,
    CT_OURS,
    CT_GRENOUILLE,
    CT_POUSSIN,
    CT_PANDA,
    CT_COUNT
} CustomerTypeId;

const CustomerType *customer_type(int id);

// Paliers de stress : 0 content, 1 neutre, 2 inquiet, 3 furieux
#define STRESS_LEVELS 4

#endif
