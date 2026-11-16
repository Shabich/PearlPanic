#include "game/progress.h"

#include <string.h>

#include "game/recipes.h"
#include "plat/plat.h"

#define SAVE_MAGIC   0x50504E43u   // "PPNC"
#define SAVE_VERSION 3

static Progress g_progress;
static bool g_has_save;

static const UpgradeInfo UPGRADES[UPG_COUNT] = {
    [UPG_FAST_TAP]   = { "Tireuse turbo", "Le thé coule 2x plus vite.",          25 },
    [UPG_CUSHIONS]   = { "Coussins doux", "Les clients patientent +25 %.",       35 },
    [UPG_GARLANDS]   = { "Guirlandes",    "L'ambiance baisse 2x moins vite.",    45 },
    [UPG_TIP_JAR]    = { "Pot à pièces",  "+1 pièce par pourboire.",             30 },
    [UPG_SECOND_CUP] = { "2e gobelet",    "Deux boissons à la fois.",            60 },
};

Progress *progress(void) { return &g_progress; }

const UpgradeInfo *upgrade_info(Upgrade u) { return &UPGRADES[u]; }

void progress_new(void)
{
    memset(&g_progress, 0, sizeof(g_progress));
    g_progress.level = 1;
    g_progress.known_recipes = 1u << 0;   // le classique est connu d'office
    g_progress.music_volume = 3;
    g_progress.sfx_volume = 4;
}

void progress_new_game(void)
{
    Progress keep = g_progress;
    progress_new();
    g_progress.music_volume = keep.music_volume;
    g_progress.sfx_volume = keep.sfx_volume;
    g_progress.lefty = keep.lefty;
    g_progress.best_rush = keep.best_rush;
}

void progress_erase(void)
{
    progress_new();
    progress_save();
    g_has_save = false;
}

// ------------------------------------------------------------------ Sauvegarde

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
    out[6] = p->level;
    out[7] = p->story;
    put32(out + 8, p->coins);
    put32(out + 12, (uint32_t)p->known_recipes);
    put32(out + 16, (uint32_t)(p->known_recipes >> 32));
    put32(out + 20, p->total_served);
    put32(out + 24, p->best_day_coins);
    put32(out + 28, p->best_rush);
    memcpy(out + 32, p->stars, LEVEL_COUNT);
    memcpy(out + 52, p->special_steps, SPECIAL_MAX);
    out[60] = p->music_volume;
    out[61] = p->sfx_volume;
    out[62] = p->lefty ? 1 : 0;
    out[63] = p->journal_read;
    put32(out + SAVE_SIZE - 4, checksum(out, SAVE_SIZE - 4));
}

bool progress_deserialize(Progress *p, const uint8_t in[SAVE_SIZE])
{
    if (get32(in) != SAVE_MAGIC || in[4] != SAVE_VERSION)
        return false;
    if (get32(in + SAVE_SIZE - 4) != checksum(in, SAVE_SIZE - 4))
        return false;
    memset(p, 0, sizeof(*p));
    p->upgrades = in[5];
    p->level = in[6] >= 1 && in[6] <= LEVEL_COUNT ? in[6] : 1;
    p->story = in[7];
    p->coins = get32(in + 8);
    p->known_recipes = get32(in + 12) | ((uint64_t)get32(in + 16) << 32) | 1u;
    p->total_served = get32(in + 20);
    p->best_day_coins = get32(in + 24);
    p->best_rush = get32(in + 28);
    for (int i = 0; i < LEVEL_COUNT; i++)
        p->stars[i] = in[32 + i] > 3 ? 3 : in[32 + i];
    for (int i = 0; i < SPECIAL_MAX; i++)
        p->special_steps[i] = in[52 + i] > SPECIAL_VISITS ? SPECIAL_VISITS : in[52 + i];
    p->music_volume = in[60] > 4 ? 4 : in[60];
    p->sfx_volume = in[61] > 4 ? 4 : in[61];
    p->lefty = in[62] & 1;
    p->journal_read = in[63];
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

// ------------------------------------------------------------------ Recettes

bool progress_recipe_known(int r)
{
    return r >= 0 && r < RECIPE_MAX && (g_progress.known_recipes >> r) & 1;
}

void progress_learn_recipe(int r)
{
    if (r >= 0 && r < RECIPE_MAX)
        g_progress.known_recipes |= (uint64_t)1 << r;
}

int progress_known_count(void)
{
    int n = 0;
    for (int i = 0; i < RECIPE_COUNT; i++)
        n += progress_recipe_known(i);
    return n;
}

int progress_known_in_world(int world, int *total)
{
    int n = 0, t = 0;
    for (int i = 0; i < RECIPE_COUNT; i++) {
        if (RECIPES[i].world != world)
            continue;
        t++;
        n += progress_recipe_known(i);
    }
    if (total)
        *total = t;
    return n;
}

// ------------------------------------------------------------------ Améliorations

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

// ------------------------------------------------------------------ Niveaux

bool progress_level_unlocked(int level)
{
    if (level <= 1)
        return true;
    if (level > LEVEL_COUNT)
        return false;
    return g_progress.stars[level - 2] > 0;
}

bool progress_world_unlocked(int world)
{
    return progress_level_unlocked(level_id(world, 0));
}

int progress_furthest_level(void)
{
    int l = 1;
    while (l < LEVEL_COUNT && progress_level_unlocked(l + 1))
        l++;
    return l;
}

int progress_total_stars(void)
{
    int n = 0;
    for (int i = 0; i < LEVEL_COUNT; i++)
        n += g_progress.stars[i];
    return n;
}

int progress_world_stars(int world)
{
    int n = 0;
    for (int i = 0; i < LEVELS_PER_WORLD; i++)
        n += g_progress.stars[world * LEVELS_PER_WORLD + i];
    return n;
}

bool progress_record_level(int level, int stars)
{
    if (level < 1 || level > LEVEL_COUNT)
        return false;
    uint8_t *s = &g_progress.stars[level - 1];
    bool first = *s == 0 && stars > 0;
    if (stars > *s)
        *s = (uint8_t)stars;
    return first;
}

bool progress_game_finished(void)
{
    return g_progress.stars[LEVEL_COUNT - 1] > 0;
}
