/*
 * Render the ErgoDox Infinity LCD screens (keyboards/ergodox/lcd.c) on the host, for
 * scripts/lcd_preview/render.py. Prints each screen as "name rrggbb" followed by 32 rows of 0/1.
 * The layer names and colours are the ones in keyboards/ergodox/keymap.c.
 */
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "st7565.h"

static unsigned char pixels[LCD_HEIGHT][LCD_WIDTH];

void st7565_clear(void) {
    memset(pixels, 0, sizeof(pixels));
}

void st7565_write_pixel(uint8_t x, uint8_t y, bool on) {
    pixels[y][x] = on;
}

static void dump(const char *name, unsigned rgb) {
    printf("%s %06x\n", name, rgb);
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) putchar('0' + pixels[y][x]);
        putchar('\n');
    }
}

int main(void) {
    const unsigned white = 0xffffff, blue = 0x0087ff, purple = 0x7400ff, green = 0x20ff00;

    lcd_draw_splash();
    dump("Start-up", white);

    lcd_draw_layer_screen("Ergodox Infinity", 0);
    dump("BASE", white);
    lcd_draw_layer_screen("Ergodox Infinity", LCD_CAPS | LCD_SHIFT);
    dump("BASE, Caps Lock and Shift", white);
    lcd_draw_layer_screen("Mouse + Nav", 0);
    dump("NAV", blue);
    lcd_draw_layer_screen("Function keys", LCD_CTRL | LCD_ALT);
    dump("FUNC, holding C+A", purple);
    lcd_draw_layer_screen("Numpad", LCD_NUM);
    dump("NUMPAD, Num Lock", green);
    lcd_draw_layer_screen("Function keys", LCD_CAPS | LCD_NUM | LCD_SCROLL | LCD_SHIFT | LCD_CTRL | LCD_ALT | LCD_GUI);
    dump("Everything on at once (shorter labels)", purple);

    uint8_t history[LCD_WPM_HISTORY];
    for (int i = 0; i < LCD_WPM_HISTORY; i++) {
        // A minute of typing: a burst, a pause, then a steady stretch.
        history[i] = i < 20 ? 0 : i < 45 ? 30 + (i - 20) * 2 : i < 60 ? 80 - (i - 45) * 5 : i < 75 ? 0 : 55 + (i % 7) * 3;
    }
    lcd_draw_wpm_screen(history[LCD_WPM_HISTORY - 1], history, LCD_WPM_HISTORY);
    dump("Right half: typing speed", white);
    lcd_draw_wpm_screen(0, history, 30);
    dump("Right half: just started", white);

    lcd_draw_suspending();
    dump("Suspending", white);
    return 0;
}
