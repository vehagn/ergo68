/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// ErgoDox Infinity: send the layer and Caps/Num/Scroll Lock state to the other half too, so both
// LCDs can show them. The ErgoDox EZ isn't a split keyboard in QMK's sense and ignores these.
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE

// ErgoDox Infinity LCDs: both show the modifiers, and both count idle time from key presses on
// either half, for the idle animation and for turning off. The idle animation starts after 30
// seconds (LCD_IDLE_MS in keymap.c) and the LCDs turn off after 5 minutes without key presses.
#define SPLIT_MODS_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define ST7565_TIMEOUT 300000
