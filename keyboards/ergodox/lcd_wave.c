/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * The idle animation: a sound wave, simulated across both LCDs (256 x 32 cells, one per pixel, the
 * left half's LCD first), in one of four scenes picked at random:
 *
 *   stones       drops fall among a few irregular stones
 *   ellipses     elliptical mirrors: a drop at one focus meets itself again at the other
 *   double slit  plane waves pass two slits in a wall, and the waves from the slits interfere
 *   lens         plane waves pass a lens, where waves go at 2/3 the speed, and focus behind it
 *
 * It solves the 2D wave equation u_tt = c^2 (u_xx + u_yy) with an explicit finite-difference scheme.
 * In space, the fourth-order Laplacian, which reaches two cells out:
 *
 *     L u = (16 * (sum of the 4 nearest) - (sum of the 4 two out) - 60 u) / 12
 *
 * In time, leapfrog with a fourth-order correction (from the Taylor series of u in time, where
 * u_tttt = c^2 L (c^2 L u), which holds for a wave speed c that changes from cell to cell too):
 *
 *     u_next = 2 u - u_back + c^2 L u + c^2 L (c^2 L u) / 12
 *
 * with time in steps and c = 1/2 cell a step (10 pixels a second at 20 steps a second). Plain
 * leapfrog, without the last term, sends the short wavelengths at the wrong speed, which shows as
 * trains of rings behind each front. Leapfrog only keeps that accuracy if it starts right: a drop is
 * a smooth bump let go at rest, so a step back it was
 *
 *     u_back = u + c^2 L u / 2 + c^2 L (c^2 L u) / 24
 *
 * worked out around each drop, with the walls and edges, when it falls.
 *
 * Walls (stones, mirrors, the slit wall) are rigid (no flow into them), and so are the top and
 * bottom edges: the two LCDs make a corridor the waves bounce along. The Laplacian mirrors the field
 * off them. The left and right ends absorb, with Mur's first-order absorbing boundary condition,
 * which lets a wave leave as if the field went on. Everywhere, the wave loses a little each step
 * (WAVE_DAMPING).
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

#define WAVE_DAMPING 128      // lose 1/WAVE_DAMPING of the wave per step: higher lasts longer
#define WAVE_CREST 450        // crests higher than this are drawn
#define DROP_AMPLITUDE 22000  // a drop's height
#define PLANE_AMPLITUDE 3000  // a plane wave's
#define PAUSE_MIN_STEPS 60    // between drops, or trains of plane waves: 3 to 7 seconds
#define PAUSE_MAX_STEPS 140
#define TRAIN_WAVES 3         // plane waves in a train
#define TRAIN_STEPS 12        // between them: a wavelength of 6 pixels

#define STONES_MAX 5   // 2 to 5 stones
#define ELLIPSES_MAX 3 // 1 to 3 mirrors

// With c^2 = 1/4: c^2 L u = 12 L u / 48 and c^2 L (c^2 L u) / 12 = 12 L (c^2 L u) / 576. In the lens,
// at 2/3 the speed, c^2 = 1/9: 108 and 1296.
#define LAP_DIV 48
#define LAP2_DIV 576
#define LENS_LAP_DIV 108
#define LENS_LAP2_DIV 1296

enum { SCENE_STONES, SCENE_ELLIPSES, SCENE_SLITS, SCENE_LENS, SCENE_COUNT };
static const char *const scene_names[SCENE_COUNT] = {"stones", "ellipses", "double slit", "lens"};

static int16_t  wave_a[WAVE_H][WAVE_W], wave_b[WAVE_H][WAVE_W];
static int16_t (*wave_u)[WAVE_W] = wave_a, (*wave_back)[WAVE_W] = wave_b; // u now and a step back
static int32_t  ring[5][WAVE_W];               // c^2 L u, for the 5 rows around the one being stepped
static uint8_t  walls[WAVE_H][WAVE_W / 8];     // one bit per cell, set in the walls
static uint8_t  lens[WAVE_H][WAVE_W / 8];      // one bit per cell, set in the lens
static uint8_t  near_wall[WAVE_H][WAVE_W / 8]; // set within two cells of a wall, and in the lens: cells that need checks
static uint8_t  scene, spot_count, train_left;
static int16_t  spots[2 * ELLIPSES_MAX][2]; // where drops fall: the foci
static int16_t  source_x;                   // where plane waves start
static uint32_t rng, step_count, next_source;

static uint32_t random_below(uint32_t n) {
    rng ^= rng << 13; // xorshift32
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng % n;
}

static bool bit(uint8_t (*bits)[WAVE_W / 8], int x, int y) {
    return bits[y][x >> 3] & (1 << (x & 7));
}

static bool is_wall(int x, int y) {
    return bit(walls, x, y);
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
// the stencil runs into a wall or an edge, it is mirrored back off it.
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
// bottom), for a cell away from the walls, the lens and the ends: the fast path.
#define LAP12_FAST(rows, x) \
    (16 * (rows[1][x] + rows[3][x] + rows[2][x - 1] + rows[2][x + 1]) - (rows[0][x] + rows[4][x] + rows[2][x - 2] + rows[2][x + 2]) - 60 * rows[2][x])

// Whether a cell needs the slow path: close enough to a wall or an end for the stencil to reach it,
// in a wall, or in the lens.
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
static const uint8_t weights[5] = {35, 155, 255, 155, 35}; // exp(-d^2 / 2) * 255

static int32_t bump(int dx, int dy) {
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

// u gets h, and u a step back gets back, raised by 1 / (WAVE_DAMPING - 1) for the damping the coming
// step gives it.
static void add_wave(int x, int y, int32_t h, int32_t back) {
    wave_u[y][x]    = clamp16(wave_u[y][x] + h);
    wave_back[y][x] = clamp16(wave_back[y][x] + back + DIV_ROUND(back, WAVE_DAMPING - 1));
}

// A drop at (cx, cy), which is never in the lens.
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
    for (int dy = -6; dy <= 6; dy++) {
        for (int dx = -6; dx <= 6; dx++) {
            int x = cx + dx, y = cy + dy;
            if (blocked(x, y)) continue;
            // At rest: a step back it was h + c^2 L h / 2 + c^2 L (c^2 L h) / 24.
            add_wave(x, y, DROP_H(x, y), DROP_H(x, y) + DIV_ROUND(DROP_LAP(x, y), 2) + DIV_ROUND(lap12_drop_lap(x, y), 2 * LAP2_DIV));
        }
    }
}

// A plane wave going right: the same bump along x, the whole height of the corridor, at column cx
// (clear of walls and the lens). It doesn't change along y, so it moves as in one dimension, where a
// step back it was the same shape half a cell (c times a step) further right.
static void plane_wave(int cx) {
    static const uint8_t behind[6] = {11, 83, 225, 225, 83, 11}; // exp(-(d + 1/2)^2 / 2) * 255, d = -3 to 2
    for (int d = -3; d <= 2; d++) {
        int32_t h = d >= -2 ? PLANE_AMPLITUDE / 255 * weights[d + 2] : 0, back = PLANE_AMPLITUDE / 255 * behind[d + 3];
        for (int y = 0; y < WAVE_H; y++) add_wave(cx + d, y, h, back);
    }
}

// The next drop or plane wave, and when the one after it comes: drops anywhere among the stones, or
// at a focus of the mirrors; plane waves in trains for the double slit and the lens.
static void source(void) {
    uint32_t pause = PAUSE_MIN_STEPS + random_below(PAUSE_MAX_STEPS - PAUSE_MIN_STEPS);
    if (scene == SCENE_STONES) {
        drop(4 + random_below(WAVE_W - 8), 4 + random_below(WAVE_H - 8));
    } else if (scene == SCENE_ELLIPSES) {
        int s = random_below(spot_count);
        drop(spots[s][0], spots[s][1]);
    } else {
        plane_wave(source_x);
        if (train_left == 0) train_left = TRAIN_WAVES;
        if (--train_left > 0) pause = TRAIN_STEPS;
    }
    next_source = step_count + pause;
}

static void add_disk(int cx, int cy, int r) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r + r) set_wall(cx + dx, cy + dy); // + r rounds the edge out
        }
    }
}

// 2 to STONES_MAX stones, each a disk of radius 2 to 4 with one or two smaller lumps on it, away from
// the edges and from each other.
static void make_stones(void) {
    int16_t placed[STONES_MAX][3];
    int     count = 2 + random_below(STONES_MAX - 1), n = 0;
    for (int tries = 0; n < count && tries < 50; tries++) {
        int  r = 2 + random_below(3), cx = 10 + random_below(WAVE_W - 20), cy = 6 + random_below(WAVE_H - 12);
        bool clear = true;
        for (int i = 0; i < n; i++) {
            int dx = cx - placed[i][0], dy = cy - placed[i][1], gap = 2 * (r + placed[i][2]) + 6;
            if (dx * dx + dy * dy < gap * gap) clear = false;
        }
        if (!clear) continue;
        placed[n][0] = cx, placed[n][1] = cy, placed[n][2] = r, n++;
        add_disk(cx, cy, r);
        for (int lumps = 1 + random_below(2); lumps > 0; lumps--) {
            add_disk(cx + random_below(2 * r + 1) - r, cy + random_below(2 * r + 1) - r, 1 + random_below(r));
        }
    }
}

// 1 to ELLIPSES_MAX elliptical mirrors (walls 2 cells thick) side by side, the middle row of the
// corridor as their long axis. Drops fall at their foci.
static void make_ellipses(void) {
    int16_t placed[ELLIPSES_MAX][2];
    int     count = 1 + random_below(ELLIPSES_MAX), n = 0;
    for (int tries = 0; n < count && tries < 50; tries++) {
        int  a = 18 + random_below(16), b = 10 + random_below(5), cx = a + 4 + random_below(WAVE_W - 2 * a - 8);
        bool clear = true;
        for (int i = 0; i < n; i++) {
            if (abs(cx - placed[i][0]) < a + placed[i][1] + 8) clear = false;
        }
        if (!clear) continue;
        placed[n][0] = cx, placed[n][1] = a, n++;
        // In half cells from the middle of the corridor (between rows 15 and 16).
        int32_t a2 = 4 * a * a, b2 = 4 * b * b, ao2 = 4 * (a + 2) * (a + 2), bo2 = 4 * (b + 2) * (b + 2);
        for (int y = 0; y < WAVE_H; y++) {
            for (int x = cx - a - 2; x <= cx + a + 2; x++) {
                int32_t X = 2 * (x - cx), Y = 2 * y + 1 - WAVE_H;
                if (X * X * b2 + Y * Y * a2 >= a2 * b2 && X * X * bo2 + Y * Y * ao2 <= ao2 * bo2) set_wall(x, y);
            }
        }
        int f = 0;
        while ((f + 1) * (f + 1) <= a * a - b * b) f++;
        for (int side = -1; side <= 1; side += 2) {
            spots[spot_count][0] = cx + side * f, spots[spot_count][1] = WAVE_H / 2, spot_count++;
        }
    }
}

// A wall across the corridor, 2 cells thick, with two slits 3 cells wide; the plane waves start 16
// to 23 cells in front of it.
static void make_slits(void) {
    int x = 96 + random_below(64), apart = 10 + random_below(5);
    int top = WAVE_H / 2 - 1 - apart / 2, bottom = WAVE_H / 2 + apart / 2; // the slits' middle rows
    for (int y = 0; y < WAVE_H; y++) {
        if (abs(y - top) <= 1 || abs(y - bottom) <= 1) continue;
        set_wall(x, y);
        set_wall(x + 1, y);
    }
    source_x = x - 16 - random_below(8);
}

// A lens: where two disks of radius r overlap, the middle row of the corridor as its axis. Its focal
// length is about r (a glass lens's would be, at 2/3 the speed of light outside). The plane waves
// start 16 to 23 cells in front of it.
static void make_lens(void) {
    int cx = 80 + random_below(96), r = 17 + random_below(5), half = 5 + random_below(3);
    for (int y = 0; y < WAVE_H; y++) {
        for (int x = cx - half; x <= cx + half; x++) {
            int32_t Y = 2 * y + 1 - WAVE_H, X1 = 2 * (x - cx + r - half), X2 = 2 * (x - cx - r + half);
            if (X1 * X1 + Y * Y <= 4 * r * r && X2 * X2 + Y * Y <= 4 * r * r) {
                set_bit(lens, x, y);
                set_bit(near_wall, x, y);
            }
        }
    }
    source_x = cx - half - 16 - random_below(8);
}

void lcd_wave_start(uint32_t seed) {
    rng = seed ? seed : 0x9e3779b9;
    memset(wave_a, 0, sizeof(wave_a));
    memset(wave_b, 0, sizeof(wave_b));
    memset(walls, 0, sizeof(walls));
    memset(lens, 0, sizeof(lens));
    memset(near_wall, 0, sizeof(near_wall));
    wave_u = wave_a, wave_back = wave_b;
    step_count = 0, spot_count = 0, train_left = 0;

    scene = random_below(SCENE_COUNT);
    if (scene == SCENE_STONES) {
        make_stones();
    } else if (scene == SCENE_ELLIPSES) {
        make_ellipses();
    } else if (scene == SCENE_SLITS) {
        make_slits();
    } else {
        make_lens();
    }
    source();
}

const char *lcd_wave_scene(void) {
    return scene_names[scene];
}

// c^2 L u along row y, into the ring.
static void ring_row(int y) {
    int16_t *rows[5];
    for (int k = 0; k < 5; k++) rows[k] = wave_u[mirror(y + k - 2)];
    int32_t *a = ring[y % 5];
    for (int x = 0; x < WAVE_W; x++) {
        if (!NEAR_WALL(x, y)) {
            a[x] = DIV_ROUND(LAP12_FAST(rows, x), LAP_DIV);
        } else if (is_wall(x, y)) {
            a[x] = 0;
        } else {
            int32_t lap = lap12_u(rows, x, y);
            a[x]        = bit(lens, x, y) ? DIV_ROUND(lap, LENS_LAP_DIV) : DIV_ROUND(lap, LAP_DIV);
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
            int32_t lap2;
            if (!NEAR_WALL(x, y)) {
                lap2 = DIV_ROUND(LAP12_FAST(a_rows, x), LAP2_DIV);
            } else if (is_wall(x, y)) {
                continue;
            } else {
                int32_t lap = lap12_ring(a_rows, x, y);
                lap2        = bit(lens, x, y) ? DIV_ROUND(lap, LENS_LAP2_DIV) : DIV_ROUND(lap, LAP2_DIV);
            }
            int32_t was  = back[x] - DIV_ROUND(back[x], WAVE_DAMPING);
            int32_t next = 2 * u[x] - was + a_rows[2][x] + lap2;
            back[x]      = clamp16(next - DIV_ROUND(next, WAVE_DAMPING));
        }
        // The ends absorb, with Mur's boundary from the new values just inside them:
        // u_next(end) = u(inner) + (c - 1) / (c + 1) (u_next(inner) - u(end)), and (c - 1) / (c + 1) = -1/3.
        back[0]          = clamp16(u[1] - DIV_ROUND(back[1] - u[0], 3));
        back[WAVE_W - 1] = clamp16(u[WAVE_W - 2] - DIV_ROUND(back[WAVE_W - 2] - u[WAVE_W - 1], 3));
    }
    int16_t (*t)[WAVE_W] = wave_u;
    wave_u = wave_back, wave_back = t;
    if (++step_count == next_source) {
        source();
    }
}

uint32_t lcd_wave_steps(void) {
    return step_count;
}

void lcd_draw_wave(uint8_t half) {
    // Walls and crests dark, and the lens's outline dotted; everything else light. A byte (8 rows of
    // a column) at a time, as the LCD stores them.
    const int x0 = half * LCD_WIDTH;
    for (int page = 0; page < LCD_HEIGHT / 8; page++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            int     gx   = x0 + x;
            uint8_t byte = 0;
            for (int b = 0; b < 8; b++) {
                int  y  = page * 8 + b;
                bool on = is_wall(gx, y) || wave_u[y][gx] > WAVE_CREST;
                if (!on && bit(lens, gx, y) && (gx + y) % 2 == 0) {
                    on = gx == 0 || gx == WAVE_W - 1 || y == 0 || y == WAVE_H - 1 || !bit(lens, gx - 1, y) || !bit(lens, gx + 1, y) || !bit(lens, gx, y - 1) || !bit(lens, gx, y + 1);
                }
                if (on) byte |= 1 << b;
            }
            st7565_write_raw_byte(byte, page * LCD_WIDTH + x);
        }
    }
}
