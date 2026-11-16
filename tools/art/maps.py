"""Cartes des mondes (écran du haut, 256x192), façon carte de niveaux.

Image fixe : sols texturés, chemin pavé, maisons et arbres en volume, eau.
Le jeu dessine par-dessus les cases, Perle et les animations ; les positions
utiles (cases, cheminées, eau, néons…) sont exportées dans source/gen/map_nodes.h.
"""
import math
import random
from pathlib import Path

from .palette import CREAM, WOOD, PINK, MINT, LAVENDER, PEACH, YELLOW, SKY, GREY, LEAF, WHITE, INK, ramp
from .pixel import Canvas, Mask, shade, fill
from .texture import (textured_fill, tufts, speckle, shadow, ellipse_shadow, tree, house,
                      cobble_path, water, darken)

W, H = 256, 192

# Types d'animations exportés (cf. scenes/map.c)
A_SMOKE, A_WATER, A_LANTERN, A_WINDOW, A_NEON, A_BOAT, A_CARS, A_FIREFLIES, A_GLOW, A_FOUNTAIN, A_GULLS = range(11)

NODES = []
BAR = []
ANIMS = []          # par monde : liste de (type, x, y, w, h, param)

GRASS = [(206, 236, 160), (186, 224, 142), (166, 210, 128), (146, 194, 116)]
BLOSSOM = [(255, 236, 242), (255, 206, 222), (240, 166, 194), (200, 120, 156)]


def bar_building(c, x, y, roof, sign):
    """Le bar de Perle (dernière étape de chaque carte) : auvent rayé et enseigne."""
    house(c, x - 15, y - 6, 30, 18, roof, (255, 248, 236), chimney=False, door=PINK[3])
    for k in range(7):
        c.rect(x - 17 + k * 5, y - 7, x - 13 + k * 5, y - 4, PINK[1] if k % 2 else WHITE)
    c.hline(x - 17, x + 17, y - 3, PINK[3])
    fill(c, Mask(W, H).round_rect(x - 9, y - 26, x + 9, y - 19, 2), sign, outline=darken(sign, 0.6))
    c.hline(x - 6, x + 6, y - 23, WHITE)


# ===================================================================== Monde 1
def map_quartier():
    c = Canvas(W, H)
    textured_fill(c, 0, 0, W - 1, H - 1, GRASS, seed=11)
    tufts(c, lambda x, y: True, 420, GRASS[0], GRASS[3], seed=12)
    speckle(c, lambda x, y: True, 90, [PINK[1], YELLOW[1], WHITE, LAVENDER[1]], seed=13)
    anims = []
    # Canal en travers, avec berges en pierre
    canal = set()
    for x in range(W):
        yc = 104 + int(6 * math.sin(x / 40))
        for y in range(yc - 7, yc + 8):
            canal.add((x, y))
    for (x, y) in canal:
        if (x, y - 2) not in canal or (x, y + 2) not in canal:
            c.set(x, y, GREY[2] if (x // 3) % 2 else GREY[1])
    inner = {(x, y) for (x, y) in canal if (x, y - 2) in canal and (x, y + 2) in canal}
    water(c, inner, [(170, 214, 240), (140, 196, 232), (112, 170, 214)], seed=14)
    anims.append((A_WATER, 0, 96, W, 16, 0))
    # Rangée de maisons en haut, et quelques-unes en bas
    roofs = [PINK[2], LAVENDER[2], PEACH[2], (120, 180, 200), YELLOW[2], (200, 130, 130)]
    rnd = random.Random(15)
    for i, (x, y) in enumerate(((10, 50), (48, 44), (86, 52), (124, 46), (162, 52), (96, 172), (232, 168))):
        chim = house(c, x, y, 26, 18, roofs[i % len(roofs)])
        if chim:
            anims.append((A_SMOKE, chim[0], chim[1], 0, 0, 0))
    # Cerisiers en fleurs
    for (x, y) in ((214, 84), (30, 128), (60, 182), (176, 186), (250, 140), (138, 82)):
        tree(c, x, y, BLOSSOM, r=8)
    # Place avec fontaine
    fill(c, Mask(W, H).ellipse(36, 82, 14, 8), (230, 222, 214), outline=GREY[2])
    fill(c, Mask(W, H).ellipse(36, 82, 8, 4), (150, 200, 236), outline=GREY[3])
    anims.append((A_FOUNTAIN, 36, 80, 0, 0, 0))
    # Guirlandes de lanternes
    for x in range(8, W, 24):
        y = 22 + int(3 * math.sin(x / 12))
        c.vline(x, 18, y - 3, INK)
        anims.append((A_LANTERN, x, y, 0, 0, 0))
    nodes = [(34, 160), (98, 140), (160, 148), (196, 84)]
    bar = (228, 50)
    cobble_path(c, nodes + [bar], [(250, 236, 214), (232, 214, 190), (210, 188, 166)],
                (176, 150, 132), (150, 120, 104), seed=16, bridge=canal,
                planks=[(214, 160, 116), (190, 136, 98), (110, 74, 60), (150, 100, 76)])
    bar_building(c, *bar, PINK[2], PINK[1])
    return c, nodes, bar, anims


# ===================================================================== Monde 2
def map_plage():
    c = Canvas(W, H)
    SAND = [(255, 242, 206), (250, 234, 194), (244, 224, 180)]
    SEA = [(140, 208, 240), (118, 194, 234), (100, 178, 224)]
    textured_fill(c, 0, 0, W - 1, H - 1, SAND, seed=21, cell=20, fine=0.1)
    speckle(c, lambda x, y: True, 220, [(226, 200, 150), WHITE], seed=22)
    # Mer en haut avec un rivage ondulé
    sea = set()
    for x in range(W):
        shore = 66 + int(5 * math.sin(x / 18) + 3 * math.sin(x / 7))
        for y in range(0, shore):
            sea.add((x, y))
    water(c, sea, SEA, seed=23)
    for x in range(W):   # écume
        shore = 66 + int(5 * math.sin(x / 18) + 3 * math.sin(x / 7))
        c.set(x, shore, WHITE); c.set(x, shore + 1, (230, 246, 250))
        c.set(x, shore + 2, (226, 200, 150))
    anims = [(A_WATER, 0, 16, W, 50, 1), (A_GULLS, 0, 20, W, 40, 0)]
    # Ponton et bateau
    for y in range(40, 74):
        c.hline(120, 128, y, WOOD[1] if y % 3 else WOOD[2])
    shadow(c, Mask(W, H).rect(120, 40, 128, 73), 2, 2)
    anims.append((A_BOAT, 142, 34, 0, 0, 0))
    # Palmiers avec ombre
    for (x, y) in ((22, 96), (232, 92), (60, 176), (204, 180)):
        ellipse_shadow(c, x + 8, y + 16, 12, 4)
        for k in range(18):
            c.rect(x + k // 6, y + k, x + 2 + k // 6, y + k, WOOD[3] if k % 3 else WOOD[2])
        for k in range(6):
            ang = -3.1 + k * 0.62
            leaf = Mask(W, H).ellipse(x + 1 + math.cos(ang) * 9, y + math.sin(ang) * 4 - 1, 9, 3)
            shade(c, leaf, LEAF)
        fill(c, Mask(W, H).ellipse(x + 1, y, 2.5, 2.5), WOOD[3])
    # Parasols et serviettes
    for (x, y, col) in ((90, 92, PINK[2]), (170, 100, (120, 190, 230)), (140, 176, YELLOW[2])):
        c.rect(x - 9, y + 8, x + 3, y + 14, col)
        c.hline(x - 9, x + 3, y + 10, WHITE)
        ellipse_shadow(c, x + 4, y + 6, 10, 4)
        um = Mask(W, H).ellipse(x, y, 10, 6)
        fill(c, um, col, outline=darken(col, 0.7))
        for k in range(-8, 9, 4):
            c.vline(x + k, y - 4, y + 4, WHITE if (k // 4) % 2 else col)
    # Rochers et coquillages
    for (x, y) in ((8, 140), (248, 130), (110, 120)):
        ellipse_shadow(c, x + 2, y + 3, 7, 3)
        shade(c, Mask(W, H).ellipse(x, y, 7, 5), [GREY[0], GREY[1], GREY[2], GREY[3]])
    speckle(c, lambda x, y: y > 80, 30, [PINK[1], PEACH[1]], seed=24)
    nodes = [(32, 150), (88, 130), (148, 154), (196, 120)]
    bar = (226, 150)
    cobble_path(c, nodes + [bar], [(214, 170, 120), (196, 150, 104), (176, 130, 90)],
                (140, 100, 76), (120, 86, 66), width=5, seed=25)
    bar_building(c, *bar, (240, 200, 110), (120, 190, 230))
    return c, nodes, bar, anims


# ===================================================================== Monde 3
def map_montagne():
    c = Canvas(W, H)
    SNOW = [(252, 252, 255), (244, 247, 255), (234, 240, 252)]
    textured_fill(c, 0, 0, W - 1, H - 1, SNOW, seed=31, cell=22, fine=0.08)
    speckle(c, lambda x, y: y > 86, 120, [(214, 224, 246), WHITE], seed=34)
    # Montagnes au fond (face éclairée / face à l'ombre)
    for (x, h, w) in ((40, 70, 64), (128, 84, 76), (214, 66, 60)):
        left = Mask(W, H).polygon([(x - w, 86), (x, 86 - h), (x, 86)])
        right = Mask(W, H).polygon([(x, 86 - h), (x + w, 86), (x, 86)])
        fill(c, left, (196, 200, 232))
        fill(c, right, (160, 164, 206))
        cap = Mask(W, H).polygon([(x - w // 3, 86 - h * 2 // 3), (x, 86 - h), (x + w // 3, 86 - h * 2 // 3),
                                  (x + w // 6, 86 - h * 3 // 5), (x, 86 - h * 2 // 3), (x - w // 6, 86 - h * 3 // 5)])
        fill(c, cap, WHITE)
    for y in range(16, 30):
        c.hline(0, W - 1, y, (236, 242, 255)) if False else None
    # Lac gelé
    lake = Mask(W, H).ellipse(44, 128, 30, 13)
    fill(c, lake, (206, 230, 246), outline=(170, 196, 226))
    for k in range(6):
        c.hline(28 + k * 5, 32 + k * 5, 122 + (k % 3) * 4, WHITE)
    anims = [(A_WATER, 16, 118, 56, 20, 2)]
    # Forêts de sapins enneigés
    def pine(x, y, s=1.0):
        ellipse_shadow(c, x + 4, y + 18, 8, 3, k=0.85)
        for k, w in enumerate((5, 8, 11)):
            tri = Mask(W, H).polygon([(x - w * s, y + k * 6 + 5), (x, y + k * 6 - 6), (x + w * s, y + k * 6 + 5)])
            shade(c, tri, [(120, 176, 140), (84, 146, 116), (62, 118, 98), (40, 84, 76)])
            c.hline(int(x - 2), int(x + 2), y + k * 6 - 4, WHITE)
            c.hline(int(x - w * s + 2), int(x - w * s + 4), y + k * 6 + 4, WHITE)
        c.rect(x - 1, y + 17, x + 1, y + 20, WOOD[3])
    for (x, y) in ((14, 90), (100, 96), (124, 100), (230, 104), (244, 170), (12, 170), (110, 178), (176, 96)):
        pine(x, y)
    # Chalets fumants
    for (x, y, roof) in ((60, 168, (150, 90, 80)), (168, 176, (120, 100, 150))):
        chim = house(c, x, y, 24, 14, roof, (226, 180, 140), door=WOOD[3])
        anims.append((A_SMOKE, chim[0], chim[1], 0, 0, 0))
        c.hline(x - 3, x + 27, y - 10, WHITE)
    nodes = [(36, 166), (96, 148), (150, 124), (204, 140)]
    bar = (220, 70)
    cobble_path(c, nodes + [bar], [(214, 196, 176), (196, 176, 156), (176, 156, 138)],
                (150, 130, 120), (130, 112, 104), width=5, seed=33)
    bar_building(c, *bar, (150, 90, 80), (200, 120, 90))
    anims.append((A_SMOKE, bar[0] + 10, bar[1] - 20, 0, 0, 0))
    return c, nodes, bar, anims


# ===================================================================== Monde 4
def map_ville():
    c = Canvas(W, H)
    ASPHALT = [(78, 74, 108), (70, 66, 100), (62, 58, 92)]
    textured_fill(c, 0, 0, W - 1, H - 1, ASPHALT, seed=41, cell=8)
    anims = []
    rnd = random.Random(42)
    # Routes : une horizontale, une verticale ; trottoirs
    roads = [(0, 100, W - 1, 114), (116, 16, 130, H - 1)]
    for (x0, y0, x1, y1) in roads:
        c.rect(x0, y0, x1, y1, (54, 50, 80))
        if x1 - x0 > y1 - y0:
            for x in range(0, W, 14):
                c.hline(x, x + 6, (y0 + y1) // 2, (240, 220, 140))
        else:
            for y in range(16, H, 14):
                c.vline((x0 + x1) // 2, y, y + 6, (240, 220, 140))
    anims.append((A_CARS, 0, 103, W, 8, 0))
    # Pâtés de maisons vus du dessus, avec toits-terrasses
    blocks = [(6, 22, 104, 92), (140, 22, 250, 92), (6, 122, 104, 186), (140, 122, 250, 186)]
    neon_cols = [(255, 120, 200), (120, 240, 255), (255, 230, 120), (180, 140, 255)]
    for bi, (x0, y0, x1, y1) in enumerate(blocks):
        c.rect(x0 - 2, y0 - 2, x1 + 2, y1 + 2, (110, 106, 140))   # trottoir
        x = x0
        while x < x1 - 10:
            w = rnd.randint(18, 30)
            w = min(w, x1 - x)
            top = Mask(W, H).rect(x, y0, x + w - 2, y1)
            col = rnd.choice([(96, 82, 140), (84, 92, 150), (110, 88, 150), (90, 74, 126)])
            shadow(c, top, 3, 3, k=0.7)
            fill(c, top, col, outline=darken(col, 0.65))
            c.hline(x, x + w - 2, y0, darken(col, 1.25) if max(col) < 200 else col)
            # Fenêtres éclairées sur la façade (vue plongeante)
            for wy in range(y0 + 4, y1 - 3, 7):
                for wx in range(x + 3, x + w - 5, 5):
                    if rnd.random() < 0.45:
                        lit = rnd.choice([(255, 230, 150), (255, 190, 220), (150, 230, 255)])
                        c.rect(wx, wy, wx + 2, wy + 2, lit)
                        if rnd.random() < 0.25:
                            anims.append((A_WINDOW, wx, wy, 3, 3, 0))
            # Climatiseur / réservoir sur le toit
            if rnd.random() < 0.6:
                c.rect(x + 3, y0 + 2, x + 7, y0 + 5, (150, 146, 170))
            x += w
        # Enseigne néon par pâté
        nx, ny = x0 + 6 + (bi * 13) % 40, y1 - 10 if bi < 2 else y0 + 4
        anims.append((A_NEON, nx, ny, 22, 5, bi % len(neon_cols)))
    # Petit parc
    park = Mask(W, H).round_rect(30, 136, 80, 176, 6)
    fill(c, park, (90, 140, 110), outline=(60, 100, 86))
    for (x, y) in ((44, 152), (66, 164), (56, 146)):
        tree(c, x, y, [(120, 190, 150), (84, 156, 120), (60, 120, 96), (40, 86, 76)], r=6)
    # Lampadaires
    for (x, y) in ((108, 96), (136, 96), (108, 120), (136, 120), (20, 96), (232, 120)):
        c.set(x, y, (200, 196, 220))
        anims.append((A_LANTERN, x, y, 0, 0, 1))
    nodes = [(20, 107), (84, 107), (123, 160), (196, 107)]
    bar = (236, 106)
    cobble_path(c, [nodes[0], nodes[1], (123, 107), nodes[2]], [(200, 150, 240), (176, 126, 220), (150, 106, 200)],
                (110, 80, 160), (240, 140, 220), width=4, seed=43, bend_amp=0)
    cobble_path(c, [nodes[2], (123, 107), nodes[3], bar], [(200, 150, 240), (176, 126, 220), (150, 106, 200)],
                (110, 80, 160), (240, 140, 220), width=4, seed=44, bend_amp=0)
    bar_building(c, *bar, (120, 90, 170), (120, 240, 255))
    anims.append((A_NEON, bar[0] - 9, bar[1] - 26, 19, 8, 0))
    return c, nodes, bar, anims


# ===================================================================== Monde 5
def map_foret():
    c = Canvas(W, H)
    FLOOR = [(150, 196, 120), (128, 180, 104), (108, 162, 92), (88, 140, 80)]
    textured_fill(c, 0, 0, W - 1, H - 1, FLOOR, seed=51, cell=12)
    tufts(c, lambda x, y: True, 380, FLOOR[0], FLOOR[3], seed=52)
    speckle(c, lambda x, y: True, 70, [(255, 200, 220), (255, 250, 200), (200, 180, 255)], seed=53)
    anims = [(A_FIREFLIES, 0, 16, W, H - 16, 0)]
    # Ruisseau sinueux
    stream = set()
    for y in range(16, H):
        xc = 70 + int(30 * math.sin(y / 26))
        for x in range(xc - 6, xc + 7):
            stream.add((x, y))
    for (x, y) in stream:
        if (x - 2, y) not in stream or (x + 2, y) not in stream:
            c.set(x, y, (110, 120, 96))
    inner = {(x, y) for (x, y) in stream if (x - 2, y) in stream and (x + 2, y) in stream}
    water(c, inner, [(170, 220, 230), (130, 196, 214), (100, 168, 196)], seed=54)
    anims.append((A_WATER, 34, 16, 72, H - 16, 3))
    # Grands arbres (feuillage dense) tout autour
    canopy = [(196, 232, 150), (150, 206, 120), (104, 168, 100), (66, 122, 84)]
    for (x, y, r) in ((14, 36, 12), (120, 34, 11), (180, 30, 12), (244, 40, 12), (232, 172, 11),
                      (150, 186, 10), (20, 120, 10), (246, 100, 9), (196, 80, 8)):
        tree(c, x, y, canopy, r=r)
    # Champignons lumineux
    for (x, y) in ((108, 100), (40, 160), (226, 140), (160, 60), (100, 176)):
        ellipse_shadow(c, x + 2, y + 4, 6, 2)
        c.rect(x - 1, y, x + 1, y + 4, CREAM[0])
        fill(c, Mask(W, H).ellipse(x, y - 1, 5, 3), (236, 110, 170), outline=(180, 70, 130))
        c.set(x - 2, y - 2, WHITE); c.set(x + 2, y - 1, WHITE)
        anims.append((A_GLOW, x, y - 1, 0, 0, 0))
    # Guirlande de lanternes entre deux arbres
    for x in range(132, 236, 13):
        y = 66 + int(5 * math.sin((x - 132) / 33 * math.pi))
        anims.append((A_LANTERN, x, y, 0, 0, 2))
    nodes = [(40, 140), (100, 130), (148, 100), (204, 116)]
    bar = (220, 64)
    cobble_path(c, nodes + [bar], [(214, 186, 150), (196, 166, 130), (176, 146, 112)],
                (130, 100, 80), (110, 86, 70), width=5, seed=55)
    # Pierres de gué là où le chemin traverse le ruisseau
    bar_building(c, *bar, (120, 160, 90), (200, 150, 230))
    return c, nodes, bar, anims


def write_header(gen_dir):
    lines = ["// Fichier généré par tools/art/maps.py — positions sur les cartes des mondes.",
             "#ifndef PP_GEN_MAP_NODES_H", "#define PP_GEN_MAP_NODES_H", "",
             "enum { MA_SMOKE, MA_WATER, MA_LANTERN, MA_WINDOW, MA_NEON, MA_BOAT, MA_CARS,",
             "       MA_FIREFLIES, MA_GLOW, MA_FOUNTAIN, MA_GULLS };", "",
             "typedef struct { unsigned char type; short x, y, w, h; unsigned char param; } MapAnim;", "",
             "static const short MAP_NODES[5][4][2] = {"]
    for nodes in NODES:
        lines.append("    { " + ", ".join(f"{{ {x}, {y} }}" for x, y in nodes) + " },")
    lines.append("};")
    lines.append("static const short MAP_BAR[5][2] = { " + ", ".join(f"{{ {x}, {y} }}" for x, y in BAR) + " };")
    most = max(len(a) for a in ANIMS)
    lines.append(f"#define MAP_ANIM_MAX {most}")
    lines.append("static const unsigned char MAP_ANIM_COUNT[5] = { " + ", ".join(str(len(a)) for a in ANIMS) + " };")
    lines.append("static const MapAnim MAP_ANIMS[5][MAP_ANIM_MAX] = {")
    for a in ANIMS:
        lines.append("    {")
        for (t, x, y, w, h, p) in a:
            lines.append(f"        {{ {t}, {x}, {y}, {w}, {h}, {p} }},")
        lines.append("    },")
    lines.append("};")
    lines += ["", "#endif", ""]
    (gen_dir / "map_nodes.h").write_text("\n".join(lines))


def generate(save):
    NODES.clear(); BAR.clear(); ANIMS.clear()
    for name, fn in (("quartier", map_quartier), ("plage", map_plage), ("montagne", map_montagne),
                     ("ville", map_ville), ("foret", map_foret)):
        c, nodes, bar, anims = fn()
        save(c.img, f"maps/{name}.png")
        NODES.append(nodes); BAR.append(bar); ANIMS.append(anims)
    write_header(Path(__file__).resolve().parent.parent.parent / "source" / "gen")
