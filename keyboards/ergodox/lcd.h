/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * ErgoDox Infinity LCD screens (128 x 32). The drawing only uses st7565_clear() and
 * st7565_write_pixel(), so scripts/lcd_preview.c can render the same code to images.
 */

#define LCD_WIDTH 128
#define LCD_HEIGHT 32
#define LCD_WPM_HISTORY LCD_WIDTH // one column per sample

// Bits for lcd_draw_layer_screen's indicators.
enum {
    LCD_CAPS   = 1 << 0,
    LCD_NUM    = 1 << 1,
    LCD_SCROLL = 1 << 2,
    LCD_SHIFT  = 1 << 3,
    LCD_CTRL   = 1 << 4,
    LCD_ALT    = 1 << 5,
    LCD_GUI    = 1 << 6,
};

// The layer name, centred, with a badge below it for each active lock and modifier.
void lcd_draw_layer_screen(const char *name, uint8_t indicators);

// Words per minute above a graph of the last LCD_WPM_HISTORY samples, oldest first.
void lcd_draw_wpm_screen(uint8_t wpm, const uint8_t *history, uint8_t count);

// Start-up and suspend screens.
void lcd_draw_splash(void);
void lcd_draw_suspending(void);
