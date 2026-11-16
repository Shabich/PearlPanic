#!/usr/bin/env python3
"""Convertit gfx/**/*.png en tableaux C RGB15 (source/gen/assets.{c,h})
et la police bitmap en source/gen/font_data.c.

Pixel = RGB15 | bit 15 (opaque) ; 0 = transparent.
Nom C : chemin relatif, '/' -> '_' ; ex. gfx/icons/pearl.png -> IMG_icons_pearl.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
GFX = ROOT / "gfx"
GEN = ROOT / "source" / "gen"
HEADER = "// Fichier généré par tools/png2c.py — ne pas modifier à la main.\n"


def rgb15(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | 0x8000


def convert_images():
    pngs = sorted(GFX.rglob("*.png"))
    c = [HEADER, '#include "gen/assets.h"\n']
    h = [HEADER, "#ifndef PP_GEN_ASSETS_H\n#define PP_GEN_ASSETS_H\n\n",
         '#include "gfx/canvas.h"\n\n']
    for p in pngs:
        rel = p.relative_to(GFX).with_suffix("")
        name = "IMG_" + "_".join(rel.parts)
        img = Image.open(p).convert("RGBA")
        w, hgt = img.size
        raw = img.tobytes()
        # Palette propre à l'image : index 0 = transparent
        palette, lookup, idx = [0], {}, []
        for i in range(0, len(raw), 4):
            if raw[i + 3] < 128:
                idx.append(0)
                continue
            col = rgb15(raw[i], raw[i + 1], raw[i + 2])
            if col not in lookup:
                if len(palette) >= 256:
                    raise SystemExit(f"{p}: plus de 255 couleurs")
                lookup[col] = len(palette)
                palette.append(col)
            idx.append(lookup[col])
        rows = []
        for i in range(0, len(idx), 32):
            rows.append("    " + ",".join(str(v) for v in idx[i:i + 32]) + ",")
        c.append(f"\nstatic const uint8_t {name}_idx[{len(idx)}] = {{\n" + "\n".join(rows) + "\n};\n")
        c.append(f"static const uint16_t {name}_pal[{len(palette)}] = {{ "
                 + ", ".join(f"0x{v:04X}" for v in palette) + " };\n")
        c.append(f"const Image {name} = {{ {w}, {hgt}, {name}_idx, {name}_pal }};\n")
        h.append(f"extern const Image {name};\n")
    h.append("\n#endif\n")
    (GEN / "assets.c").write_text("".join(c))
    (GEN / "assets.h").write_text("".join(h))
    print(f"{len(pngs)} images -> source/gen/assets.c")


def convert_font():
    """Police 6x11 (Latin-1, 0x20..0xFF) issue de la police bitmap par défaut de Pillow."""
    font = ImageFont.load_default_imagefont()
    W, H = 6, 11
    out = [HEADER, '#include "gfx/font.h"\n\n',
           f"const uint8_t FONT_DATA[{224 * H}] = {{\n"]
    for code in range(0x20, 0x100):
        img = Image.new("1", (W, H), 0)
        ImageDraw.Draw(img).text((0, 0), chr(code), font=font, fill=1)
        rows = []
        for y in range(H):
            bits = 0
            for x in range(W):
                if img.getpixel((x, y)):
                    bits |= 1 << x
            rows.append(f"0x{bits:02X}")
        out.append(f"    {', '.join(rows)}, // 0x{code:02X}\n")
    out.append("};\n")
    (GEN / "font_data.c").write_text("".join(out))
    print("font -> source/gen/font_data.c")


if __name__ == "__main__":
    GEN.mkdir(parents=True, exist_ok=True)
    convert_images()
    convert_font()
