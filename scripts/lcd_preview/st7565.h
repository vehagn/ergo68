/* Stand-in for QMK's drivers/lcd/st7565.h, with only what keyboards/ergodox/lcd.c uses. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

void st7565_clear(void);
void st7565_write_pixel(uint8_t x, uint8_t y, bool on);
