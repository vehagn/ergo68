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

// BMP Boost: measure the battery at the BT input (analog input AIN4, behind the board's 1:3
// divider) instead of the BMP's default, which on a BMP Boost is its VCC output: that stays flat
// while the cells run down, so Batt couldn't show low cells. Comment out BMP_BATTERY_AIN for an
// original BLE Micro Pro. The BMP reports the BT voltage itself (it accounts for the divider:
// Batt read 2259/2238 mV with a multimeter showing 2.27 V at BT). MIN/MAX are the range the battery
// percentage counts between, in BT millivolts, and below MIN the BMP shuts the half down. Two NiMH
// cells in series, less the kit's 0.32 V diode and switch drop, as in the ZMK config:
//   MAX 2280 mV = 1.30 V per cell: 100%
//   MIN 1780 mV = 1.05 V per cell: 0%, and the half switches off to protect the cells
// (1.10 V per cell, time to charge, is 1880 mV or about 20%.)
#define BMP_BATTERY_AIN    4
#define BMP_BATTERY_MIN_MV 1780
#define BMP_BATTERY_MAX_MV 2280

// BLE Micro Pro: put a half to sleep after this many minutes without a key press on it while on
// battery, to save the cells (the Ergo68 config from the Web Configurator leaves auto-sleep off).
// Comment out to turn it off. Not tried on this keyboard yet: check that a key press wakes it up.
#define BMP_AUTO_SLEEP_MINUTES 30
