#!/usr/bin/env python3
"""Génère les graphismes placeholder (tâche 2.2) en PNG dans gfx/.

Les PNG sont la source de vérité : un artiste peut les remplacer à la main,
puis `make assets` (tools/png2c.py) les convertit en C.
Transparence : canal alpha (pixel alpha < 128 = transparent).
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))
from art import backgrounds, customers, equipment  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
GFX = ROOT / "gfx"

INK = (70, 46, 58, 255)
WHITE = (255, 255, 255, 255)
BLUSH = (255, 140, 160, 255)
SWEAT = (140, 200, 255, 255)
ANGRY = (230, 60, 70, 255)

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
    customers.generate(save)
    backgrounds.generate(save)
    equipment.generate(save)
    for name, fn in (("pearl", icon_pearl), ("popping", icon_popping), ("jelly", icon_jelly),
                     ("cream", icon_cream), ("ice", icon_ice), ("lock", icon_lock),
                     ("heart", icon_heart), ("coin", icon_coin)):
        save(fn(), f"icons/{name}.png")
    rom_icon()
    print("assets generated in gfx/")
