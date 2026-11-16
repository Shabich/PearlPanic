"""Équipement du poste d'assemblage.

Les sprites ont une « fenêtre » transparente : le jeu peint d'abord la couleur de
l'ingrédient derrière, puis pose le sprite par-dessus (une seule image pour tous
les thés, laits et sirops).
"""
from .palette import WOOD, GREY, WHITE, INK, PINK, CREAM, MINT, ramp
from .pixel import Canvas, Mask, shade, fill

STEEL = [(250, 250, 252), (226, 222, 232), (190, 182, 198), (140, 128, 150), (100, 88, 110)]


TAP_WINDOW = (6, 3, 25, 17)


def tap(pressed=False):
    """Tireuse 38x34 : réservoir vitré (fenêtre transparente, cf. tap_mask)."""
    c = Canvas(38, 34)
    body = Mask(38, 34).round_rect(2, 0, 35, 21, 4)
    shade(c, body, STEEL)
    win = Mask(38, 34).round_rect(*TAP_WINDOW, 3)
    for (x, y) in win.on:
        c.set(x, y, (0, 0, 0, 0))
    for (x, y) in win.edge():
        c.set(x, y, STEEL[3])
    for k in range(4):                      # reflets sur la vitre
        c.set(8 + k, 12 - k, WHITE)
    c.set(9, 5, WHITE)
    # bec
    c.rect(14, 22, 19, 23, STEEL[3])
    c.rect(15, 24, 18, 25 if not pressed else 27, STEEL[2])
    # levier à droite
    lever = PINK
    if pressed:
        c.rect(29, 10, 30, 16, STEEL[3]); c.rect(27, 15, 32, 18, lever[2]); c.hline(27, 32, 15, lever[1])
    else:
        c.rect(29, 6, 30, 12, STEEL[3]); c.rect(27, 3, 32, 6, lever[2]); c.hline(27, 32, 3, lever[1])
    # plaque pour l'étiquette
    plate = Mask(38, 34).round_rect(1, 23, 36, 33, 3)
    fill(c, plate, CREAM[0], outline=WOOD[3] if not pressed else PINK[3])
    c.hline(2, 35, 24, WHITE)
    return c


def tap_mask():
    """Masque de teinte de la vitre : blanc = couleur, gris = ombre."""
    c = Canvas(38, 34)
    win = Mask(38, 34).round_rect(*TAP_WINDOW, 3)
    x0, y0, x1, y1 = win.bbox()
    for (x, y) in win.on:
        c.set(x, y, (128, 128, 128) if y >= y1 - 3 or x >= x1 - 2 else WHITE)
    return c


PUMP_BOTTLE = (4, 6, 33, 23)


def pump(pressed=False):
    """Pompe à sirop 38x24 : bouteille (fenêtre transparente, cf. pump_mask)."""
    c = Canvas(38, 24)
    head_y = 2 if pressed else 0
    c.rect(13, head_y, 24, head_y + 2, STEEL[2]); c.hline(13, 24, head_y, STEEL[1])
    c.rect(18, head_y + 3, 19, 6, STEEL[3])
    c.rect(24, head_y + 1, 28, head_y + 2, STEEL[3])   # bec
    bottle = Mask(38, 24).round_rect(*PUMP_BOTTLE, 5)
    for (x, y) in bottle.edge():
        c.set(x, y, STEEL[3])
    c.rect(15, 5, 22, 6, STEEL[2])                      # col
    for k in range(3):
        c.set(7, 10 + k, WHITE)
    c.set(8, 9, WHITE)
    return c


def pump_mask():
    c = Canvas(38, 24)
    bottle = Mask(38, 24).round_rect(*PUMP_BOTTLE, 5)
    edge = bottle.edge()
    x0, y0, x1, y1 = bottle.bbox()
    for (x, y) in bottle.on:
        if (x, y) not in edge:
            c.set(x, y, (128, 128, 128) if y >= y1 - 3 or x >= x1 - 3 else WHITE)
    return c


def bin_box(holding=False):
    """Bac en bois 52x27 (garnitures)."""
    c = Canvas(52, 27)
    tones = [(250, 214, 170), WOOD[0], WOOD[1], WOOD[3]] if not holding else ramp(PINK[1])
    box = Mask(52, 27).round_rect(0, 0, 51, 26, 3)
    shade(c, box, tones, light=(-0.2, -0.9))
    inner = Mask(52, 27).round_rect(4, 3, 47, 16, 2)
    fill(c, inner, (122, 82, 70) if not holding else PINK[3])
    c.hline(5, 46, 4, (150, 104, 88) if not holding else PINK[2])
    # étiquette
    c.rect(6, 17, 45, 25, CREAM[0])
    c.hline(6, 45, 25, CREAM[2])
    return c


def generate(save):
    save(tap(False).img, "station/tap.png")
    save(tap(True).img, "station/tap_on.png")
    save(pump(False).img, "station/pump.png")
    save(pump(True).img, "station/pump_on.png")
    save(tap_mask().img, "station/tap_mask.png")
    save(pump_mask().img, "station/pump_mask.png")
    save(bin_box(False).img, "station/bin.png")
    save(bin_box(True).img, "station/bin_on.png")
