/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ergodox_stonegarden, ported to current QMK. Builds for:
 *   - ErgoDox Infinity (input_club/ergodox_infinity): the LCDs show the active layer and take its
 *     colour, like the old visualizer did, and FUNC has the per-key backlight controls.
 *   - ErgoDox EZ (ergodox_ez/base, /glow or /shine): the three LEDs show the active layers, and
 *     FUNC has the RGB controls on the Glow and Shine.
 * The layers are the Ergo68's (keyboards/ergo68) wherever the two keyboards have the same keys.
 */

#include QMK_KEYBOARD_H
#include "layers.h"

// Layer keys. NAV is held; FUNC and NUMPAD latch on, and the key in the same spot on every other
// layer (BASE) takes you back to the base layer. Same as on the Ergo68.
#define L_BASE  TO(_BASE)
#define L_NAV   MO(_NAV)
#define L_FUNC  TG(_FUNC)
#define L_NUMPD TG(_NUMPAD)

#define FN_SCLN LT(_FUNC, KC_SCLN) // ; on tap, FUNC while held
#define CA_MENU LCA_T(KC_MENU)     // Ctrl+Alt while held (for Ctrl+Alt+Fx), Menu on tap

// Lighting keys on FUNC, for whichever lighting this board has.
#if defined(LED_MATRIX_ENABLE) // ErgoDox Infinity: single-colour per-key backlight
#    define LI_TOGG LM_TOGG
#    define LI_NEXT LM_NEXT
#    define LI_PREV LM_PREV
#    define LI_HUEU KC_NO
#    define LI_HUED KC_NO
#    define LI_SATU KC_NO
#    define LI_SATD KC_NO
#    define LI_VALU LM_BRIU
#    define LI_VALD LM_BRID
#    define LI_SPDU LM_SPDU
#    define LI_SPDD LM_SPDD
#    define LI_ON   LM_ON
#    define LI_OFF  LM_OFF
#elif defined(RGB_MATRIX_ENABLE) // ErgoDox EZ Glow
#    define LI_TOGG RM_TOGG
#    define LI_NEXT RM_NEXT
#    define LI_PREV RM_PREV
#    define LI_HUEU RM_HUEU
#    define LI_HUED RM_HUED
#    define LI_SATU RM_SATU
#    define LI_SATD RM_SATD
#    define LI_VALU RM_VALU
#    define LI_VALD RM_VALD
#    define LI_SPDU RM_SPDU
#    define LI_SPDD RM_SPDD
#    define LI_ON   RM_ON
#    define LI_OFF  RM_OFF
#elif defined(RGBLIGHT_ENABLE) // ErgoDox EZ Shine
#    define LI_TOGG UG_TOGG
#    define LI_NEXT UG_NEXT
#    define LI_PREV UG_PREV
#    define LI_HUEU UG_HUEU
#    define LI_HUED UG_HUED
#    define LI_SATU UG_SATU
#    define LI_SATD UG_SATD
#    define LI_VALU UG_VALU
#    define LI_VALD UG_VALD
#    define LI_SPDU UG_SPDU
#    define LI_SPDD UG_SPDD
#    define LI_ON   KC_NO // underglow has no on/off keys, only UG_TOGG
#    define LI_OFF  KC_NO
#else
#    define LI_TOGG KC_NO
#    define LI_NEXT KC_NO
#    define LI_PREV KC_NO
#    define LI_HUEU KC_NO
#    define LI_HUED KC_NO
#    define LI_SATU KC_NO
#    define LI_SATD KC_NO
#    define LI_VALU KC_NO
#    define LI_VALD KC_NO
#    define LI_SPDU KC_NO
#    define LI_SPDD KC_NO
#    define LI_ON   KC_NO
#    define LI_OFF  KC_NO
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* BASE (diagrams of every layer are in README.md). Same as the Ergo68 where the keys overlap
     * (keyboards/ergo68); the eight thumb keys the Ergo68 doesn't have keep their old functions.
     */
    [_BASE] = LAYOUT_ergodox(
        /* left hand */
        KC_BSLS, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_ESC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    L_FUNC,
        KC_GRV,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_LBRC,
        KC_LCTL, KC_LALT, KC_LGUI, KC_TILD, L_NAV,
                                                              CA_MENU, KC_PSCR,
                                                                       KC_HOME,
                                                     KC_BSPC, KC_DEL,  KC_END,
        /* right hand */
        KC_ESC,  KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_SLSH,
        L_NUMPD, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_QUOT,
                 KC_H,    KC_J,    KC_K,    KC_L,    FN_SCLN, KC_EQL,
        KC_RBRC, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_COLN, KC_MINS,
                          L_NAV,   KC_DLR,  KC_NUBS, KC_APP,  KC_RALT,
        L_FUNC,  KC_INS,
        KC_PGUP,
        KC_PGDN, KC_ENT,  KC_SPC
    ),

    /* NAV (held): mouse on the left, arrows on HJKL and Home/PgDn/PgUp/End below them */
    [_NAV] = LAYOUT_ergodox(
        /* left hand */
        _______, _______, _______, _______, _______, _______, _______,
        _______, _______, KC_WBAK, MS_BTN1, MS_BTN2, KC_WFWD, L_BASE,
        _______, _______, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT,
        _______, _______, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, MS_BTN3,
        _______, _______, _______, _______, _______,
                                                              _______, _______,
                                                                       _______,
                                                     _______, _______, _______,
        /* right hand */
        _______, _______, _______, _______, _______, _______, _______,
        L_BASE,  _______, _______, _______, _______, _______, _______,
                 KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
        _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
                          _______, _______, _______, _______, _______,
        _______, _______,
        _______,
        _______, _______, _______
    ),

    /* FUNC: F-keys and media, lighting controls on the left. Keep QK_BOOT in both bottom corners and
     * L_FUNC on the left inner key of the second row: on the Infinity they're how each half gets into
     * bootloader mode without the program button (README).
     */
    [_FUNC] = LAYOUT_ergodox(
        /* left hand */
        LI_SATU, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   _______,
        LI_SATD, LI_NEXT, _______, _______, _______, _______, L_BASE,
        LI_VALU, LI_TOGG, LI_NEXT, LI_PREV, LI_HUEU, LI_HUED,
        LI_VALD, LI_OFF,  _______, _______, _______, _______, _______,
        QK_BOOT, _______, _______, _______, _______,
                                                              NK_TOGG, _______,
                                                                       _______,
                                                     LI_ON,   LI_OFF,  _______,
        /* right hand */
        _______, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        L_BASE,  KC_INS,  KC_SCRL, KC_PSCR, KC_PAUS, _______, KC_F12,
                 KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, _______, KC_MPLY,
        _______, KC_MRWD, KC_MUTE, _______, KC_MFFD, _______, KC_MSTP,
                          _______, _______, _______, _______, QK_BOOT,
        L_BASE,  _______,
        _______,
        _______, _______, _______
    ),

    /* NUMPAD: numpad on the right, Boot, Reboot and EEPROM clear on the left */
    [_NUMPAD] = LAYOUT_ergodox(
        /* left hand */
        QK_BOOT, _______, _______, _______, _______, _______, _______,
        QK_RBT,  _______, _______, _______, _______, _______, L_BASE,
        EE_CLR,  _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______,
                                                              _______, _______,
                                                                       _______,
                                                     _______, _______, _______,
        /* right hand */
        _______, _______, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS, _______,
        L_BASE,  _______, KC_P7,   KC_P8,   KC_P9,   KC_PPLS, _______,
                 _______, KC_P4,   KC_P5,   KC_P6,   KC_PCMM, _______,
        _______, _______, KC_P1,   KC_P2,   KC_P3,   KC_PEQL, _______,
                          _______, KC_P0,   KC_PDOT, KC_COMM, _______,
        _______, _______,
        _______,
        _______, _______, _______
    )

};
// clang-format on

#ifdef ST7565_ENABLE
/*
 * ErgoDox Infinity LCDs: show the active layer's name and Caps/Num/Scroll Lock, and colour the
 * backlight by layer, with the names and colours from the old ergodox_stonegarden visualizer.
 */
typedef struct {
    const char *name;
    uint8_t     r, g, b;
} layer_style_t;

// The visualizer's colours were hue/saturation 0/0, 148/255, 80/255 and 190/255; these are the same in RGB,
// since the Infinity's QMK build has no HSV conversion (that comes with RGB lighting, which it doesn't have).
static const layer_style_t layer_styles[] = {
    [_BASE]   = {"Ergodox Infinity", 255, 255, 255}, // white
    [_NAV]    = {"Mouse + Navigation", 0, 135, 255}, // blue
    [_FUNC]   = {"Function keys", 116, 0, 255},      // purple
    [_NUMPAD] = {"Numpad", 32, 255, 0},              // green
};

static uint8_t lcd_layer = UINT8_MAX; // not shown yet

static void lcd_show_layer_colour(uint8_t layer) {
    // Half brightness, like QMK's default for these LCDs.
    const layer_style_t *style = &layer_styles[layer];
    ergodox_infinity_lcd_color(style->r * 128, style->g * 128, style->b * 128);
}

void st7565_on_user(void) {
    lcd_show_layer_colour(lcd_layer < ARRAY_SIZE(layer_styles) ? lcd_layer : _BASE);
}

void st7565_task_user(void) {
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    if (layer >= ARRAY_SIZE(layer_styles)) {
        layer = _BASE;
    }
    if (layer != lcd_layer) {
        lcd_layer = layer;
        lcd_show_layer_colour(layer);
    }

    led_t host = host_keyboard_led_state();
    st7565_set_cursor(0, 0);
    st7565_write_ln(layer_styles[layer].name, false);
    st7565_write(host.caps_lock ? "Caps " : "     ", false);
    st7565_write(host.num_lock ? "Num " : "    ", false);
    st7565_write_ln(host.scroll_lock ? "Scroll" : "", false);
    st7565_write_ln("", false);
    st7565_write_ln("", false);
}
#endif

#ifdef KEYBOARD_ergodox_ez
// ErgoDox EZ: the three LEDs show which of NAV (held), FUNC and NUMPAD are on, as on the Ergo68.
layer_state_t layer_state_set_user(layer_state_t state) {
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    if (layer_state_cmp(state, _NAV)) {
        ergodox_right_led_1_on();
    }
    if (layer_state_cmp(state, _FUNC)) {
        ergodox_right_led_2_on();
    }
    if (layer_state_cmp(state, _NUMPAD)) {
        ergodox_right_led_3_on();
    }
    return state;
}
#endif
