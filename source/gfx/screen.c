#include "gfx/screen.h"

#include "plat/plat.h"

#define MAX_DIRTY 24

typedef struct {
    Surface bg;
    DynamicDrawFn draw;
    Rect dirty[MAX_DIRTY];
    int dirty_count;
    bool all_dirty;
} ScreenState;

static uint16_t g_bg_pixels[2][SCREEN_W * SCREEN_H];
static ScreenState g_screens[2];

void screen_init(void)
{
    for (int i = 0; i < 2; i++) {
        surface_init(&g_screens[i].bg, g_bg_pixels[i], SCREEN_W, SCREEN_H, SCREEN_W);
        g_screens[i].draw = 0;
        g_screens[i].dirty_count = 0;
        g_screens[i].all_dirty = true;
    }
}

Surface *screen_background(ScreenId id)
{
    surface_reset_clip(&g_screens[id].bg);
    return &g_screens[id].bg;
}

void screen_set_dynamic(ScreenId id, DynamicDrawFn fn)
{
    g_screens[id].draw = fn;
    g_screens[id].all_dirty = true;
}

void screen_dirty_all(ScreenId id)
{
    g_screens[id].all_dirty = true;
}

void screen_dirty(ScreenId id, Rect r)
{
    ScreenState *s = &g_screens[id];
    r = rect_intersect(r, rect_make(0, 0, SCREEN_W, SCREEN_H));
    if (rect_empty(r) || s->all_dirty)
        return;
    // Fusionne avec une zone existante qui chevauche
    for (int i = 0; i < s->dirty_count; i++) {
        if (!rect_empty(rect_intersect(s->dirty[i], r))) {
            s->dirty[i] = rect_union(s->dirty[i], r);
            return;
        }
    }
    if (s->dirty_count == MAX_DIRTY) {
        s->all_dirty = true;
        return;
    }
    s->dirty[s->dirty_count++] = r;
}

static void compose(Surface *front, ScreenState *s, Rect r)
{
    surface_copy_rect(front, &s->bg, r);
    if (s->draw) {
        front->clip = r;
        s->draw(front);
        surface_reset_clip(front);
    }
}

void screen_present(void)
{
    for (int i = 0; i < 2; i++) {
        ScreenState *s = &g_screens[i];
        Surface *front = plat_screen(i);
        if (s->all_dirty) {
            compose(front, s, rect_make(0, 0, SCREEN_W, SCREEN_H));
        } else {
            for (int k = 0; k < s->dirty_count; k++)
                compose(front, s, s->dirty[k]);
        }
        s->all_dirty = false;
        s->dirty_count = 0;
    }
}

void screen_track(ScreenId id, Tracked *t, Rect now, bool visible, bool force)
{
    bool moved = !t->shown || t->last.x != now.x || t->last.y != now.y ||
                 t->last.w != now.w || t->last.h != now.h;
    if (t->shown && (moved || !visible || force))
        screen_dirty(id, t->last);
    if (visible && (moved || force || !t->shown))
        screen_dirty(id, now);
    t->last = now;
    t->shown = visible;
}
