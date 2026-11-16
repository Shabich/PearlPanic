"""Intérieurs et postes de travail des bars 2 à 5 (même disposition que le Petit Bar,
pour que les animations du jeu tombent aux mêmes endroits : fenêtres, guirlande,
plantes suspendues, pots au sol)."""
import random

from .palette import WHITE, INK, ramp
from .pixel import Canvas, Mask, shade, fill
from .texture import value_noise, darken

W, H = 256, 192
COUNTER_Y = 124
WINDOWS = [(14, 30, 44, 50), (198, 30, 44, 50)]

THEMES = {
    "plage": {
        "wall": "bamboo", "wall_tones": [(236, 226, 160), (220, 206, 132), (196, 180, 108), (160, 146, 84)],
        "wainscot": [(150, 206, 232), (120, 186, 220), (90, 150, 196)],
        "frame": [(250, 250, 250), (226, 226, 230), (190, 190, 200), (140, 140, 150)],
        "jars": [(120, 200, 230), (250, 200, 110), (250, 150, 170), (150, 220, 200), (255, 240, 200)],
        "board": (60, 120, 140),
        "counter": [(250, 226, 160), (236, 206, 132), (210, 176, 104), (170, 136, 80)],
        "trim": [(150, 206, 232), (90, 150, 196)],
        "tiles": [(214, 240, 250), (186, 226, 244), (150, 200, 226)],
        "work": [(255, 244, 214), (240, 222, 180)],
        "towel": [(120, 186, 230), (255, 255, 255)],
    },
    "montagne": {
        "wall": "logs", "wall_tones": [(214, 160, 116), (190, 136, 96), (160, 110, 78), (120, 80, 60)],
        "wainscot": [(190, 186, 196), (160, 156, 170), (120, 116, 132)],
        "frame": [(150, 100, 76), (120, 80, 60), (96, 62, 50), (70, 44, 40)],
        "jars": [(220, 80, 80), (90, 150, 110), (250, 220, 150), (200, 160, 120), (240, 240, 240)],
        "board": (70, 84, 80),
        "counter": [(170, 116, 84), (146, 96, 70), (120, 78, 58), (90, 58, 46)],
        "trim": [(230, 90, 90), (170, 50, 60)],
        "tiles": [(200, 150, 120), (180, 128, 100), (150, 104, 84)],
        "work": [(200, 150, 110), (176, 128, 92)],
        "towel": [(220, 70, 80), (255, 240, 236)],
    },
    "ville": {
        "wall": "brick", "wall_tones": [(96, 78, 130), (84, 68, 116), (70, 56, 100), (54, 42, 80)],
        "wainscot": [(50, 44, 70), (40, 34, 58), (255, 120, 200)],
        "frame": [(170, 170, 190), (140, 140, 160), (110, 110, 130), (80, 80, 100)],
        "jars": [(120, 240, 255), (255, 120, 200), (255, 230, 120), (180, 140, 255), (140, 255, 180)],
        "board": (30, 26, 50),
        "counter": [(120, 116, 140), (100, 96, 120), (80, 76, 100), (60, 56, 80)],
        "trim": [(120, 240, 255), (60, 160, 200)],
        "tiles": [(70, 62, 100), (60, 52, 88), (255, 120, 200)],
        "work": [(150, 146, 166), (130, 126, 150)],
        "towel": [(255, 120, 200), (40, 34, 58)],
    },
    "foret": {
        "wall": "bark", "wall_tones": [(206, 176, 136), (186, 154, 116), (160, 130, 96), (126, 100, 76)],
        "wainscot": [(150, 196, 120), (120, 170, 100), (90, 140, 84)],
        "frame": [(150, 110, 80), (126, 92, 66), (100, 72, 54), (76, 54, 44)],
        "jars": [(245, 180, 50), (200, 50, 90), (180, 160, 230), (250, 200, 220), (150, 220, 120)],
        "board": (60, 90, 66),
        "counter": [(176, 136, 96), (150, 112, 78), (124, 90, 64), (96, 70, 52)],
        "trim": [(150, 206, 110), (90, 150, 80)],
        "tiles": [(190, 226, 160), (166, 210, 140), (130, 180, 116)],
        "work": [(236, 214, 176), (214, 190, 150)],
        "towel": [(150, 206, 110), (255, 250, 230)],
    },
}


def wall_texture(c, theme, seed):
    tones = theme["wall_tones"]
    noise = value_noise(W, H, 6, seed)
    kind = theme["wall"]
    for y in range(16, 98):
        for x in range(W):
            v = noise(x, y)
            if kind == "bamboo":       # lattes verticales avec nœuds
                col = tones[0] if x % 9 < 4 else tones[1]
                if x % 9 == 8:
                    col = tones[3]
                if (y + (x // 9) * 7) % 22 == 0:
                    col = tones[2]
            elif kind == "logs":       # rondins horizontaux
                band = (y - 16) % 12
                col = tones[0] if band < 3 else tones[1] if band < 9 else tones[2]
                if band == 11:
                    col = tones[3]
                if v > 0.82 and band in (5, 6):
                    col = tones[3]     # nœuds du bois
            elif kind == "brick":      # briques sombres
                row = (y - 16) // 6
                off = 6 if row % 2 else 0
                joint = (y - 16) % 6 == 5 or (x + off) % 12 == 11
                col = tones[3] if joint else tones[0] if v < 0.4 else tones[1] if v < 0.75 else tones[2]
            else:                       # écorce + mousse
                col = tones[0] if v < 0.35 else tones[1] if v < 0.7 else tones[2]
                if (x * 3 + y) % 17 == 0:
                    col = tones[3]
                if y < 26 and v > 0.6:
                    col = (150, 196, 120)
            c.set(x, y, col)
    # Soubassement
    ws = theme["wainscot"]
    for y in range(98, COUNTER_Y):
        for x in range(W):
            c.set(x, y, ws[0] if x % 12 else ws[1])
    c.hline(0, W - 1, 97, ws[2])
    c.hline(0, W - 1, 96, darken(tones[2], 0.9))


def window_frame(c, x0, y0, w, h, frame):
    m = Mask(W, H).round_rect(x0, y0, x0 + w - 1, y0 + h - 1, 6)
    fill(c, m, frame[1], outline=frame[3])
    glass = Mask(W, H).round_rect(x0 + 3, y0 + 3, x0 + w - 4, y0 + h - 4, 4)
    fill(c, glass, (200, 220, 240))     # repeint par le jeu (vue animée)
    c.rect(x0 - 3, y0 + h, x0 + w + 2, y0 + h + 2, frame[1])
    c.hline(x0 - 3, x0 + w + 2, y0 + h + 3, frame[3])


def shelf(c, x0, x1, y, theme, seed):
    wood = theme["counter"]
    c.rect(x0, y, x1, y + 2, wood[1])
    c.hline(x0, x1, y, wood[0])
    c.hline(x0, x1, y + 3, wood[3])
    rnd = random.Random(seed)
    x, i = x0 + 4, 0
    while x + 12 < x1:
        h = 12 + (i % 3) * 2
        col = theme["jars"][i % len(theme["jars"])]
        jar = Mask(W, H).round_rect(x, y - h, x + 10, y - 1, 3)
        fill(c, jar, (236, 246, 250), outline=(170, 170, 190))
        fill(c, Mask(W, H).round_rect(x + 2, y - h + 4, x + 8, y - 2, 2), col)
        c.rect(x + 1, y - h - 2, x + 9, y - h, wood[2])
        c.vline(x + 2, y - h + 2, y - 3, WHITE)
        if rnd.random() < 0.3:
            c.set(x + 5, y - h + 6, WHITE)
        x += 14
        i += 1


def menu_board(c, x0, y0, theme):
    board = Mask(W, H).round_rect(x0, y0, x0 + 55, y0 + 34, 3)
    fill(c, board, theme["board"], outline=theme["frame"][3])
    for k in range(4):
        y = y0 + 8 + k * 6
        c.set(x0 + 6, y, theme["jars"][k % 5]); c.set(x0 + 6, y + 1, theme["jars"][k % 5])
        c.hline(x0 + 9, x0 + 36 - k * 3, y, (220, 230, 226))
        c.hline(x0 + 42, x0 + 48, y, (255, 230, 140))
    c.vline(x0 + 8, y0 + 35, y0 + 40, theme["frame"][3])
    c.vline(x0 + 47, y0 + 35, y0 + 40, theme["frame"][3])


def wire(c, y):
    prev = None
    for x in range(0, W + 1, 2):
        yy = y + int(4 * abs(((x % 64) - 32) / 32.0) ** 2 * -1 + 4)
        if prev:
            c.set(x - 1, (prev + yy) // 2, INK)
        c.set(x, yy, (110, 80, 100))
        prev = yy


def pots(c, theme):
    # Pots au sol et pots suspendus (le feuillage est dessiné/animé par le jeu)
    for cx in (10, 246):
        base = 122
        pot = Mask(W, H).polygon([(cx - 8, base - 14), (cx + 8, base - 14), (cx + 6, base), (cx - 6, base)])
        shade(c, pot, ramp(theme["trim"][0], 4))
        c.rect(cx - 9, base - 16, cx + 9, base - 13, theme["trim"][0])
    for x in (70, 186):
        y = 58
        c.vline(x - 4, 16, y, (110, 80, 70))
        c.vline(x + 4, 16, y, (110, 80, 70))
        shade(c, Mask(W, H).round_rect(x - 6, y + 2, x + 6, y + 9, 3), ramp(theme["trim"][0], 4))


def counter(c, theme):
    t = theme["counter"]
    c.rect(0, COUNTER_Y, W - 1, COUNTER_Y + 4, t[1])
    c.hline(0, W - 1, COUNTER_Y, t[0])
    c.hline(0, W - 1, COUNTER_Y + 1, t[0])
    c.hline(0, W - 1, COUNTER_Y + 5, t[3])
    for y in range(COUNTER_Y + 6, H):
        for x in range(W):
            c.set(x, y, t[2] if x % 16 == 0 else t[1] if (x // 16) % 2 else t[0])
    trim = theme["trim"]
    for x in range(0, W, 8):
        sc = Mask(W, H).ellipse(x + 4, COUNTER_Y + 6, 4, 3)
        sc.on = {(px, py) for (px, py) in sc.on if py >= COUNTER_Y + 6}
        fill(c, sc, trim[0], outline=trim[1])
    c.hline(0, W - 1, COUNTER_Y + 6, trim[1])


def interior(name, seed):
    theme = THEMES[name]
    c = Canvas(W, H, (*theme["wall_tones"][1], 255))
    wall_texture(c, theme, seed)
    for w in WINDOWS:
        window_frame(c, *w, theme["frame"])
    shelf(c, 76, 180, 48, theme, seed)
    menu_board(c, 100, 56, theme)
    wire(c, 18)
    pots(c, theme)
    counter(c, theme)
    return c


def station(name, seed):
    theme = THEMES[name]
    c = Canvas(W, H)
    t = theme["tiles"]
    for y in range(0, 68):
        for x in range(W):
            row = y // 8
            off = 8 if row % 2 else 0
            joint = (y % 8 == 7) or ((x + off) % 16 == 15)
            c.set(x, y, t[2] if joint else t[0] if (y % 8) < 2 else t[1])
    wood = theme["counter"]
    c.rect(0, 38, W - 1, 40, wood[1])
    c.hline(0, W - 1, 38, wood[0])
    c.hline(0, W - 1, 41, wood[3])
    work = theme["work"]
    rnd = random.Random(seed)
    for y in range(68, H):
        c.hline(0, W - 1, y, work[0])
    for y in range(72, H, 6):
        x = rnd.randint(-40, 0)
        while x < W:
            ln = rnd.randint(18, 50)
            c.hline(max(0, x), min(W - 1, x + ln), y + rnd.randint(0, 2), work[1])
            x += ln + rnd.randint(14, 40)
    c.rect(0, 67, W - 1, 69, wood[1])
    c.hline(0, W - 1, 67, wood[0])
    c.hline(0, W - 1, 70, wood[3])
    towel = theme["towel"]
    for y in range(176, H):
        for x in range(72, 184):
            c.set(x, y, towel[0] if ((x // 4) + (y // 4)) % 2 else towel[1])
    c.hline(72, 183, 176, darken(towel[0], 0.8))
    return c


def generate(save):
    for i, name in enumerate(("plage", "montagne", "ville", "foret")):
        save(interior(name, 60 + i).img, f"bg/shop_{name}.png")
        save(station(name, 70 + i).img, f"bg/station_{name}.png")
