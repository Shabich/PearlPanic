// Rendu de la boutique (séparé de la logique de shop.c)
#ifndef PP_SHOP_DRAW_H
#define PP_SHOP_DRAW_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx/canvas.h"
#include "scenes/cup_anim.h"
#include "scenes/scenes.h"

typedef struct {
    ShopSession *session;
    uint32_t frame;
    int pouring;            // tireuse active, -1 sinon
    int pump_flash[4];
    int held;               // Ingredient tenu au stylet (ING_NONE sinon)
    int held_from;
    bool shaking;
    int shake_dir, shake_reversals;
    int wobble;
    bool paused;
    int end_timer;
    char toast1[48], toast2[96];
    uint16_t toast_color;
    int toast_timer;
    int toast_id;
    int coin_popup, coin_popup_timer, coin_popup_slot;
    CupAnim cup_anim[2];
    int jolt[3];            // sursaut quand le stress monte (frames restantes)
    uint32_t panel_key;     // contenu du ticket (pour relancer la machine à écrire)
    uint32_t panel_since;
    int panel_reveal;       // caractères affichés (-1 = tout)
    int coins_shown;        // compteur de pièces qui défile jusqu'au vrai total
} ShopView;

#define PAUSE_BUTTONS 3
Rect pause_button_rect(int i);

void shop_draw_backgrounds(const ShopView *v);
void shop_draw_top(Surface *s);
void shop_draw_bottom(Surface *s);

uint32_t shop_slot_hash(const ShopView *v, int slot);
uint32_t shop_tutorial_hash(const ShopView *v);
uint32_t shop_actions_hash(const ShopView *v);
uint32_t shop_cup_hash(const ShopView *v);

// Liste lisible des ingrédients d'une recette (« thé noir 70 %, perles ×2 »)
void recipe_describe(int recipe, char *buf, int size);

// Images de client par type et humeur (+ pose : normal, yeux fermés, parle)
enum { POSE_NORMAL, POSE_BLINK, POSE_TALK, CUSTOMER_POSES };
const Image *customer_image(int type, int mood);
const Image *customer_image_ex(int type, int mood, int pose);
const Image *topping_icon(int ing);
// Accessoire d'habitué (Accessory) sur une image de client posée en (x, y)
void draw_accessory(Surface *s, int acc, int x, int y);

#endif
