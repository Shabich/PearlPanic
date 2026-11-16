"""Petits outils de pixel art procédural : masques, ombrage par tons, contours."""
import math
from PIL import Image


class Canvas:
    """Image RGBA manipulée pixel par pixel."""

    def __init__(self, w, h, bg=(0, 0, 0, 0)):
        self.w, self.h = w, h
        self.img = Image.new("RGBA", (w, h), bg)
        self.px = self.img.load()

    def set(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[x, y] = c if len(c) == 4 else (*c, 255)

    def get(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            return self.px[x, y]
        return (0, 0, 0, 0)

    def rect(self, x0, y0, x1, y1, c):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.set(x, y, c)

    def hline(self, x0, x1, y, c):
        self.rect(x0, y, x1, y, c)

    def vline(self, x, y0, y1, c):
        self.rect(x, y0, x, y1, c)

    def paste(self, other, ox, oy):
        for y in range(other.h):
            for x in range(other.w):
                p = other.px[x, y]
                if p[3] >= 128:
                    self.set(ox + x, oy + y, p)

    def save(self, path):
        self.img.save(path)


class Mask:
    """Ensemble de pixels (forme) avec opérations booléennes."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.on = set()

    def ellipse(self, cx, cy, rx, ry):
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1.0:
                    self.on.add((x, y))
        return self

    def rect(self, x0, y0, x1, y1):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.on.add((x, y))
        return self

    def round_rect(self, x0, y0, x1, y1, r):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                dx = max(x0 + r - x, 0, x - (x1 - r))
                dy = max(y0 + r - y, 0, y - (y1 - r))
                if dx * dx + dy * dy <= r * r + r * 0.6:
                    self.on.add((x, y))
        return self

    def polygon(self, pts):
        xs = [p[0] for p in pts]
        ys = [p[1] for p in pts]
        for y in range(int(min(ys)), int(max(ys)) + 1):
            for x in range(int(min(xs)), int(max(xs)) + 1):
                if _inside(pts, x + 0.5, y + 0.5):
                    self.on.add((x, y))
        return self

    def union(self, other):
        self.on |= other.on
        return self

    def minus(self, other):
        self.on -= other.on
        return self

    def contains(self, x, y):
        return (x, y) in self.on

    def edge(self):
        """Pixels de la forme qui touchent l'extérieur (4-voisinage)."""
        return {(x, y) for (x, y) in self.on
                if any((x + dx, y + dy) not in self.on for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))}

    def bbox(self):
        xs = [p[0] for p in self.on]
        ys = [p[1] for p in self.on]
        return min(xs), min(ys), max(xs), max(ys)


def _inside(pts, x, y):
    n = len(pts)
    inside = False
    j = n - 1
    for i in range(n):
        xi, yi = pts[i]
        xj, yj = pts[j]
        if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi + 1e-9) + xi:
            inside = not inside
        j = i
    return inside


def shade(canvas, mask, tones, light=(-0.45, -0.5), outline=True, hilite=None):
    """Ombrage « en boule » d'une forme avec une rampe `tones` (clair -> sombre).

    La lumière vient d'en haut à gauche : un reflet clair près de la source,
    le ton principal au milieu, un croissant d'ombre en bas à droite.
    Le dernier ton sert de contour sélectif (plus sombre en bas à droite).
    """
    x0, y0, x1, y1 = mask.bbox()
    cx, cy = (x0 + x1 + 1) / 2, (y0 + y1 + 1) / 2
    rx, ry = max(1, (x1 - x0 + 1) / 2), max(1, (y1 - y0 + 1) / 2)
    lx, ly = cx + light[0] * rx, cy + light[1] * ry
    edge = mask.edge() if outline else set()
    body = tones[:-1] if outline and len(tones) > 2 else tones
    for (x, y) in mask.on:
        if (x, y) in edge:
            continue
        # distance au point lumineux, normalisée par la taille de la forme
        d = (((x + 0.5 - lx) / rx) ** 2 + ((y + 0.5 - ly) / ry) ** 2) ** 0.5
        if len(body) >= 3:
            idx = 0 if d < 0.42 else 1 if d < 1.18 else 2
        elif len(body) == 2:
            idx = 0 if d < 1.05 else 1
        else:
            idx = 0
        canvas.set(x, y, body[min(idx, len(body) - 1)])
    if outline:
        for (x, y) in edge:
            canvas.set(x, y, tones[-1])
    if hilite:
        hx, hy, hr = hilite
        for y in range(int(hy - hr), int(hy + hr) + 1):
            for x in range(int(hx - hr), int(hx + hr) + 1):
                if (x - hx) ** 2 + (y - hy) ** 2 <= hr * hr and mask.contains(x, y) and (x, y) not in edge:
                    canvas.set(x, y, tones[0])


def fill(canvas, mask, color, outline=None):
    edge = mask.edge() if outline else set()
    for (x, y) in mask.on:
        canvas.set(x, y, outline if (x, y) in edge else color)


def scale_preview(img, k=4, bg=(255, 244, 230, 255)):
    out = Image.new("RGBA", img.size, bg)
    out.alpha_composite(img)
    return out.resize((img.width * k, img.height * k), Image.NEAREST)
