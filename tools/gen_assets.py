#!/usr/bin/env python3
"""Génère les graphismes placeholder (tâche 2.2) en PNG dans gfx/.

Les PNG sont la source de vérité : un artiste peut les remplacer à la main,
puis `make assets` (tools/png2c.py) les convertit en C.
Transparence : canal alpha (pixel alpha < 128 = transparent).
"""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
GFX = ROOT / "gfx"

INK = (70, 46, 58, 255)
WHITE = (255, 255, 255, 255)
BLUSH = (255, 140, 160, 255)
SWEAT = (140, 200, 255, 255)
ANGRY = (230, 60, 70, 255)

# (nom, couleur du corps, couleur des détails, style d'oreilles)
CUSTOMERS = [
    ("lapin",      (255, 190, 205), (225, 130, 155), "bunny"),
    ("chat",       (205, 190, 245), (150, 130, 210), "cat"),
    ("ours",       (230, 195, 160), (180, 135, 100), "bear"),
    ("grenouille", (170, 225, 170), (100, 170, 110), "frog"),
    ("poussin",    (255, 230, 130), (235, 170, 60),  "chick"),
    ("panda",      (250, 250, 250), (70, 60, 70),    "panda"),
]
MOODS = ["content", "neutre", "inquiet", "furieux"]


def rgba(c):
    return c if len(c) == 4 else (*c, 255)


def customer(body, detail, style, mood):
    W, H = 40, 48
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    body, detail = rgba(body), rgba(detail)
    if mood == 3:  # furieux : teinte rouge
        body = (min(255, int(body[0] * 0.85 + 40)), int(body[1] * 0.85), int(body[2] * 0.85), 255)

    # Oreilles / accessoires derrière la tête
    if style == "bunny":
        d.ellipse((9, 0, 16, 20), fill=body, outline=detail)
        d.ellipse((24, 0, 31, 20), fill=body, outline=detail)
        d.ellipse((11, 4, 14, 16), fill=BLUSH)
        d.ellipse((26, 4, 29, 16), fill=BLUSH)
    elif style == "cat":
        d.polygon([(5, 18), (9, 4), (17, 13)], fill=body, outline=detail)
        d.polygon([(35, 18), (31, 4), (23, 13)], fill=body, outline=detail)
    elif style in ("bear", "panda"):
        ear = detail if style == "panda" else body
        d.ellipse((4, 9, 15, 20), fill=ear, outline=detail)
        d.ellipse((25, 9, 36, 20), fill=ear, outline=detail)
    elif style == "frog":
        d.ellipse((6, 6, 18, 18), fill=body, outline=detail)
        d.ellipse((22, 6, 34, 18), fill=body, outline=detail)
    elif style == "chick":
        d.line((20, 6, 20, 13), fill=detail, width=2)
        d.line((17, 8, 20, 13), fill=detail)

    # Corps (tête-blob)
    d.ellipse((3, 12, 36, 47), fill=body, outline=detail)

    # Yeux
    ey = 26 if style != "frog" else 12
    exs = (13, 26)
    if style == "panda":
        for ex in exs:
            d.ellipse((ex - 4, ey - 3, ex + 4, ey + 4), fill=detail)
    if style == "frog":
        for ex in (12, 28):
            d.ellipse((ex - 3, ey - 3, ex + 3, ey + 3), fill=WHITE)
        exs = (12, 28)
    eye_col = WHITE if style == "panda" else INK
    for ex in exs:
        if mood == 0:      # yeux fermés heureux ^^
            d.line((ex - 2, ey + 1, ex, ey - 1), fill=eye_col)
            d.line((ex, ey - 1, ex + 2, ey + 1), fill=eye_col)
        elif mood == 3:    # sourcils froncés
            d.rectangle((ex - 1, ey, ex, ey + 2), fill=eye_col)
            if ex < 20:
                d.line((ex - 3, ey - 4, ex + 2, ey - 2), fill=INK)
            else:
                d.line((ex - 2, ey - 2, ex + 3, ey - 4), fill=INK)
        else:
            d.rectangle((ex - 1, ey - 1, ex, ey + 2), fill=eye_col)

    # Joues
    cy = 33
    d.ellipse((6, cy - 1, 11, cy + 2), fill=BLUSH)
    d.ellipse((28, cy - 1, 33, cy + 2), fill=BLUSH)

    # Bouche / bec
    my = 35
    if style == "chick":
        d.polygon([(17, my - 3), (23, my - 3), (20, my + 1)], fill=detail)
    elif mood == 0:
        d.arc((16, my - 4, 23, my + 2), 20, 160, fill=INK)
    elif mood == 1:
        d.line((18, my, 21, my), fill=INK)
    elif mood == 2:
        d.line((17, my + 1, 19, my), fill=INK)
        d.line((19, my, 22, my + 1), fill=INK)
    else:
        d.arc((16, my, 23, my + 6), 200, 340, fill=INK)

    # Effets d'humeur
    if mood == 2:  # goutte de sueur
        d.polygon([(33, 14), (31, 19), (35, 19)], fill=SWEAT)
        d.ellipse((31, 17, 35, 21), fill=SWEAT)
    if mood == 3:  # veine de colère
        d.line((30, 12, 34, 12), fill=ANGRY, width=2)
        d.line((34, 16, 38, 16), fill=ANGRY, width=2)
        d.line((32, 10, 32, 14), fill=ANGRY, width=2)
        d.line((36, 14, 36, 18), fill=ANGRY, width=2)
    return img


def new16():
    img = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    return img, ImageDraw.Draw(img)


def icon_pearl():
    img, d = new16()
    for x, y in ((2, 8), (8, 9), (8, 3), (2, 2)):
        d.ellipse((x, y, x + 5, y + 5), fill=(74, 44, 42, 255))
        d.point((x + 1, y + 1), fill=(170, 130, 120, 255))
    return img


def icon_popping():
    img, d = new16()
    for x, y in ((1, 8), (8, 8), (4, 2)):
        d.ellipse((x, y, x + 6, y + 6), fill=(255, 160, 70, 255), outline=(230, 110, 40, 255))
        d.point((x + 2, y + 2), fill=WHITE)
    return img


def icon_jelly():
    img, d = new16()
    for x, y in ((1, 8), (8, 7), (4, 1)):
        d.rectangle((x, y, x + 6, y + 6), fill=(150, 220, 120, 255), outline=(90, 170, 80, 255))
        d.point((x + 2, y + 2), fill=WHITE)
    return img


def icon_cream():
    img, d = new16()
    for box in ((1, 6, 9, 14), (6, 5, 15, 14), (4, 1, 12, 9)):
        d.ellipse(box, fill=(255, 252, 240, 255), outline=(220, 205, 180, 255))
    d.ellipse((5, 5, 12, 12), fill=(255, 252, 240, 255))
    return img


def icon_ice():
    img, d = new16()
    for x, y in ((1, 7), (8, 6), (4, 0)):
        d.rectangle((x, y, x + 7, y + 7), fill=(215, 240, 255, 255), outline=(140, 190, 230, 255))
        d.line((x + 1, y + 1, x + 3, y + 1), fill=WHITE)
    return img


def icon_lock():
    img, d = new16()
    d.arc((4, 1, 11, 10), 180, 360, fill=INK, width=2)
    d.rectangle((3, 6, 12, 14), fill=(240, 200, 90, 255), outline=INK)
    d.rectangle((7, 9, 8, 11), fill=INK)
    return img


def icon_heart():
    img, d = new16()
    d.ellipse((1, 2, 8, 9), fill=(255, 110, 140, 255))
    d.ellipse((7, 2, 14, 9), fill=(255, 110, 140, 255))
    d.polygon([(1, 6), (14, 6), (8, 14)], fill=(255, 110, 140, 255))
    d.point((4, 4), fill=WHITE)
    return img


def icon_coin():
    img, d = new16()
    d.ellipse((1, 1, 14, 14), fill=(250, 210, 90, 255), outline=(200, 150, 50, 255))
    d.ellipse((4, 4, 11, 11), outline=(225, 175, 60, 255))
    d.point((5, 4), fill=WHITE)
    return img


def rom_icon():
    img = Image.new("RGB", (32, 32), (178, 230, 206))
    d = ImageDraw.Draw(img)
    d.rectangle((19, 0, 21, 8), fill=(214, 120, 140))
    d.polygon([(6, 6), (25, 6), (22, 31), (9, 31)], fill=(222, 184, 150), outline=(60, 40, 50))
    d.line((6, 6, 25, 6), fill=(255, 255, 255))
    for x, y in ((11, 27), (15, 28), (19, 27), (13, 24), (17, 25)):
        d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=(74, 44, 42))
    img.quantize(colors=16).save(ROOT / "icon.png")


def save(img, rel):
    out = GFX / rel
    out.parent.mkdir(parents=True, exist_ok=True)
    img.save(out)


if __name__ == "__main__":
    for name, body, detail, style in CUSTOMERS:
        for m, mood in enumerate(MOODS):
            save(customer(body, detail, style, m), f"customers/{name}_{m}.png")
    for name, fn in (("pearl", icon_pearl), ("popping", icon_popping), ("jelly", icon_jelly),
                     ("cream", icon_cream), ("ice", icon_ice), ("lock", icon_lock),
                     ("heart", icon_heart), ("coin", icon_coin)):
        save(fn(), f"icons/{name}.png")
    rom_icon()
    print("assets generated in gfx/")
