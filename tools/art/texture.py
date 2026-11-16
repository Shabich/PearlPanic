"""Outils de texture pour les grands décors : bruit, sols, ombres portées, objets en volume."""
import math
import random

from .palette import WHITE, ramp
from .pixel import Mask, shade, fill


def value_noise(w, h, cell, seed):
    """Bruit lissé (interpolation bilinéaire d'une grille aléatoire), valeurs 0..1."""
    rnd = random.Random(seed)
    gw, gh = w // cell + 2, h // cell + 2
    grid = [[rnd.random() for _ in range(gw)] for _ in range(gh)]

    def at(x, y):
        gx, gy = x / cell, y / cell
        x0, y0 = int(gx), int(gy)
        fx, fy = gx - x0, gy - y0
        fx = fx * fx * (3 - 2 * fx)
        fy = fy * fy * (3 - 2 * fy)
        a = grid[y0][x0] + (grid[y0][x0 + 1] - grid[y0][x0]) * fx
        b = grid[y0 + 1][x0] + (grid[y0 + 1][x0 + 1] - grid[y0 + 1][x0]) * fx
        return a + (b - a) * fy
    return at


def textured_fill(c, x0, y0, x1, y1, tones, seed, cell=14, fine=0.18, region=None):
    """Remplit une zone avec 3-4 tons choisis par bruit (grandes plaques + grain fin)."""
    big = value_noise(c.w, c.h, cell, seed)
    small = value_noise(c.w, c.h, 3, seed + 1)
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if region and (x, y) not in region:
                continue
            v = big(x, y) * (1 - fine) + small(x, y) * fine
            idx = min(len(tones) - 1, int(v * len(tones)))
            c.set(x, y, tones[idx])


def tufts(c, region_fn, count, light, dark, seed):
    """Petites touffes d'herbe « ʌ » (ton clair au-dessus, sombre en dessous)."""
    rnd = random.Random(seed)
    for _ in range(count):
        x, y = rnd.randrange(c.w), rnd.randrange(c.h)
        if not region_fn(x, y):
            continue
        c.set(x, y, light)
        c.set(x - 1, y + 1, dark)
        c.set(x + 1, y + 1, dark)


def speckle(c, region_fn, count, colors, seed):
    rnd = random.Random(seed)
    for _ in range(count):
        x, y = rnd.randrange(c.w), rnd.randrange(c.h)
        if region_fn(x, y):
            c.set(x, y, rnd.choice(colors))


def darken(col, k):
    return tuple(int(v * k) for v in col[:3])


def shadow(c, mask, dx=3, dy=3, k=0.78):
    """Ombre portée : assombrit les pixels déjà dessinés sous la forme décalée."""
    for (x, y) in mask.on:
        px, py = x + dx, y + dy
        if (px, py) in mask.on:
            continue
        p = c.get(px, py)
        if p[3] >= 128:
            c.set(px, py, darken(p, k))


def ellipse_shadow(c, cx, cy, rx, ry, k=0.75):
    m = Mask(c.w, c.h).ellipse(cx, cy, rx, ry)
    for (x, y) in m.on:
        p = c.get(x, y)
        if p[3] >= 128:
            c.set(x, y, darken(p, k))


def tree(c, x, y, tones, r=8, trunk=(110, 74, 60)):
    """Arbre vu de dessus en 3/4 : ombre, tronc, feuillage en grappes ombrées."""
    ellipse_shadow(c, x + 4, y + r // 2 + 3, r + 2, r // 2 + 2)
    c.rect(x - 1, y, x + 1, y + r // 2 + 2, trunk)
    blobs = [(0, -r // 2, r), (-r // 2, 0, r * 3 // 4), (r // 2, 0, r * 3 // 4), (0, -r, r * 2 // 3)]
    for dx, dy, rr in blobs:
        shade(c, Mask(c.w, c.h).ellipse(x + dx, y + dy, rr, rr * 0.9), tones)
    c.set(x - r // 3, y - r, tones[0])
    c.set(x - r // 3 + 1, y - r, tones[0])


def house(c, x, y, w, h, roof_base, wall=(255, 240, 222), chimney=True, door=(150, 100, 80)):
    """Maison en 3/4 : toit à deux pans avec tuiles, façade, fenêtres, ombre portée.
    Renvoie la position de la cheminée (pour la fumée animée) ou None."""
    roof = ramp(roof_base, 4)
    body = Mask(c.w, c.h).rect(x, y, x + w, y + h)
    roof_m = Mask(c.w, c.h).polygon([(x - 3, y + 2), (x + w // 2, y - h * 3 // 4), (x + w + 3, y + 2)])
    shadow(c, body.union(Mask(c.w, c.h).polygon([(x - 3, y + 2), (x + w // 2, y - h * 3 // 4),
                                                 (x + w + 3, y + 2)])), 4, 3)
    fill(c, body, wall, outline=darken(wall, 0.62))
    c.hline(x + 1, x + w - 1, y + h - 1, darken(wall, 0.85))
    # Fenêtres et porte
    for wx in (x + 3, x + w - 8):
        c.rect(wx, y + 4, wx + 4, y + 8, (170, 210, 240))
        c.set(wx, y + 4, WHITE)
        c.hline(wx - 1, wx + 5, y + 9, darken(wall, 0.7))
    c.rect(x + w // 2 - 2, y + h - 7, x + w // 2 + 2, y + h - 1, door)
    cx = None
    if chimney:
        cx = (x + w * 3 // 4, y - h // 2)
        c.rect(cx[0] - 2, cx[1] - 4, cx[0] + 2, cx[1] + 3, darken(roof_base, 0.7))
        c.hline(cx[0] - 3, cx[0] + 3, cx[1] - 5, darken(roof_base, 0.55))
    shade(c, roof_m, roof, light=(-0.6, -0.6))
    # Rangs de tuiles
    for k in range(1, 4):
        yy = y + 2 - k * (h * 3 // 4) // 4
        half = (w // 2 + 3) * (4 - k) // 4
        c.hline(x + w // 2 - half + 1, x + w // 2 + half - 1, yy, roof[2])
    return (cx[0], cx[1] - 6) if cx else None


def cobble_path(c, pts, stone, gap, edge, width=6, seed=0, bend_amp=7, bridge=None, planks=None):
    """Chemin pavé qui relie les points (légère courbe), avec bordure et pierres.
    `bridge` : pixels (ex. une rivière) où le chemin devient un pont en planches."""
    rnd = random.Random(seed)
    region = set()
    centers = []
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        n = max(1, int(math.hypot(x1 - x0, y1 - y0)))
        nx, ny = -(y1 - y0) / n, (x1 - x0) / n
        for i in range(n + 1):
            t = i / n
            bend = math.sin(t * math.pi) * bend_amp
            x = x0 + (x1 - x0) * t + nx * bend
            y = y0 + (y1 - y0) * t + ny * bend
            centers.append((x, y))
            for dy in range(-width, width + 1):
                for dx in range(-width, width + 1):
                    if dx * dx + dy * dy <= width * width:
                        region.add((int(x + dx), int(y + dy)))
    # Ombre sous le chemin (bord bas plus sombre)
    for (x, y) in region:
        if (x, y + 1) not in region:
            p = c.get(x, y + 1)
            if p[3] >= 128:
                c.set(x, y + 1, darken(p, 0.8))
    for (x, y) in region:
        c.set(x, y, gap)
    # Pavés : petites pierres de 3x2 en quinconce
    for (x, y) in region:
        if (x + (y // 3) * 2) % 4 < 3 and y % 3 < 2:
            v = rnd.random()
            col = stone[0] if v < 0.25 else stone[1] if v < 0.8 else stone[2]
            c.set(x, y, col)
    # Bordure
    for (x, y) in region:
        if any((x + dx, y + dy) not in region for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            c.set(x, y, edge)
    # Pont : planches + garde-fous là où le chemin passe sur l'eau
    if bridge and planks:
        for (x, y) in region:
            if (x, y) in bridge:
                border = any((x + dx, y + dy) not in region for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                c.set(x, y, planks[3] if border else planks[0] if (x + y) % 4 else planks[1])
        for (x, y) in region:
            if (x, y) in bridge and (x, y + 1) not in region:
                c.set(x, y + 1, planks[2])
    return region


def water(c, region, tones, seed, ripple=WHITE):
    """Eau : bandes de tons par bruit + reflets en petits traits."""
    big = value_noise(c.w, c.h, 18, seed)
    for (x, y) in region:
        v = big(x, y)
        c.set(x, y, tones[0] if v < 0.3 else tones[1] if v < 0.7 else tones[2])
    rnd = random.Random(seed + 3)
    pts = list(region)
    for _ in range(len(pts) // 60):
        x, y = rnd.choice(pts)
        if all((x + k, y) in region for k in range(4)):
            c.hline(x, x + 3, y, tones[0] if rnd.random() < 0.6 else ripple)


def pine(c, x, y, snow=True):
    """Sapin (enneigé) avec ombre."""
    ellipse_shadow(c, x + 4, y + 18, 8, 3, k=0.85)
    for k, w in enumerate((5, 8, 11)):
        tri = Mask(c.w, c.h).polygon([(x - w, y + k * 6 + 5), (x, y + k * 6 - 6), (x + w, y + k * 6 + 5)])
        shade(c, tri, [(120, 176, 140), (84, 146, 116), (62, 118, 98), (40, 84, 76)])
        if snow:
            c.hline(x - 2, x + 2, y + k * 6 - 4, WHITE)
            c.hline(x - w + 2, x - w + 4, y + k * 6 + 4, WHITE)
    c.rect(x - 1, y + 17, x + 1, y + 20, (110, 74, 60))
