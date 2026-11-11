# Ergo68 Stonegarden

Keymap for a [Yushakobo Ergo68](https://shop.yushakobo.jp/en/products/ergo68) running fully wireless on two
[BLE Micro Pro](https://github.com/sekigon-gonnoc/BLE-Micro-Pro) controllers. It is a port of the
[ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) layout and works the same way: this repository
is a QMK keymap folder that you build with `make` and flash onto the board.

The BLE Micro Pro (an nRF52840) is not supported by mainline QMK, so the keymap is built against sekigon-gonnoc's
BLE Micro Pro fork of Vial-QMK ([`sekigon-gonnoc/vial-qmk`, branch `dev/ble-micro-pro`](https://github.com/sekigon-gonnoc/vial-qmk/tree/dev/ble-micro-pro)).

## How BLE Micro Pro firmware is put together

Each of the two BLE Micro Pros (BMP) holds four things:

| Part                 | What it is                                                                                              | How it gets there                                     |
|----------------------|---------------------------------------------------------------------------------------------------------|-------------------------------------------------------|
| Bootloader           | UF2 bootloader                                                                                          | Web Configurator, once                                |
| Application firmware | The generic `ble_micro_pro` QMK/Vial firmware, the same for every keyboard. This repo builds it with this keymap compiled in | Drag the `.uf2` built from this repo onto the BMP     |
| `CONFIG.BIN`         | Keyboard config: Ergo68 matrix (10×7, pins), left or right, master or slave                            | Web Configurator, once per half                       |
| Keymap               | Lives in (emulated) EEPROM so that Vial/Remap can edit it                                               | Loaded from `keymap.c` automatically, see below       |

Normally the EEPROM keymap survives a firmware update, so flashing a new build would not change your keys.
`keymap.c` therefore keeps a fingerprint of its layers and reloads the EEPROM keymap from the firmware whenever
the layers change. That gives you the same edit → build → flash loop as on the Ergodox.

## Layout

A port of the Ergodox layout. The Ergo68 has eight keys fewer, so:

* Home/End, PgUp/PgDn, Ins and PrtSc moved from the thumb clusters to a navigation block on the FUNC layer.
* Ctrl+Alt/Menu moved to the left thumb on the FUNC layer. Hold it and press an F-key for Ctrl+Alt+Fx (TTY switching).
* The right-hand Esc became the FUNC key (Esc is still on the left).
* The RGB/backlight keys on MEDIA became Bluetooth controls. The NKRO toggle is gone (the BMP firmware has no NKRO).

`MEDIA`, `NUMPD` and `FUNC` latch their layer on, and `BASE` (the key in the same spot) returns to the base layer.
Holding `;` gives MEDIA momentarily. Blank keys fall through to the base layer.

### BASE

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
|  \   |  1   |  2   |  3   |  4   |  5   |                |  6   |  7   |  8   |  9   |  0   |  /   |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
| Tab  |  Q   |  W   |  E   |  R   |  T   | Esc  |  | FUNC |  Y   |  U   |  I   |  O   |  P   |  '   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|  `   |  A   |  S   |  D   |  F   |  G   |MEDIA |  |NUMPD |  H   |  J   |  K   |  L   |;/MED |  =   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|Shift |  Z   |  X   |  C   |  V   |  B   |  [   |  |  ]   |  N   |  M   |  ,   |  .   |  :   |  -   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
| Ctrl | GUI  | Alt  |  ~   |  $   | Bspc | Del  |  |Enter |Space | Left | Down |  Up  |Right |AltGr |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

### MEDIA (hold `;` or tap MEDIA)

`Pair` advertises to a new host and looks for the other half, `Host1`–`Host5` switch between paired hosts, `BLE`/`USB`
choose where key presses go, `Batt` types the battery voltage, `Sleep` puts the keyboard in deep sleep.

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Boot |Host1 |Host2 |Host3 |Host4 |Host5 |                |      |      |      |      |      | Boot |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|EEClr | Pair | BLE  | USB  | Batt |      |      |  |      | Prev | Mute |      | Next |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      | BASE |  | BASE | Rwd  | Vol- | Vol+ | Fwd  |      | Play |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |      |      |      |      |      | Stop |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|Sleep |      |      |      |      |      |      |  |      |      |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

### NUMPAD

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Boot |      |      | Btn4 | Btn5 |      |                |      |NumLk |  /   |  *   |  -   | Boot |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|      | Acc0 | Back | Btn1 | Btn2 | Frwd |      |  |      |  (   |  7   |  8   |  9   |  +   |EEClr |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      | Acc1 |  M←  |  M↓  |  M↑  |  M→  | BASE |  | BASE |  )   |  4   |  5   |  6   |  .   |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      | Acc2 |  W←  |  W↓  |  W↑  |  W→  | Btn3 |  |      |  0   |  1   |  2   |  3   |  ,   |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |  0   |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

### FUNC

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Boot |  F1  |  F2  |  F3  |  F4  |  F5  |                |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|EEClr |      |      |      |      |      |      |  | BASE | Ins  | Home | PgUp |      |PrtSc | F12  |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      | BASE |  | BASE | Del  | End  | PgDn |      |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |S-Ins |      |  |      |      |      |      |      | RGUI |RShft |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      | C+A  |      |      |      |  |      |      |      |      |      |      |RCtrl |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

## One-time setup with the Web Configurator

This installs the bootloader, the stock firmware and the Ergo68 config on both BMPs. You need Chrome or Edge (it uses
Web Serial). The [BMP getting started guide](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/getting_started) has
screenshots of every step (in Japanese).

1. Open the [BLE Micro Pro Web Configurator](https://sekigon-gonnoc.github.io/BLE-Micro-Pro-WebConfigurator/) and start the
   guided setup. Select **ergo68** and leave *Use with LPME-IO* unchecked.
2. Connect the **left** BMP with USB. The left half is the master, the one that talks to your computer.
   1. Update the bootloader (latest version), then unplug and replug.
   2. Update the firmware (latest `ble_micro_pro_vial_1_4_x`).
   3. Write the config with *Is Slave* unchecked and *Is Left* checked.
3. Swap to the **right** BMP and repeat. The wizard checks *Is Slave* and unchecks *Is Left* for you.

Set up one half at a time. With two BMPs the halves talk to each other over Bluetooth, so no TRRS cable is needed.

## Building

### Like ergodox_stonegarden

Set up the [QMK build environment](https://docs.qmk.fm/newbs_getting_started) (the `qmk` CLI and `arm-none-eabi-gcc`),
then:

```bash
git clone -b dev/ble-micro-pro https://github.com/sekigon-gonnoc/vial-qmk.git qmk_firmware_bmp
```

```bash
cd qmk_firmware_bmp
```

```bash
git submodule update --init lib/printf
```

```bash
git clone https://github.com/vehagn/ergo68.git keyboards/ble_micro_pro/keymaps/ergo68
```

```bash
make ble_micro_pro:ergo68:uf2
```

The firmware is written to `.build/ble_micro_pro_ergo68.uf2`.

### With build.sh

`./build.sh` does the same in a `.bmp-qmk/` folder next to the keymap, using a pinned version of the fork, and copies the
result to `build/ble_micro_pro_ergo68.uf2`.

### On GitHub

Every push runs [the build workflow](.github/workflows/build.yml). Download the `ergo68-ble-micro-pro-firmware` artifact
from the run; it is a zip file containing the `.uf2`.

## Flashing

1. Put the BMP in bootloader mode, either:
   * switch the battery off, then hold the reset switch on the PCB while plugging in USB, or
   * with the keyboard on USB, press `Boot` (top-left key on the MEDIA, NUMPAD and FUNC layers).

   The BMP shows up as a USB drive that contains `INFO_UF2.TXT`.
2. Copy `ble_micro_pro_ergo68.uf2` onto the drive and wait for it to finish. Don't unplug it while it copies.
3. Unplug and replug. The drive now contains `VERSION.TXT`, and its `Build from:` line shows the
   `ergo68-<commit>` you just built.
4. Do the same for the other half. Only the left half uses the keymap, but keeping both halves on the same build avoids
   version mismatches between them.

On the first boot after flashing a changed keymap, the keymap from `keymap.c` is loaded automatically.

## Pairing and wireless use

* **Left and right half** pair during the first few seconds after both are switched on. Switch on the left half, then
  the right half within a few seconds.
* **Computers and phones:** the keyboard advertises for a few seconds after it starts, or press `Pair` (hold `;` + Q).
  It appears as *ergo68*. Pair with up to five hosts and switch between them with `Host1`–`Host5` (hold `;` + 1–5).
  To pair another device, turn Bluetooth off on the devices that are already paired first.
* **USB or Bluetooth:** `USB` (hold `;` + E) and `BLE` (hold `;` + W) choose where key presses go.
* **Battery:** `Batt` (hold `;` + R) types the battery voltage in millivolts.

Pairing problems are usually fixed by removing the pairing on both sides and pairing again; see the
[BMP FAQ](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/FAQ).

## Changing the keymap

* **In `keymap.c` (recommended):** edit, build, flash. A changed keymap replaces whatever is on the keyboard, including
  changes made in Vial.
* **In Vial:** [VIA Custom UI for Vial](https://sekigon-gonnoc.github.io/via-custom-ui-for-vial/),
  [Vial](https://vial.rocks/) and [Remap](https://remap-keys.app/) edit the keymap live, without flashing. Those edits
  last until the layers in `keymap.c` change. Copy changes you want to keep into `keymap.c`, or define
  `KEYMAP_SYNC_DISABLE` in `config.h` if you would rather keep the keymap in Vial only.

`LAYOUT_ergo68` takes its keys in the same order as `LAYOUT` of the wired keyboard in mainline QMK
([`yushakobo/ergo68`](https://github.com/qmk/qmk_firmware/tree/master/keyboards/yushakobo/ergo68)), so layers can be
copied between the two if you ever go back to Pro Micros.

## Troubleshooting

* **The keymap doesn't change after flashing.** A saved default EEPROM image (`DEFAULT.BIN` on the BMP's drive) takes
  priority over `keymap.c`. Open the BMP's USB serial console (e.g. `screen /dev/ttyACM0` on Linux, TeraTerm on
  Windows), run `default erase`, then press `EEClr` (left key on the second row of the MEDIA or FUNC layer).
* **The BMP doesn't show up as a drive in bootloader mode.** The battery was probably still on, so the BMP never
  restarted. Switch it off and try again.
* **The per-key LEDs are off.** The BMP config for the Ergo68 disables them to save battery, and they only work on
  USB power.

`vial.json` is copied unchanged from the stock BMP firmware (`keyboards/ble_micro_pro/keymaps/vial`), because the fork's
build needs it in the keymap folder.
