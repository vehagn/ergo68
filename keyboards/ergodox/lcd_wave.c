/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * The idle animation: a sound wave, simulated across both LCDs (256 x 32 cells, one per pixel, the
 * left half's LCD first).
 *
 * It solves the 2D wave equation u_tt = c^2 (u_xx + u_yy) with an explicit finite-difference scheme.
 * In space, the fourth-order Laplacian, which reaches two cells out:
 *
 *     L u = (16 * (sum of the 4 nearest) - (sum of the 4 two out) - 60 u) / 12
 *
 * In time, leapfrog with a fourth-order correction (from the Taylor series of u in time, where
 * u_tttt = c^4 L^2 u):
 *
 *     u_next = 2 u - u_back + c^2 L u + c^4 L^2 u / 12
 *
 * with time in steps and c = 1/2 cell a step (10 pixels a second at 20 steps a second). Plain
 * leapfrog, without the last term, sends the short wavelengths at the wrong speed, which shows as
 * trains of rings behind each front. Leapfrog only keeps that accuracy if it starts right: a drop is
 * a smooth bump let go at rest, so a step back it was
 *
 *     u_back = u + c^2 L u / 2 + c^4 L^2 u / 24
 *
 * worked out around each drop, with the disks and edges, when it falls.
 *
 * Up to five small disks at random places are rigid obstacles (no flow into them), and so are the
 * top and bottom edges: the two LCDs make a corridor the waves bounce along. The Laplacian mirrors
 * the field off them. The left and right ends absorb, with Mur's first-order absorbing boundary
 * condition, which lets a wave leave as if the field went on. Everywhere, the wave loses a little
 * each step (WAVE_DAMPING).
 *
 * Each half simulates the whole field, so they need no data from each other while it runs: the
 * integer maths is exact and the same on both, so they only have to start from the same random
 * seed (keymap.c sends it from the half on USB to the other).
 */
#include "lcd.h"

#include <stdlib.h>
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

// With c^2 = 1/4: c^2 L u = 12 L u / 48 and c^4 L^2 u / 12 = 12 L (c^2 L u) / 576.
#define LAP_DIV 48
#define LAP2_DIV 576

static int16_t  wave_a[WAVE_H][WAVE_W], wave_b[WAVE_H][WAVE_W];
static int16_t (*wave_u)[WAVE_W] = wave_a, (*wave_back)[WAVE_W] = wave_b; // u now and a step back
static int32_t  ring[5][WAVE_W];               // c^2 L u, for the 5 rows around the one being stepped
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

static bool blocked(int x, int y) {
    return x < 0 || x >= WAVE_W || y < 0 || y >= WAVE_H || is_wall(x, y);
}

static void set_bit(uint8_t (*bits)[WAVE_W / 8], int x, int y) {
    if (x >= 0 && x < WAVE_W && y >= 0 && y < WAVE_H) {
        bits[y][x >> 3] |= 1 << (x & 7);
    }
}

static void set_wall(int x, int y) {
    set_bit(walls, x, y);
    // Cells whose Laplacian reaches this one need the checks in the slow path.
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

// x / d for a constant d, rounded to the nearest, so rounding doesn't push the wave up or down: a
// multiply by 2^32 / d (one instruction on the Cortex-M4, where dividing takes up to 12 cycles).
#define DIV_ROUND(x, d) ((int32_t)(((int64_t)(x) * (int64_t)((0x100000000LL + (d) / 2) / (d)) + 0x80000000LL) >> 32))

// Row y, mirrored back in at the top and bottom (a rigid edge halfway between two rows).
static int mirror(int y) {
    return y < 0 ? -1 - y : y >= WAVE_H ? 2 * WAVE_H - 1 - y : y;
}

static const int8_t dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

// The body of a function returning 12 L f at (x, y), for f read with get(x, y), the slow way: where
// the stencil runs into a disk or an edge, it is mirrored back off it.
#define LAP12_MIRRORED(get)                                                                               \
    int32_t here = get(x, y), near = 0, far = 0;                                                          \
    for (int d = 0; d < 4; d++) {                                                                         \
        int sx = dirs[d][0], sy = dirs[d][1];                                                             \
        if (blocked(x + sx, y + sy)) {                                                                    \
            near += here;                                                                                 \
            far += blocked(x - sx, y - sy) ? here : get(x - sx, y - sy);                                  \
        } else {                                                                                          \
            near += get(x + sx, y + sy);                                                                  \
            far += blocked(x + 2 * sx, y + 2 * sy) ? get(x + sx, y + sy) : get(x + 2 * sx, y + 2 * sy); \
        }                                                                                                 \
    }                                                                                                     \
    return 16 * near - far - 60 * here;

// 12 L f at column x of the middle one of rows (rows y - 2 to y + 2 of f, mirrored at the top and
// bottom), for a cell away from the disks and the ends: the fast path.
#define LAP12_FAST(rows, x) \
    (16 * (rows[1][x] + rows[3][x] + rows[2][x - 1] + rows[2][x + 1]) - (rows[0][x] + rows[4][x] + rows[2][x - 2] + rows[2][x + 2]) - 60 * rows[2][x])

// Whether a cell is close enough to a disk or an end for the stencil to reach it (or in a disk).
#define NEAR_WALL(x, y) ((x) < 2 || (x) >= WAVE_W - 2 || (near_wall[y][(x) >> 3] & (1 << ((x) & 7))))

// The slow path, from the same rows.
#define ROW_GET(gx, gy) rows[2 + (gy) - y][gx]
static int32_t lap12_u(int16_t *const rows[5], int x, int y) {
    LAP12_MIRRORED(ROW_GET)
}

static int32_t lap12_ring(int32_t *const rows[5], int x, int y) {
    LAP12_MIRRORED(ROW_GET)
}

// A drop: a smooth bump (about a Gaussian, radius 2) raised around (cx, cy) and let go at rest.
static int32_t bump(int dx, int dy) {
    static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255
    return DROP_AMPLITUDE / 255 * weights[dx + 2] * weights[dy + 2] / 255;
}

static int     drop_x, drop_y;               // the drop falling
static int32_t drop_h[5][5], drop_lap[9][9]; // its bump h, and c^2 L h, around it
#define DROP_H(gx, gy) (abs((gx) - drop_x) > 2 || abs((gy) - drop_y) > 2 ? 0 : drop_h[(gy) - drop_y + 2][(gx) - drop_x + 2])
#define DROP_LAP(gx, gy) (abs((gx) - drop_x) > 4 || abs((gy) - drop_y) > 4 ? 0 : drop_lap[(gy) - drop_y + 4][(gx) - drop_x + 4])

static int32_t lap12_drop_h(int x, int y) {
    LAP12_MIRRORED(DROP_H)
}

static int32_t lap12_drop_lap(int x, int y) {
    LAP12_MIRRORED(DROP_LAP)
}

static void drop(int cx, int cy) {
    drop_x = cx, drop_y = cy;
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) drop_h[dy + 2][dx + 2] = blocked(cx + dx, cy + dy) ? 0 : bump(dx, dy);
    }
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            drop_lap[dy + 4][dx + 4] = blocked(cx + dx, cy + dy) ? 0 : DIV_ROUND(lap12_drop_h(cx + dx, cy + dy), LAP_DIV);
        }
    }
    // u gets the bump h; a step back it was h + c^2 L h / 2 + c^4 L^2 h / 24, raised by
    // 1 / (WAVE_DAMPING - 1) for the damping the coming step gives it.
    for (int dy = -6; dy <= 6; dy++) {
        for (int dx = -6; dx <= 6; dx++) {
            int x = cx + dx, y = cy + dy;
            if (blocked(x, y)) continue;
            int32_t back    = DROP_H(x, y) + DIV_ROUND(DROP_LAP(x, y), 2) + DIV_ROUND(lap12_drop_lap(x, y), 2 * LAP2_DIV);
            wave_u[y][x]    = clamp16(wave_u[y][x] + DROP_H(x, y));
            wave_back[y][x] = clamp16(wave_back[y][x] + back + DIV_ROUND(back, WAVE_DAMPING - 1));
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
    wave_u = wave_a, wave_back = wave_b;
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

// c^2 L u along row y, into the ring.
static void ring_row(int y) {
    int16_t *rows[5];
    for (int k = 0; k < 5; k++) rows[k] = wave_u[mirror(y + k - 2)];
    int32_t *a = ring[y % 5];
    for (int x = 0; x < WAVE_W; x++) {
        if (!NEAR_WALL(x, y)) {
            a[x] = DIV_ROUND(LAP12_FAST(rows, x), LAP_DIV);
        } else {
            a[x] = is_wall(x, y) ? 0 : DIV_ROUND(lap12_u(rows, x, y), LAP_DIV);
        }
    }
}

void lcd_wave_step(void) {
    // Row by row, with c^2 L u worked out two rows ahead. The new u goes where u a step back was
    // (each cell reads its own before overwriting it). The damping takes from u a step back as well
    // as from the new u, so the wave dies down without changing course.
    ring_row(0);
    ring_row(1);
    for (int y = 0; y < WAVE_H; y++) {
        if (y + 2 < WAVE_H) ring_row(y + 2);
        int32_t *a_rows[5];
        for (int k = 0; k < 5; k++) a_rows[k] = ring[mirror(y + k - 2) % 5];
        int16_t *u = wave_u[y], *back = wave_back[y];
        for (int x = 1; x < WAVE_W - 1; x++) {
            int32_t lap;
            if (!NEAR_WALL(x, y)) {
                lap = LAP12_FAST(a_rows, x);
            } else if (is_wall(x, y)) {
                continue;
            } else {
                lap = lap12_ring(a_rows, x, y);
            }
            int32_t was  = back[x] - DIV_ROUND(back[x], WAVE_DAMPING);
            int32_t next = 2 * u[x] - was + a_rows[2][x] + DIV_ROUND(lap, LAP2_DIV);
            back[x]      = clamp16(next - DIV_ROUND(next, WAVE_DAMPING));
        }
        // The ends absorb, with Mur's boundary from the new values just inside them:
        // u_next(end) = u(inner) + (c - 1) / (c + 1) (u_next(inner) - u(end)), and (c - 1) / (c + 1) = -1/3.
        back[0]          = clamp16(u[1] - DIV_ROUND(back[1] - u[0], 3));
        back[WAVE_W - 1] = clamp16(u[WAVE_W - 2] - DIV_ROUND(back[WAVE_W - 2] - u[WAVE_W - 1], 3));
    }
    int16_t (*t)[WAVE_W] = wave_u;
    wave_u = wave_back, wave_back = t;
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
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = page * 8 + bit;
                if (is_wall(x0 + x, y) || wave_u[y][x0 + x] > WAVE_CREST) {
                    byte |= 1 << bit;
                }
            }
            st7565_write_raw_byte(byte, page * LCD_WIDTH + x);
        }
    }
}
