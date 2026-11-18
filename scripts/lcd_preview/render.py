#!/usr/bin/env python3
"""Turn scripts/lcd_preview/preview.c's output into a PNG of every ErgoDox Infinity LCD screen.

    preview | python3 scripts/lcd_preview/render.py build/ergodox_lcd_preview.png

Each screen is drawn as the LCD looks: dark pixels on the backlight's colour (at the half
brightness the keymap uses), enlarged 4 times, with its name below.
"""

import sys

from PIL import Image, ImageDraw, ImageFont

SCALE, MARGIN, LABEL = 4, 16, 18
W, H = 128, 32

screens, lines = [], sys.stdin.read().split("\n")
i = 0
while i < len(lines) and lines[i]:
    name, rgb = lines[i].rsplit(" ", 1)
    rows = lines[i + 1 : i + 1 + H]
    screens.append((name, int(rgb, 16), rows))
    i += 1 + H

font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 13)
img_w = W * SCALE + 2 * MARGIN
img_h = len(screens) * (H * SCALE + LABEL + MARGIN) + MARGIN
img = Image.new("RGB", (img_w, img_h), (40, 40, 40))
draw = ImageDraw.Draw(img)
y = MARGIN
for name, rgb, rows in screens:
    r, g, b = (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255
    back = tuple(int(30 + c * 0.6) for c in (r, g, b))  # half brightness, never quite black
    ink = (20, 20, 25)
    draw.rectangle([MARGIN - 2, y - 2, MARGIN + W * SCALE + 1, y + H * SCALE + 1], fill=(10, 10, 10))
    draw.rectangle([MARGIN, y, MARGIN + W * SCALE - 1, y + H * SCALE - 1], fill=back)
    for py, row in enumerate(rows):
        for px, bit in enumerate(row):
            if bit == "1":
                x0, y0 = MARGIN + px * SCALE, y + py * SCALE
                draw.rectangle([x0, y0, x0 + SCALE - 1, y0 + SCALE - 1], fill=ink)
    draw.text((MARGIN, y + H * SCALE + 3), name, fill=(220, 220, 220), font=font)
    y += H * SCALE + LABEL + MARGIN
img.save(sys.argv[1])
print(f"Wrote {sys.argv[1]} ({len(screens)} screens)")
