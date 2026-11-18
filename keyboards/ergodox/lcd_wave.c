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
 * in 16-bit integers. Up to five small disks at random places are rigid obstacles (a Neumann
 * boundary: no flow into them), so the wave reflects off them as off a solid object. The screen
 * edges absorb instead, with Mur's first-order absorbing boundary condition (ABC):
 *
 *     u_next(edge) = u(inner) + (c dt - dx) / (c dt + dx) * (u_next(inner) - u(edge)),  factor -1/3
 *
 * which lets a wave leave as if the field went on. It's exact head-on and less so at grazing angles,
 * but measured on this grid it lets out all but 1% of a pulse's energy within 15 seconds (reflecting
 * edges kept 57%; a few cells of graded damping in front of the edges, a cheap PML, did worse than
 * Mur alone). A trace of damping clears the integer rounding that's left. Random drops start new
 * waves now and then.
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
static uint8_t  walls[WAVE_H][WAVE_W / 8]; // one bit per cell, set inside the disks
static uint32_t rng;
static uint16_t step_count, next_drop;

#define DISKS_MAX 5   // obstacles: 1 to 5 disks, radius 2 to 4
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

    // 1 to DISKS_MAX disks of radius 2 to 4, away from the edges and from each other.
    int16_t placed[DISKS_MAX][3];
    int     count = 1 + random_below(DISKS_MAX), n = 0;
    for (int tries = 0; n < count && tries < 50; tries++) {
        int r = 2 + random_below(3), cx = 8 + random_below(WAVE_W - 16), cy = 5 + random_below(WAVE_H - 10);
        bool clear = true;
        for (int i = 0; i < n; i++) {
            int dx = cx - placed[i][0], dy = cy - placed[i][1], gap = r + placed[i][2] + 6;
            if (dx * dx + dy * dy < gap * gap) clear = false;
        }
        if (!clear) continue;
        placed[n][0] = cx, placed[n][1] = cy, placed[n][2] = r, n++;
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx * dx + dy * dy <= r * r + r) set_wall(cx + dx, cy + dy); // + r rounds the edge out
            }
        }
    }
    random_drop();
}

static int16_t clamp16(int32_t v) {
    return v > INT16_MAX ? INT16_MAX : v < INT16_MIN ? INT16_MIN : (int16_t)v;
}

void lcd_wave_step(void) {
    // The inside of the grid. A disk's cells count as the cell's own height (no flow into the disk);
    // the disks keep away from the edges, so the edge cells needn't be checked.
    for (int y = 1; y < WAVE_H - 1; y++) {
        for (int x = 1; x < WAVE_W - 1; x++) {
            if (is_wall(x, y)) {
                u_prev[y][x] = 0;
                continue;
            }
            int32_t here = u[y][x];
            int32_t sum  = (is_wall(x, y - 1) ? here : u[y - 1][x]) + (is_wall(x, y + 1) ? here : u[y + 1][x]) + (is_wall(x - 1, y) ? here : u[y][x - 1]) + (is_wall(x + 1, y) ? here : u[y][x + 1]);
            int32_t next = sum / 4 + here - u_prev[y][x];
            next -= next / 1024; // a trace of damping, for what integer rounding leaves behind
            u_prev[y][x] = clamp16(next); // u_prev isn't needed any more, so it takes the new step
        }
    }
    // The edges: Mur's absorbing boundary, from the new values just inside them.
    for (int y = 1; y < WAVE_H - 1; y++) {
        u_prev[y][0]          = clamp16(u[y][1] - (u_prev[y][1] - u[y][0]) / 3);
        u_prev[y][WAVE_W - 1] = clamp16(u[y][WAVE_W - 2] - (u_prev[y][WAVE_W - 2] - u[y][WAVE_W - 1]) / 3);
    }
    for (int x = 0; x < WAVE_W; x++) {
        u_prev[0][x]          = clamp16(u[1][x] - (u_prev[1][x] - u[0][x]) / 3);
        u_prev[WAVE_H - 1][x] = clamp16(u[WAVE_H - 2][x] - (u_prev[WAVE_H - 2][x] - u[WAVE_H - 1][x]) / 3);
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
