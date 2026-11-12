/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ergodox_stonegarden, ported to current QMK. Builds for:
 *   - ErgoDox Infinity (input_club/ergodox_infinity): the LCDs show the active layer and take its
 *     colour, like the old visualizer did, and MEDIA has the per-key backlight controls.
 *   - ErgoDox EZ (ergodox_ez/base, /glow or /shine): the three LEDs show the latched layers, and
 *     MEDIA has the RGB controls on the Glow and Shine.
 */

#include QMK_KEYBOARD_H
#include "layers.h"

// Layer keys. MEDIA/NUMPAD/FUNC latch their layer on and the key in the same spot on that layer
// (BASE) takes you back.
#define L_BASE  TO(_BASE)
#define L_MEDIA TG(_MEDIA)
#define L_NUMPD TG(_NUMPAD)
#define L_FUNC  TG(_FUNC)

#define MED_SCL LT(_MEDIA, KC_SCLN) // ; on tap, MEDIA while held
#define CA_MENU LCA_T(KC_MENU)      // Ctrl+Alt while held (for Ctrl+Alt+Fx), Menu on tap
#define S_INS   LSFT(KC_INS)

// Lighting keys on MEDIA, for whichever lighting this board has.
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
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_ergodox(
        /* left hand */
        KC_BSLS, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_ESC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    L_MEDIA,
        KC_GRV,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_LBRC,
        KC_LCTL, KC_LGUI, KC_LALT, KC_TILD, KC_DLR,
                                                              CA_MENU, KC_PSCR,
                                                                       KC_HOME,
                                                     KC_BSPC, KC_DEL,  KC_END,
        /* right hand */
        KC_ESC,  KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_SLSH,
        L_NUMPD, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_QUOT,
                 KC_H,    KC_J,    KC_K,    KC_L,    MED_SCL, KC_EQL,
        KC_RBRC, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_COLN, KC_MINS,
                          KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_RALT,
        L_FUNC,  KC_INS,
        KC_PGUP,
        KC_PGDN, KC_ENT,  KC_SPC
    ),

    /* MEDIA: lighting on the left, media keys on the right */
    [_MEDIA] = LAYOUT_ergodox(
        /* left hand */
        QK_BOOT, LI_HUED, LI_HUEU, LI_SATD, LI_SATU, LI_PREV, LI_NEXT,
        EE_CLR,  _______, _______, _______, _______, _______, L_BASE,
        _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,
        LI_TOGG, LI_SPDD, LI_SPDU, LI_VALD, LI_VALU,
                                                              _______, _______,
                                                                       _______,
                                                     _______, _______, _______,
        /* right hand */
        _______, _______, _______, _______, _______, _______, QK_BOOT,
        L_BASE,  KC_MPRV, KC_MUTE, _______, KC_MNXT, _______, _______,
                 KC_MRWD, KC_VOLD, KC_VOLU, KC_MFFD, _______, KC_MPLY,
        _______, _______, _______, _______, _______, _______, KC_MSTP,
                          LI_NEXT, LI_TOGG, LI_VALD, LI_VALU, _______,
        _______, _______,
        _______,
        _______, _______, _______
    ),

    /* NUMPAD: mouse keys on the left, numpad on the right */
    [_NUMPAD] = LAYOUT_ergodox(
        /* left hand */
        QK_BOOT, _______, _______, MS_BTN4, MS_BTN5, _______, _______,
        _______, MS_ACL0, KC_WBAK, MS_BTN1, MS_BTN2, KC_WFWD, L_BASE,
        _______, MS_ACL1, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT,
        _______, MS_ACL2, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, MS_BTN3,
        _______, _______, _______, _______, _______,
                                                              _______, _______,
                                                                       _______,
                                                     _______, _______, _______,
        /* right hand */
        _______, _______, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS, QK_BOOT,
        L_BASE,  KC_LPRN, KC_P7,   KC_P8,   KC_P9,   KC_PPLS, EE_CLR,
                 KC_RPRN, KC_P4,   KC_P5,   KC_P6,   KC_PDOT, _______,
        _______, KC_P0,   KC_P1,   KC_P2,   KC_P3,   KC_PCMM, _______,
                          _______, _______, _______, _______, _______,
        L_FUNC,  _______,
        _______,
        _______, _______, KC_P0
    ),

    /* FUNC: F-keys. Hold Ctrl+Alt/Menu on the base layer and tap FUNC, then an F-key, for Ctrl+Alt+Fx. */
    [_FUNC] = LAYOUT_ergodox(
        /* left hand */
        QK_BOOT, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   _______,
        EE_CLR,  _______, _______, _______, _______, _______, L_BASE,
        _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, S_INS,   _______,
        _______, _______, _______, _______, _______,
                                                              NK_TOGG, _______,
                                                                       _______,
                                                     _______, _______, _______,
        /* right hand */
        QK_BOOT, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        L_BASE,  _______, _______, _______, _______, _______, KC_F12,
                 _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, KC_RGUI, KC_RSFT,
                          _______, _______, _______, _______, KC_RCTL,
        L_BASE,  _______,
        _______,
        _______, _______, _______
    ),
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
    [_MEDIA]  = {"Media", 0, 135, 255},              // blue
    [_NUMPAD] = {"Mouse + Numpad", 32, 255, 0},      // green
    [_FUNC]   = {"Function keys", 116, 0, 255},      // purple
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
// ErgoDox EZ: the three LEDs show which of MEDIA, NUMPAD and FUNC are latched on.
layer_state_t layer_state_set_user(layer_state_t state) {
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    if (layer_state_cmp(state, _MEDIA)) {
        ergodox_right_led_1_on();
    }
    if (layer_state_cmp(state, _NUMPAD)) {
        ergodox_right_led_2_on();
    }
    if (layer_state_cmp(state, _FUNC)) {
        ergodox_right_led_3_on();
    }
    return state;
}
#endif
