#include "core/input.h"

#include <stdlib.h>

#define DRAG_THRESHOLD_PX  6
#define TAP_MAX_FRAMES     20

static Input g_in;

void input_feed(const RawInput *raw)
{
    uint32_t prev = g_in.held;
    g_in.held = raw->keys;
    g_in.down = raw->keys & ~prev;
    g_in.up   = prev & ~raw->keys;

    bool was_touching = g_in.touching;
    g_in.touching   = raw->touching;
    g_in.touch_down = g_in.touching && !was_touching;
    g_in.touch_up   = !g_in.touching && was_touching;
    g_in.tapped     = false;
    g_in.dx = g_in.dy = 0;

    if (g_in.touching) {
        if (g_in.touch_down) {
            g_in.start_x = g_in.x = raw->tx;
            g_in.start_y = g_in.y = raw->ty;
            g_in.touch_frames = 0;
            g_in.dragging = false;
        } else {
            g_in.dx = raw->tx - g_in.x;
            g_in.dy = raw->ty - g_in.y;
            g_in.x = raw->tx;
            g_in.y = raw->ty;
        }
        g_in.touch_frames++;
        if (abs(g_in.x - g_in.start_x) > DRAG_THRESHOLD_PX ||
            abs(g_in.y - g_in.start_y) > DRAG_THRESHOLD_PX)
            g_in.dragging = true;
    } else if (g_in.touch_up) {
        g_in.tapped = !g_in.dragging && g_in.touch_frames <= TAP_MAX_FRAMES;
    } else {
        g_in.dragging = false;
    }
}

const Input *input_get(void)
{
    return &g_in;
}

static bool in_rect(int px, int py, int x, int y, int w, int h)
{
    return px >= x && px < x + w && py >= y && py < y + h;
}

bool input_touch_in(int x, int y, int w, int h)
{
    return g_in.touching && in_rect(g_in.x, g_in.y, x, y, w, h);
}

bool input_pressed_in(int x, int y, int w, int h)
{
    return g_in.touch_down && in_rect(g_in.x, g_in.y, x, y, w, h);
}

bool input_tapped_in(int x, int y, int w, int h)
{
    return g_in.tapped && in_rect(g_in.start_x, g_in.start_y, x, y, w, h);
}
