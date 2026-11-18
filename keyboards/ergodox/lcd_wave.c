/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * The idle animation: a sound wave, simulated across both LCDs (256 x 32 cells, one per pixel, the
 * left half's LCD first).
 *
 * It solves the 2D wave equation u_tt = c^2 (u_xx + u_yy) with the usual explicit finite-difference
 * scheme, at a Courant number of 1/2 (c dt / dx = 1/2, stable up to 1/sqrt(2)):
 *
 *     u_next = u + (u_north + u_south + u_east + u_west) / 4 - u_prev
 *
 * in 16-bit integers, with a little damping each step so the energy stays bounded. The wave is
 * held at zero on a few random short walls and at the screen edges, so it reflects off them.
 * Random drops start new waves now and then.
 *
 * Each half simulates the whole field, so they need no data from each other while it runs: the
 * integer maths is exact and the same on both, so they only have to start from the same random
 * seed (keymap.c sends it from the half on USB to the other).
 */
#include "lcd.h"

#include <string.h>

#include "st7565.h"

#define WAVE_W (2 * LCD_WIDTH)
#define WAVE_H LCD_HEIGHT

static int16_t  wave_a[WAVE_H][WAVE_W], wave_b[WAVE_H][WAVE_W];
static int16_t (*u)[WAVE_W] = wave_a, (*u_prev)[WAVE_W] = wave_b;
static uint8_t  walls[WAVE_H][WAVE_W / 8]; // one bit per cell
static uint32_t rng;
static uint16_t step_count, next_drop;

#define WALL_COUNT 7
#define WAVE_CALM 240 // heights below this aren't drawn, so small ripples don't speckle the screen
#define DROP_AMPLITUDE 3200
#define DROP_MIN_STEPS 45  // between drops: 1.5 to 4 seconds at 30 steps a second
#define DROP_MAX_STEPS 120

static uint32_t random32(void) { // xorshift32
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

static uint32_t random_below(uint32_t n) {
    return random32() % n;
}

static bool is_wall(int x, int y) {
    return walls[y][x >> 3] & (1 << (x & 7));
}

static void set_wall(int x, int y) {
    if (x >= 0 && x < WAVE_W && y >= 0 && y < WAVE_H) {
        walls[y][x >> 3] |= 1 << (x & 7);
    }
}

// A drop: a smooth bump (about a Gaussian, radius 2) added around (cx, cy).
static void drop(int cx, int cy) {
    static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int x = cx + dx, y = cy + dy;
            if (x < 0 || x >= WAVE_W || y < 0 || y >= WAVE_H || is_wall(x, y)) continue;
            int32_t w = (int32_t)weights[dx + 2] * weights[dy + 2]; // up to 255 * 255
            u[y][x] += (int16_t)(DROP_AMPLITUDE * w / (255 * 255));
        }
    }
}

static void random_drop(void) {
    int x = 4 + random_below(WAVE_W - 8), y = 4 + random_below(WAVE_H - 8);
    drop(x, y);
    next_drop = step_count + DROP_MIN_STEPS + random_below(DROP_MAX_STEPS - DROP_MIN_STEPS);
}

void lcd_wave_start(uint32_t seed) {
    rng = seed ? seed : 0x9e3779b9;
    memset(wave_a, 0, sizeof(wave_a));
    memset(wave_b, 0, sizeof(wave_b));
    memset(walls, 0, sizeof(walls));
    u = wave_a, u_prev = wave_b;
    step_count = 0;

    // Short walls: horizontal, vertical or diagonal, 4 to 9 cells long.
    static const int8_t directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    for (int i = 0; i < WALL_COUNT; i++) {
        int x = 6 + random_below(WAVE_W - 12), y = 3 + random_below(WAVE_H - 6);
        int d = random_below(4), len = 4 + random_below(6);
        for (int j = 0; j < len; j++) {
            set_wall(x + j * directions[d][0], y + j * directions[d][1]);
        }
    }
    random_drop();
}

void lcd_wave_step(void) {
    for (int y = 0; y < WAVE_H; y++) {
        for (int x = 0; x < WAVE_W; x++) {
            if (is_wall(x, y)) {
                u_prev[y][x] = 0;
                continue;
            }
            // Outside the screen counts as zero: the edges reflect like the walls.
            int32_t sum = (y > 0 ? u[y - 1][x] : 0) + (y < WAVE_H - 1 ? u[y + 1][x] : 0) + (x > 0 ? u[y][x - 1] : 0) + (x < WAVE_W - 1 ? u[y][x + 1] : 0);
            int32_t next = sum / 4 + u[y][x] - u_prev[y][x];
            next -= next / 128; // damping: about 21% a second
            if (next > INT16_MAX) next = INT16_MAX;
            if (next < INT16_MIN) next = INT16_MIN;
            u_prev[y][x] = (int16_t)next; // u_prev isn't needed any more, so it takes the new step
        }
    }
    int16_t (*t)[WAVE_W] = u;
    u = u_prev, u_prev = t;
    step_count++;
    if (step_count == next_drop) {
        random_drop();
    }
}

uint16_t lcd_wave_steps(void) {
    return step_count;
}

void lcd_draw_wave(uint8_t half) {
    // Crests dark, shaded by height with a 4x4 ordered dither; troughs and calm water light.
    static const uint8_t bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    st7565_clear();
    const int x0 = half * LCD_WIDTH;
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            int gx = x0 + x;
            if (is_wall(gx, y)) {
                st7565_write_pixel(x, y, true);
                continue;
            }
            int level = (u[y][gx] - WAVE_CALM) / 32; // calm water stays blank, then 16 shades
            if (level > bayer[y & 3][gx & 3]) {
                st7565_write_pixel(x, y, true);
            }
        }
    }
}
