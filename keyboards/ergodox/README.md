# ErgoDox (Infinity and EZ)

The [ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) keymap, ported to current QMK (0.34.7). The same
keymap builds for the ErgoDox Infinity (`input_club/ergodox_infinity`) and the ErgoDox EZ (`ergodox_ez/base`, `/glow` or
`/shine`). See the [top-level README](../../README.md) for the build environment. ZMK doesn't support either board's
microcontroller, so these are QMK only.

## What changed from the old repository

* **Same layers and keys.** BASE, MEDIA, NUMPAD and FUNC are where they were, with the same latching behaviour:
  `MEDIA`, `NUMPAD` and `FUNC` switch their layer on and `BASE` returns. They're now QMK's standard `TG()`/`TO()` keys
  instead of custom keycodes, and old keycode names are updated (`RESET` → `QK_BOOT`, `EEP_RST` → `EE_CLR`, mouse keys
  → `MS_*`, `MAGIC_TOGGLE_NKRO` → `NK_TOGG`).
* **The LCDs (Infinity).** QMK's old LCD "visualizer" no longer exists; the LCDs now use QMK's ST7565 driver. As
  before, they show the active layer's name (*Ergodox Infinity*, *Media*, *Mouse + Numpad*, *Function keys*) and their
  backlight takes the layer's colour from the old visualizer. The second line shows Caps, Num and Scroll Lock. Both
  halves show it.
* **Lighting keys on MEDIA.** The old RGB and backlight keys now control whatever lighting the board has: the per-key
  backlight on the Infinity (on/off, effect, brightness, speed), RGB matrix on the EZ Glow, RGB underglow on the EZ Shine.
  On the plain EZ they do nothing. The RGB mode keys (plain, breathing, rainbow, …) are gone.
* **The EZ's three LEDs** show which of MEDIA, NUMPAD and FUNC are latched on.
* **One firmware for both Infinity halves.** There's no `MASTER=right` build any more; plug the left half into the
  computer.

## ErgoDox Infinity

Build with `just ergodox-infinity`, which gives `build/ergodox_infinity.bin`. Each half is flashed separately with
the same file:

1. Run `just flash-ergodox-infinity` on your own machine (it uses dfu-util, which may need `sudo` or a udev rule). It
   waits for a half in bootloader mode.
2. Put the half that's plugged into USB in bootloader mode, see below.
3. Do the same for the other half.

**Bootloader mode without the program button.** The program button is on the back (above the thumb cluster), behind
the case. With this firmware already on the keyboard, a key does the same for the half that's plugged into USB:

* **Left half** (normal use, USB in the left half): tap `MEDIA` (left half, inner column, second row) and press the
  top-left key. Holding `;` and pressing the top-left key works too.
* **Right half:** plug USB into the right half instead. The half on USB always acts as the left half, and on the
  Infinity every key has the same matrix position as its mirror image on the other half, so the right half's keys now
  do what their left-hand mirror images do. Tap the right half's inner-column key in the second row (normally `NUMPD`,
  now `MEDIA`), then press its top-right key. This only uses the right half, so it works with the halves disconnected.

Keymap changes only need the left half flashed: the half on USB runs the keymap, the other only reports its key
presses. After a QMK update, flash both halves so their split protocol matches.

Then connect the halves and plug the left half into the computer. If you'd rather be able to plug in either half, add
`#define EE_HANDS` to `config.h` and set each half's side once as described in QMK's
[ErgoDox Infinity readme](https://github.com/qmk/qmk_firmware/tree/master/keyboards/input_club/ergodox_infinity).

## ErgoDox EZ

Build with `just ergodox-ez`, which gives `build/ergodox_ez_base.hex`. For the EZ Glow or Shine, build with
`just ez_variant=glow ergodox-ez` or `just ez_variant=shine ergodox-ez` so their lights work.

Flash it with ZSA's [Keymapp](https://www.zsa.io/flash) (choose the `.hex` file), QMK Toolbox, or
`just flash-ergodox-ez` (teensy_loader_cli) on your own machine. Press the reset button (the small hole on the top right)
or `Boot` when it asks.
