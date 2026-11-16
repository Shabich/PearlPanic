#include "gfx/font.h"

#include <stdbool.h>
#include <stddef.h>

// Décode un caractère UTF-8 en code Latin-1 ('?' si hors plage)
static int next_char(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    int c = s[0];
    if (c < 0x80) {
        *p += 1;
        return c;
    }
    if ((c & 0xE0) == 0xC0 && s[1]) {
        int cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        *p += 2;
        return cp <= 0xFF ? cp : '?';
    }
    // Séquences plus longues : on saute
    int len = (c & 0xF0) == 0xE0 ? 3 : (c & 0xF8) == 0xF0 ? 4 : 1;
    for (int i = 1; i < len; i++)
        if (!s[i]) { len = i; break; }
    *p += len;
    // Quelques remplacements utiles
    if (c == 0xE2 && s[1] == 0x80) {
        switch (s[2]) {
        case 0xA6: return 0x85;  // « … » : rendu en trois points
        case 0x93:               // tiret demi-cadratin
        case 0x94: return '-';   // tiret cadratin
        case 0x98:
        case 0x99: return '\''; // apostrophes typographiques
        case 0x9C:
        case 0x9D: return '"';
        }
    }
    return '?';
}

static void draw_glyph(Surface *s, int x, int y, int code, uint16_t color, int scale)
{
    if (code < 0x20 || code > 0xFF)
        code = '?';
    const uint8_t *g = FONT_DATA + (code - 0x20) * FONT_H;
    for (int row = 0; row < FONT_H; row++) {
        uint8_t bits = g[row];
        for (int col = 0; bits; col++, bits >>= 1) {
            if (!(bits & 1))
                continue;
            if (scale == 1)
                draw_pixel(s, x + col, y + row, color);
            else
                draw_rect(s, x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}

int text_draw(Surface *s, int x, int y, const char *str, uint16_t color, int scale)
{
    int x0 = x;
    while (*str && *str != '\n') {
        int c = next_char(&str);
        if (c == 0x85) {  // points de suspension
            for (int i = 0; i < 3; i++, x += FONT_W * scale)
                draw_glyph(s, x, y, '.', color, scale);
            continue;
        }
        draw_glyph(s, x, y, c, color, scale);
        x += FONT_W * scale;
    }
    return x - x0;
}

int text_draw_shadow(Surface *s, int x, int y, const char *str, uint16_t color,
                     uint16_t shadow, int scale)
{
    text_draw(s, x + scale, y + scale, str, shadow, scale);
    return text_draw(s, x, y, str, color, scale);
}

int text_width(const char *str, int scale)
{
    int n = 0;
    while (*str && *str != '\n') {
        int c = next_char(&str);
        n += c == 0x85 ? 3 : 1;
    }
    return n * FONT_W * scale;
}

void text_draw_aligned(Surface *s, int x, int y, int w, const char *str,
                       uint16_t color, TextAlign align)
{
    int tw = text_width(str, 1);
    if (align == ALIGN_CENTER)
        x += (w - tw) / 2;
    else if (align == ALIGN_RIGHT)
        x += w - tw;
    text_draw(s, x, y, str, color, 1);
}

// Découpe la ligne suivante qui tient dans `max_chars` caractères.
// Renvoie la longueur en octets de la ligne et avance *p après l'espace/\n.
static size_t wrap_next(const char **p, int max_chars)
{
    const char *s = *p;
    const char *last_break = NULL;
    const char *cur = s;
    int count = 0;
    while (*cur && *cur != '\n') {
        if (*cur == ' ')
            last_break = cur;
        const char *before = cur;
        next_char(&cur);
        count++;
        if (count > max_chars) {
            const char *end = last_break ? last_break : before;
            *p = last_break ? last_break + 1 : before;
            return (size_t)(end - s);
        }
    }
    *p = *cur == '\n' ? cur + 1 : cur;
    return (size_t)(cur - s);
}

static int wrapped(Surface *s, int x, int y, int w, const char *str, uint16_t color,
                   TextAlign align, bool draw)
{
    int max_chars = w / FONT_W;
    int lines = 0;
    char buf[64];
    while (*str) {
        const char *line = str;
        size_t len = wrap_next(&str, max_chars);
        if (draw) {
            if (len >= sizeof(buf))
                len = sizeof(buf) - 1;
            for (size_t i = 0; i < len; i++)
                buf[i] = line[i];
            buf[len] = 0;
            text_draw_aligned(s, x, y + lines * FONT_LINE, w, buf, color, align);
        }
        lines++;
    }
    return lines;
}

int text_draw_wrapped(Surface *s, int x, int y, int w, const char *str,
                      uint16_t color, TextAlign align)
{
    return wrapped(s, x, y, w, str, color, align, true);
}

int text_count_lines(int w, const char *str)
{
    return wrapped(NULL, 0, 0, w, str, 0, ALIGN_LEFT, false);
}
