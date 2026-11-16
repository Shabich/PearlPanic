#include "core/scene.h"

#include <stddef.h>

static const Scene *g_current;
static const Scene *g_pending;

void scene_change(const Scene *next)
{
    g_pending = next;
}

void scene_update(void)
{
    if (g_pending) {
        if (g_current && g_current->exit)
            g_current->exit();
        g_current = g_pending;
        g_pending = NULL;
        if (g_current->enter)
            g_current->enter();
    }
    if (g_current && g_current->update)
        g_current->update();
}

const Scene *scene_current(void)
{
    return g_current;
}
