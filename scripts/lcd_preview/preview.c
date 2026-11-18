/*
 * Render the ErgoDox Infinity LCD screens (keyboards/ergodox/lcd.c) on the host, for
 * scripts/lcd_preview/render.py. Prints each screen as "name rrggbb width" followed by 32 rows of 0/1.
 * The layer names and colours are the ones in keyboards/ergodox/keymap.c.
 *
 *   preview        every screen, with the idle animation's first frame across both LCDs
 *   preview idle   the idle animation, one full cycle across both LCDs
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
    printf("%s %06x %d\n", name, rgb, LCD_WIDTH);
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) putchar('0' + pixels[y][x]);
        putchar('\n');
    }
}

// Both LCDs side by side, left half first.
static void dump_idle(const char *name, unsigned rgb, uint16_t frame) {
    static unsigned char both[LCD_HEIGHT][2 * LCD_WIDTH];
    for (int half = 0; half < 2; half++) {
        lcd_draw_idle(half, frame);
        for (int y = 0; y < LCD_HEIGHT; y++) memcpy(&both[y][half * LCD_WIDTH], pixels[y], LCD_WIDTH);
    }
    printf("%s %06x %d\n", name, rgb, 2 * LCD_WIDTH);
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < 2 * LCD_WIDTH; x++) putchar('0' + both[y][x]);
        putchar('\n');
    }
}

int main(int argc, char **argv) {
    const unsigned white = 0xffffff, blue = 0x0087ff, purple = 0x7400ff, green = 0x20ff00, dim = 0x808080;

    if (argc > 1 && strcmp(argv[1], "idle") == 0) {
        for (uint16_t frame = 0; frame < 4; frame++) {
            char name[32];
            snprintf(name, sizeof(name), "Idle, frame %u", frame);
            dump_idle(name, dim, frame);
        }
        return 0;
    }

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

    dump_idle("Idle, across both LCDs (dimmed backlight)", dim, 0);

    lcd_draw_suspending();
    dump("Suspending", white);
    return 0;
}
