"""Décors plein écran 256x192 (tâches 2.3, 2.4).

Écran haut : intérieur de la boutique (mur, fenêtres, étagère, guirlande, plantes, comptoir).
Écran bas  : poste d'assemblage (crédence carrelée, plan de travail).
Zones recouvertes par le jeu : HUD (y < 16), ticket de commande (x 4..252, y 130..188).
"""
import random

from .palette import (CREAM, WOOD, PINK, MINT, LAVENDER, PEACH, YELLOW, SKY, GREY, LEAF,
                      WHITE, INK, ramp)
from .pixel import Canvas, Mask, shade, fill

W, H = 256, 192
COUNTER_Y = 124


def wall(c):
    # Papier peint : rayures crème / rose très pâle + petits motifs de perles
    for y in range(16, COUNTER_Y):
        for x in range(W):
            stripe = (x // 10) % 2
            c.set(x, y, CREAM[1] if stripe else (255, 232, 228))
    for y in range(22, COUNTER_Y - 8, 14):
        for x in range(5 + (y // 14 % 2) * 10, W, 20):
            c.set(x, y, PINK[1]); c.set(x + 1, y, PINK[1])
            c.set(x, y + 1, PINK[1]); c.set(x + 1, y + 1, PINK[2])
    # Soubassement en lambris
    for y in range(98, COUNTER_Y):
        for x in range(W):
            c.set(x, y, MINT[1] if x % 12 else MINT[2])
    c.hline(0, W - 1, 97, MINT[3])
    c.hline(0, W - 1, 98, MINT[0])
    # Plinthe ombrée sous le papier peint
    c.hline(0, W - 1, 96, CREAM[2])


def window(c, x0, y0, w, h):
    frame = WOOD[:]
    m = Mask(W, H).round_rect(x0, y0, x0 + w - 1, y0 + h - 1, 6)
    fill(c, m, frame[1], outline=frame[3])
    glass = Mask(W, H).round_rect(x0 + 3, y0 + 3, x0 + w - 4, y0 + h - 4, 4)
    # Ciel en dégradé par paliers
    gx0, gy0, gx1, gy1 = glass.bbox()
    for (x, y) in glass.on:
        t = (y - gy0) / max(1, gy1 - gy0)
        c.set(x, y, SKY[0] if t < 0.35 else SKY[1] if t < 0.75 else (214, 238, 236))
    # Nuages
    rnd = random.Random(x0)
    for _ in range(2):
        cx, cy = rnd.randint(gx0 + 6, gx1 - 6), rnd.randint(gy0 + 5, gy0 + h // 2)
        cloud = Mask(W, H).ellipse(cx, cy, 6, 3).union(Mask(W, H).ellipse(cx - 4, cy + 1, 4, 2.5)) \
            .union(Mask(W, H).ellipse(cx + 4, cy + 1, 4, 2.5))
        cloud.on &= glass.on
        fill(c, cloud, WHITE)
    # Collines au loin
    for (x, y) in glass.on:
        if y > gy1 - 6 + ((x * 3) % 7 == 0):
            c.set(x, y, LEAF[0] if (x + y) % 5 else LEAF[1])
    # Croisillons
    c.vline(x0 + w // 2, y0 + 3, y0 + h - 4, frame[2])
    c.hline(x0 + 3, x0 + w - 4, y0 + h // 2, frame[2])
    # Reflet
    for k in range(5):
        c.set(gx0 + 2 + k, gy0 + 8 - k, WHITE)
    # Rebord
    c.rect(x0 - 3, y0 + h, x0 + w + 2, y0 + h + 2, frame[1])
    c.hline(x0 - 3, x0 + w + 2, y0 + h + 3, frame[3])


def plant_pot(c, cx, base_y, tall=False):
    """Pot de fleurs (le feuillage est dessiné et animé par le jeu)."""
    pot = Mask(W, H).polygon([(cx - 8, base_y - 14), (cx + 8, base_y - 14), (cx + 6, base_y), (cx - 6, base_y)])
    shade(c, pot, ramp(PEACH[2], 4))
    c.rect(cx - 9, base_y - 16, cx + 9, base_y - 13, PEACH[1])
    c.hline(cx - 9, cx + 9, base_y - 12, PEACH[3])


def hanging_plant(c, x, y):
    """Pot suspendu (les lianes sont dessinées et animées par le jeu, scenes/decor.c)."""
    c.vline(x - 4, 16, y, WOOD[3])
    c.vline(x + 4, 16, y, WOOD[3])
    pot = Mask(W, H).round_rect(x - 6, y + 2, x + 6, y + 9, 3)
    shade(c, pot, ramp(PINK[1], 4))
    c.hline(x - 5, x + 5, y + 4, PINK[0])


def shelf(c, x0, x1, y):
    c.rect(x0, y, x1, y + 2, WOOD[1])
    c.hline(x0, x1, y, WOOD[0])
    c.hline(x0, x1, y + 3, WOOD[3])
    for bx in (x0 + 4, x1 - 6):
        c.rect(bx, y + 4, bx + 2, y + 7, WOOD[2])
    # Bocaux de garnitures
    jar_cols = [(74, 44, 42), PEACH[2], LEAF[1], LAVENDER[2], PINK[2], YELLOW[2], (150, 85, 50)]
    x = x0 + 4
    i = 0
    while x + 12 < x1:
        h = 12 + (i % 3) * 2
        jar = Mask(W, H).round_rect(x, y - h, x + 10, y - 1, 3)
        fill(c, jar, (236, 246, 250), outline=GREY[2])
        content = Mask(W, H).round_rect(x + 2, y - h + 4, x + 8, y - 2, 2)
        col = jar_cols[i % len(jar_cols)]
        fill(c, content, col)
        if i % 7 == 0:  # perles : petits points clairs
            for k in range(3):
                c.set(x + 3 + k * 2, y - 4 - (k % 2) * 3, (150, 110, 100))
        c.rect(x + 1, y - h - 2, x + 9, y - h, WOOD[2] if i % 2 else PINK[2])
        c.vline(x + 2, y - h + 2, y - 3, WHITE)
        x += 14
        i += 1


def menu_board(c, x0, y0):
    board = Mask(W, H).round_rect(x0, y0, x0 + 55, y0 + 34, 3)
    fill(c, board, (74, 92, 88), outline=WOOD[3])
    c.rect(x0 + 1, y0 + 1, x0 + 54, y0 + 2, WOOD[1])
    # « MENU » en craie (pixels)
    letters = {
        "M": ["10001", "11011", "10101", "10001", "10001"],
        "E": ["1111", "1000", "1110", "1000", "1111"],
        "N": ["1001", "1101", "1011", "1001", "1001"],
        "U": ["1001", "1001", "1001", "1001", "0110"],
    }
    x = x0 + 12
    for ch in "MENU":
        rows = letters[ch]
        for j, row in enumerate(rows):
            for i, b in enumerate(row):
                if b == "1":
                    c.set(x + i, y0 + 6 + j, CREAM[0])
        x += len(rows[0]) + 2
    # lignes du menu + petits gobelets dessinés à la craie
    for k in range(3):
        y = y0 + 15 + k * 6
        c.set(x0 + 6, y, PINK[1]); c.set(x0 + 6, y + 1, PINK[1])
        c.hline(x0 + 9, x0 + 34 - k * 4, y, (200, 220, 214))
        c.hline(x0 + 42, x0 + 48, y, YELLOW[1])
    # pied du chevalet
    c.vline(x0 + 8, y0 + 35, y0 + 40, WOOD[3])
    c.vline(x0 + 47, y0 + 35, y0 + 40, WOOD[3])


def string_lights(c, y):
    cols = [PINK[1], YELLOW[1], MINT[1], LAVENDER[1], PEACH[1]]
    prev = None
    for x in range(0, W + 1, 2):
        yy = y + int(4 * abs(((x % 64) - 32) / 32.0) ** 2 * -1 + 4)
        if prev:
            c.set(x - 1, (prev + yy) // 2, INK)
        c.set(x, yy, (110, 80, 100))
        prev = yy


def counter_top(c):
    tones = WOOD
    c.rect(0, COUNTER_Y, W - 1, COUNTER_Y + 4, tones[1])
    c.hline(0, W - 1, COUNTER_Y, tones[0])
    c.hline(0, W - 1, COUNTER_Y + 1, tones[0])
    c.hline(0, W - 1, COUNTER_Y + 5, tones[3])
    # Façade en lattes
    for y in range(COUNTER_Y + 6, H):
        for x in range(W):
            c.set(x, y, tones[2] if x % 16 == 0 else tones[1] if (x // 16) % 2 else (228, 176, 132))
    # Feston rose sous le plateau
    for x in range(0, W, 8):
        scallop = Mask(W, H).ellipse(x + 4, COUNTER_Y + 6, 4, 3)
        scallop.on = {(px, py) for (px, py) in scallop.on if py >= COUNTER_Y + 6}
        fill(c, scallop, PINK[1], outline=PINK[3])
    c.hline(0, W - 1, COUNTER_Y + 6, PINK[2])


def shop_top():
    c = Canvas(W, H, (*CREAM[1], 255))
    c.rect(0, 0, W - 1, 15, PINK[1])
    wall(c)
    window(c, 14, 30, 44, 50)
    window(c, 198, 30, 44, 50)
    shelf(c, 76, 180, 48)
    menu_board(c, 100, 56)
    string_lights(c, 18)
    hanging_plant(c, 70, 58)
    hanging_plant(c, 186, 58)
    plant_pot(c, 10, 122, tall=True)
    plant_pot(c, 246, 122, tall=True)
    counter_top(c)
    return c


def station_bottom():
    c = Canvas(W, H)
    # Crédence : carreaux métro menthe
    for y in range(0, 68):
        for x in range(W):
            row = y // 8
            off = 8 if row % 2 else 0
            joint = (y % 8 == 7) or ((x + off) % 16 == 15)
            c.set(x, y, MINT[2] if joint else MINT[0] if (y % 8) < 2 else MINT[1])
    # Petite étagère entre tireuses et pompes
    c.rect(0, 38, W - 1, 40, WOOD[1])
    c.hline(0, W - 1, 38, WOOD[0])
    c.hline(0, W - 1, 41, WOOD[3])
    # Plan de travail (bois clair, veinures douces)
    rnd = random.Random(5)
    for y in range(68, H):
        for x in range(W):
            c.set(x, y, CREAM[1])
    for y in range(72, H, 6):
        x = rnd.randint(-40, 0)
        while x < W:
            ln = rnd.randint(18, 50)
            c.hline(max(0, x), min(W - 1, x + ln), y + rnd.randint(0, 2), CREAM[2])
            x += ln + rnd.randint(14, 40)
    # Rebord avant du plan de travail
    c.rect(0, 67, W - 1, 69, WOOD[1])
    c.hline(0, W - 1, 67, WOOD[0])
    c.hline(0, W - 1, 70, WOOD[3])
    # Torchon à carreaux sous le gobelet (zone centrale)
    for y in range(176, H):
        for x in range(72, 184):
            chk = ((x // 4) + (y // 4)) % 2
            c.set(x, y, PINK[1] if chk else PINK[0])
    c.hline(72, 183, 176, PINK[2])
    return c


def title_top():
    """Écran titre : ciel, devanture de la boutique avec auvent rayé."""
    c = Canvas(W, H)
    # Ciel en paliers
    for y in range(H):
        col = SKY[0] if y < 40 else (236, 244, 255) if y < 70 else (250, 240, 250)
        c.hline(0, W - 1, y, col)
    # Façade
    c.rect(16, 100, 239, 181, CREAM[1])
    for y in range(100, 182, 6):
        c.hline(16, 239, y, CREAM[2])
    # Auvent rayé avec festons
    for x in range(10, 246):
        for y in range(92, 106):
            c.set(x, y, PINK[1] if (x // 12) % 2 else CREAM[0])
    for x in range(10, 246, 12):
        sc = Mask(W, H).ellipse(x + 6, 106, 6, 4)
        sc.on = {(px, py) for (px, py) in sc.on if py >= 106}
        fill(c, sc, PINK[1] if (x // 12) % 2 else CREAM[0], outline=PINK[3])
    c.hline(10, 245, 91, PINK[3])
    # Vitrines + porte
    window(c, 26, 118, 70, 46)
    window(c, 160, 118, 70, 46)
    door = Mask(W, H).round_rect(108, 120, 147, 181, 8)
    fill(c, door, MINT[2], outline=MINT[4])
    c.rect(114, 128, 141, 150, SKY[1])
    c.set(141, 160, YELLOW[2]); c.set(142, 160, YELLOW[2])
    # Petit gobelet peint sur la vitre de la porte
    cup = Mask(W, H).polygon([(122, 133), (133, 133), (131, 147), (124, 147)])
    fill(c, cup, PEACH[1], outline=WOOD[3])
    for (x, y) in ((126, 144), (128, 145), (130, 144)):
        c.set(x, y, (74, 44, 42))
    c.vline(130, 128, 133, PINK[3])
    # Trottoir
    c.rect(0, 182, W - 1, H - 1, GREY[1])
    c.hline(0, W - 1, 182, GREY[2])
    for x in range(0, W, 24):
        c.vline(x, 183, H - 1, GREY[2])
    plant_pot(c, 8, 181, tall=True)
    plant_pot(c, 248, 181, tall=True)
    return c


def generate(save):
    save(title_top().img, "bg/title.png")
    save(shop_top().img, "bg/shop_top.png")
    save(station_bottom().img, "bg/station.png")
