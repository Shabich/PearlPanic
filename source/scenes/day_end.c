// Fin de journée : bilan + boutique d'améliorations (tâches 6.2, 6.3)
#include <stdio.h>

#include "core/input.h"
#include "core/tween.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "game/specials.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/fx.h"
#include "scenes/scenes.h"
#include "scenes/ui.h"

static Watch w_shop;
static int g_flash_upgrade = -1, g_flash_timer;
static bool g_collapsed;
static bool g_rush;
static bool g_new_record;
static uint32_t g_frame;
static int g_stars, g_level;
static bool g_first_clear;     // niveau réussi pour la première fois
static bool g_play_ending;     // dernier niveau : cinématique de fin à suivre

// Étoile 7x7 agrandie
static void big_star(Surface *s, int cx, int cy, bool full, int scale)
{
    static const uint8_t STAR[7] = { 0x08, 0x08, 0x7F, 0x3E, 0x1C, 0x36, 0x63 };
    uint16_t c = full ? COL_YELLOW : COL_GREY;
    uint16_t edge = full ? RGB888(220, 150, 50) : COL_GREY_DARK;
    if (scale < 1)
        return;
    for (int j = 0; j < 7; j++)
        for (int i = 0; i < 7; i++)
            if (STAR[j] >> i & 1)
                draw_rect(s, cx - 3 * scale + i * scale, cy - 3 * scale + j * scale, scale, scale,
                          (j == 6 || i == 0 || i == 6) ? edge : c);
}

// Les étoiles gagnées apparaissent une à une avec un petit rebond
static void draw_top_fx(Surface *s)
{
    if (!g_rush) {
        for (int k = 0; k < 3; k++) {
            int start = 20 + k * 22;
            bool full = k < g_stars;
            int scale = 3;
            if (full && (int)g_frame < start + 14) {
                int p = ease_out_back(tw_progress((int)g_frame - start, 14));
                scale = (int)g_frame < start ? 0 : 1 + p * 3 / 256;
            }
            if (!full && scale)
                scale = 3;
            big_star(s, 128 - 36 + k * 36, 38, full, scale);
        }
    }
    fx_draw(SCREEN_TOP, s);
}

static Rect upgrade_rect(int i)
{
    int x = i == UPG_COUNT - 1 && (UPG_COUNT & 1) ? 70 : 8 + (i & 1) * 124;
    return rect_make(x, 24 + (i >> 1) * 41, 116, 38);
}
static Rect next_rect(void) { return rect_make(132, 150, 116, 24); }
static Rect retry_rect(void) { return rect_make(8, 150, 116, 24); }

static void draw_summary(Surface *s, const Day *d)
{
    char buf[96];
    ui_polka_background(s, COL_PINK_PALE, COL_WHITE);
    if (g_collapsed)
        snprintf(buf, sizeof(buf), "Niveau %d-%d raté…", d->world + 1, d->level + 1);
    else
        snprintf(buf, sizeof(buf), "Niveau %d-%d réussi !", d->world + 1, d->level + 1);
    text_draw_shadow(s, (SCREEN_W - text_width(buf, 2)) / 2, 4, buf, g_collapsed ? COL_BAD : COL_PINK_DARK,
                     COL_WHITE, 2);

    // Habitués venus aujourd'hui
    int y = 56;
    if (g_collapsed) {
        text_draw_aligned(s, 0, y, SCREEN_W, "L'ambiance s'est effondrée : on réessaie ?", COL_BAD, ALIGN_CENTER);
    } else {
        for (int k = 0; k < SPECIALS_PER_LEVEL; k++) {
            if (d->specials[k] < 0 || d->special_result[k] < 0)
                continue;
            const char *name = SPECIALS[d->specials[k]].name;
            if (d->special_result[k])
                snprintf(buf, sizeof(buf), "Visite réussie : %s !", name);
            else
                snprintf(buf, sizeof(buf), "%s reviendra…", name);
            text_draw_aligned(s, 0, y, SCREEN_W, buf, COL_WOOD_DARK, ALIGN_CENTER);
            break;
        }
    }

    draw_round_rect(s, 16, 70, 224, 72, COL_CREAM, COL_WOOD_DARK);
    y = 75;
    snprintf(buf, sizeof(buf), "Clients servis      %d (%d parfaits)", d->served, d->perfect);
    text_draw(s, 26, y, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Clients perdus      %d", d->lost);
    text_draw(s, 26, y += 13, buf, d->lost ? COL_BAD : COL_INK, 1);
    snprintf(buf, sizeof(buf), "Pièces gagnées      %d (+%d pourboires)", d->coins, d->tips);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Ambiance finale     %d %%", d->ambiance / 10);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    int total;
    int known = progress_known_in_world(d->world, &total);
    snprintf(buf, sizeof(buf), "Recettes du bar     %d / %d", known, total);
    text_draw(s, 26, y += 13, buf, COL_LAV_DARK, 1);

    if (d->objective.type != OBJ_NONE) {
        char obj[48];
        objective_describe(&d->objective, obj, sizeof(obj));
        if (d->objective_met)
            snprintf(buf, sizeof(buf), "Objectif réussi : %s (+%d)", obj, d->objective.reward);
        else
            snprintf(buf, sizeof(buf), "Objectif manqué : %s", obj);
        text_draw_aligned(s, 0, 148, SCREEN_W, buf, d->objective_met ? COL_MINT_DARK : COL_INK_SOFT,
                          ALIGN_CENTER);
    }
    if (d->unlocked_count) {
        int n = 0;
        n += snprintf(buf, sizeof(buf), "Découvert : ");
        for (int i = 0; i < d->unlocked_count && n < (int)sizeof(buf); i++)
            n += snprintf(buf + n, sizeof(buf) - n, "%s%s", i ? ", " : "", RECIPES[d->unlocked[i]].name);
        text_draw_wrapped(s, 16, 162, 224, buf, COL_LAV_DARK, ALIGN_CENTER);
    }
    // Indication des étoiles
    text_draw_aligned(s, 0, 180, SCREEN_W, "Étoiles : fini, objectif, sans perte",
                      COL_INK_SOFT, ALIGN_CENTER);
}

static void draw_rush_summary(Surface *s, const Day *d)
{
    char buf[64];
    ui_polka_background(s, COL_PINK_PALE, COL_WHITE);
    text_draw_shadow(s, (SCREEN_W - text_width("Fin du Rush", 2)) / 2, 10, "Fin du Rush", COL_BAD,
                     COL_WHITE, 2);
    draw_round_rect(s, 32, 44, 192, 92, COL_CREAM, COL_WOOD_DARK);
    uint32_t sec = d->frame / FPS;
    int y = 52;
    snprintf(buf, sizeof(buf), "Tenu          %lu:%02lu", (unsigned long)(sec / 60), (unsigned long)(sec % 60));
    text_draw(s, 44, y, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Servis        %d (%d parfaits)", d->served, d->perfect);
    text_draw(s, 44, y += 14, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Perdus        %d", d->lost);
    text_draw(s, 44, y += 14, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Score         %d pièces", d->coins + d->tips);
    text_draw(s, 44, y += 14, buf, COL_PINK_DARK, 1);
    snprintf(buf, sizeof(buf), "Record        %lu", (unsigned long)progress()->best_rush);
    text_draw(s, 44, y += 14, buf, COL_LAV_DARK, 1);
    if (g_new_record)
        text_draw_shadow(s, (SCREEN_W - text_width("Nouveau record !", 2)) / 2, 150, "Nouveau record !",
                         COL_PINK_DARK, COL_YELLOW, 2);
}

static Rect title_rect(void) { return rect_make(56, 84, 144, 28); }

static void draw_rush_bottom(Surface *s)
{
    text_draw_aligned(s, 0, 50, SCREEN_W, "Les pièces du Rush ne comptent pas", COL_INK_SOFT, ALIGN_CENTER);
    text_draw_aligned(s, 0, 62, SCREEN_W, "dans la partie principale.", COL_INK_SOFT, ALIGN_CENTER);
    ui_button(s, title_rect(), "Retour au titre", BTN_STYLE_PRIMARY);
}

static void draw_shop(Surface *s)
{
    char buf[48];
    draw_rect(s, 0, 0, SCREEN_W, 20, COL_PINK);
    snprintf(buf, sizeof(buf), "Améliorations - %lu pièces", (unsigned long)progress()->coins);
    text_draw_aligned(s, 0, 5, SCREEN_W, buf, COL_INK, ALIGN_CENTER);
    for (int i = 0; i < UPG_COUNT; i++) {
        const UpgradeInfo *u = upgrade_info(i);
        Rect r = upgrade_rect(i);
        bool owned = progress_has_upgrade(i);
        bool affordable = progress()->coins >= u->cost;
        uint16_t fill = owned ? COL_MINT : g_flash_upgrade == i && g_flash_timer ? COL_PINK : COL_CREAM;
        // Les améliorations qu'on peut se payer pulsent doucement
        uint16_t border = owned ? COL_MINT_DARK
                        : affordable && ((g_frame / 20) & 1) ? COL_PINK_DARK : COL_WOOD_DARK;
        draw_round_rect(s, r.x, r.y, r.w, r.h, fill, border);
        text_draw(s, r.x + 4, r.y + 3, u->name, COL_INK, 1);
        text_draw_wrapped(s, r.x + 4, r.y + 14, r.w - 8, u->desc, COL_INK_SOFT, ALIGN_LEFT);
        if (owned) {
            text_draw(s, r.x + r.w - 4 - text_width("OK", 1), r.y + 3, "OK", COL_MINT_DARK, 1);
        } else {
            snprintf(buf, sizeof(buf), "%d", u->cost);
            int tw = text_width(buf, 1);
            text_draw(s, r.x + r.w - 4 - tw, r.y + 3, buf, affordable ? COL_INK : COL_BAD, 1);
            draw_image(s, &IMG_icons_coin, r.x + r.w - 21 - tw, r.y + 1);
        }
    }
    ui_button(s, retry_rect(), "Rejouer", g_collapsed ? BTN_STYLE_PRIMARY : BTN_STYLE_NORMAL);
    ui_button(s, next_rect(), g_play_ending ? "La suite !" : "Carte", g_collapsed ? BTN_STYLE_NORMAL : BTN_STYLE_PRIMARY);
    text_draw_aligned(s, 0, 179, SCREEN_W, "Partie sauvegardée", COL_INK_SOFT, ALIGN_CENTER);
}

static void day_end_enter(void)
{
    ShopSession *ss = shop_session();
    Day *d = &ss->day;
    Progress *p = progress();
    g_collapsed = d->collapsed;
    g_rush = d->endless;
    ui_polka_background(screen_background(SCREEN_BOTTOM), COL_CREAM, COL_CREAM_DARK);
    screen_set_dynamic(SCREEN_TOP, NULL);

    if (g_rush) {
        uint32_t score = (uint32_t)(d->coins + d->tips);
        g_new_record = score > p->best_rush;
        if (g_new_record)
            p->best_rush = score;
        progress_save();
        g_frame = 0;
        fx_clear();
        screen_set_dynamic(SCREEN_TOP, draw_top_fx);
        draw_rush_summary(screen_background(SCREEN_TOP), d);
        screen_set_dynamic(SCREEN_BOTTOM, draw_rush_bottom);
        plat_sfx(g_new_record ? SFX_UNLOCK : SFX_CASH);
        return;
    }

    int earned = d->coins + d->tips;
    if (d->objective_met)
        earned += d->objective.reward;
    p->coins += earned;
    p->total_served += d->served;
    if ((uint32_t)earned > p->best_day_coins)
        p->best_day_coins = earned;
    g_level = d->number;
    g_stars = d->stars;
    g_first_clear = progress_record_level(d->number, d->stars);
    g_play_ending = d->number == LEVEL_COUNT && d->stars > 0 && !(p->story & STORY_ENDING);
    p->level = (uint8_t)d->number;
    progress_save();

    g_frame = 0;
    fx_clear();
    screen_set_dynamic(SCREEN_TOP, draw_top_fx);
    draw_summary(screen_background(SCREEN_TOP), d);
    screen_set_dynamic(SCREEN_BOTTOM, draw_shop);
    watch_reset(&w_shop);
    plat_sfx(d->collapsed ? SFX_ANGRY : SFX_CASH);
}

static void day_end_update(void)
{
    const Input *in = input_get();
    g_frame++;
    // Pièces qui jaillissent et étincelles pendant le bilan
    if (!g_collapsed && (g_frame % 24) == 0 && g_frame < 60 * 8)
        fx_emit(SCREEN_TOP, FX_COIN, 40 + (int)(g_frame * 37 % 176), 150, 2, 0);
    if ((g_frame % 30) == 15)
        fx_emit(SCREEN_TOP, g_collapsed ? FX_SMOKE : FX_SPARKLE, 128, 20, g_collapsed ? 1 : 3, 0);
    for (int k = 0; k < g_stars && !g_rush; k++)
        if ((int)g_frame == 20 + k * 22 + 6) {
            fx_emit(SCREEN_TOP, FX_STAR, 128 - 36 + k * 36, 38, 6, 0);
            plat_sfx(k == 2 ? SFX_UNLOCK : SFX_PLOP);
        }
    if (g_frame < 120)
        screen_dirty(SCREEN_TOP, rect_make(70, 20, 116, 36));
    if (g_frame == 90 && g_first_clear)   // niveau suivant débloqué : petite fête
        fx_emit(SCREEN_TOP, FX_HEART, 128, 40, 8, 0);
    fx_update();
    if (scene_transitioning())
        return;
    if (g_rush) {
        Rect t = title_rect();
        if (input_tapped_in(t.x, t.y, t.w, t.h) || (in->down & (BTN_A | BTN_START)))
            scene_change(&SCENE_TITLE);
        return;
    }
    if (g_flash_timer)
        g_flash_timer--;
    for (int i = 0; i < UPG_COUNT; i++) {
        Rect r = upgrade_rect(i);
        if (!input_tapped_in(r.x, r.y, r.w, r.h))
            continue;
        if (progress_buy_upgrade(i)) {
            progress_save();
            plat_sfx(SFX_UNLOCK);
        } else {
            plat_sfx(SFX_CLICK);
        }
        g_flash_upgrade = i;
        g_flash_timer = 10;
    }
    Rect n = next_rect(), r = retry_rect();
    if (input_tapped_in(r.x, r.y, r.w, r.h) || (g_collapsed && (in->down & BTN_A))) {
        progress()->level = (uint8_t)g_level;
        shop_begin_day();
        scene_change(&SCENE_SHOP);
    } else if (input_tapped_in(n.x, n.y, n.w, n.h) || (in->down & BTN_START) || (!g_collapsed && (in->down & BTN_A))) {
        if (g_play_ending) {
            cutscene_play(CUT_ENDING, &SCENE_MAP);
            scene_change(&SCENE_CUTSCENE);
        } else {
            scene_change(&SCENE_MAP);
        }
    }
    uint32_t h = hash_add(hash_begin(), (int32_t)progress()->coins);
    h = hash_add(h, progress()->upgrades);
    h = hash_add(h, g_flash_timer ? g_flash_upgrade : -1);
    h = hash_add(h, (int32_t)(g_frame / 20));
    watch(SCREEN_BOTTOM, &w_shop, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
}

const Scene SCENE_DAY_END = {
    .name = "day_end",
    .enter = day_end_enter,
    .update = day_end_update,
    .exit = NULL,
};
