/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * The idle animation: a sound wave, simulated across both LCDs (256 x 32 cells, one per pixel, the
 * left half's LCD first).
 *
 * It solves the 2D wave equation u_tt = c^2 (u_xx + u_yy) with an explicit finite-difference scheme:
 * leapfrog in time, and in space the fourth-order Laplacian, which reaches two cells out,
 *
 *     u_xx ~ (-u[x-2] + 16 u[x-1] - 30 u + 16 u[x+1] - u[x+2]) / 12   (and the same for y)
 *
 * so at a Courant number of 1/2 (c dt / dx = 1/2; stable up to sqrt(3/8) ~ 0.61 in 2D):
 *
 *     u_next = 2 u - u_prev + (16 * (sum of the 4 nearest) - (sum of the 4 two out) - 60 u) / 48
 *
 * The usual second-order stencil, u_next = u + (sum of the 4 nearest) / 4 - u_prev, sends short
 * wavelengths too slowly (numerical dispersion), which shows as a train of rings behind each front;
 * the wider stencil keeps them much closer together. Where it would reach into a disk or past an
 * edge, a cell uses the second-order stencil instead. All in 16-bit integers, minus a little damping
 * each step (WAVE_DAMPING). Drops are smooth bumps let go at rest, so each spreads as a ring.
 *
 * Up to five small disks at random places are rigid obstacles (no flow into them), and so are the
 * top and bottom edges: the two LCDs make a corridor the waves bounce along. The left and right ends
 * absorb, with Mur's first-order absorbing boundary condition:
 *
 *     u_next(end) = u(inner) - (u_next(inner) - u(end)) / 3
 *
 * which lets a wave leave as if the field went on.
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

#define WAVE_DAMPING 128     // lose 1/WAVE_DAMPING of the wave per step: higher lasts longer
#define WAVE_CREST 450       // crests higher than this are drawn
#define DISKS_MAX 5          // 1 to 5 disks, radius 2 to 4
#define DROP_AMPLITUDE 22000 // a drop's height
#define DROP_MIN_STEPS 60    // between drops: 3 to 7 seconds at 20 steps a second
#define DROP_MAX_STEPS 140

static int16_t  wave_a[WAVE_H][WAVE_W], wave_b[WAVE_H][WAVE_W];
static int16_t (*u)[WAVE_W] = wave_a, (*u_prev)[WAVE_W] = wave_b;
static uint8_t  walls[WAVE_H][WAVE_W / 8];     // one bit per cell, set inside the disks
static uint8_t  near_wall[WAVE_H][WAVE_W / 8]; // set within two cells of a disk: cells that need checks
static uint32_t rng, step_count, next_drop;

static uint32_t random_below(uint32_t n) {
    rng ^= rng << 13; // xorshift32
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng % n;
}

static bool is_wall(int x, int y) {
    return walls[y][x >> 3] & (1 << (x & 7));
}

static void set_bit(uint8_t (*bits)[WAVE_W / 8], int x, int y) {
    if (x >= 0 && x < WAVE_W && y >= 0 && y < WAVE_H) {
        bits[y][x >> 3] |= 1 << (x & 7);
    }
}

static void set_wall(int x, int y) {
    set_bit(walls, x, y);
    // Cells whose fourth-order stencil reaches this one need the checks in lcd_wave_step.
    set_bit(near_wall, x, y);
    for (int d = 1; d <= 2; d++) {
        set_bit(near_wall, x - d, y);
        set_bit(near_wall, x + d, y);
        set_bit(near_wall, x, y - d);
        set_bit(near_wall, x, y + d);
    }
}

static int16_t clamp16(int32_t v) {
    return v > INT16_MAX ? INT16_MAX : v < INT16_MIN ? INT16_MIN : (int16_t)v;
}

// A drop: a smooth bump (about a Gaussian, radius 2) raised around (cx, cy) and let go at rest.
static void drop(int cx, int cy) {
    static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int x = cx + dx, y = cy + dy;
            if (x < 0 || x >= WAVE_W || y < 0 || y >= WAVE_H || is_wall(x, y)) continue;
            int32_t h    = DROP_AMPLITUDE * (int32_t)weights[dx + 2] * weights[dy + 2] / (255 * 255);
            u[y][x]      = clamp16(u[y][x] + h);
            u_prev[y][x] = clamp16(u_prev[y][x] + h); // the same a step earlier: at rest
        }
    }
}

static void random_drop(void) {
    drop(4 + random_below(WAVE_W - 8), 4 + random_below(WAVE_H - 8));
    next_drop = step_count + DROP_MIN_STEPS + random_below(DROP_MAX_STEPS - DROP_MIN_STEPS);
}

void lcd_wave_start(uint32_t seed) {
    rng = seed ? seed : 0x9e3779b9;
    memset(wave_a, 0, sizeof(wave_a));
    memset(wave_b, 0, sizeof(wave_b));
    memset(walls, 0, sizeof(walls));
    memset(near_wall, 0, sizeof(near_wall));
    u = wave_a, u_prev = wave_b;
    step_count = 0;

    // 1 to DISKS_MAX disks of radius 2 to 4, away from the edges and from each other.
    int16_t placed[DISKS_MAX][3];
    int     count = 1 + random_below(DISKS_MAX), n = 0;
    for (int tries = 0; n < count && tries < 50; tries++) {
        int  r = 2 + random_below(3), cx = 8 + random_below(WAVE_W - 16), cy = 5 + random_below(WAVE_H - 10);
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

// The second-order update, for cells next to a disk or an edge: a disk's cells count as the cell's
// own value (no flow into the disk).
static int32_t step_near(int x, int y) {
    int32_t here = u[y][x];
    int32_t sum  = (is_wall(x, y - 1) ? here : u[y - 1][x]) + (is_wall(x, y + 1) ? here : u[y + 1][x]) + (is_wall(x - 1, y) ? here : u[y][x - 1]) + (is_wall(x + 1, y) ? here : u[y][x + 1]);
    return sum / 4 + here - u_prev[y][x];
}

void lcd_wave_step(void) {
    // The inside of the grid; u_prev takes the new step in place (each cell reads its own old value
    // before overwriting it). Nearly all cells are at least two from a disk and an edge and take the
    // fourth-order stencil; the rest the second-order one.
    for (int y = 1; y < WAVE_H - 1; y++) {
        bool inner_row = y >= 2 && y < WAVE_H - 2;
        for (int x = 1; x < WAVE_W - 1; x++) {
            int32_t next;
            if (inner_row && x >= 2 && x < WAVE_W - 2 && !(near_wall[y][x >> 3] & (1 << (x & 7)))) {
                int32_t here = u[y][x];
                int32_t near = u[y - 1][x] + u[y + 1][x] + u[y][x - 1] + u[y][x + 1];
                int32_t far  = u[y - 2][x] + u[y + 2][x] + u[y][x - 2] + u[y][x + 2];
                next         = 2 * here - u_prev[y][x] + (16 * near - far - 60 * here) / 48;
            } else if (is_wall(x, y)) {
                u_prev[y][x] = 0;
                continue;
            } else {
                next = step_near(x, y);
            }
            u_prev[y][x] = clamp16(next - next / WAVE_DAMPING);
        }
    }
    // The ends absorb (Mur's boundary, from the new values just inside them)...
    for (int y = 1; y < WAVE_H - 1; y++) {
        u_prev[y][0]          = clamp16(u[y][1] - (u_prev[y][1] - u[y][0]) / 3);
        u_prev[y][WAVE_W - 1] = clamp16(u[y][WAVE_W - 2] - (u_prev[y][WAVE_W - 2] - u[y][WAVE_W - 1]) / 3);
    }
    // ...and the top and bottom are rigid: no flow through them.
    for (int x = 0; x < WAVE_W; x++) {
        u_prev[0][x]          = u_prev[1][x];
        u_prev[WAVE_H - 1][x] = u_prev[WAVE_H - 2][x];
    }
    int16_t (*t)[WAVE_W] = u;
    u = u_prev, u_prev = t;
    if (++step_count == next_drop) {
        random_drop();
    }
}

uint32_t lcd_wave_steps(void) {
    return step_count;
}

void lcd_draw_wave(uint8_t half) {
    // Crests dark, everything else light, a byte (8 rows of a column) at a time as the LCD stores them.
    const int x0 = half * LCD_WIDTH;
    for (int page = 0; page < LCD_HEIGHT / 8; page++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            int     gx   = x0 + x;
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = page * 8 + bit;
                if (is_wall(gx, y) || u[y][gx] > WAVE_CREST) {
                    byte |= 1 << bit;
                }
            }
            st7565_write_raw_byte(byte, page * LCD_WIDTH + x);
        }
    }
}
