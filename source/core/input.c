#include "core/input.h"

#include <stdlib.h>

#include <nds.h>

#define DRAG_THRESHOLD_PX  6
#define TAP_MAX_FRAMES     15

static Input g_in;

void input_update(void)
{
    scanKeys();
    g_in.held = keysHeld();
    g_in.down = keysDown();
    g_in.up   = keysUp();

    bool was_touching = g_in.touching;
    g_in.touching   = (g_in.held & KEY_TOUCH) != 0;
    g_in.touch_down = g_in.touching && !was_touching;
    g_in.touch_up   = !g_in.touching && was_touching;
    g_in.tapped     = false;
    g_in.dx = g_in.dy = 0;

    if (g_in.touching) {
        touchPosition tp;
        touchRead(&tp);
        if (g_in.touch_down) {
            g_in.start_x = g_in.x = tp.px;
            g_in.start_y = g_in.y = tp.py;
            g_in.touch_frames = 0;
            g_in.dragging = false;
        } else {
            g_in.dx = tp.px - g_in.x;
            g_in.dy = tp.py - g_in.y;
            g_in.x = tp.px;
            g_in.y = tp.py;
        }
        g_in.touch_frames++;
        if (abs(g_in.x - g_in.start_x) > DRAG_THRESHOLD_PX ||
            abs(g_in.y - g_in.start_y) > DRAG_THRESHOLD_PX)
            g_in.dragging = true;
    } else if (g_in.touch_up) {
        g_in.tapped = !g_in.dragging && g_in.touch_frames <= TAP_MAX_FRAMES;
        g_in.dragging = false;
    }
}

const Input *input_get(void)
{
    return &g_in;
}

bool input_touch_in(int x, int y, int w, int h)
{
    return g_in.touching &&
           g_in.x >= x && g_in.x < x + w &&
           g_in.y >= y && g_in.y < y + h;
}
