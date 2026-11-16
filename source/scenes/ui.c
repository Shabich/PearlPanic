#include "scenes/ui.h"

#include "gfx/font.h"
#include "gfx/palette.h"

uint32_t hash_begin(void) { return 2166136261u; }

uint32_t hash_add(uint32_t h, int32_t v)
{
    for (int i = 0; i < 4; i++)
        h = (h ^ ((uint32_t)v >> (i * 8) & 0xFF)) * 16777619u;
    return h;
}

uint32_t hash_ptr(uint32_t h, const void *p)
{
    return hash_add(h, (int32_t)(uintptr_t)p);
}

void watch(ScreenId id, Watch *w, Rect r, uint32_t h)
{
    if (!w->valid || w->last != h)
        screen_dirty(id, r);
    w->last = h;
    w->valid = true;
}

void watch_reset(Watch *w) { w->valid = false; }

void ui_button(Surface *s, Rect r, const char *label, ButtonStyle style)
{
    uint16_t fill, border, text;
    switch (style) {
    case BTN_STYLE_PRIMARY:  fill = COL_PINK;       border = COL_PINK_DARK; text = COL_INK; break;
    case BTN_STYLE_DISABLED: fill = COL_GREY;       border = COL_GREY_DARK; text = COL_GREY_DARK; break;
    case BTN_STYLE_PRESSED:  fill = COL_PINK_DARK;  border = COL_INK;       text = COL_WHITE; break;
    default:                 fill = COL_CREAM;      border = COL_WOOD_DARK; text = COL_INK; break;
    }
    int press = style == BTN_STYLE_PRESSED ? 1 : 0;
    if (!press)
        draw_round_rect(s, r.x + 1, r.y + 2, r.w, r.h, color_darken(border, 4), color_darken(border, 4));
    draw_round_rect(s, r.x + press, r.y + press, r.w, r.h, fill, border);
    draw_hline(s, r.x + 2 + press, r.y + 1 + press, r.w - 4, color_mix(fill, COL_WHITE, 128));
    if (label) {
        int lines = text_count_lines(r.w - 4, label);
        int y = r.y + press + (r.h - lines * FONT_LINE) / 2;
        text_draw_wrapped(s, r.x + 2 + press, y, r.w - 4, label, text, ALIGN_CENTER);
    }
}

void ui_gauge(Surface *s, int x, int y, int w, int h, int value, int max, uint16_t color)
{
    draw_rect(s, x, y, w, h, COL_INK);
    draw_rect(s, x + 1, y + 1, w - 2, h - 2, COL_CREAM_DARK);
    if (max <= 0)
        return;
    if (value < 0)
        value = 0;
    if (value > max)
        value = max;
    int fill = (w - 2) * value / max;
    draw_rect(s, x + 1, y + 1, fill, h - 2, color);
    if (h > 3)
        draw_hline(s, x + 1, y + 1, fill, color_mix(color, COL_WHITE, 110));
}

void ui_bubble(Surface *s, Rect r, uint16_t fill, uint16_t border)
{
    draw_round_rect(s, r.x, r.y, r.w, r.h, fill, border);
    int cx = r.x + r.w / 2;
    int by = r.y + r.h - 1;
    for (int i = 0; i < 4; i++) {
        draw_hline(s, cx - 4 + i, by + i, 9 - 2 * i, fill);
        draw_pixel(s, cx - 5 + i, by + i, border);
        draw_pixel(s, cx + 5 - i, by + i, border);
    }
    draw_pixel(s, cx, by + 4, border);
}

uint16_t ui_patience_color(int ratio256)
{
    if (ratio256 > 128)
        return color_mix(COL_WARN, COL_GOOD, (ratio256 - 128) * 2);
    return color_mix(COL_BAD, COL_WARN, ratio256 * 2);
}

void ui_polka_background(Surface *s, uint16_t base, uint16_t dots)
{
    draw_fill(s, base);
    for (int y = 6; y < s->h; y += 16)
        for (int x = ((y / 16) & 1) * 8 + 4; x < s->w; x += 16)
            draw_disc(s, x, y, 1, dots);
}
