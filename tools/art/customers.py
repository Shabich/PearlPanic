"""Clients chibi 40x48 : 6 espèces x 4 humeurs (content, neutre, inquiet, furieux).

Seule la partie haute (≈ 46 px) dépasse du comptoir : grosse tête, épaules visibles.
"""
from .palette import INK, WHITE, BLUSH, SWEAT, RED, ramp, PINK, LAVENDER, YELLOW, MINT, LEAF, PEACH, GREY, CREAM
from .pixel import Canvas, Mask, shade, fill

W, H = 40, 48
HEAD = (20, 26, 16, 14)          # cx, cy, rx, ry

SPECIES = {
    #  nom          fourrure               vêtement            style
    "lapin":      ((255, 196, 212),        MINT[1],            "bunny"),
    "chat":       ((206, 192, 246),        YELLOW[1],          "cat"),
    "ours":       ((226, 184, 146),        (240, 120, 130),    "bear"),
    "grenouille": ((170, 222, 150),        PINK[1],            "frog"),
    "poussin":    ((255, 226, 128),        (150, 196, 236),    "chick"),
    "panda":      ((250, 248, 250),        LEAF[1],            "panda"),
}
ORDER = ["lapin", "chat", "ours", "grenouille", "poussin", "panda"]


def tint_angry(tones):
    return [tuple(min(255, int(c * 0.88 + a)) for c, a in zip(t, (34, 0, 0))) for t in tones]


def draw_ears(c, style, fur, inner):
    if style == "bunny":
        for ex in (13, 27):
            m = Mask(W, H).ellipse(ex, 9, 4.2, 10)
            shade(c, m, fur)
            fill(c, Mask(W, H).ellipse(ex, 10, 1.8, 6.5), inner)
    elif style == "cat":
        for pts, ipts in (([(5, 20), (8, 3), (18, 13)], [(8, 15), (9, 7), (14, 12)]),
                          ([(35, 20), (32, 3), (22, 13)], [(32, 15), (31, 7), (26, 12)])):
            shade(c, Mask(W, H).polygon(pts), fur)
            fill(c, Mask(W, H).polygon(ipts), inner)
    elif style in ("bear", "panda"):
        ear = ramp((70, 60, 72)) if style == "panda" else fur
        for ex in (8, 32):
            shade(c, Mask(W, H).ellipse(ex, 14, 6, 6), ear)
            if style == "bear":
                fill(c, Mask(W, H).ellipse(ex, 14.5, 3, 3), inner)
    elif style == "frog":
        for ex in (11, 29):
            shade(c, Mask(W, H).ellipse(ex, 14, 7, 6.5), fur)
    elif style == "chick":
        for x0, x1 in ((19, 12), (21, 8), (23, 13)):
            pass
        c.vline(20, 7, 12, fur[-1])
        c.set(19, 8, fur[-1]); c.set(18, 7, fur[-1])
        c.set(21, 9, fur[-1]); c.set(22, 8, fur[-1])


def draw_eyes(c, style, mood, eye_col, blink=False):
    # Position des yeux
    if style == "frog":
        pos = [(11, 14), (29, 14)]
        for (x, y) in pos:
            fill(c, Mask(W, H).ellipse(x, y, 4, 4), WHITE)
    else:
        pos = [(13, 27), (27, 27)]
    if style == "panda":
        for (x, y) in pos:
            fill(c, Mask(W, H).ellipse(x, y + 0.5, 4.5, 4), (70, 60, 72))

    for i, (x, y) in enumerate(pos):
        side = -1 if i == 0 else 1
        if blink and mood != 0:  # paupières fermées
            c.hline(x - 1, x + 1, y, eye_col)
            c.set(x - 2, y - 1, eye_col) if mood == 3 else None
            if mood == 3:
                c.set(x - 2 * side, y - 5, INK); c.set(x - side, y - 4, INK); c.set(x, y - 4, INK)
            continue
        if mood == 0:  # ^ ^
            c.set(x - 2, y + 1, eye_col); c.set(x - 1, y, eye_col)
            c.set(x, y - 1, eye_col); c.set(x + 1, y - 1, eye_col)
            c.set(x + 2, y, eye_col); c.set(x + 3, y + 1, eye_col)
        else:
            c.rect(x - 1, y - 2, x + 1, y + 1, eye_col)
            c.set(x - 1, y - 2, WHITE if eye_col != WHITE else INK)
            if mood == 2:  # sourcils inquiets (montent vers le centre)
                c.set(x - 2 * side, y - 4, INK); c.set(x - side, y - 5, INK); c.set(x, y - 5, INK)
            if mood == 3:  # sourcils froncés (descendent vers le centre)
                c.set(x - 2 * side, y - 5, INK); c.set(x - side, y - 4, INK); c.set(x, y - 4, INK)
                c.set(x + side, y - 3, INK)


def draw_mouth(c, style, mood, fur, talk=False):
    y = 33 if style != "frog" else 31
    if style == "chick":
        beak = ramp((250, 160, 70))
        if talk:  # bec ouvert
            shade(c, Mask(W, H).polygon([(16, 29), (24, 29), (20, 32)]), beak)
            shade(c, Mask(W, H).polygon([(17, 33), (23, 33), (20, 36)]), beak)
            c.hline(18, 22, 32, INK)
        else:
            shade(c, Mask(W, H).polygon([(16, 30), (24, 30), (20, 35)]), beak)
        if mood == 3:
            c.hline(18, 22, 32, beak[-1])
        return
    if style in ("bear", "panda"):
        fill(c, Mask(W, H).ellipse(20, 33, 4, 3), CREAM[0] if style == "bear" else WHITE)
        c.set(19, 31, INK); c.set(20, 31, INK); c.set(21, 31, INK); c.set(20, 32, INK)
        y = 34
    if talk:  # bouche ouverte (parle)
        h = 3 if mood != 3 else 2
        c.rect(18, y - 1, 22, y - 1 + h, INK)
        c.hline(19, 21, y - 1 + h, RED[1])
        if mood == 3:
            c.hline(18, 22, y - 2, INK)
        return
    if mood == 0:
        c.hline(18, 22, y, INK); c.set(17, y - 1, INK); c.set(23, y - 1, INK)
        c.hline(19, 21, y + 1, RED[1])
    elif mood == 1:
        c.set(18, y, INK); c.hline(19, 21, y + 1, INK); c.set(22, y, INK)
    elif mood == 2:
        c.set(17, y + 1, INK); c.set(18, y, INK); c.set(19, y + 1, INK)
        c.set(20, y, INK); c.set(21, y + 1, INK); c.set(22, y, INK); c.set(23, y + 1, INK)
    else:
        c.hline(18, 22, y, INK); c.set(17, y + 1, INK); c.set(23, y + 1, INK)


def customer(name, mood, blink=False, talk=False):
    fur_base, cloth_base, style = SPECIES[name]
    fur = ramp(fur_base, 4, light=0.45, dark=0.32)
    if style == "panda":
        fur = [(255, 255, 255), (244, 240, 246), (214, 204, 218), (150, 132, 156)]
    if mood == 3:
        fur = tint_angry(fur)
    inner = PINK[1] if style != "bear" else PEACH[2]
    cloth = ramp(cloth_base, 4, light=0.3, dark=0.4)
    c = Canvas(W, H)

    # Épaules + vêtement (écharpe / col) en bas
    body = Mask(W, H).round_rect(8, 37, 31, 52, 6)
    shade(c, body, fur)
    scarf = Mask(W, H).round_rect(9, 38, 30, 42, 2)
    shade(c, scarf, cloth)

    draw_ears(c, style, fur, inner)

    # Tête
    cx, cy, rx, ry = HEAD
    head = Mask(W, H).ellipse(cx, cy, rx, ry)
    shade(c, head, fur, hilite=(13, 18, 2.2))
    if style == "frog":
        # les yeux de la grenouille sont sur le dessus : on les redessine par-dessus la tête
        for ex in (11, 29):
            shade(c, Mask(W, H).ellipse(ex, 14, 7, 6.5), fur)

    eye_col = WHITE if style == "panda" else INK
    draw_eyes(c, style, mood, eye_col, blink)

    # Joues
    for bx in (9, 31):
        fill(c, Mask(W, H).ellipse(bx, 31.5, 2.6, 1.6), BLUSH if mood != 3 else RED[0])

    draw_mouth(c, style, mood, fur, talk)

    # Effets d'humeur
    if mood == 2:
        drop = Mask(W, H).polygon([(35, 13), (32, 19), (38, 19)]).union(Mask(W, H).ellipse(35, 19, 3, 2.6))
        shade(c, drop, ramp(SWEAT, 3, light=0.5, dark=0.3))
        c.set(34, 17, WHITE)
    if mood == 3:
        for (x, y) in ((31, 10), (35, 10), (31, 14), (35, 14)):
            pass
        red = RED[1]
        c.rect(32, 9, 33, 12, red); c.rect(30, 11, 31, 11, red); c.rect(34, 11, 35, 11, red)
        c.rect(36, 13, 37, 16, red); c.rect(35, 14, 35, 14, red); c.rect(38, 14, 38, 14, red)
    return c


def generate(save):
    for name in ORDER:
        for mood in range(4):
            save(customer(name, mood).img, f"customers/{name}_{mood}.png")
            save(customer(name, mood, blink=True).img, f"customers/{name}_{mood}b.png")
            save(customer(name, mood, talk=True).img, f"customers/{name}_{mood}t.png")
