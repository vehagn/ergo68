/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Ergo68 (Yushakobo) keymap, a port of the ergodox_stonegarden layout to the Ergo68's 68 keys.
 *
 * The same keymap builds for both kinds of controller:
 *   - BLE Micro Pro (Bluetooth in one or both halves, or USB): the ble_micro_pro target of
 *     sekigon-gonnoc/vial-qmk. PROTOCOL_BMP is defined there.
 *   - Pro Micro (wired, TRRS cable between the halves): yushakobo/ergo68 in mainline QMK.
 */

#include QMK_KEYBOARD_H
#include "layers.h"
#include "layout.h"

#ifdef PROTOCOL_BMP
#    include "bmp_custom_keycodes.h"
// The BLE Micro Pro fork is based on QMK 0.25, from before the mouse keycodes were renamed.
#    define MS_UP   KC_MS_U
#    define MS_DOWN KC_MS_D
#    define MS_LEFT KC_MS_L
#    define MS_RGHT KC_MS_R
#    define MS_BTN1 KC_BTN1
#    define MS_BTN2 KC_BTN2
#    define MS_BTN3 KC_BTN3
#    define MS_BTN4 KC_BTN4
#    define MS_BTN5 KC_BTN5
#    define MS_WHLU KC_WH_U
#    define MS_WHLD KC_WH_D
#    define MS_WHLL KC_WH_L
#    define MS_WHLR KC_WH_R
#    define MS_ACL0 KC_ACL0
#    define MS_ACL1 KC_ACL1
#    define MS_ACL2 KC_ACL2
#endif

// Layer keys. NAV is held; FUNC and NUMPAD latch on, and the inner key in the same spot on every
// other layer (BASE) takes you back to the base layer.
#define L_BASE  TO(_BASE)
#define L_NAV   MO(_NAV)
#define L_FUNC  TG(_FUNC)
#define L_NUMPD TG(_NUMPAD)

#define FN_SCLN LT(_FUNC, KC_SCLN) // ; on tap, FUNC while held

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* BASE (diagrams of every layer are in README.md) */
    [_BASE] = LAYOUT_ergo68(
        KC_BSLS, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                                KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_SLSH,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_ESC,           KC_ESC,  KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_QUOT,
        KC_GRV,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    L_FUNC,           L_NUMPD, KC_H,    KC_J,    KC_K,    KC_L,    FN_SCLN, KC_EQL,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_LBRC,          KC_RBRC, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_COLN, KC_MINS,
        KC_LCTL, KC_LALT, KC_LGUI, XXXXXXX, L_NAV,   KC_BSPC, KC_DEL,           KC_ENT,  KC_SPC,  L_NAV,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    ),

    /* NAV (held): mouse on the left, arrows on HJKL and Home/PgDn/PgUp/End below them */
    [_NAV] = LAYOUT_ergo68(
        _______, _______, _______, _______, _______, _______,                             _______, _______, _______, _______, _______, _______,
        _______, _______, _______, MS_BTN1, MS_BTN2, _______, _______,          _______, _______, _______, _______, _______, _______, _______,
        _______, _______, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, L_BASE,           L_BASE,  KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
        _______, _______, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, _______,          _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
        _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______
    ),

#ifdef PROTOCOL_BMP
    /* FUNC (BLE Micro Pro): F-keys and media, with wireless controls on the left.
     * Host1-5 = switch to the n-th paired host, Pair = advertise to a new host (and look for the
     * other half), BLE/USB = where key presses go, Batt = type the battery voltage, Sleep = deep sleep.
     * Keep the F-keys and the right half the same as in the Pro Micro version below.
     */
    [_FUNC] = LAYOUT_ergo68(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                               KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        _______, ADV_ID1, ADV_ID2, ADV_ID3, ADV_ID4, ADV_ID5, _______,          _______, _______, _______, _______, _______, _______, KC_F12,
        _______, AD_WO_L, SEL_BLE, SEL_USB, BATT_LV, _______, L_BASE,           L_BASE,  KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, _______, KC_MPLY,
        _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______,
        ENT_SLP, _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______
    ),
#else
    /* FUNC (Pro Micro): F-keys and media, with RGB lighting controls on the left.
     * Keep the F-keys and the right half the same as in the BLE Micro Pro version above.
     */
    [_FUNC] = LAYOUT_ergo68(
        RM_SATU, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                               KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        RM_SATD, RM_NEXT, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, KC_F12,
        RM_VALU, RM_TOGG, RM_NEXT, RM_PREV, RM_HUEU, RM_HUED, L_BASE,           L_BASE,  KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, _______, KC_MPLY,
        RM_VALD, RM_OFF,  _______, _______, _______, _______, _______,          _______, _______, RM_NEXT, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, RM_ON,   RM_OFF,           _______, _______, _______, _______, _______, _______, _______
    ),
#endif

    /* NUMPAD: numpad on the right, Boot, Reboot and EEPROM clear on the left */
    [_NUMPAD] = LAYOUT_ergo68(
        QK_BOOT, _______, _______, _______, _______, _______,                             _______, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS, _______,
        QK_RBT,  _______, _______, _______, _______, _______, _______,          _______, _______, KC_P7,   KC_P8,   KC_P9,   KC_PPLS, _______,
        EE_CLR,  _______, _______, _______, _______, _______, L_BASE,          L_BASE,  _______, KC_P4,   KC_P5,   KC_P6,   KC_PCMM, _______,
        _______, _______, _______, _______, _______, _______, _______,          _______, _______, KC_P1,   KC_P2,   KC_P3,   KC_PEQL, _______,
        _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______, KC_P0,   KC_PDOT, KC_COMM, _______
    ),
};

#if defined(ENCODER_MAP_ENABLE)
// The Ergo68 has no encoders, but the ble_micro_pro target enables ENCODER_MAP_ENABLE
// and QMK requires the encoder map to have as many layers as the keymap.
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]   = {{KC_NO, KC_NO}},
    [_NAV]    = {{KC_NO, KC_NO}},
    [_FUNC]   = {{KC_NO, KC_NO}},
    [_NUMPAD] = {{KC_NO, KC_NO}},
};
#endif
// clang-format on

#ifdef RGB_MATRIX_ENABLE
/*
 * Pro Micro build only (the BLE Micro Pro config leaves the LEDs off). The three LEDs at the top
 * of each half are indicators, as in Yushakobo's default keymap: Caps/Num/Scroll Lock on the left,
 * and on the right which of NAV (held), FUNC and NUMPAD are on.
 */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    led_t host = host_keyboard_led_state();
    RGB_MATRIX_INDICATOR_SET_COLOR(0, 0, 0, host.caps_lock ? 100 : 0);
    RGB_MATRIX_INDICATOR_SET_COLOR(1, 0, 0, host.num_lock ? 100 : 0);
    RGB_MATRIX_INDICATOR_SET_COLOR(2, 0, 0, host.scroll_lock ? 100 : 0);

    bool nav = layer_state_is(_NAV), func = layer_state_is(_FUNC), numpad = layer_state_is(_NUMPAD);
    RGB_MATRIX_INDICATOR_SET_COLOR(37, nav ? 70 : 0, 0, nav ? 100 : 0);    // purple
    RGB_MATRIX_INDICATOR_SET_COLOR(38, func ? 100 : 0, func ? 40 : 0, 0);  // orange
    RGB_MATRIX_INDICATOR_SET_COLOR(39, 0, numpad ? 100 : 0, 0);            // green
    return false;
}
#endif

#if defined(DYNAMIC_KEYMAP_ENABLE) && !defined(KEYMAP_SYNC_DISABLE)
/*
 * Both builds run the keymap stored in EEPROM, which is what Vial/Remap/VIA edit. keymaps[] above
 * is only copied there when the EEPROM is reset: the BLE Micro Pro firmware never does that on its
 * own after an update, and mainline VIA only does it for a build from a different day. So flashing
 * a changed keymap could otherwise keep running the old one.
 *
 * To get the "edit keymap.c, build, flash, done" workflow, keep a fingerprint of keymaps[] in the
 * user EEPROM word and reload the EEPROM keymap whenever it changes. Live edits survive a re-flash
 * as long as the layers in this file stay the same (on the Pro Micro, VIA itself still resets
 * them for a build from another day). Define KEYMAP_SYNC_DISABLE in config.h to turn this off.
 */
#    define SYNC_KEYMAP
static uint32_t keymap_fingerprint(void) {
    const uint8_t *data = (const uint8_t *)keymaps;
    uint32_t       hash = 2166136261u; // 32-bit FNV-1a
    for (size_t i = 0; i < sizeof(keymaps); i++) {
        hash ^= pgm_read_byte(data + i);
        hash *= 16777619u;
    }
    return hash;
}

static void sync_keymap(void) {
    uint32_t fingerprint = keymap_fingerprint();
    if (eeconfig_read_user() != fingerprint) {
        dynamic_keymap_reset();
        eeconfig_update_user(fingerprint);
    }
}
#endif

#if defined(PROTOCOL_BMP) && defined(BMP_AUTO_SLEEP_MINUTES)
#    include "state_controller.h"
#endif

void keyboard_post_init_user(void) {
#ifdef SYNC_KEYMAP
    sync_keymap();
#endif
#if defined(PROTOCOL_BMP) && defined(BMP_AUTO_SLEEP_MINUTES)
    // The Ergo68 config from the Web Configurator leaves auto-sleep off (reserved[2] = 0).
    // Only applies on battery: the firmware never auto-sleeps while connected to USB.
    if (get_auto_sleep_timeout() == 0) {
        set_auto_sleep_timeout(BMP_AUTO_SLEEP_MINUTES * 60UL * 1000UL);
    }
#endif
}
