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
    # --- La Paillote (bord de mer)
    "phoque":     ((176, 196, 214),        (240, 120, 130),    "seal"),
    "crabe":      ((250, 140, 110),        (150, 196, 236),    "crab"),
    "pingouin":   ((84, 96, 130),          (240, 120, 130),    "penguin"),
    "poulpe":     ((236, 150, 210),        YELLOW[1],          "octopus"),
    "tortue":     ((160, 210, 140),        (150, 196, 236),    "turtle"),
    "dauphin":    ((150, 200, 240),        YELLOW[1],          "dolphin"),
    # --- Le Chalet (montagne)
    "renard":     ((246, 160, 90),         (120, 160, 210),    "fox"),
    "hibou":      ((176, 136, 108),        (200, 90, 100),     "owl"),
    "ecureuil":   ((214, 128, 80),         LEAF[1],            "squirrel"),
    "marmotte":   ((196, 160, 120),        (240, 120, 130),    "marmot"),
    "biche":      ((222, 170, 122),        (200, 90, 100),     "deer"),
    "yeti":       ((246, 248, 255),        (120, 160, 210),    "yeti"),
    # --- Le Kiosque Néon (ville la nuit)
    "raton":      ((170, 166, 176),        (120, 230, 200),    "raccoon"),
    "pigeon":     ((186, 184, 206),        (240, 120, 130),    "pigeon"),
    "chauvesouris": ((140, 110, 170),      (120, 230, 200),    "bat"),
    "souris":     ((214, 210, 220),        (255, 150, 190),    "mouse"),
    "carlin":     ((236, 206, 160),        (120, 160, 210),    "pug"),
    "chat_noir":  ((82, 76, 96),           (255, 150, 190),    "blackcat"),
    # --- L'Arbre aux Thés (forêt enchantée)
    "champignon": ((252, 236, 214),        LEAF[1],            "mushroom"),
    "luciole":    ((230, 236, 140),        LAVENDER[1],        "firefly"),
    "escargot":   ((200, 222, 150),        PINK[1],            "snail"),
    "papillon":   ((214, 196, 248),        YELLOW[1],          "butterfly"),
    "herisson":   ((236, 206, 170),        LEAF[1],            "hedgehog"),
    "dragon":     ((150, 222, 190),        PINK[1],            "dragon"),
}
ORDER = ["lapin", "chat", "ours", "grenouille", "poussin", "panda",
         "phoque", "crabe", "pingouin", "poulpe", "tortue", "dauphin",
         "renard", "hibou", "ecureuil", "marmotte", "biche", "yeti",
         "raton", "pigeon", "chauvesouris", "souris", "carlin", "chat_noir",
         "champignon", "luciole", "escargot", "papillon", "herisson", "dragon"]

BEAKS = ("chick", "penguin", "owl", "pigeon")
MUZZLES = ("bear", "panda", "pug", "seal", "marmot", "dolphin")


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
    elif style in ("fox", "blackcat"):
        for pts, ipts in (([(4, 20), (7, 2), (17, 12)], [(7, 15), (8, 7), (13, 12)]),
                          ([(36, 20), (33, 2), (23, 12)], [(33, 15), (32, 7), (27, 12)])):
            shade(c, Mask(W, H).polygon(pts), fur)
            fill(c, Mask(W, H).polygon(ipts), inner if style == "blackcat" else (250, 240, 230))
        if style == "fox":
            for x in (7, 33):
                c.set(x, 3, fur[-1]); c.set(x, 4, fur[-1])
    elif style in ("mouse", "marmot", "raccoon", "pug", "hedgehog", "yeti"):
        size = {"mouse": 8, "marmot": 4, "raccoon": 5.5, "pug": 5, "hedgehog": 4, "yeti": 0}[style]
        if style == "hedgehog":
            spikes = ramp((150, 110, 90))
            for k in range(9):
                ang = -2.6 + k * 0.4
                import math
                tip = (20 + math.cos(ang) * 22, 27 + math.sin(ang) * 20)
                base1 = (20 + math.cos(ang - 0.25) * 14, 27 + math.sin(ang - 0.25) * 12)
                base2 = (20 + math.cos(ang + 0.25) * 14, 27 + math.sin(ang + 0.25) * 12)
                shade(c, Mask(W, H).polygon([base1, tip, base2]), spikes)
        if size:
            for ex in (7, 33):
                shade(c, Mask(W, H).ellipse(ex, 14, size, size), fur)
                fill(c, Mask(W, H).ellipse(ex, 14.5, size * 0.55, size * 0.55),
                     inner if style == "mouse" else fur[2])
        if style == "pug":
            for ex, ang in ((8, 1), (32, -1)):   # oreilles tombantes
                shade(c, Mask(W, H).polygon([(ex - 5, 12), (ex + 5, 12), (ex + ang * 4, 22)]), ramp((90, 70, 70)))
    elif style == "squirrel":
        tail = Mask(W, H).ellipse(33, 24, 7, 15)
        shade(c, tail, ramp((226, 150, 96)))
        for pts in ([(6, 18), (8, 4), (16, 13)], [(34, 18), (32, 4), (24, 13)]):
            shade(c, Mask(W, H).polygon(pts), fur)
        c.set(8, 3, fur[-1]); c.set(32, 3, fur[-1])
    elif style == "deer":
        for ex, ang in ((6, -1), (34, 1)):
            shade(c, Mask(W, H).polygon([(ex - 3 * ang, 17), (ex + 7 * ang, 10), (ex + 5 * ang, 20)]), fur)
            shade(c, Mask(W, H).ellipse(ex, 15, 6, 3.2), fur)
            fill(c, Mask(W, H).ellipse(ex, 15, 3.5, 1.6), inner)
    elif style == "bat":
        for side in (-1, 1):
            ex = 20 + side * 11
            wing = Mask(W, H).polygon([(ex - side * 4, 18), (ex + side * 3, 1), (ex + side * 10, 14)])
            shade(c, wing, fur)
            fill(c, Mask(W, H).polygon([(ex - side * 1, 15), (ex + side * 3, 6), (ex + side * 7, 13)]), inner)
    elif style == "owl":
        for side in (-1, 1):
            shade(c, Mask(W, H).polygon([(20 + side * 7, 15), (20 + side * 13, 5), (20 + side * 15, 16)]), fur)
    elif style == "turtle":
        shell = Mask(W, H).ellipse(20, 40, 18, 9)
        shade(c, shell, ramp((150, 120, 80)))
        for x in (10, 20, 30):
            c.hline(x - 2, x + 2, 36, (110, 84, 60))
    elif style == "dolphin":
        shade(c, Mask(W, H).polygon([(18, 13), (26, 2), (28, 15)]), fur)
    elif style == "octopus":
        for k in range(5):
            x = 6 + k * 7
            shade(c, Mask(W, H).ellipse(x, 42, 3.5, 7), fur)
    elif style == "snail":
        shell = Mask(W, H).ellipse(29, 30, 11, 12)
        shade(c, shell, ramp((236, 180, 120)))
        import math
        for k in range(30):
            a = k * 0.42
            r = 1 + k * 0.28
            c.set(int(29 + math.cos(a) * r), int(30 + math.sin(a) * r), (176, 110, 70))
        for ex in (11, 19):   # antennes à yeux
            c.vline(ex, 6, 14, fur[-1])
    elif style in ("firefly", "butterfly"):
        if style == "butterfly":
            for side in (-1, 1):
                for wy, wr in ((16, 9), (32, 6)):
                    wing = Mask(W, H).ellipse(20 + side * 15, wy, wr * 0.8, wr)
                    shade(c, wing, ramp((250, 200, 120) if wy < 20 else (255, 170, 200)))
                    c.set(20 + side * 15, wy, WHITE)
        else:
            for side in (-1, 1):   # petites ailes translucides
                fill(c, Mask(W, H).ellipse(20 + side * 15, 20, 5, 8), (226, 240, 255), outline=(170, 200, 230))
        for side in (-1, 1):
            c.set(20 + side * 4, 12, fur[-1]); c.set(20 + side * 5, 9, fur[-1])
            c.set(20 + side * 6, 6, fur[-1]); c.set(20 + side * 7, 4, fur[-1])
            fill(c, Mask(W, H).ellipse(20 + side * 8, 3, 2, 2), (255, 236, 120) if style == "firefly" else fur[1])
    elif style == "dragon":
        for side in (-1, 1):   # ailes
            wing = Mask(W, H).polygon([(20 + side * 12, 28), (20 + side * 20, 10), (20 + side * 18, 30)])
            shade(c, wing, ramp((120, 190, 170)))
        for side in (-1, 1):   # cornes
            shade(c, Mask(W, H).polygon([(20 + side * 6, 14), (20 + side * 9, 3), (20 + side * 11, 14)]),
                  ramp((250, 236, 190)))
    elif style == "crab":
        for ex in (13, 27):    # yeux sur pédoncules
            c.vline(ex, 9, 15, fur[-1])
    elif style == "pigeon":
        pass
    elif style == "chick":
        c.vline(20, 7, 12, fur[-1])
        c.set(19, 8, fur[-1]); c.set(18, 7, fur[-1])
        c.set(21, 9, fur[-1]); c.set(22, 8, fur[-1])


def head_shape(style):
    if style == "octopus":
        return Mask(W, H).ellipse(20, 23, 15, 16)
    if style == "snail":
        return Mask(W, H).ellipse(15, 29, 12, 12)
    if style in ("crab",):
        return Mask(W, H).ellipse(20, 28, 17, 12)
    if style == "dolphin":
        return Mask(W, H).ellipse(20, 26, 16, 14).union(Mask(W, H).ellipse(20, 34, 7, 4))
    return Mask(W, H).ellipse(*HEAD[:2], HEAD[2], HEAD[3])


def draw_face_extras(c, style, fur, mood):
    """Masques, museaux, taches : dessinés sur la tête, avant les yeux."""
    if style == "penguin":
        fill(c, Mask(W, H).ellipse(20, 29, 12, 10), (250, 250, 252))
    elif style == "raccoon":
        fill(c, Mask(W, H).ellipse(20, 26, 15, 4), (70, 64, 76))
        fill(c, Mask(W, H).ellipse(20, 33, 6, 4), (240, 236, 240))
    elif style == "fox":
        for side in (-1, 1):
            fill(c, Mask(W, H).ellipse(20 + side * 8, 32, 7, 6), (252, 244, 236))
    elif style == "owl":
        for side in (-1, 1):
            fill(c, Mask(W, H).ellipse(20 + side * 7, 26, 6, 6), (246, 230, 200), outline=fur[2])
    elif style == "deer":
        for (x, y) in ((10, 20), (30, 19), (14, 15), (26, 15)):
            c.set(x, y, (250, 240, 230)); c.set(x + 1, y, (250, 240, 230))
        fill(c, Mask(W, H).ellipse(20, 33, 6, 4), (250, 236, 220))
    elif style == "yeti":
        fill(c, Mask(W, H).ellipse(20, 29, 11, 9), (170, 200, 236))
        for k in range(7):   # mèches de fourrure
            x = 8 + k * 4
            c.set(x, 13, fur[2]); c.set(x + 1, 12, fur[2])
    elif style == "pigeon":
        fill(c, Mask(W, H).round_rect(8, 36, 31, 40, 2), (130, 200, 170))   # cou irisé
        c.hline(9, 30, 38, (180, 140, 220))
    elif style == "mushroom":
        cap = Mask(W, H).ellipse(20, 16, 19, 11)
        cap.on = {(x, y) for (x, y) in cap.on if y <= 20}
        shade(c, cap, ramp((236, 80, 90)))
        for (x, y, r) in ((11, 12, 2.2), (22, 9, 2.6), (30, 14, 2), (17, 17, 1.6)):
            fill(c, Mask(W, H).ellipse(x, y, r, r), (255, 250, 245))
    elif style == "firefly":
        glow = Mask(W, H).ellipse(20, 40, 9, 5)
        fill(c, glow, (255, 244, 150))
    elif style == "hedgehog":
        fill(c, Mask(W, H).ellipse(20, 30, 11, 9), (250, 236, 214))
    elif style == "crab":
        for side in (-1, 1):   # pinces levées devant
            claw = Mask(W, H).ellipse(20 + side * 15, 37, 4, 4.5)
            shade(c, claw, fur)
            c.hline(20 + side * 15 - 1, 20 + side * 15 + 1, 33, (0, 0, 0, 0))
            c.set(20 + side * 15, 34, fur[-1])
    elif style == "dragon":
        for (x, y) in ((12, 20), (28, 20), (20, 16)):
            c.set(x, y, fur[2]); c.set(x + 1, y + 1, fur[2])


def draw_eyes(c, style, mood, eye_col, blink=False):
    # Position des yeux
    if style == "frog":
        pos = [(11, 14), (29, 14)]
        for (x, y) in pos:
            fill(c, Mask(W, H).ellipse(x, y, 4, 4), WHITE)
    elif style == "crab":
        pos = [(13, 8), (27, 8)]
        for (x, y) in pos:
            fill(c, Mask(W, H).ellipse(x, y, 3.5, 3.5), WHITE, outline=(200, 90, 80))
    elif style == "snail":
        pos = [(11, 5), (19, 5)]
        for (x, y) in pos:
            fill(c, Mask(W, H).ellipse(x, y, 3, 3), WHITE, outline=(140, 170, 100))
    elif style == "owl":
        pos = [(13, 26), (27, 26)]
    elif style == "octopus":
        pos = [(13, 25), (27, 25)]
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
        elif style == "blackcat":
            fill(c, Mask(W, H).ellipse(x, y, 2.6, 3), (250, 220, 90))
            c.vline(x, y - 2, y + 2, INK)
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
    if style == "snail":
        y = 34
    if style in BEAKS:
        beak = ramp((250, 160, 70)) if style != "pigeon" else ramp((90, 80, 96))
        if style == "owl":
            beak = ramp((240, 190, 90))
        if talk:  # bec ouvert
            shade(c, Mask(W, H).polygon([(16, 29), (24, 29), (20, 32)]), beak)
            shade(c, Mask(W, H).polygon([(17, 33), (23, 33), (20, 36)]), beak)
            c.hline(18, 22, 32, INK)
        else:
            shade(c, Mask(W, H).polygon([(16, 30), (24, 30), (20, 35)]), beak)
        if mood == 3:
            c.hline(18, 22, 32, beak[-1])
        return
    if style == "bat" and not talk:   # petits crocs
        c.set(18, y + 1, WHITE); c.set(22, y + 1, WHITE)
    if style in MUZZLES:
        muzzle = {"bear": CREAM[0], "panda": WHITE, "pug": (110, 90, 90), "seal": (236, 236, 240),
                  "marmot": (240, 222, 196), "dolphin": (226, 240, 252)}[style]
        fill(c, Mask(W, H).ellipse(20, 33, 4.5, 3.2), muzzle)
        nose = INK if style != "pug" else (40, 30, 40)
        c.set(19, 31, nose); c.set(20, 31, nose); c.set(21, 31, nose); c.set(20, 32, nose)
        if style == "seal":   # moustaches
            for side in (-1, 1):
                c.hline(20 + side * 6, 20 + side * 9, 32, fur[-1])
                c.hline(20 + side * 6, 20 + side * 9, 34, fur[-1])
        if style == "marmot" and not talk:   # grandes dents
            c.rect(19, 35, 21, 36, WHITE)
            c.vline(20, 35, 36, (220, 220, 220))
        y = 34
    if style in ("mouse",):
        c.set(20, 30, PINK[2])
        for side in (-1, 1):
            c.hline(20 + side * 3, 20 + side * 8, 31, (150, 140, 160))
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
    if style == "blackcat":
        fur = [(120, 112, 140), (86, 80, 102), (64, 58, 80), (40, 34, 52)]
    if style == "yeti":
        fur = [(255, 255, 255), (240, 244, 252), (206, 214, 232), (150, 160, 190)]
    if mood == 3 and style in ("blackcat", "yeti"):
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
    head = head_shape(style)
    shade(c, head, fur, hilite=(13, 18, 2.2) if style not in ("snail",) else (9, 22, 2))
    draw_face_extras(c, style, fur, mood)
    if style == "frog":
        # les yeux de la grenouille sont sur le dessus : on les redessine par-dessus la tête
        for ex in (11, 29):
            shade(c, Mask(W, H).ellipse(ex, 14, 7, 6.5), fur)

    eye_col = WHITE if style in ("panda",) else INK
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
