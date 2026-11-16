// Courbes d'animation (tâche 1.9) — t et résultats en 1/256
#ifndef PP_TWEEN_H
#define PP_TWEEN_H

static inline int tw_clamp(int t) { return t < 0 ? 0 : t > 256 ? 256 : t; }

// Interpolation linéaire a -> b
static inline int tw_lerp(int a, int b, int t256) { return a + (b - a) * t256 / 256; }

// Progression d'un compteur de frames (0..duration) en 0..256
static inline int tw_progress(int frame, int duration)
{
    return duration <= 0 ? 256 : tw_clamp(frame * 256 / duration);
}

static inline int ease_in_quad(int t)
{
    t = tw_clamp(t);
    return t * t / 256;
}

static inline int ease_out_quad(int t)
{
    t = 256 - tw_clamp(t);
    return 256 - t * t / 256;
}

// Dépasse légèrement la cible puis revient (≈ +10 %)
static inline int ease_out_back(int t)
{
    t = tw_clamp(t);
    int u = t - 256;                         // -256..0
    // 1 + 2.7 u^3 + 1.7 u^2   (u normalisé)
    long long u2 = (long long)u * u / 256;
    long long u3 = u2 * u / 256;
    return (int)(256 + u3 * 27 / 10 + u2 * 17 / 10);
}

// Rebondit en arrivant (balle qui tombe) : courbe classique 7.5625 x², en 1/256
static inline int tw_bounce_seg(int x, int offset)
{
    int v = (int)((long long)x * x * 121 / 16 / 256) + offset;
    return v > 256 ? 256 : v;   // arrondis aux jointures des segments
}

static inline int ease_out_bounce(int t)
{
    t = tw_clamp(t);
    if (t >= 256)
        return 256;
    if (t < 93)                         // x < 1/2.75
        return tw_bounce_seg(t, 0);
    if (t < 186)                        // x < 2/2.75
        return tw_bounce_seg(t - 140, 192);
    if (t < 233)                        // x < 2.5/2.75
        return tw_bounce_seg(t - 209, 240);
    return tw_bounce_seg(t - 245, 252);
}

#endif
