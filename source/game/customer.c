#include "game/customer.h"

static const CustomerType TYPES[CT_COUNT] = {
    [CT_LAPIN]      = { "Lapin",      80, 3, 60, 1300 },   // pressé mais généreux
    [CT_CHAT]       = { "Chat",       100, 2, 80, 1150 },  // exigeant
    [CT_OURS]       = { "Ours",       130, 1, 50, 700 },  // très patient
    [CT_GRENOUILLE] = { "Grenouille", 100, 2, 60, 900 },
    [CT_POUSSIN]    = { "Poussin",    70, 2, 50, 1550 },   // très impatient
    [CT_PANDA]      = { "Panda",      115, 3, 70, 820 },
};

const CustomerType *customer_type(int id)
{
    if (id < 0 || id >= CT_COUNT)
        id = 0;
    return &TYPES[id];
}
