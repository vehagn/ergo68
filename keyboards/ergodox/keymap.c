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
 * ErgoDox Infinity LCDs (the drawing is in lcd.c, the fonts in lcd_font.h). Both halves show the
 * active layer's name and a badge for each active lock and modifier. As with the old visualizer,
 * the backlight takes the layer's colour and fades between colours, there's a start-up screen, and
 * it says it's suspending. After a while without key presses, both LCDs show a simulated sound
 * wave together (lcd_wave.c), with the backlight dimmed.
 */
#    include "lcd.h"
#    include "transactions.h"

typedef struct {
    const char *name;
    uint8_t     r, g, b;
} layer_style_t;

// The old visualizer's colours (hue/saturation 0/0, 148/255, 80/255 and 190/255), in RGB, since the
// Infinity's QMK build has no HSV conversion (that comes with RGB lighting, which it doesn't have).
static const layer_style_t layer_styles[] = {
    [_BASE]   = {"Ergodox Infinity", 255, 255, 255}, // white
    [_NAV]    = {"Mouse + Nav", 0, 135, 255},        // blue
    [_FUNC]   = {"Function keys", 116, 0, 255},      // purple
    [_NUMPAD] = {"Numpad", 32, 255, 0},              // green
};

#    define LCD_FADE_MS 250         // between layer colours
#    define LCD_FADE_IN_MS 800      // at start-up and when the LCD comes back on
#    define LCD_FADE_OUT_MS 1000    // when suspending or timing out
#    define LCD_SPLASH_MS 2000      // how long the start-up screen stays
#    define LCD_IDLE_MS 30000       // without key presses before the idle animation starts
#    define LCD_IDLE_FRAME_MS 250   // per frame of the zen garden (shown until the wave starts)
#    define LCD_IDLE_DIM_MS 2000    // how long the backlight takes to dim when idle
#    define LCD_WAVE_STEP_MS 33     // per step of the wave simulation: 30 steps a second

// Backlight colour, 0-255 per channel, shown at the half brightness QMK uses for these LCDs.
static uint8_t  lcd_rgb[3], fade_from[3], fade_to[3];
static uint16_t fade_start, fade_ms;
static bool     fading;

static void lcd_apply_colour(void) {
    ergodox_infinity_lcd_color(lcd_rgb[0] * 128, lcd_rgb[1] * 128, lcd_rgb[2] * 128);
}

static void lcd_fade_to(uint8_t r, uint8_t g, uint8_t b, uint16_t ms) {
    memcpy(fade_from, lcd_rgb, 3);
    fade_to[0] = r, fade_to[1] = g, fade_to[2] = b;
    fade_start = timer_read();
    fade_ms    = ms;
    fading     = true;
}

static void lcd_fade_task(void) {
    if (!fading) {
        return;
    }
    uint16_t elapsed = timer_elapsed(fade_start);
    for (int i = 0; i < 3; i++) {
        lcd_rgb[i] = elapsed >= fade_ms ? fade_to[i] : fade_from[i] + (int32_t)(fade_to[i] - fade_from[i]) * elapsed / fade_ms;
    }
    fading = elapsed < fade_ms;
    lcd_apply_colour();
}

static uint8_t lcd_layer(void) {
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    return layer < ARRAY_SIZE(layer_styles) ? layer : _BASE;
}

// The layer's colour, at a quarter of it while idle.
static void lcd_fade_to_layer(uint16_t ms, bool dim) {
    const layer_style_t *style = &layer_styles[lcd_layer()];
    uint8_t              shift = dim ? 2 : 0;
    lcd_fade_to(style->r >> shift, style->g >> shift, style->b >> shift, ms);
}

static bool     splash = true, redraw = true, idle;
static uint16_t splash_start, idle_frame = UINT16_MAX;
static uint8_t  shown_layer = UINT8_MAX, shown_indicators;

/*
 * The wave starts from a random seed that the half on USB picks when the keyboard goes idle and
 * sends to the other half, so both simulate the same wave. The other half receives it in the split
 * transport's thread, so it only stores it here; its main loop starts the wave.
 */
static bool              wave_running;
static uint32_t          wave_start;
static uint16_t          wave_drawn;
static volatile bool     wave_seed_received;
static volatile uint32_t wave_seed;

static void wave_seed_handler(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(uint32_t)) {
        memcpy((void *)&wave_seed, in_data, sizeof(uint32_t));
        wave_seed_received = true;
    }
}

static void wave_begin(uint32_t seed) {
    lcd_wave_start(seed);
    wave_running = true;
    wave_start   = timer_read32();
    wave_drawn   = UINT16_MAX;
}

static void wave_task(void) {
    // Catch up with the clock, a few steps at a time so the keyboard stays responsive.
    uint32_t target = timer_elapsed32(wave_start) / LCD_WAVE_STEP_MS;
    for (int i = 0; i < 4 && lcd_wave_steps() < target; i++) {
        lcd_wave_step();
    }
    if (lcd_wave_steps() != wave_drawn) {
        wave_drawn = lcd_wave_steps();
        lcd_draw_wave(is_keyboard_left() ? 0 : 1);
    }
}

void keyboard_post_init_user(void) {
    lcd_apply_colour(); // dark, then fade in behind the start-up screen
    lcd_fade_to(255, 255, 255, LCD_FADE_IN_MS);
    splash_start = timer_read();
    lcd_draw_splash();
    transaction_register_rpc(USER_LCD_WAVE_SEED, wave_seed_handler);
}

void st7565_on_user(void) {
    lcd_fade_to_layer(LCD_FADE_IN_MS, false);
}

void st7565_off_user(void) {
    lcd_fade_to(0, 0, 0, LCD_FADE_OUT_MS);
}

void housekeeping_task_user(void) {
    lcd_fade_task();
}

void st7565_task_user(void) {
    if (splash) {
        if (timer_elapsed(splash_start) < LCD_SPLASH_MS) {
            return;
        }
        splash = false;
        redraw = true;
        lcd_fade_to_layer(LCD_FADE_MS, false);
    }

    // Idle: both halves count from the last key press on either half (SPLIT_ACTIVITY_ENABLE), so
    // they start the animation together and keep roughly the same frame.
    uint32_t quiet = last_input_activity_elapsed();
    if (quiet >= LCD_IDLE_MS) {
        if (!idle) {
            idle = true;
            lcd_fade_to_layer(LCD_IDLE_DIM_MS, true);
            if (is_keyboard_master()) {
                uint32_t seed = timer_read32() * 2654435761u + shown_indicators + 1;
                wave_begin(seed);
                transaction_rpc_send(USER_LCD_WAVE_SEED, sizeof(seed), &seed);
            }
        }
        if (!is_keyboard_master() && wave_seed_received) {
            wave_seed_received = false;
            wave_begin(wave_seed);
        }
        if (wave_running) {
            wave_task();
        } else {
            // Until the seed arrives from the other half (it normally does at once): the zen garden.
            uint16_t frame = (quiet - LCD_IDLE_MS) / LCD_IDLE_FRAME_MS;
            if (frame != idle_frame) {
                idle_frame = frame;
                lcd_draw_idle(is_keyboard_left() ? 0 : 1, frame);
            }
        }
        return;
    }
    if (idle) {
        idle               = false;
        wave_running       = false;
        wave_seed_received = false;
        idle_frame         = UINT16_MAX;
        redraw             = true;
        lcd_fade_to_layer(LCD_FADE_MS, false);
    }

    uint8_t layer = lcd_layer();
    if (layer != shown_layer) {
        if (shown_layer != UINT8_MAX) {
            lcd_fade_to_layer(LCD_FADE_MS, false);
        }
        shown_layer = layer;
        redraw      = true;
    }

    // The modifiers reach the other half with SPLIT_MODS_ENABLE, the locks with SPLIT_LED_STATE_ENABLE.
    led_t   host       = host_keyboard_led_state();
    uint8_t mods       = get_mods() | get_oneshot_mods();
    uint8_t indicators = (host.caps_lock ? LCD_CAPS : 0) | (host.num_lock ? LCD_NUM : 0) | (host.scroll_lock ? LCD_SCROLL : 0) | (mods & MOD_MASK_SHIFT ? LCD_SHIFT : 0) | (mods & MOD_MASK_CTRL ? LCD_CTRL : 0) | (mods & MOD_MASK_ALT ? LCD_ALT : 0) | (mods & MOD_MASK_GUI ? LCD_GUI : 0);
    if (redraw || indicators != shown_indicators) {
        shown_indicators = indicators;
        lcd_draw_layer_screen(layer_styles[layer].name, indicators);
    }
    redraw = false;
}

static bool suspended;

void suspend_power_down_user(void) {
    // Called over and over while the computer sleeps, with the rest of the firmware stopped.
    if (!suspended) {
        suspended = true;
        lcd_draw_suspending();
        st7565_render();
        lcd_fade_to(0, 0, 0, LCD_FADE_OUT_MS);
    }
    lcd_fade_task();
}

void suspend_wakeup_init_user(void) {
    suspended = false;
    redraw    = true;
    lcd_fade_to_layer(LCD_FADE_IN_MS, false);
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
