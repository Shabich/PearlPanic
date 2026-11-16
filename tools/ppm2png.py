#!/usr/bin/env python3
"""Convertit les captures PPM d'un dossier en PNG agrandis x2 (make snap)."""
import sys
from pathlib import Path
from PIL import Image

folder = Path(sys.argv[1])
for ppm in sorted(folder.glob("*.ppm")):
    img = Image.open(ppm)
    img.resize((img.width * 2, img.height * 2), Image.NEAREST).save(ppm.with_suffix(".png"))
    ppm.unlink()
    print("wrote", ppm.with_suffix(".png"))
