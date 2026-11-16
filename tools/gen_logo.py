"""Logo Pearl Panic (docs/logos/, utilisé par le README) : la boutique de l'écran titre, vitrines pleines de clients,
file d'attente sur le trottoir et Perle devant la porte."""
import random
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "docs" / "logos"
GFX = ROOT / "gfx"

W, H = 256, 232
WHITE = (255, 255, 255)
INK = (70, 46, 58)
INK_SOFT = (140, 110, 120)
PINK_DARK = (225, 120, 145)
PINK = [(255, 228, 234), (255, 194, 210), (240, 146, 176), (204, 100, 140)]
FRAME = {(220, 164, 120), (136, 88, 72)}
WALL = [(255, 244, 230), (248, 228, 210), (236, 210, 190)]
YELLOW = [(255, 246, 200), (255, 228, 140), (240, 188, 84)]
LEAF = [(190, 226, 150), (140, 196, 116), (92, 152, 96), (60, 108, 80)]
SKY1 = (196, 228, 250)
STONE = [(178, 166, 182), (218, 210, 220), (232, 226, 234)]

rng = random.Random(7)
FONT = ImageFont.load_default_imagefont()


def sprite(name):
    return Image.open(GFX / "customers" / f"{name}.png").convert("RGBA")


def text_mask(s, scale):
    m = Image.new("1", (6 * len(s), 11), 0)
    d = ImageDraw.Draw(m)
    for i, ch in enumerate(s):
        d.text((i * 6, 0), ch, font=FONT, fill=1)
    return m.resize((m.width * scale, m.height * scale), Image.NEAREST)


def title(img, y):
    # Rendu identique à title.c : contour blanc (pas 2), ombre prune, texte rose
    m = text_mask("Pearl Panic", 3)
    x = (W - m.width) // 2
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            if dx or dy:
                img.paste(WHITE, (x + dx * 2, y + dy * 2), m)
    img.paste(INK, (x + 3, y + 3), m)
    img.paste(PINK_DARK, (x, y), m)
    sub = text_mask("~ bubble tea rush ~", 1)
    sx = (W - sub.width) // 2
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        img.paste(WHITE, (sx + dx, y + 40 + dy), sub)
    img.paste(INK_SOFT, (sx, y + 40), sub)


def cloud(d, cx, cy, size):
    def ell(x, y, rx, ry):
        d.ellipse((x - rx, y - ry, x + rx, y + ry), fill=WHITE)
    ell(cx, cy, 6 * size // 4, 3 * size // 4 + 1)
    ell(cx - size, cy + 1, size, size // 2 + 1)
    ell(cx + size, cy + 1, size, size // 2 + 1)
    d.line((cx - size * 2, cy + size, cx + size * 2, cy + size), fill=SKY1)


def heart(d, x, y, c, outline):
    for px, py in ((1, 0), (2, 0), (4, 0), (5, 0), (0, 1), (1, 1), (2, 1), (3, 1), (4, 1), (5, 1), (6, 1),
                   (0, 2), (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2), (1, 3), (2, 3), (3, 3), (4, 3),
                   (5, 3), (2, 4), (3, 4), (4, 4), (3, 5)):
        d.point((x + px, y + py), fill=c)
    d.point((x + 1, y + 1), fill=WHITE)


def sparkle(d, x, y, c):
    d.point((x, y), fill=WHITE)
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        d.point((x + dx, y + dy), fill=c)


def interior(box):
    """Intérieur du bar vu à travers la vitrine : mur crème à pois, lampes suspendues."""
    x0, y0, x1, y1 = box
    im = Image.new("RGBA", (x1 - x0 + 1, y1 - y0 + 1), WALL[1])
    d = ImageDraw.Draw(im)
    for y in range(2, im.height, 8):
        for x in range((y // 8) % 2 * 4 + 2, im.width, 8):
            d.point((x, y), fill=WALL[2])
    # étagère de bocaux en fond
    d.rectangle((0, 9, im.width, 10), fill=(184, 124, 92))
    for i, x in enumerate(range(3, im.width - 4, 9)):
        c = [PINK[1], (212, 198, 248), YELLOW[1], LEAF[0]][i % 4]
        d.rectangle((x, 3, x + 4, 8), fill=c, outline=(184, 124, 92))
    return im


def fill_window(img, box, crowd):
    x0, y0, x1, y1 = box
    base = img.copy()
    layer = interior(box)
    for name, sx, sy in crowd:
        layer.alpha_composite(sprite(name), (sx - x0, sy - y0))
    # lampes devant la foule
    d = ImageDraw.Draw(layer)
    for lx in (layer.width // 4, 3 * layer.width // 4):
        d.line((lx, 0, lx, 3), fill=INK_SOFT)
        d.pieslice((lx - 4, 2, lx + 4, 9), 180, 360, fill=YELLOW[1], outline=YELLOW[2])
    # reflets de vitre
    for k in (6, 13):
        for t in range(14):
            px, py = k + t, layer.height - 3 - t * 2
            if 0 <= py < layer.height:
                c = layer.getpixel((px, py))
                layer.putpixel((px, py), tuple((v + 255) // 2 for v in c[:3]) + (255,))
    # ne remplace que le verre : le cadre et les croisillons restent devant
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if base.getpixel((x, y))[:3] not in FRAME:
                img.putpixel((x, y), layer.getpixel((x - x0, y - y0)))


def main():
    bg = Image.open(GFX / "bg/title.png").convert("RGBA")
    img = Image.new("RGBA", (W, H), WHITE)
    img.alpha_composite(bg, (0, 0))
    d = ImageDraw.Draw(img)

    # trottoir prolongé vers le bas
    for y in range(192, H):
        d.line((0, y, W, y), fill=STONE[1])
    for y in (182, 206):
        d.line((0, y, W, y), fill=STONE[0])
    for x in range(0, W, 24):
        d.line((x, 182, x, 206), fill=STONE[0])
    for x in range(12, W, 24):
        d.line((x, 206, x, H), fill=STONE[0])
    d.line((0, 183, W, 183), fill=STONE[2])

    # ciel : nuages, cœurs, étincelles
    for cx, cy, s in ((30, 12, 6), (226, 22, 5), (128, 82, 4), (40, 74, 5), (214, 70, 4)):
        cloud(d, cx, cy, s)
    for x, y in ((14, 46), (236, 44), (58, 86), (196, 88)):
        sparkle(d, x, y, YELLOW[1])

    # vitrines bondées (verre : x 29..93 et 163..227, y 121..161)
    # rang du fond (sommets des têtes) puis rang de devant (visages entiers)
    left = [("lapin_0", 19, 113), ("hibou_0b", 41, 115), ("panda_0", 63, 114),
            ("renard_2", 8, 125), ("grenouille_1t", 30, 124), ("chat_1t", 52, 125), ("souris_0", 74, 126)]
    right = [("dragon_0", 153, 114), ("ours_1", 175, 113), ("carlin_1", 197, 115),
             ("poussin_0", 142, 126), ("pingouin_0t", 164, 124), ("ecureuil_0t", 186, 125), ("phoque_0b", 208, 126)]
    fill_window(img, (29, 121, 93, 161), left)
    fill_window(img, (163, 121, 227, 161), right)
    d = ImageDraw.Draw(img)

    # guirlande lumineuse sous le store
    for i, x in enumerate(range(14, 244, 9)):
        y = 112 + (2 if i % 2 else 0)
        d.line((x, y - 2, x + 9, 114 - (2 if i % 2 else 0) - 2), fill=INK_SOFT)
        d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=[YELLOW[1], PINK[1], (212, 198, 248)][i % 3])

    # plantes dans les pots
    for px in (9, 247):
        for dx, dy, c in ((-5, -6, LEAF[2]), (5, -6, LEAF[2]), (0, -10, LEAF[1]), (-3, -3, LEAF[1]), (3, -3, LEAF[1])):
            d.ellipse((px + dx - 4, 166 + dy - 4, px + dx + 4, 166 + dy + 4), fill=c, outline=LEAF[3])
        for fx, fy in ((-4, -10), (4, -8), (0, -14)):
            d.ellipse((px + fx - 2, 166 + fy - 2, px + fx + 2, 166 + fy + 2), fill=PINK[1], outline=PINK[3])

    # file d'attente sur le trottoir, Perle au centre devant la porte
    queue = ["biche_0", "tortue_1t", "escargot_0", "raton_0b", "marmotte_1",
             None, "herisson_0t", "papillon_0", "yeti_0", "poulpe_1t", "crabe_0"]
    xs = [-12, 10, 32, 54, 76, 108, 140, 162, 184, 206, 228]
    for name, x in zip(queue, xs):
        hop = -2 if rng.random() < 0.35 else 0
        if name is None:
            perle = Image.open(GFX / "heroine/perle_happy.png").convert("RGBA")
            img.alpha_composite(perle, (x, 176))
        else:
            img.alpha_composite(sprite(name), (x, 180 + hop))

    # petits cœurs qui s'envolent de la file
    for x, y, c in ((44, 168, PINK[2]), (196, 164, PINK[2]), (122, 160, PINK[1]), (100, 96, PINK[1]), (150, 92, PINK[1])):
        heart(d, x, y, c, PINK[3])

    title(img, 18)

    OUT.mkdir(parents=True, exist_ok=True)
    img = img.convert("RGB")
    img.save(OUT / "logo.png")
    img.resize((W * 3, H * 3), Image.NEAREST).save(OUT / "logo_grand.png")


main()
