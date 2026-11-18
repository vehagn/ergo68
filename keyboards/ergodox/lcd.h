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

// One frame of the idle animation, a zen stone garden that spans both LCDs. half: 0 for the left
// LCD, 1 for the right. Each frame moves the ripples one pixel outward.
void lcd_draw_idle(uint8_t half, uint16_t frame);

// The other idle animation (lcd_wave.c): a simulated sound wave across both LCDs, reflecting off a
// few random walls. Start it with the same seed on both halves, step it, then draw a half.
void     lcd_wave_start(uint32_t seed);
void     lcd_wave_step(void);
uint16_t lcd_wave_steps(void);
void     lcd_draw_wave(uint8_t half);

// Start-up and suspend screens.
void lcd_draw_splash(void);
void lcd_draw_suspending(void);
