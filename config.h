/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// Same Vial settings as the stock BLE Micro Pro firmware
// (keyboards/ble_micro_pro/keymaps/vial in sekigon-gonnoc/vial-qmk).
#define VIAL_KEYBOARD_UID {0x05, 0xE4, 0xA1, 0x7F, 0xDC, 0x87, 0xCB, 0x2A}
#define VIAL_UNLOCK_COMBO_ROWS { 0, 1 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// Uncomment to stop a new firmware from replacing the keymap stored on the keyboard
// when the layers in keymap.c change (e.g. if you mostly edit the keymap in Vial).
// #define KEYMAP_SYNC_DISABLE
