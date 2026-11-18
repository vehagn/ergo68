/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// ErgoDox Infinity: send the layer and Caps/Num/Scroll Lock state to the other half too, so both
// LCDs can show them. The ErgoDox EZ isn't a split keyboard in QMK's sense and ignores these.
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE

// ErgoDox Infinity LCDs: the other half shows the typing speed, and its LCD turns on and off with
// key presses on either half.
#define SPLIT_WPM_ENABLE
#define SPLIT_ACTIVITY_ENABLE
