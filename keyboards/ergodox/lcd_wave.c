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
 * boundary: no flow into them), so the wave reflects off them as off a solid object.
 *
 * The two LCDs make a corridor: the top and bottom edges are rigid too, so waves bounce between them
 * and travel along the strip, and the left and right ends absorb, with Mur's first-order absorbing
 * boundary condition (ABC):
 *
 *     u_next(edge) = u(inner) + (c dt - dx) / (c dt + dx) * (u_next(inner) - u(edge)),  factor -1/3
 *
 * which lets a wave leave as if the field went on. Measured on this grid, a drop stays visible for
 * about 13 seconds this way; with all four edges absorbing it was gone within one or two, as the top
 * and bottom are never more than 16 pixels from a drop. (Mur on all edges let out all but 1% of the
 * energy within 15 seconds, reflecting edges kept 57%, and a few cells of graded damping in front of
 * the edges, a cheap PML, did worse than Mur alone.) Random drops start new waves now and then.
 *
 * The grid carries pixel-scale wavelengths at the wrong speed (numerical dispersion), so ripples
 * gather behind the wavefronts. A viscosity term, nu * laplacian(u - u_prev), takes them out: like
 * air, it damps short wavelengths far more than long ones. Plain damping on top decides how long a
 * drop's waves last: with 1/128 per step, about 9 seconds (1/1024 kept them going past 15).
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
static uint8_t  walls[WAVE_H][WAVE_W / 8];     // one bit per cell, set inside the disks
static uint8_t  near_wall[WAVE_H][WAVE_W / 8]; // set on and next to the disks: cells that need checks
static uint32_t rng;
static uint32_t step_count, next_drop;

#define DISKS_MAX 5   // obstacles: 1 to 5 disks, radius 2 to 4
#define WAVE_DAMPING 128  // lose 1/128 of the wave per step: a drop fades out in about 9 seconds
#define WAVE_VISCOSITY 16 // viscosity nu = 1/16 (0: none; 1/4 is already unstable), see lcd_wave_step
#define WAVE_CREST 450    // crests above this are solid, above 2/3 of it a checkerboard
#define DROP_AMPLITUDE 22000 // strong enough for the ring to stay above WAVE_CREST as it spreads
#define DROP_MIN_STEPS 60   // between drops: 3 to 7 seconds at 20 steps a second
#define DROP_MAX_STEPS 140

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

static void set_bit(uint8_t (*bits)[WAVE_W / 8], int x, int y) {
    if (x >= 0 && x < WAVE_W && y >= 0 && y < WAVE_H) {
        bits[y][x >> 3] |= 1 << (x & 7);
    }
}

static void set_wall(int x, int y) {
    set_bit(walls, x, y);
    // The cell and its four neighbours need the slow path in lcd_wave_step.
    set_bit(near_wall, x, y);
    set_bit(near_wall, x - 1, y);
    set_bit(near_wall, x + 1, y);
    set_bit(near_wall, x, y - 1);
    set_bit(near_wall, x, y + 1);
}

static int16_t clamp16(int32_t v) {
    return v > INT16_MAX ? INT16_MAX : v < INT16_MIN ? INT16_MIN : (int16_t)v;
}

// A drop: a smooth bump (about a Gaussian, radius 2) raised around (cx, cy) and let go at rest, so it
// spreads as a ring. (A bump that starts moving, added to u only, leaves a raised wake inside the
// ring in 2D; after reflecting off the top and bottom, those wakes piled up into dark lobes.)
static void drop(int cx, int cy) {
    static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int x = cx + dx, y = cy + dy;
            if (x < 0 || x >= WAVE_W || y < 0 || y >= WAVE_H || is_wall(x, y)) continue;
            int32_t w = (int32_t)weights[dx + 2] * weights[dy + 2]; // up to 255 * 255
            int32_t h = DROP_AMPLITUDE * w / (255 * 255);
            u[y][x]      = clamp16(u[y][x] + h);
            u_prev[y][x] = clamp16(u_prev[y][x] + h); // the same a step earlier: at rest
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
    memset(near_wall, 0, sizeof(near_wall));
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

// The previous step's heights of the rows just above and at the one being updated, as the update
// overwrites them (u_prev takes the new step in place).
static int16_t old_above[WAVE_W], old_here[WAVE_W];

void lcd_wave_step(void) {
    // The inside of the grid. A disk's cells count as the cell's own value (no flow into the disk);
    // the disks keep away from the edges, so the edge cells needn't be checked. Cells away from the
    // disks, nearly all of them, take a fast path without the checks.
    memcpy(old_above, u_prev[0], sizeof(old_above));
    for (int y = 1; y < WAVE_H - 1; y++) {
        memcpy(old_here, u_prev[y], sizeof(old_here));
        const int16_t *above = u[y - 1], *row = u[y], *below = u[y + 1], *old_below = u_prev[y + 1];
        for (int x = 1; x < WAVE_W - 1; x++) {
            int32_t here = row[x], next;
            if (!(near_wall[y][x >> 3] & (1 << (x & 7)))) {
                next = (above[x] + below[x] + row[x - 1] + row[x + 1]) / 4 + here - old_here[x];
#if WAVE_VISCOSITY > 0
                // Viscosity: nu * laplacian of the velocity (u - u_prev). Like air, it damps short
                // wavelengths much more than long ones, so the grid's pixel-scale ripples die out
                // while the wavefronts carry on.
                int32_t lap_v = (above[x] - old_above[x]) + (below[x] - old_below[x]) + (row[x - 1] - old_here[x - 1]) + (row[x + 1] - old_here[x + 1]) - 4 * (here - old_here[x]);
                next += lap_v / WAVE_VISCOSITY;
#endif
            } else if (is_wall(x, y)) {
                u_prev[y][x] = 0;
                continue;
            } else {
                // Next to a disk: its cells count as this cell's own value.
                bool n = !is_wall(x, y - 1), s = !is_wall(x, y + 1), w = !is_wall(x - 1, y), e = !is_wall(x + 1, y);
                next = ((n ? above[x] : here) + (s ? below[x] : here) + (w ? row[x - 1] : here) + (e ? row[x + 1] : here)) / 4 + here - old_here[x];
#if WAVE_VISCOSITY > 0
                int32_t v     = here - old_here[x];
                int32_t lap_v = (n ? above[x] - old_above[x] : v) + (s ? below[x] - old_below[x] : v) + (w ? row[x - 1] - old_here[x - 1] : v) + (e ? row[x + 1] - old_here[x + 1] : v) - 4 * v;
                next += lap_v / WAVE_VISCOSITY;
#endif
            }
            next -= next / WAVE_DAMPING; // plain damping: how quickly the waves die out
            u_prev[y][x] = clamp16(next); // u_prev isn't needed any more, so it takes the new step
        }
        memcpy(old_above, old_here, sizeof(old_above));
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
    step_count++;
    if (step_count == next_drop) {
        random_drop();
    }
}

uint32_t lcd_wave_steps(void) {
    return step_count;
}

void lcd_draw_wave(uint8_t half) {
    // Crests dark, troughs and calm water light: crests above WAVE_CREST solid, a band below them as
    // a checkerboard, for soft edges without the scattered dots a finer dither made of the faint
    // ripples. Written a byte (8 rows of a column) at a time, as the LCD stores them.
    const int x0 = half * LCD_WIDTH;
    for (int page = 0; page < LCD_HEIGHT / 8; page++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            int     gx   = x0 + x;
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int     y = page * 8 + bit;
                int16_t h = u[y][gx];
                if (is_wall(gx, y) || h > WAVE_CREST || (h > WAVE_CREST * 2 / 3 && ((gx + y) & 1) == 0)) {
                    byte |= 1 << bit;
                }
            }
            st7565_write_raw_byte(byte, page * LCD_WIDTH + x);
        }
    }
}
