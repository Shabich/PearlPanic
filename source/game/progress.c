#include "game/progress.h"

#include <string.h>

#include "game/recipes.h"
#include "plat/plat.h"

#define SAVE_MAGIC   0x50504E43u   // "PPNC"
#define SAVE_VERSION 1

static Progress g_progress;
static bool g_has_save;

static const UpgradeInfo UPGRADES[UPG_COUNT] = {
    [UPG_FAST_TAP] = { "Tireuse turbo",    "Le thé coule 2x plus vite.",          25 },
    [UPG_CUSHIONS] = { "Coussins doux", "Les clients patientent +25 %.",     35 },
    [UPG_GARLANDS] = { "Guirlandes",       "L'ambiance baisse 2x moins vite.",    45 },
    [UPG_TIP_JAR]  = { "Pot à pièces", "+1 pièce par pourboire.",             30 },
};

Progress *progress(void) { return &g_progress; }

const UpgradeInfo *upgrade_info(Upgrade u) { return &UPGRADES[u]; }

void progress_new(void)
{
    memset(&g_progress, 0, sizeof(g_progress));
    g_progress.day = 1;
    g_progress.known_recipes = 1u << 0;   // le classique est connu d'office
}

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static uint32_t get32(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint32_t checksum(const uint8_t *p, int n)
{
    uint32_t h = 2166136261u;     // FNV-1a
    for (int i = 0; i < n; i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}

void progress_serialize(const Progress *p, uint8_t out[SAVE_SIZE])
{
    memset(out, 0, SAVE_SIZE);
    put32(out + 0, SAVE_MAGIC);
    out[4] = SAVE_VERSION;
    out[5] = p->upgrades;
    out[6] = p->day & 0xFF;
    out[7] = p->day >> 8;
    put32(out + 8, p->coins);
    put32(out + 12, p->known_recipes);
    put32(out + 16, p->total_served);
    put32(out + 20, p->best_day_coins);
    put32(out + 28, checksum(out, 28));
}

bool progress_deserialize(Progress *p, const uint8_t in[SAVE_SIZE])
{
    if (get32(in) != SAVE_MAGIC || in[4] != SAVE_VERSION)
        return false;
    if (get32(in + 28) != checksum(in, 28))
        return false;
    p->upgrades = in[5];
    p->day = in[6] | (in[7] << 8);
    p->coins = get32(in + 8);
    p->known_recipes = get32(in + 12) | 1u;
    p->total_served = get32(in + 16);
    p->best_day_coins = get32(in + 20);
    if (p->day == 0)
        p->day = 1;
    return true;
}

bool progress_load(void)
{
    uint8_t buf[SAVE_SIZE];
    progress_new();
    g_has_save = plat_save_read(buf, SAVE_SIZE) && progress_deserialize(&g_progress, buf);
    if (!g_has_save)
        progress_new();
    return g_has_save;
}

void progress_save(void)
{
    uint8_t buf[SAVE_SIZE];
    progress_serialize(&g_progress, buf);
    if (plat_save_write(buf, SAVE_SIZE))
        g_has_save = true;
}

bool progress_has_save(void) { return g_has_save; }

bool progress_recipe_known(int r)
{
    return r >= 0 && r < 32 && (g_progress.known_recipes >> r) & 1;
}

void progress_learn_recipe(int r)
{
    if (r >= 0 && r < 32)
        g_progress.known_recipes |= 1u << r;
}

int progress_known_count(void)
{
    int n = 0;
    for (int i = 0; i < RECIPE_COUNT; i++)
        n += progress_recipe_known(i);
    return n;
}

bool progress_has_upgrade(Upgrade u)
{
    return (g_progress.upgrades >> u) & 1;
}

bool progress_buy_upgrade(Upgrade u)
{
    if (progress_has_upgrade(u) || g_progress.coins < UPGRADES[u].cost)
        return false;
    g_progress.coins -= UPGRADES[u].cost;
    g_progress.upgrades |= 1u << u;
    return true;
}
