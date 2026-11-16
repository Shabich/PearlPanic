// Disposition des deux écrans de la boutique (partagée entre logique et dessin)
#ifndef PP_SHOP_LAYOUT_H
#define PP_SHOP_LAYOUT_H

#include "game/ingredients.h"
#include "gfx/canvas.h"

// ---------------------------------------------------------------- Écran bas
// Rangée des tireuses (liquides)
#define TAP_Y      4
#define TAP_H      34
#define TAP_W      38
#define TAP_STEP   42
#define TAP_X0     4
#define TAP_COUNT  6

// Rangée des pompes à sirop
#define PUMP_Y     42
#define PUMP_H     24
#define PUMP_COUNT 4

// Sélecteur de client (à droite des pompes)
#define SEL_X      172
#define SEL_Y      42
#define SEL_W      80
#define SEL_H      24

// Colonnes de gauche (garnitures) et de droite (crème + actions)
#define COL_L_X    4
#define COL_R_X    200
#define COL_W      52
#define ROW_Y0     72
#define ROW_STEP   30
#define ROW_H      27

// Gobelet
#define CUP_AREA_X 60
#define CUP_AREA_Y 68
#define CUP_AREA_W 136
#define CUP_AREA_H 124
#define CUP_CX     128
#define CUP_TOP_Y  92        // haut du liquide possible
#define CUP_BOT_Y  176       // fond intérieur
#define CUP_TOP_HW 28        // demi-largeur en haut
#define CUP_BOT_HW 22        // demi-largeur en bas

static const Ingredient TAP_INGS[TAP_COUNT] = {
    ING_TEA_BLACK, ING_TEA_GREEN, ING_TEA_JASMINE, ING_TEA_MATCHA, ING_MILK, ING_MILK_OAT,
};
static const Ingredient PUMP_INGS[PUMP_COUNT] = {
    ING_SYRUP_BROWN, ING_SYRUP_TARO, ING_SYRUP_STRAWBERRY, ING_SYRUP_MANGO,
};
// Bacs : 4 à gauche + la crème en haut à droite
#define BIN_COUNT 5
static const Ingredient BIN_INGS[BIN_COUNT] = {
    ING_PEARLS, ING_POPPING, ING_JELLY, ING_ICE, ING_CREAM,
};

static inline Rect tap_rect(int i) { return rect_make(TAP_X0 + i * TAP_STEP, TAP_Y, TAP_W, TAP_H); }
static inline Rect pump_rect(int i) { return rect_make(TAP_X0 + i * TAP_STEP, PUMP_Y, TAP_W, PUMP_H); }
static inline Rect bin_rect(int i)
{
    if (i < 4)
        return rect_make(COL_L_X, ROW_Y0 + i * ROW_STEP, COL_W, ROW_H);
    return rect_make(COL_R_X, ROW_Y0, COL_W, ROW_H);
}

typedef enum { ACT_SEAL, ACT_TRASH, ACT_SERVE, ACT_COUNT } StationAction;
static inline Rect action_rect(int a)
{
    return rect_make(COL_R_X, ROW_Y0 + (a + 1) * ROW_STEP, COL_W, ROW_H);
}

static inline Rect sel_prev_rect(void) { return rect_make(SEL_X, SEL_Y, 20, SEL_H); }
static inline Rect sel_next_rect(void) { return rect_make(SEL_X + SEL_W - 20, SEL_Y, 20, SEL_H); }
static inline Rect cup_rect(void) { return rect_make(CUP_AREA_X, CUP_AREA_Y, CUP_AREA_W, CUP_AREA_H); }

// --------------------------------------------------------------- Écran haut
#define HUD_H        16
#define COUNTER_Y    124
#define SLOT_W       84
static inline int slot_cx(int i) { return 44 + i * 84; }
#define CUSTOMER_Y   78      // haut de l'image client (40x48) au repos
#define BUBBLE_Y     19
#define BUBBLE_H     44
#define BUBBLE_W     80
#define GAUGE_Y      68
static inline Rect slot_rect(int i) { return rect_make(slot_cx(i) - 42, HUD_H, 84, COUNTER_Y - HUD_H); }
#define PANEL_X      4
#define PANEL_Y      (COUNTER_Y + 6)
#define PANEL_W      248
#define PANEL_H      58

#endif
