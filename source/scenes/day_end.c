// Fin de journée : bilan + boutique d'améliorations (tâches 6.2, 6.3)
#include <stdio.h>

#include "core/input.h"
#include "game/progress.h"
#include "game/recipes.h"
#include "gen/assets.h"
#include "gfx/font.h"
#include "gfx/palette.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"
#include "scenes/ui.h"

static Watch w_shop;
static int g_flash_upgrade = -1, g_flash_timer;
static bool g_collapsed;

static Rect upgrade_rect(int i) { return rect_make(8 + (i & 1) * 124, 26 + (i >> 1) * 52, 116, 46); }
static Rect next_rect(void) { return rect_make(56, 136, 144, 28); }

static void draw_summary(Surface *s, const Day *d)
{
    char buf[64];
    ui_polka_background(s, COL_PINK_PALE, COL_WHITE);
    snprintf(buf, sizeof(buf), "Fin du jour %d", d->number);
    text_draw_shadow(s, (SCREEN_W - text_width(buf, 2)) / 2, 8, buf, COL_PINK_DARK, COL_WHITE, 2);
    if (g_collapsed)
        text_draw_aligned(s, 0, 30, SCREEN_W, "Ambiance effondrée : on réessaie !",
                          COL_BAD, ALIGN_CENTER);

    draw_round_rect(s, 16, 44, 224, 100, COL_CREAM, COL_WOOD_DARK);
    int y = 50;
    snprintf(buf, sizeof(buf), "Clients servis      %d", d->served);
    text_draw(s, 26, y, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "dont parfaits       %d", d->perfect);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Clients perdus      %d", d->lost);
    text_draw(s, 26, y += 13, buf, d->lost ? COL_BAD : COL_INK, 1);
    snprintf(buf, sizeof(buf), "Recettes            %d pièces", d->coins);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Pourboires          %d pièces", d->tips);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Ambiance finale     %d %%", d->ambiance / 10);
    text_draw(s, 26, y += 13, buf, COL_INK, 1);
    snprintf(buf, sizeof(buf), "Livre de recettes   %d / %d", progress_known_count(), RECIPE_COUNT);
    text_draw(s, 26, y += 13, buf, COL_LAV_DARK, 1);

    if (d->unlocked_count) {
        int n = 0;
        n += snprintf(buf, sizeof(buf), "Découvert : ");
        for (int i = 0; i < d->unlocked_count && n < (int)sizeof(buf); i++)
            n += snprintf(buf + n, sizeof(buf) - n, "%s%s", i ? ", " : "", RECIPES[d->unlocked[i]].name);
        text_draw_wrapped(s, 16, 150, 224, buf, COL_LAV_DARK, ALIGN_CENTER);
    }
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
        draw_round_rect(s, r.x, r.y, r.w, r.h, fill, owned ? COL_MINT_DARK : COL_WOOD_DARK);
        text_draw(s, r.x + 4, r.y + 3, u->name, COL_INK, 1);
        text_draw_wrapped(s, r.x + 4, r.y + 15, r.w - 8, u->desc, COL_INK_SOFT, ALIGN_LEFT);
        if (owned) {
            text_draw(s, r.x + 4, r.y + r.h - 13, "Acquis !", COL_MINT_DARK, 1);
        } else {
            snprintf(buf, sizeof(buf), "%d", u->cost);
            int tw = text_width(buf, 1);
            text_draw(s, r.x + r.w - 4 - tw, r.y + 3, buf, affordable ? COL_INK : COL_BAD, 1);
            draw_image(s, &IMG_icons_coin, r.x + r.w - 21 - tw, r.y + 1);
        }
    }
    snprintf(buf, sizeof(buf), g_collapsed ? "Réessayer le jour %d" : "Jour %d !", progress()->day);
    ui_button(s, next_rect(), buf, BTN_STYLE_PRIMARY);
    text_draw_aligned(s, 0, 172, SCREEN_W, "Partie sauvegardée", COL_INK_SOFT, ALIGN_CENTER);
}

static void day_end_enter(void)
{
    ShopSession *ss = shop_session();
    Day *d = &ss->day;
    Progress *p = progress();
    g_collapsed = d->collapsed;

    int earned = d->coins + d->tips;
    p->coins += earned;
    p->total_served += d->served;
    if ((uint32_t)earned > p->best_day_coins)
        p->best_day_coins = earned;
    if (!d->collapsed)
        p->day = d->number + 1;
    progress_save();

    draw_summary(screen_background(SCREEN_TOP), d);
    screen_set_dynamic(SCREEN_TOP, NULL);
    ui_polka_background(screen_background(SCREEN_BOTTOM), COL_CREAM, COL_CREAM_DARK);
    screen_set_dynamic(SCREEN_BOTTOM, draw_shop);
    watch_reset(&w_shop);
    plat_sfx(d->collapsed ? SFX_ANGRY : SFX_CASH);
}

static void day_end_update(void)
{
    const Input *in = input_get();
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
    Rect n = next_rect();
    if (input_tapped_in(n.x, n.y, n.w, n.h) || (in->down & (BTN_A | BTN_START))) {
        shop_begin_day();
        scene_change(&SCENE_SHOP);
    }
    uint32_t h = hash_add(hash_begin(), (int32_t)progress()->coins);
    h = hash_add(h, progress()->upgrades);
    h = hash_add(h, g_flash_timer ? g_flash_upgrade : -1);
    watch(SCREEN_BOTTOM, &w_shop, rect_make(0, 0, SCREEN_W, SCREEN_H), h);
}

const Scene SCENE_DAY_END = {
    .name = "day_end",
    .enter = day_end_enter,
    .update = day_end_update,
    .exit = NULL,
};
