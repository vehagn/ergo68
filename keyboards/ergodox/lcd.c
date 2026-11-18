/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "lcd.h"

#include <string.h>

#include "lcd_font.h"
#include "st7565.h"

static void fill_rect(int x, int y, int w, int h, bool on) {
    for (int i = x; i < x + w; i++) {
        for (int j = y; j < y + h; j++) {
            if (i >= 0 && i < LCD_WIDTH && j >= 0 && j < LCD_HEIGHT) {
                st7565_write_pixel(i, j, on);
            }
        }
    }
}

static int text_width(const lcd_font_t *font, const char *text) {
    return (int)strlen(text) * font->width;
}

// Draw text with its top-left corner at (x, y). With invert, set pixels are cleared instead.
static void draw_text(const lcd_font_t *font, int x, int y, const char *text, bool invert) {
    for (; *text; text++, x += font->width) {
        uint8_t c = (uint8_t)*text;
        if (c < font->first || c > font->last) {
            c = '?';
        }
        const uint16_t *columns = &font->columns[(c - font->first) * font->width];
        for (int col = 0; col < font->width; col++) {
            for (int row = 0; row < font->height; row++) {
                if (columns[col] & (1u << row)) {
                    int px = x + col, py = y + row;
                    if (px >= 0 && px < LCD_WIDTH && py >= 0 && py < LCD_HEIGHT) {
                        st7565_write_pixel(px, py, !invert);
                    }
                }
            }
        }
    }
}

static void draw_centered(const lcd_font_t *font, int y, const char *text) {
    draw_text(font, (LCD_WIDTH - text_width(font, text)) / 2, y, text, false);
}

/*
 * Indicator badges: white text on a black box. Full labels if they fit, else three letters, else
 * one. The small font's capitals sit on rows 2-7 of its 10, so a 10-row box with the text one row
 * up has an even border around them.
 */
#define BADGE_HEIGHT 10
#define BADGE_PAD 2
#define BADGE_GAP 3

static const char *const badge_labels[][3] = {
    {"CAPS", "CAP", "C"}, {"NUM", "NUM", "N"},  {"SCRL", "SCR", "S"}, {"SHIFT", "SFT", "S"},
    {"CTRL", "CTL", "C"}, {"ALT", "ALT", "A"},  {"GUI", "GUI", "G"},
};
#define BADGE_COUNT (sizeof(badge_labels) / sizeof(badge_labels[0]))

static void draw_badges(uint8_t indicators, int y) {
    for (int form = 0; form < 3; form++) {
        int total = 0, count = 0;
        for (unsigned i = 0; i < BADGE_COUNT; i++) {
            if (indicators & (1 << i)) {
                total += text_width(&lcd_font_small, badge_labels[i][form]) + 2 * BADGE_PAD;
                count++;
            }
        }
        if (count == 0) {
            return;
        }
        total += (count - 1) * BADGE_GAP;
        if (total > LCD_WIDTH && form < 2) {
            continue;
        }
        int x = (LCD_WIDTH - total) / 2;
        for (unsigned i = 0; i < BADGE_COUNT; i++) {
            if (indicators & (1 << i)) {
                const char *label = badge_labels[i][form];
                int         w     = text_width(&lcd_font_small, label) + 2 * BADGE_PAD;
                fill_rect(x, y, w, BADGE_HEIGHT, true);
                draw_text(&lcd_font_small, x + BADGE_PAD, y - 1, label, true);
                x += w + BADGE_GAP;
            }
        }
        return;
    }
}

void lcd_draw_layer_screen(const char *name, uint8_t indicators) {
    st7565_clear();
    draw_centered(&lcd_font_large, 2, name);
    draw_badges(indicators, LCD_HEIGHT - BADGE_HEIGHT);
}

/*
 * The idle animation: a zen stone garden across both LCDs (256 x 32, the left half's LCD first).
 * Raked lines in the sand, and around each stone rings that slowly ripple outward.
 */
typedef struct {
    int16_t x, y;
    uint8_t r; // stone radius
} stone_t;

static const stone_t stones[] = {{34, 17, 5}, {101, 9, 3}, {164, 20, 6}, {226, 11, 4}};

#define RING_SPACING 4 // pixels between rings
#define RING_ZONE 14   // how far the rings reach beyond a stone

static uint16_t isqrt(uint32_t n) {
    uint32_t root = 0, bit = 1u << 30;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return (uint16_t)root;
}

void lcd_draw_idle(uint8_t half, uint16_t frame) {
    st7565_clear();
    const int x0    = half * LCD_WIDTH;
    const int phase = frame % RING_SPACING;
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            // The nearest stone decides this pixel, if it's within its rings.
            int      gx = x0 + x;
            uint16_t best = UINT16_MAX;
            uint8_t  r    = 0;
            for (unsigned i = 0; i < sizeof(stones) / sizeof(stones[0]); i++) {
                int32_t  dx = gx - stones[i].x, dy = y - stones[i].y;
                uint16_t d  = isqrt((uint32_t)(dx * dx + dy * dy));
                if (d < best) {
                    best = d;
                    r    = stones[i].r;
                }
            }
            bool on;
            if (best <= r) {
                on = true; // the stone
            } else if (best <= r + RING_ZONE) {
                on = (best - r - 1 + RING_SPACING - phase) % RING_SPACING == 0; // its rings
            } else {
                on = y % 4 == 2 && gx % 8 != 7; // raked lines, with a gap now and then
            }
            if (on) {
                st7565_write_pixel(x, y, true);
            }
        }
    }
}

void lcd_draw_splash(void) {
    st7565_clear();
    draw_centered(&lcd_font_large, 1, "stonegarden");
    draw_centered(&lcd_font_small, 19, "Ergodox Infinity");
}

void lcd_draw_suspending(void) {
    st7565_clear();
    draw_centered(&lcd_font_large, 8, "Suspending...");
}
