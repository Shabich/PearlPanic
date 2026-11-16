#!/usr/bin/env python3
"""Synthétise les effets sonores placeholder (tâche 7.3) en WAV 16 bits mono.

Les fichiers audio/snd_*.wav sont convertis par mmutil au build ;
l'identifiant maxmod correspondant est SFX_SND_<NOM>.
"""
import math
import random
import struct
import wave
from pathlib import Path

RATE = 16000
OUT = Path(__file__).resolve().parent.parent / "audio"
random.seed(42)


def env(t, dur, attack=0.005, release=None):
    release = release if release is not None else dur * 0.6
    if t < attack:
        return t / attack
    if t > dur - release:
        return max(0.0, (dur - t) / release)
    return 1.0


def square(phase, duty=0.5):
    return 1.0 if (phase % 1.0) < duty else -1.0


def tri(phase):
    p = phase % 1.0
    return 4 * p - 1 if p < 0.5 else 3 - 4 * p


def render(dur, fn):
    n = int(RATE * dur)
    return [fn(i / RATE) for i in range(n)]


def save(name, samples, gain=0.6):
    OUT.mkdir(exist_ok=True)
    with wave.open(str(OUT / f"snd_{name}.wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(
            struct.pack("<h", int(max(-1, min(1, s * gain)) * 32000)) for s in samples))


class Noise:
    """Bruit filtré passe-bas (lissage exponentiel)."""
    def __init__(self, smooth):
        self.v = 0.0
        self.smooth = smooth

    def __call__(self):
        self.v += (random.uniform(-1, 1) - self.v) * self.smooth
        return self.v


def tone_seq(notes, note_dur, wave_fn=tri, duty=None):
    """Suite de notes (fréquences) jouées l'une après l'autre."""
    out = []
    for f in notes:
        ph = 0.0
        for i in range(int(RATE * note_dur)):
            t = i / RATE
            ph += f / RATE
            s = wave_fn(ph) if duty is None else square(ph, duty)
            out.append(s * env(t, note_dur, release=note_dur * 0.7))
    return out


def note(n):
    """Numéro MIDI -> fréquence."""
    return 440.0 * 2 ** ((n - 69) / 12)


def pour():
    nz = Noise(0.15)
    def f(t):
        bubble = 0.5 + 0.5 * math.sin(2 * math.pi * 23 * t)
        return nz() * 2.2 * bubble * env(t, 0.22, 0.02, 0.08)
    return render(0.22, f)


def plop():
    ph = [0.0]
    def f(t):
        fr = 900 * math.exp(-t * 30) + 180
        ph[0] += fr / RATE
        return math.sin(2 * math.pi * ph[0]) * env(t, 0.09, 0.002, 0.07)
    return render(0.09, f)


def pump():
    nz = Noise(0.4)
    ph = [0.0]
    def f(t):
        ph[0] += (220 - t * 600) / RATE
        return (0.6 * nz() + 0.5 * tri(ph[0])) * env(t, 0.1, 0.003, 0.08)
    return render(0.1, f)


def seal():
    nz = Noise(0.05)
    def f(t):
        nz.smooth = 0.05 + t * 2.5
        return nz() * 1.6 * env(t, 0.28, 0.01, 0.06)
    return render(0.28, f)


def shake():
    nz = Noise(0.6)
    return render(0.06, lambda t: nz() * env(t, 0.06, 0.002, 0.05))


def trash():
    nz = Noise(0.5)
    def f(t):
        nz.smooth = max(0.03, 0.5 - t * 2.2)
        return nz() * 1.5 * env(t, 0.25, 0.005, 0.15)
    return render(0.25, f)


def cash():
    return tone_seq([note(95), note(100)], 0.12, duty=0.25) + [0.0] * 800


def happy():
    return tone_seq([note(72), note(76), note(79), note(84)], 0.07)


def angry():
    ph = [0.0]
    def f(t):
        ph[0] += (300 - t * 500) / RATE
        return square(ph[0], 0.4) * 0.6 * env(t, 0.35, 0.01, 0.2)
    return render(0.35, f)


def unlock():
    seq = tone_seq([note(n) for n in (79, 84, 88, 91, 96)], 0.07, duty=0.25)
    return seq + [s * 0.4 for s in tone_seq([note(96)], 0.25)]


def bell():
    out = []
    for f0, start in ((note(88), 0.0), (note(84), 0.12)):
        n0 = int(start * RATE)
        seg = render(0.3, lambda t, f0=f0: (math.sin(2 * math.pi * f0 * t)
                                            + 0.3 * math.sin(2 * math.pi * f0 * 2.7 * t))
                     * math.exp(-t * 12))
        if len(out) < n0 + len(seg):
            out += [0.0] * (n0 + len(seg) - len(out))
        for i, s in enumerate(seg):
            out[n0 + i] += s * 0.6
    return out


def click():
    return render(0.025, lambda t: square(t * 2000) * env(t, 0.025, 0.001, 0.02) * 0.5)


if __name__ == "__main__":
    for name, fn in (("pour", pour), ("plop", plop), ("pump", pump), ("seal", seal),
                     ("shake", shake), ("trash", trash), ("cash", cash), ("happy", happy),
                     ("angry", angry), ("unlock", unlock), ("bell", bell), ("click", click)):
        save(name, fn())
    print("sfx -> audio/")
