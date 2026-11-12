/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// ErgoDox Infinity: send the layer and Caps/Num/Scroll Lock state to the other half too, so both
// LCDs can show them. The ErgoDox EZ isn't a split keyboard in QMK's sense and ignores these.
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
