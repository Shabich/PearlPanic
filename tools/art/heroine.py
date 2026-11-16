"""Perle, la loutre gérante de Pearl Panic : tablier, chouchou jaune.

- Portraits / acteurs 40x48 pour les cinématiques : poses normale, contente, parle,
  surprise, yeux fermés, et deux pas de marche.
- Petit sprite 20x24 pour la carte du monde (2 pas de marche, de face).
"""
from .palette import INK, WHITE, BLUSH, RED, PINK, YELLOW, CREAM, ramp
from .pixel import Canvas, Mask, shade, fill

W, H = 40, 48
FUR = ramp((176, 120, 92), 4, light=0.45, dark=0.32)
FACE = (250, 232, 210)
APRON = ramp(PINK[1], 4)
SCRUNCHIE = ramp(YELLOW[1], 3)


def otter(pose="normal", step=0):
    c = Canvas(W, H)
    # Queue plate de loutre, derrière, côté droit
    tail = Mask(W, H).polygon([(26, 44), (38, 34), (39, 37), (30, 47)])
    shade(c, tail, FUR)
    # Corps + tablier (les pieds bougent pendant la marche)
    body = Mask(W, H).round_rect(9, 34, 30, 47, 7)
    shade(c, body, FUR)
    apron = Mask(W, H).round_rect(12, 36, 27, 47, 3)
    shade(c, apron, APRON)
    c.hline(13, 26, 37, WHITE)                 # bretelle
    fill(c, Mask(W, H).ellipse(20, 42, 2, 1.6), WHITE)   # petite poche
    if step:
        c.rect(10 if step == 1 else 26, 45, 13 if step == 1 else 29, 47, FUR[3])
    # Bras (levés quand elle est contente ou surprise)
    for side in (-1, 1):
        lift = pose in ("happy", "surprised")
        arm = Mask(W, H).ellipse(20 + side * 12, 36 if lift else 40, 3, 4.5)
        shade(c, arm, FUR)
    # Chignon tenu par le chouchou
    shade(c, Mask(W, H).ellipse(20, 8, 4.5, 4), FUR)
    shade(c, Mask(W, H).ellipse(20, 12, 6, 2.2), SCRUNCHIE)
    c.set(17, 11, WHITE)
    # Petites oreilles basses
    for ex in (6, 34):
        shade(c, Mask(W, H).ellipse(ex, 20, 3, 3), FUR)
        c.set(ex, 20, FUR[2])
    # Tête large + grand museau clair
    head = Mask(W, H).ellipse(20, 24, 16, 12)
    shade(c, head, FUR, hilite=(12, 16, 2))
    fill(c, Mask(W, H).ellipse(20, 29, 12, 6.5), FACE)
    fill(c, Mask(W, H).ellipse(13, 25, 4, 3), FACE)
    fill(c, Mask(W, H).ellipse(27, 25, 4, 3), FACE)
    # Yeux brillants
    for x in (13, 27):
        if pose in ("happy", "blink"):
            c.set(x - 2, 24, INK); c.set(x - 1, 23, INK); c.set(x, 23, INK); c.set(x + 1, 24, INK)
        elif pose == "surprised":
            fill(c, Mask(W, H).ellipse(x, 23, 2.4, 2.8), INK)
            c.set(x - 1, 22, WHITE)
        else:
            c.rect(x - 1, 21, x + 1, 25, INK)
            c.set(x - 1, 21, WHITE)
            c.set(x + 1, 24, WHITE)
    # Joues, gros nez, moustaches
    for bx in (8, 32):
        fill(c, Mask(W, H).ellipse(bx, 28, 2.4, 1.5), BLUSH)
    fill(c, Mask(W, H).ellipse(20, 27, 2.5, 1.6), (80, 52, 56))
    c.set(19, 26, WHITE)
    for side in (-1, 1):
        c.hline(20 + side * 5, 20 + side * 11, 29, FUR[3])
        c.set(20 + side * 11, 28, FUR[3])
        c.hline(20 + side * 5, 20 + side * 10, 31, FUR[3])
    # Bouche
    if pose in ("talk", "surprised"):
        c.rect(18, 31, 22, 34, INK)
        c.hline(19, 21, 34, RED[1])
    elif pose == "happy":
        c.hline(17, 23, 31, INK); c.set(16, 30, INK); c.set(24, 30, INK)
        c.hline(18, 22, 32, RED[1])
    else:
        c.set(18, 31, INK); c.set(19, 32, INK); c.set(20, 31, INK); c.set(21, 32, INK); c.set(22, 31, INK)
    return c


def map_otter(step=0):
    """Petit sprite de carte 20x24 (réduction stylisée)."""
    c = Canvas(20, 24)
    body = Mask(20, 24).round_rect(5, 15, 14, 23, 4)
    shade(c, body, FUR)
    shade(c, Mask(20, 24).round_rect(7, 16, 12, 23, 2), APRON)
    if step:
        c.rect(5 if step == 1 else 12, 22, 7 if step == 1 else 14, 23, FUR[3])
    for ex in (4, 15):
        shade(c, Mask(20, 24).ellipse(ex, 6, 2.5, 2.5), FUR)
    head = Mask(20, 24).ellipse(10, 10, 7.5, 6.5)
    shade(c, head, FUR, hilite=(7, 7, 1))
    fill(c, Mask(20, 24).ellipse(10, 13, 5, 3), FACE)
    shade(c, Mask(20, 24).ellipse(10, 4, 3, 1.6), SCRUNCHIE)
    c.set(7, 10, INK); c.set(7, 11, INK); c.set(13, 10, INK); c.set(13, 11, INK)
    c.set(10, 12, (90, 60, 60))
    c.set(4, 13, BLUSH); c.set(16, 13, BLUSH)
    return c


def generate(save):
    for pose in ("normal", "happy", "talk", "surprised", "blink"):
        save(otter(pose).img, f"heroine/perle_{pose}.png")
    save(otter("normal", 1).img, "heroine/perle_walk1.png")
    save(otter("normal", 2).img, "heroine/perle_walk2.png")
    for step in range(3):
        save(map_otter(step).img, f"heroine/map_{step}.png")
