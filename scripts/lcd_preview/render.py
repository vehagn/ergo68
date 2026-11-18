#!/usr/bin/env python3
"""Turn scripts/lcd_preview/preview.c's output into pictures of the ErgoDox Infinity LCD screens.

    preview      | python3 scripts/lcd_preview/render.py keyboards/ergodox/lcd_preview.png
    preview idle | python3 scripts/lcd_preview/render.py keyboards/ergodox/lcd_idle.gif

A .png gets every screen stacked, each with its name below; a .gif gets the screens as frames of
an animation. Screens are drawn as the LCD looks: dark pixels on the backlight's colour, enlarged
4 times. Screens 256 pixels wide are both LCDs side by side.
"""

import sys

from PIL import Image, ImageDraw, ImageFont

SCALE, MARGIN, LABEL, GAP = 4, 16, 18, 24
H = 32


def read_screens(text):
    screens, lines, i = [], text.split("\n"), 0
    while i < len(lines) and lines[i]:
        name, rgb, width = lines[i].rsplit(" ", 2)
        screens.append((name, int(rgb, 16), int(width), lines[i + 1 : i + 1 + H]))
        i += 1 + H
    return screens


def lcd_width(width):
    # Two LCDs get a gap between them, as on the keyboard.
    return width * SCALE + (GAP if width > 128 else 0)


def draw_screen(draw, x, y, rgb, width, rows):
    r, g, b = (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255
    back = tuple(int(30 + c * 0.6) for c in (r, g, b))  # the backlight, never quite black
    ink = (20, 20, 25)
    for lcd in range(width // 128):
        lx = x + lcd * (128 * SCALE + GAP)
        draw.rectangle([lx - 2, y - 2, lx + 128 * SCALE + 1, y + H * SCALE + 1], fill=(10, 10, 10))
        draw.rectangle([lx, y, lx + 128 * SCALE - 1, y + H * SCALE - 1], fill=back)
        for py, row in enumerate(rows):
            for px in range(128):
                if row[lcd * 128 + px] == "1":
                    x0, y0 = lx + px * SCALE, y + py * SCALE
                    draw.rectangle([x0, y0, x0 + SCALE - 1, y0 + SCALE - 1], fill=ink)


def main():
    screens = read_screens(sys.stdin.read())
    out = sys.argv[1]
    if out.endswith(".gif"):
        width = max(lcd_width(w) for _, _, w, _ in screens) + 2 * MARGIN
        frames = []
        for name, rgb, w, rows in screens:
            img = Image.new("RGB", (width, H * SCALE + 2 * MARGIN), (40, 40, 40))
            draw_screen(ImageDraw.Draw(img), MARGIN, MARGIN, rgb, w, rows)
            frames.append(img)
        duration = int(sys.argv[2]) if len(sys.argv) > 2 else 250  # ms per frame
        frames[0].save(out, save_all=True, append_images=frames[1:], duration=duration, loop=0)
    else:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 13)
        width = max(lcd_width(w) for _, _, w, _ in screens) + 2 * MARGIN
        height = len(screens) * (H * SCALE + LABEL + MARGIN) + MARGIN
        img = Image.new("RGB", (width, height), (40, 40, 40))
        draw = ImageDraw.Draw(img)
        y = MARGIN
        for name, rgb, w, rows in screens:
            draw_screen(draw, MARGIN, y, rgb, w, rows)
            draw.text((MARGIN, y + H * SCALE + 3), name, fill=(220, 220, 220), font=font)
            y += H * SCALE + LABEL + MARGIN
        img.save(out)
    print(f"Wrote {out} ({len(screens)} screens)")


if __name__ == "__main__":
    main()
