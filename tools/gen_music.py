#!/usr/bin/env python3
"""Génère une petite boucle lo-fi au format ProTracker MOD (tâche 7.2).

4 voies : mélodie, arpèges, basse, batterie. Progression Fmaj7 - Em7 - Dm7 - Cmaj7.
Sortie : audio/music_shop.mod (identifiant maxmod MOD_MUSIC_SHOP).
"""
import math
import random
import struct
from pathlib import Path

OUT = Path(__file__).resolve().parent.parent / "audio" / "music_shop.mod"
random.seed(7)

ROWS = 64
CH = 4


def period(midi):
    """Numéro MIDI -> période Amiga ; C-2 (MIDI 60 ici) = 428 pour un cycle de 32 échantillons."""
    return int(round(428 * 2 ** (-(midi - 60) / 12)))


def cycle(fn, n=32, amp=100):
    return bytes((int(fn(i / n) * amp) & 0xFF) for i in range(n))


def sample_lead():   # triangle doux
    return cycle(lambda p: (4 * p - 1) if p < 0.5 else (3 - 4 * p), amp=90)


def sample_keys():   # carré arrondi
    return cycle(lambda p: math.tanh(4 * math.sin(2 * math.pi * p)) * 0.8, amp=80)


def sample_bass():   # sinus + harmonique
    return cycle(lambda p: math.sin(2 * math.pi * p) * 0.8 + 0.2 * math.sin(4 * math.pi * p), amp=110)


def sample_hat():
    rnd = random.Random(1)
    n = 600
    return bytes((int(rnd.uniform(-1, 1) * 60 * math.exp(-i / 90)) & 0xFF) for i in range(n))


def sample_kick():
    n = 1600
    out, ph = [], 0.0
    for i in range(n):
        t = i / 8287
        ph += (150 * math.exp(-t * 25) + 45) / 8287
        out.append(int(math.sin(2 * math.pi * ph) * 120 * math.exp(-t * 9)) & 0xFF)
    return bytes(out)


# (données, volume, boucle ?)
SAMPLES = [
    ("lead", sample_lead(), 40, True),
    ("keys", sample_keys(), 30, True),
    ("bass", sample_bass(), 48, True),
    ("hat", sample_hat(), 26, False),
    ("kick", sample_kick(), 56, False),
]
S_LEAD, S_KEYS, S_BASS, S_HAT, S_KICK = 1, 2, 3, 4, 5

# Accords (MIDI) : Fmaj7, Em7, Dm7, Cmaj7 — 60 = do central
CHORDS = [
    [53, 57, 60, 64],
    [52, 55, 59, 62],
    [50, 53, 57, 60],
    [48, 52, 55, 59],
]
PENTA = [60, 62, 64, 67, 69, 72, 74, 76]


def empty_pattern():
    return [[None] * CH for _ in range(ROWS)]


def put(pat, row, ch, smp=0, midi=None, eff=0, param=0):
    pat[row][ch] = (smp, period(midi) if midi else 0, eff, param)


def decay(pat, row, ch, length, speed):
    """Ajoute un fondu (volume slide) sur les lignes suivantes, puis coupe."""
    for r in range(row + 1, min(ROWS, row + length)):
        if pat[r][ch] is None:
            pat[r][ch] = (0, 0, 0xA, speed)


def build_pattern(variant):
    pat = empty_pattern()
    for bar, chord in enumerate(CHORDS):
        base = bar * 16
        # Basse : fondamentale sur 1 et 3, quinte en levée
        root = chord[0] - 12
        put(pat, base, 2, S_BASS, root)
        decay(pat, base, 2, 6, 0x02)
        put(pat, base + 8, 2, S_BASS, root)
        decay(pat, base + 8, 2, 4, 0x02)
        put(pat, base + 14, 2, S_BASS, root + 7)
        decay(pat, base + 14, 2, 2, 0x04)
        # Arpèges en croches
        for k in range(8):
            n = chord[[0, 2, 1, 3, 2, 1, 3, 2][k]] + 12
            put(pat, base + k * 2, 1, S_KEYS, n, 0xC, 0x18 if k % 2 else 0x22)
            decay(pat, base + k * 2, 1, 2, 0x06)
        # Batterie : kick sur 1 et 3 (+ syncope), charleston sur les contretemps
        for k in range(0, 16, 2):
            if k in (0, 6, 8) or (variant and k == 12):
                put(pat, base + k, 3, S_KICK, 60)
            else:
                put(pat, base + k, 3, S_HAT, 60, 0xC, 0x14 if k % 4 else 0x1C)
        # Mélodie : rare sur la variante A, plus présente sur B
        rnd = random.Random(bar * 13 + variant * 101)
        steps = [0, 6, 10] if variant == 0 else [0, 3, 6, 8, 11, 14]
        for s in steps:
            if rnd.random() < 0.25 and s:
                continue
            tones = [n for n in PENTA if (n % 12) in [c % 12 for c in chord]] or PENTA
            n = rnd.choice(tones if s % 4 == 0 else PENTA) + 12
            put(pat, base + s, 0, S_LEAD, n)
            decay(pat, base + s, 0, 4 if variant else 6, 0x03)
    # Vitesse 6, tempo 84 BPM sur la première ligne de la voie mélodie si libre
    first = pat[0][0]
    smp, per = (first[0], first[1]) if first else (0, 0)
    pat[0][0] = (smp, per, 0xF, 84)
    if pat[1][0] is None:
        pat[1][0] = (0, 0, 0xF, 6)
    return pat


def encode_pattern(pat):
    out = bytearray()
    for row in pat:
        for cell in row:
            smp, per, eff, par = cell if cell else (0, 0, 0, 0)
            out += bytes([(smp & 0xF0) | ((per >> 8) & 0x0F), per & 0xFF,
                          ((smp & 0x0F) << 4) | (eff & 0x0F), par & 0xFF])
    return out


def oneshot(data):
    """Son joué une seule fois : en MOD, les 2 premiers octets sont rejoués en boucle
    après la fin (replen = 1). Ils doivent être nuls, sinon on entend un bourdonnement."""
    data = b"\0\0" + data
    if len(data) % 2:
        data += b"\0"
    return data


def main():
    for i, (name, smp, vol, loop) in enumerate(SAMPLES):
        if not loop:
            SAMPLES[i] = (name, oneshot(smp), vol, loop)
    patterns = [build_pattern(0), build_pattern(1)]
    order = [0, 1, 0, 1]
    data = bytearray(b"Pearl Panic cozy".ljust(20, b"\0"))
    for i in range(31):
        if i < len(SAMPLES):
            name, smp, vol, loop = SAMPLES[i]
            words = len(smp) // 2
            data += name.encode().ljust(22, b"\0")
            data += struct.pack(">HBBHH", words, 0, vol, 0 if loop else 0, words if loop else 1)
        else:
            data += b"\0" * 22 + struct.pack(">HBBHH", 0, 0, 0, 0, 1)
    data += bytes([len(order), 127])
    data += bytes(order + [0] * (128 - len(order)))
    data += b"M.K."
    for p in patterns:
        data += encode_pattern(p)
    for _, smp, _, _ in SAMPLES:
        data += smp[: (len(smp) // 2) * 2]
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_bytes(bytes(data))
    print("music ->", OUT.relative_to(OUT.parent.parent), len(data), "octets")


if __name__ == "__main__":
    main()
