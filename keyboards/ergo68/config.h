/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// BLE Micro Pro: same Vial settings as the stock firmware
// (keyboards/ble_micro_pro/keymaps/vial in sekigon-gonnoc/vial-qmk). Unused in the Pro Micro build.
#define VIAL_KEYBOARD_UID {0x05, 0xE4, 0xA1, 0x7F, 0xDC, 0x87, 0xCB, 0x2A}
#define VIAL_UNLOCK_COMBO_ROWS { 0, 1 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// BLE Micro Pro: don't show the keyboard's own USB drive (ergo68, with VERSION.TXT and the config
// files) while it runs, as in the fork's vial_no_msc keymap. The bootloader's BLEMICROPRO drive for
// flashing is unaffected. Remove to get the drive back.
#define DISABLE_MSC 1

// Uncomment to stop a new firmware from replacing the keymap stored on the keyboard when the
// layers in keymap.c change (e.g. if you mostly edit the keymap in Vial or Remap).
// #define KEYMAP_SYNC_DISABLE

// BLE Micro Pro: uncomment to put a half to sleep after this many minutes without a key press on
// it while on battery, to save the cells. The Ergo68 config from the Web Configurator leaves
// auto-sleep off. Not tried on this keyboard yet: check that a key press wakes it up again.
// #define BMP_AUTO_SLEEP_MINUTES 30
