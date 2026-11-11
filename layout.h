/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/*
 * Physical layout of the Yushakobo Ergo68, for the generic BLE Micro Pro target.
 *
 * The `ble_micro_pro` keyboard in sekigon-gonnoc/vial-qmk does not know about the
 * Ergo68: its matrix is a fixed 32x32 array, and the real size and pins come from
 * CONFIG.BIN at runtime (10 rows x 7 cols for the Ergo68: rows 0-4 are the left
 * half, rows 5-9 the right half). This macro places the 68 keys at the same matrix
 * positions as `keyboards/yushakobo/ergo68/keyboard.json` in upstream QMK, and the
 * argument order is identical to that keyboard's LAYOUT, so keymaps can be copied
 * between the two.
 *
 * Arguments are named kRC after their matrix position. Unused positions ([0][6],
 * [5][6] and everything outside 10x7) stay KC_NO.
 *
 *  ,-----------------------------------------.                         ,-----------------------------------------.
 *  | k00 | k01 | k02 | k03 | k04 | k05 |                               | k55 | k54 | k53 | k52 | k51 | k50 |
 *  |-----+-----+-----+-----+-----+-----+-----.                   ,-----+-----+-----+-----+-----+-----+-----|
 *  | k10 | k11 | k12 | k13 | k14 | k15 | k16 |                   | k66 | k65 | k64 | k63 | k62 | k61 | k60 |
 *  |-----+-----+-----+-----+-----+-----+-----|                   |-----+-----+-----+-----+-----+-----+-----|
 *  | k20 | k21 | k22 | k23 | k24 | k25 | k26 |                   | k76 | k75 | k74 | k73 | k72 | k71 | k70 |
 *  |-----+-----+-----+-----+-----+-----+-----|                   |-----+-----+-----+-----+-----+-----+-----|
 *  | k30 | k31 | k32 | k33 | k34 | k35 | k36 |                   | k86 | k85 | k84 | k83 | k82 | k81 | k80 |
 *  |-----+-----+-----+-----+-----+-----+-----|                   |-----+-----+-----+-----+-----+-----+-----|
 *  | k40 | k41 | k42 |  k43  | k44 | k45 | k46 |           | k96 | k95 | k94 |  k93  | k92 | k91 | k90 |
 *  `-----------------'       `-----|     |     |           |     |     |-----'       `-----------------'
 *                                  `-----------'           `-----------'
 *  k16/k26/k36 and k66/k76/k86 are the inner column, offset half a row upwards.
 *  k43/k44 and k93/k94 are 1u thumb keys; k45/k46 and k95/k96 are 2u (tall) thumb keys.
 */

// clang-format off
#define LAYOUT_ergo68( \
    k00, k01, k02, k03, k04, k05,                     k55, k54, k53, k52, k51, k50, \
    k10, k11, k12, k13, k14, k15, k16,           k66, k65, k64, k63, k62, k61, k60, \
    k20, k21, k22, k23, k24, k25, k26,           k76, k75, k74, k73, k72, k71, k70, \
    k30, k31, k32, k33, k34, k35, k36,           k86, k85, k84, k83, k82, k81, k80, \
    k40, k41, k42, k43, k44, k45, k46,           k96, k95, k94, k93, k92, k91, k90  \
) { \
    [0] = { k00, k01, k02, k03, k04, k05, KC_NO }, \
    [1] = { k10, k11, k12, k13, k14, k15, k16   }, \
    [2] = { k20, k21, k22, k23, k24, k25, k26   }, \
    [3] = { k30, k31, k32, k33, k34, k35, k36   }, \
    [4] = { k40, k41, k42, k43, k44, k45, k46   }, \
    [5] = { k50, k51, k52, k53, k54, k55, KC_NO }, \
    [6] = { k60, k61, k62, k63, k64, k65, k66   }, \
    [7] = { k70, k71, k72, k73, k74, k75, k76   }, \
    [8] = { k80, k81, k82, k83, k84, k85, k86   }, \
    [9] = { k90, k91, k92, k93, k94, k95, k96   }, \
}
// clang-format on
