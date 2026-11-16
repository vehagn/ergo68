/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * BLE Micro Pro: the Ergo68's hardware config, built into the firmware.
 *
 * The generic ble_micro_pro firmware gets its matrix, pins and split mode from a config that the
 * BLE Micro Pro Web Configurator writes to the BMP's flash. With ERGO68_BMP_HALF set (see
 * rules.mk), this file supplies that config instead, so a freshly flashed half works without the
 * Web Configurator, and whatever config is stored is ignored. The values are those of
 * sekigon-gonnoc's Ergo68 configs (BLE-Micro-Pro/bmp-vial-config/yushakobo_ergo68_*_config.json).
 */

#include <string.h>
#include "bmp.h"

#if defined(ERGO68_BMP_LEFT) || defined(ERGO68_BMP_RIGHT)

#    ifdef ERGO68_BMP_LEFT
#        define ERGO68_IS_LEFT true
#        define ERGO68_MODE    SPLIT_MASTER // talks to the computer and to the right half
#    else
#        define ERGO68_IS_LEFT false
#        define ERGO68_MODE    SPLIT_SLAVE
#    endif

bool bmp_config_overwrite(bmp_api_config_t const *const config_on_storage, bmp_api_config_t *const keyboard_config) {
    bmp_api_config_t config = {
        .version     = CONFIG_VERSION,
        .device_info = {.vid = 0x3265, .pid = 0x0011, .name = "ergo68", .manufacture = "yushakobo"},
        .matrix =
            {
                // 10x7 in all; each half scans its own 5x7 with the same pins.
                .rows            = 10,
                .cols            = 7,
                .device_rows     = 5,
                .device_cols     = 7,
                .layer           = 8,
                .debounce        = 1,
                .diode_direction = MATRIX_COL2ROW,
                .row_pins        = {7, 8, 9, 10, 11},
                .col_pins        = {20, 19, 18, 17, 16, 15, 14},
                .is_left_hand    = ERGO68_IS_LEFT,
            },
        .mode             = ERGO68_MODE,
        .startup          = STARTUP_ADV_NO_WL,
        .param_peripheral = {.max_interval = 30, .min_interval = 30, .slave_latency = 16},
        .param_central    = {.max_interval = 30, .min_interval = 30, .slave_latency = 0},
        .led              = {.pin = 1, .num = 0}, // the LEDs can't light on a BMP, so drive none
        // .encoder stays zero: no encoders.
    };
    // Keep the stored indicator LED and auto-sleep settings. Both pointers may be the same config.
    memcpy(config.reserved, config_on_storage->reserved, sizeof(config.reserved));
    *keyboard_config = config;
    return true;
}

#endif
