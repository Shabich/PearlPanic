"""Palette maître de Pearl Panic (tâche 2.1).

Règles de la direction artistique :
- pas de noir pur : le trait le plus sombre est une prune (INK) ;
- chaque matière a 3-4 tons, du plus clair (lumière en haut à gauche) au plus sombre ;
- contours « sélectifs » : on détoure avec le ton le plus sombre de la matière,
  l'encre prune est réservée aux yeux, bouches et petits détails ;
- couleurs pastel, saturées juste ce qu'il faut pour rester lisibles sur l'écran DS.
"""

INK = (66, 40, 62)
INK_SOFT = (122, 88, 112)
WHITE = (255, 255, 255)

CREAM = [(255, 250, 240), (255, 240, 222), (240, 218, 196), (212, 182, 160)]
WOOD = [(240, 196, 150), (220, 164, 120), (184, 124, 92), (136, 88, 72), (98, 62, 58)]
PINK = [(255, 228, 234), (255, 194, 210), (240, 146, 176), (204, 100, 140), (150, 64, 104)]
MINT = [(222, 248, 234), (178, 230, 206), (128, 198, 170), (82, 152, 132), (52, 106, 100)]
LAVENDER = [(240, 234, 255), (212, 198, 248), (170, 150, 226), (124, 104, 186)]
PEACH = [(255, 236, 214), (255, 210, 172), (240, 168, 128), (200, 120, 96)]
YELLOW = [(255, 246, 200), (255, 228, 140), (240, 188, 84), (196, 136, 60)]
SKY = [(232, 246, 255), (196, 228, 250), (150, 196, 236), (108, 150, 204)]
GREY = [(246, 242, 246), (218, 210, 220), (178, 166, 182), (130, 116, 136)]
RED = [(255, 150, 150), (232, 88, 104), (176, 52, 84)]
LEAF = [(190, 226, 150), (140, 196, 116), (92, 152, 96), (60, 108, 80)]

BLUSH = (255, 150, 170)
SWEAT = (150, 206, 255)


def ramp(base, n=4, light=0.35, dark=0.45):
    """Construit une rampe claire -> sombre autour d'une couleur, avec un léger
    décalage de teinte (lumière chaude, ombres froides et violacées)."""
    r, g, b = base
    out = []
    for i in range(n):
        t = i / (n - 1)
        if t < 0.34:
            k = light * (0.34 - t) / 0.34
            c = (r + (255 - r) * k, g + (250 - g) * k, b + (235 - b) * k)
        else:
            k = dark * (t - 0.34) / 0.66
            c = (r + (90 - r) * k, g + (60 - g) * k, b + (110 - b) * k)
        out.append(tuple(int(max(0, min(255, v))) for v in c))
    return out
