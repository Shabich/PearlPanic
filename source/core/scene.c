#include "core/scene.h"

#include <stddef.h>

#include "plat/plat.h"

// Fondu au blanc entre deux scènes (tâche 1.10)
#define FADE_FRAMES 8

static const Scene *g_current;
static const Scene *g_pending;
static int g_fade_out;   // frames écoulées du fondu sortant (0 = aucun)
static int g_fade_in;    // frames restantes du fondu entrant

void scene_change(const Scene *next)
{
    g_pending = next;
    if (g_current && !g_fade_out)
        g_fade_out = 1;
}

bool scene_transitioning(void)
{
    return g_fade_out > 0;
}

static void switch_now(void)
{
    if (g_current && g_current->exit)
        g_current->exit();
    g_current = g_pending;
    g_pending = NULL;
    if (g_current->enter)
        g_current->enter();
}

void scene_update(void)
{
    if (g_pending && !g_current) {
        switch_now();
    } else if (g_fade_out) {
        plat_brightness(16 * g_fade_out / FADE_FRAMES);
        if (g_fade_out++ < FADE_FRAMES)
            return;   // la scène sortante est figée pendant le fondu
        g_fade_out = 0;
        switch_now();
        g_fade_in = FADE_FRAMES;
    }
    if (g_fade_in) {
        g_fade_in--;
        plat_brightness(16 * g_fade_in / FADE_FRAMES);
    }
    if (g_current && g_current->update)
        g_current->update();
}

const Scene *scene_current(void)
{
    return g_current;
}
