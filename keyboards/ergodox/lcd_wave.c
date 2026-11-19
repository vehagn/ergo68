/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * The idle animation: a sound wave, simulated across both LCDs (the left half's LCD first).
 *
 * It solves the 2D wave equation u_tt = c^2 (u_xx + u_yy) on a grid of cells of 2 x 2 pixels, 128 x
 * 16 for the screens. Drawing interpolates between the cells (bilinear) and follows the ridge of
 * each crest pixel by pixel, so the waves still show as thin lines at the full 256 x 32 pixels.
 * Written as a first-order system in u and its rate of change v,
 *
 *     u_t = v,    v_t = c^2 L u
 *
 * where L is the fourth-order Laplacian, which reaches two cells out:
 *
 *     L u = (16 * (sum of the 4 nearest) - (sum of the 4 two out) - 60 u) / 12
 *
 * It steps in time with a fourth-order Runge-Kutta method (RK4): the five-stage one of Carpenter and
 * Kennedy (1994), which needs only one extra value per variable and cell (the classic four-stage one
 * needs three, too many for the RAM). With U = (u, v), its rates of change F(U) and time in steps,
 *
 *     dU = A_i dU + F(U),    U = U + B_i dU    for i = 1 to 5
 *
 * and c = 1/4 cell a step (10 pixels a second at 20 steps a second). Its error from the time step
 * stays well under a percent, far below what the drawing can show; with leapfrog, the usual step
 * for the wave equation, it was tens of percent. Drops are smooth bumps let go at rest, so each
 * spreads as a ring.
 *
 * Up to five small disks at random places are rigid obstacles (no flow into them), and so are the
 * top and bottom edges: the two LCDs make a corridor the waves bounce along. The Laplacian mirrors
 * the field off them. The left and right ends absorb: the grid goes on for WAVE_PAD hidden cells
 * past each end of the screens, and at the far ends u follows the one-way wave equation (u_t =
 * c u_x at the left end, taken halfway between the last two cells, as in Mur's boundary condition),
 * which lets a wave leave as if the field went on. What a slanting wave still sends back from there
 * has the hidden cells to cross twice before it shows, fading on the way. Everywhere, the wave loses
 * a little each step (WAVE_DAMPING).
 *
 * Each half simulates the whole field, so they need no data from each other while it runs: the
 * integer maths is exact and the same on both, so they only have to start from the same random
 * seed (keymap.c sends it from the half on USB to the other).
 */
#include "lcd.h"

#include <string.h>

#include "st7565.h"

#define WAVE_PAD 16                                // hidden cells past each end of the screens
#define WAVE_W (2 * LCD_WIDTH / 2 + 2 * WAVE_PAD) // a cell is 2 x 2 pixels
#define WAVE_H (LCD_HEIGHT / 2)

#define WAVE_DAMPING 128      // lose 1/WAVE_DAMPING of the wave per step: higher lasts longer
#define WAVE_CREST 7000       // crests higher than this are drawn, along their ridges
#define WAVE_SOLID 60000      // and higher than this, whole
#define WAVE_MAX (1L << 20)   // u and v stay within +-WAVE_MAX, so no sum can overflow 32 bits
#define DISKS_MAX 5           // 1 to 5 disks, radius 1 or 2 cells
#define DROP_AMPLITUDE 260000 // a drop's height
#define DROP_MIN_STEPS 60     // between drops: 3 to 7 seconds at 20 steps a second
#define DROP_MAX_STEPS 140

// The method's coefficients times 2^30: A_i, and B_i / 2 and B_i / 192 for the units of du and dv.
static const int32_t rk_a[5]  = {0, -448706480, -1280063135, -1822982432, -1625842093};
static const int32_t rk_bu[5] = {80347576, 203586987, 441820617, 375514604, 82171984};
static const int32_t rk_bv[5] = {836954, 2120698, 4602298, 3911610, 855958};

static int32_t  wave_u[WAVE_H][WAVE_W], wave_v[WAVE_H][WAVE_W];   // the wave: u and v = u_t
static int32_t  wave_du[WAVE_H][WAVE_W], wave_dv[WAVE_H][WAVE_W]; // dU, times 2 and 192 (see stage)
static uint8_t  walls[WAVE_H][WAVE_W / 8];                        // one bit per cell, set inside the disks
static uint8_t  near_wall[WAVE_H][WAVE_W / 8];                    // set within two cells of a disk
static int16_t  disks[DISKS_MAX][3]; // as drawn: centre x and y in half pixels, (2 * radius in pixels)^2
static uint8_t  disk_count;
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
    // Cells whose Laplacian reaches this one need the checks in lap12.
    set_bit(near_wall, x, y);
    for (int d = 1; d <= 2; d++) {
        set_bit(near_wall, x - d, y);
        set_bit(near_wall, x + d, y);
        set_bit(near_wall, x, y - d);
        set_bit(near_wall, x, y + d);
    }
}

static int32_t clamp(int32_t v) {
    return v > WAVE_MAX ? WAVE_MAX : v < -WAVE_MAX ? -WAVE_MAX : v;
}

// k * x, for k times 2^30, rounded.
static int32_t q30(int32_t k, int32_t x) {
    return (int32_t)(((int64_t)k * x + (1 << 29)) >> 30);
}

// Row y, mirrored back in at the top and bottom (a rigid edge halfway between two rows).
static int mirror(int y) {
    return y < 0 ? -1 - y : y >= WAVE_H ? 2 * WAVE_H - 1 - y : y;
}

// A drop: a smooth bump (about a Gaussian, radius 2 cells) raised around (cx, cy) and let go at rest.
static void drop(int cx, int cy) {
    static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int x = cx + dx, y = cy + dy;
            if (blocked(x, y)) continue;
            wave_u[y][x] = clamp(wave_u[y][x] + DROP_AMPLITUDE / 255 * weights[dx + 2] * weights[dy + 2] / 255);
        }
    }
}

static void random_drop(void) {
    drop(WAVE_PAD + 2 + random_below(LCD_WIDTH - 4), 2 + random_below(WAVE_H - 4));
    next_drop = step_count + DROP_MIN_STEPS + random_below(DROP_MAX_STEPS - DROP_MIN_STEPS);
}

void lcd_wave_start(uint32_t seed) {
    rng = seed ? seed : 0x9e3779b9;
    memset(wave_u, 0, sizeof(wave_u));
    memset(wave_v, 0, sizeof(wave_v));
    memset(wave_du, 0, sizeof(wave_du));
    memset(wave_dv, 0, sizeof(wave_dv));
    memset(walls, 0, sizeof(walls));
    memset(near_wall, 0, sizeof(near_wall));
    step_count = 0;

    // 1 to DISKS_MAX disks of radius 1 or 2 cells, away from the edges and from each other.
    int16_t placed[DISKS_MAX][3]; // centre and radius, in cells
    int     count = 1 + random_below(DISKS_MAX);
    disk_count    = 0;
    for (int tries = 0; disk_count < count && tries < 50; tries++) {
        int  r = 1 + random_below(2), cx = WAVE_PAD + 4 + random_below(LCD_WIDTH - 8), cy = 3 + random_below(WAVE_H - 6);
        bool clear = true;
        for (int i = 0; i < disk_count; i++) {
            int dx = cx - placed[i][0], dy = cy - placed[i][1], gap = r + placed[i][2] + 3;
            if (dx * dx + dy * dy < gap * gap) clear = false;
        }
        if (!clear) continue;
        placed[disk_count][0] = cx, placed[disk_count][1] = cy, placed[disk_count][2] = r;
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx * dx + dy * dy <= r * r + r) set_wall(cx + dx, cy + dy); // + r rounds the edge out
            }
        }
        // Drawn as a circle of about the area of its cells, around the middle of the centre cell.
        disks[disk_count][0] = 4 * (cx - WAVE_PAD) + 2;
        disks[disk_count][1] = 4 * cy + 2;
        disks[disk_count][2] = 16 * (r * r + r) + 14;
        disk_count++;
    }
    random_drop();
}

// 12 L u at column x of row y, given rows y - 2 to y + 2 of u (mirrored at the top and bottom).
static int32_t lap12(int32_t *const rows[5], int x, int y) {
    const int32_t *row = rows[2];
    int32_t        here = row[x];
    if (x >= 2 && x < WAVE_W - 2 && !(near_wall[y][x >> 3] & (1 << (x & 7)))) {
        int32_t near = rows[1][x] + rows[3][x] + row[x - 1] + row[x + 1];
        int32_t far  = rows[0][x] + rows[4][x] + row[x - 2] + row[x + 2];
        return 16 * near - far - 60 * here;
    }
    // Near a disk or an end: where the stencil runs into one, mirror it back off it.
    static const int8_t dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int32_t             near = 0, far = 0;
    for (int d = 0; d < 4; d++) {
        int sx = dirs[d][0], sy = dirs[d][1];
        if (blocked(x + sx, y + sy)) {
            near += here;
            far += blocked(x - sx, y - sy) ? here : rows[2 - sy][x - sx];
        } else {
            near += rows[2 + sy][x + sx];
            far += blocked(x + 2 * sx, y + 2 * sy) ? rows[2 + sy][x + sx] : rows[2 + 2 * sy][x + 2 * sx];
        }
    }
    return 16 * near - far - 60 * here;
}

// One stage: dU = a dU + F(U), then U = U + b dU. A single pass, with U changed two rows behind the
// rates: by the time a row changes, no rate still to come needs its old values. du and dv are kept
// times 2 and times 192 (12 / c^2), so that they are whole numbers: nothing is rounded but the
// products with the coefficients, to the nearest, so rounding doesn't push the wave up or down.
static void stage(int32_t a, int32_t bu, int32_t bv) {
    for (int t = 0; t < WAVE_H + 2; t++) {
        if (t < WAVE_H) {
            int      y = t;
            int32_t *rows[5];
            for (int k = 0; k < 5; k++) rows[k] = wave_u[mirror(y + k - 2)];
            const int32_t *u = rows[2], *v = wave_v[y];
            int32_t       *du = wave_du[y], *dv = wave_dv[y];
            for (int x = 0; x < WAVE_W; x++) {
                if (is_wall(x, y)) continue;
                int32_t ut2 = 2 * v[x], vt192 = 0;
                if (x == 0) {
                    ut2 = u[1] - u[0] - 2 * v[1]; // (u_t[0] + u_t[1]) / 2 = c (u[1] - u[0])
                } else if (x == WAVE_W - 1) {
                    ut2 = u[x - 1] - u[x] - 2 * v[x - 1];
                } else {
                    vt192 = lap12(rows, x, y);
                }
                du[x] = q30(a, du[x]) + ut2;
                dv[x] = q30(a, dv[x]) + vt192;
            }
        }
        if (t >= 2) {
            int y = t - 2;
            for (int x = 0; x < WAVE_W; x++) {
                wave_u[y][x] = clamp(wave_u[y][x] + q30(bu, wave_du[y][x]));
                wave_v[y][x] = clamp(wave_v[y][x] + q30(bv, wave_dv[y][x]));
            }
        }
    }
}

void lcd_wave_step(void) {
    for (int i = 0; i < 5; i++) {
        stage(rk_a[i], rk_bu[i], rk_bv[i]);
    }
    for (int y = 0; y < WAVE_H; y++) {
        for (int x = 0; x < WAVE_W; x++) {
            wave_u[y][x] -= wave_u[y][x] / WAVE_DAMPING;
            wave_v[y][x] -= wave_v[y][x] / WAVE_DAMPING;
        }
    }
    if (++step_count == next_drop) {
        random_drop();
    }
}

uint32_t lcd_wave_steps(void) {
    return step_count;
}

// 16 u at pixel (gx, gy) of both screens, interpolated between the 4 cells around the pixel's middle,
// which is a quarter of a cell from its own cell's middle each way: weights 3 and 1, in 16ths.
static int32_t pixel16(int gx, int gy) {
    int            cx = WAVE_PAD + ((gx - 1) >> 1), wx = gx & 1 ? 3 : 1; // wx for cell cx, 4 - wx for cx + 1
    int            cy = (gy - 1) >> 1, wy = gy & 1 ? 3 : 1;
    const int32_t *r0 = wave_u[mirror(cy)], *r1 = wave_u[mirror(cy + 1)];
    return wy * (wx * r0[cx] + (4 - wx) * r0[cx + 1]) + (4 - wy) * (wx * r1[cx] + (4 - wx) * r1[cx + 1]);
}

// A column of pixel16, with a row more above and below.
static void pixel_column(int32_t *column, int gx) {
    for (int gy = -1; gy <= LCD_HEIGHT; gy++) column[gy + 1] = pixel16(gx, gy);
}

void lcd_draw_wave(uint8_t half) {
    // Dark along the ridge of each crest higher than WAVE_CREST (a pixel at least as high as its
    // neighbours either side, across the crest), and where a crest is higher than WAVE_SOLID, so a
    // fresh drop shows whole; everything else light. Column by column, keeping the columns either
    // side, and a byte (8 rows of a column) at a time as the LCD stores them.
    static int32_t columns[3][LCD_HEIGHT + 2];
    int32_t       *left = columns[0], *mid = columns[1], *right = columns[2];
    const int      gx0 = half * LCD_WIDTH;
    pixel_column(left, gx0 - 1);
    pixel_column(mid, gx0);
    for (int x = 0; x < LCD_WIDTH; x++) {
        int gx = gx0 + x;
        pixel_column(right, gx + 1);
        for (int page = 0; page < LCD_HEIGHT / 8; page++) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int     gy = page * 8 + bit, i = gy + 1;
                int32_t u = mid[i], l = left[i], r = right[i], a = mid[i - 1], b = mid[i + 1];
                // Across the crest: the way the surface curves more (2nd differences), up or down. On a
                // ring's outer slope that is outwards, where it curves up, so the slope isn't a ridge.
                int32_t dx2 = l + r - 2 * u, dy2 = a + b - 2 * u;
                bool    ridge = (dx2 < 0 ? -dx2 : dx2) >= (dy2 < 0 ? -dy2 : dy2) ? u >= l && u >= r : u >= a && u >= b;
                bool on    = u > 16 * WAVE_SOLID || (u > 16 * WAVE_CREST && ridge);
                for (int d = 0; d < disk_count && !on; d++) {
                    int dx = 2 * gx + 1 - disks[d][0], dy = 2 * gy + 1 - disks[d][1];
                    on     = dx * dx + dy * dy <= disks[d][2];
                }
                if (on) byte |= 1 << bit;
            }
            st7565_write_raw_byte(byte, page * LCD_WIDTH + x);
        }
        int32_t *t = left;
        left = mid, mid = right, right = t;
    }
}
