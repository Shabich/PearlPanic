#!/usr/bin/env python3
"""Assemble les séquences de captures (make snap) en planches pour vérifier les animations.

Usage : python3 tools/seq_sheet.py build/snap PREFIX x y w h [scale]
  (coordonnées dans la capture 512x768, déjà agrandie x2)
"""
import sys
from pathlib import Path
from PIL import Image

folder, prefix = Path(sys.argv[1]), sys.argv[2]
x, y, w, h = map(int, sys.argv[3:7])
files = sorted(folder.glob(f"seq_{prefix}_*.png"))
ims = [Image.open(f).crop((x, y, x + w, y + h)) for f in files]
sheet = Image.new("RGB", (len(ims) * (w + 4), h), (40, 40, 40))
for i, im in enumerate(ims):
    sheet.paste(im, (i * (w + 4), 0))
out = folder / f"sheet_{prefix}.png"
sheet.save(out)
print(out)
