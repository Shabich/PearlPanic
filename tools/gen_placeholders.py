#!/usr/bin/env python3
"""Génère les graphismes placeholder (tâche 2.2) et l'icône de la ROM (tâche 0.4).

Usage : python3 tools/gen_placeholders.py   (depuis la racine du projet)
Les PNG produits sont convertis par grit au build (fichiers .grit associés).
Le magenta (255, 0, 255) est la couleur transparente des sprites.
"""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
MAGENTA = (255, 0, 255)

# Palette pastel provisoire (tâche 2.1 la remplacera)
CREAM = (255, 244, 230)
TAPIOCA = (74, 44, 42)
TAPIOCA_HI = (150, 104, 96)
PINK = (255, 182, 193)
PINK_DARK = (214, 120, 140)
MILK_TEA = (222, 184, 150)
MINT = (178, 230, 206)
INK = (60, 40, 50)
WHITE = (255, 255, 255)


def save_sprite(img: Image.Image, name: str, grit: str) -> None:
    out = ROOT / "gfx" / "sprites" / f"{name}.png"
    img.save(out)
    out.with_suffix(".grit").write_text(grit)
    print("wrote", out.relative_to(ROOT))


# Sprites 16 couleurs, tuiles 8x8, palette générée, magenta transparent
GRIT_SPRITE_16 = "-gt -gB4 -p -gTFF00FF\n"


def pearl() -> Image.Image:
    img = Image.new("RGB", (16, 16), MAGENTA)
    d = ImageDraw.Draw(img)
    d.ellipse((1, 1, 14, 14), fill=TAPIOCA)
    d.ellipse((4, 3, 7, 6), fill=TAPIOCA_HI)
    d.point((5, 4), fill=WHITE)
    return img


def customer() -> Image.Image:
    """Petit lapin-blob rose, 32x32."""
    img = Image.new("RGB", (32, 32), MAGENTA)
    d = ImageDraw.Draw(img)
    d.ellipse((7, 0, 12, 14), fill=PINK, outline=PINK_DARK)    # oreille G
    d.ellipse((19, 0, 24, 14), fill=PINK, outline=PINK_DARK)   # oreille D
    d.ellipse((3, 9, 28, 31), fill=PINK, outline=PINK_DARK)    # corps
    d.rectangle((11, 18, 12, 20), fill=INK)                    # yeux
    d.rectangle((19, 18, 20, 20), fill=INK)
    d.ellipse((7, 21, 10, 23), fill=PINK_DARK)                 # joues
    d.ellipse((21, 21, 24, 23), fill=PINK_DARK)
    d.line((15, 22, 16, 22), fill=INK)                         # bouche
    return img


def cup() -> Image.Image:
    """Gobelet de bubble tea, 32x32."""
    img = Image.new("RGB", (32, 32), MAGENTA)
    d = ImageDraw.Draw(img)
    d.rectangle((19, 0, 21, 8), fill=PINK_DARK)                # paille
    d.polygon([(6, 6), (25, 6), (22, 31), (9, 31)], fill=MILK_TEA, outline=INK)
    d.line((6, 6, 25, 6), fill=WHITE)                          # opercule
    for x, y in [(11, 27), (15, 28), (19, 27), (13, 24), (17, 25)]:
        d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=TAPIOCA)
    return img


def icon() -> None:
    """Icône 32x32 de la ROM, 16 couleurs max."""
    img = Image.new("RGB", (32, 32), MINT)
    c = cup()
    for y in range(32):
        for x in range(32):
            px = c.getpixel((x, y))
            if px != MAGENTA:
                img.putpixel((x, y), px)
    out = ROOT / "icon.png"
    img.quantize(colors=16).save(out)
    print("wrote", out.relative_to(ROOT))


if __name__ == "__main__":
    save_sprite(pearl(), "pearl", GRIT_SPRITE_16)
    save_sprite(customer(), "customer", GRIT_SPRITE_16)
    save_sprite(cup(), "cup", GRIT_SPRITE_16)
    icon()
