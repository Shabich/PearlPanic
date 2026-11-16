// Espèces de clients (tâches 4.1, 4.7) : 6 par monde
#ifndef PP_CUSTOMER_H
#define PP_CUSTOMER_H

#include <stdint.h>

typedef struct {
    const char *name;
    uint8_t patience_pct;    // 100 = normal
    uint8_t tip;             // pourboire de base si boisson excellente
    uint8_t picky;           // score minimal pour être content
    uint16_t voice;          // hauteur de la voix (‰, 1000 = normal)
} CustomerType;

typedef enum {
    // Le Petit Bar
    CT_LAPIN, CT_CHAT, CT_OURS, CT_GRENOUILLE, CT_POUSSIN, CT_PANDA,
    // La Paillote
    CT_PHOQUE, CT_CRABE, CT_PINGOUIN, CT_POULPE, CT_TORTUE, CT_DAUPHIN,
    // Le Chalet
    CT_RENARD, CT_HIBOU, CT_ECUREUIL, CT_MARMOTTE, CT_BICHE, CT_YETI,
    // Le Kiosque Néon
    CT_RATON, CT_PIGEON, CT_CHAUVESOURIS, CT_SOURIS, CT_CARLIN, CT_CHAT_NOIR,
    // L'Arbre aux Thés
    CT_CHAMPIGNON, CT_LUCIOLE, CT_ESCARGOT, CT_PAPILLON, CT_HERISSON, CT_DRAGON,
    CT_COUNT
} CustomerTypeId;

const CustomerType *customer_type(int id);

// Paliers de stress : 0 content, 1 neutre, 2 inquiet, 3 furieux
#define STRESS_LEVELS 4

#endif
