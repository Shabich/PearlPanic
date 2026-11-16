#include "game/customer.h"

// nom, patience %, pourboire, exigence, voix
static const CustomerType TYPES[CT_COUNT] = {
    [CT_LAPIN]       = { "Lapin",        80, 3, 60, 1300 },   // pressé mais généreux
    [CT_CHAT]        = { "Chat",        100, 2, 80, 1150 },   // exigeant
    [CT_OURS]        = { "Ours",        130, 1, 50, 700 },    // très patient
    [CT_GRENOUILLE]  = { "Grenouille",  100, 2, 60, 900 },
    [CT_POUSSIN]     = { "Poussin",      70, 2, 50, 1550 },   // très impatient
    [CT_PANDA]       = { "Panda",       115, 3, 70, 820 },
    [CT_PHOQUE]      = { "Phoque",      120, 2, 55, 760 },
    [CT_CRABE]       = { "Crabe",        75, 2, 75, 1250 },
    [CT_PINGOUIN]    = { "Pingouin",     90, 3, 65, 1100 },
    [CT_POULPE]      = { "Poulpe",      105, 2, 70, 880 },
    [CT_TORTUE]      = { "Tortue",      145, 1, 50, 650 },
    [CT_DAUPHIN]     = { "Dauphin",      85, 3, 60, 1450 },
    [CT_RENARD]      = { "Renard",       90, 3, 75, 1050 },
    [CT_HIBOU]       = { "Hibou",       125, 2, 80, 800 },
    [CT_ECUREUIL]    = { "Écureuil",     70, 2, 55, 1500 },
    [CT_MARMOTTE]    = { "Marmotte",    135, 1, 50, 900 },
    [CT_BICHE]       = { "Biche",       100, 3, 65, 1200 },
    [CT_YETI]        = { "Yéti",        110, 2, 60, 600 },
    [CT_RATON]       = { "Raton",        85, 3, 65, 1000 },
    [CT_PIGEON]      = { "Pigeon",       75, 1, 50, 1100 },
    [CT_CHAUVESOURIS]= { "Chauve-souris", 90, 2, 70, 1600 },
    [CT_SOURIS]      = { "Souris",       80, 2, 60, 1650 },
    [CT_CARLIN]      = { "Carlin",      115, 2, 55, 780 },
    [CT_CHAT_NOIR]   = { "Chat noir",   100, 3, 85, 1150 },
    [CT_CHAMPIGNON]  = { "Champignon",  130, 2, 55, 850 },
    [CT_LUCIOLE]     = { "Luciole",      75, 3, 65, 1700 },
    [CT_ESCARGOT]    = { "Escargot",    160, 1, 50, 620 },
    [CT_PAPILLON]    = { "Papillon",     85, 3, 75, 1400 },
    [CT_HERISSON]    = { "Hérisson",     95, 2, 60, 1250 },
    [CT_DRAGON]      = { "Dragonnet",   110, 3, 80, 720 },
};

const CustomerType *customer_type(int id)
{
    if (id < 0 || id >= CT_COUNT)
        id = 0;
    return &TYPES[id];
}
