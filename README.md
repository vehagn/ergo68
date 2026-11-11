# Ergo68 Stonegarden

Keymap for a [Yushakobo Ergo68](https://shop.yushakobo.jp/en/products/ergo68), ported from the
[ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) layout. Like that repository, this is a QMK keymap
folder that you build with `make` and flash onto the board. The same keymap builds for the kit's Pro Micros (wired) and
for [BLE Micro Pros](https://github.com/sekigon-gonnoc/BLE-Micro-Pro) (Bluetooth).

## Hardware setups

| Setup                    | Left half     | Right half                                              | Between the halves | To the computer               | Firmware                                     |
|--------------------------|---------------|---------------------------------------------------------|--------------------|-------------------------------|----------------------------------------------|
| Wired (the kit's chips)  | Pro Micro     | Pro Micro                                               | TRS or TRRS cable  | USB                           | `yushakobo_ergo68_ergo68.hex` on both halves |
| Bluetooth in one half    | BLE Micro Pro | [LPME-IO](https://shop.yushakobo.jp/products/lpme-io2a) | 4-pole TRRS cable  | Bluetooth, or USB (left half) | `ble_micro_pro_ergo68.uf2` on the left half  |
| Bluetooth in both halves | BLE Micro Pro | BLE Micro Pro                                           | Bluetooth          | Bluetooth, or USB (left half) | `ble_micro_pro_ergo68.uf2` on both halves    |

* The controllers sit in con-through sockets, so you switch setups by swapping them. Yushakobo's
  [build guide](https://github.com/yushakobo/build-documents/tree/master/Ergo68) (section EX1) shows how to fit a BLE
  Micro Pro (BMP) and the jumper an LPME-IO needs.
* For Bluetooth in one half, the other half must have an LPME-IO. The BMP firmware only supports a cable to the other
  half through an LPME-IO, not to a Pro Micro or a second BMP.
* Never unplug the cable between the halves while the keyboard is connected to the computer. Yushakobo warns that this
  can damage it.
* The per-key LEDs work in the wired setup, including the indicator LEDs. With a BMP they only light on USB power, and
  the BMP's Ergo68 config leaves them off.

The BMP is an nRF52840 that mainline QMK doesn't support, so its firmware is built from sekigon-gonnoc's BLE Micro Pro
fork of Vial-QMK ([`sekigon-gonnoc/vial-qmk`, branch `dev/ble-micro-pro`](https://github.com/sekigon-gonnoc/vial-qmk/tree/dev/ble-micro-pro)).
The Pro Micro firmware is built from mainline QMK ([`yushakobo/ergo68`](https://github.com/qmk/qmk_firmware/tree/master/keyboards/yushakobo/ergo68)).

## Layout

The Ergo68 has eight keys fewer than the Ergodox, so:

* Home/End, PgUp/PgDn, Ins and PrtSc moved from the thumb clusters to a navigation block on the FUNC layer.
* Ctrl+Alt/Menu moved to the left thumb on the FUNC layer. Hold it and press an F-key for Ctrl+Alt+Fx (TTY switching).
* The right-hand Esc became the FUNC key (Esc is still on the left).
* The left half of MEDIA has Bluetooth controls in the BMP firmware and the Ergodox's RGB controls in the Pro Micro
  firmware. The NKRO toggle is gone.

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

### MEDIA, BLE Micro Pro firmware (hold `;` or tap MEDIA)

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

### MEDIA, Pro Micro firmware

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Boot | Hue- | Hue+ | Sat- | Sat+ |Mode- |                |      |      |      |      |      | Boot |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|EEClr |      |      |      |      |      |Mode+ |  |      | Prev | Mute |      | Next |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      | BASE |  | BASE | Rwd  | Vol- | Vol+ | Fwd  |      | Play |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |      |      |      |      |      | Stop |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
| RGB  | Spd- | Spd+ | Bri- | Bri+ |      |      |  |      |      |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

In the Pro Micro firmware the three LEDs at the top of each half are indicators. On the left they show Caps Lock, Num
Lock and Scroll Lock. On the right they show which of MEDIA (purple), NUMPAD (green) and FUNC (orange) are latched on.

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

## Building

`./build.sh` builds both firmwares into `build/`: `ble_micro_pro_ergo68.uf2` for BMPs and `yushakobo_ergo68_ergo68.hex`
for Pro Micros. `./build.sh bmp` or `./build.sh promicro` builds just one. Each fetches its QMK tree at a pinned version.

### In the devcontainer (recommended)

The repo comes with a [devcontainer](.devcontainer/devcontainer.json) based on QMK's own build image, so you don't need
QMK or the compilers on your machine. You need Docker and VS Code with the
[Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension, or a
GitHub Codespace.

1. Open the repo in VS Code and run **Dev Containers: Reopen in Container**.
2. Run `./build.sh` in the terminal, or **Terminal → Run Build Task** (Ctrl+Shift+B).
3. The firmware appears in `build/` in the repo folder on your own machine. In a Codespace, right-click the file in
   the Explorer and choose **Download**.

The QMK trees live in a Docker volume mounted at `/qmk`, so container rebuilds don't download them again. The BMP fork
is based on an older QMK that wants Python packages the current QMK image lacks; `build.sh` installs those in a
virtualenv inside the fork's folder the first time. Flashing needs
the USB device, so do it on your own machine, not in the container.

### On GitHub

Every push runs [the build workflow](.github/workflows/build.yml) in the same devcontainer. Download the `ergo68-firmware`
artifact from the run; it is a zip file with both firmwares.

### Without the devcontainer

With the [QMK build environment](https://docs.qmk.fm/newbs_getting_started) installed, `./build.sh` also works directly
on your machine; it puts the QMK trees in `.qmk/`. Or do it by hand, like ergodox_stonegarden. For the Pro Micros, in
your normal `qmk_firmware` checkout:

```bash
git clone https://github.com/vehagn/ergo68.git keyboards/yushakobo/ergo68/keymaps/ergo68
```

```bash
make yushakobo/ergo68:ergo68:flash
```

For BMPs, in a checkout of the BMP fork:

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

## Wired: Pro Micro

These are the controllers that come with the kit (ATmega32U4 Pro Micros with the Caterina bootloader). Both halves get
the same `yushakobo_ergo68_ergo68.hex`; the PCB tells each half which side it is. Connect one half at a time with USB.

The easiest way to flash is [Remap](https://remap-keys.app/), the web page Yushakobo's build guide uses (Chrome or Edge):

1. Click **Flash Firmware** on Remap's front page and drop `yushakobo_ergo68_ergo68.hex` into the dialog.
2. Choose the **caterina** bootloader and click **Flash**.
3. When it asks for the serial port, press the half's reset switch, pick the port that appears and click **Connect**.
   The Pro Micro only waits about 8 seconds after a reset, so be quick; Remap shows *successfully* when it's done.

QMK Toolbox ([download](https://github.com/qmk/qmk_toolbox/releases): open the `.hex`, tick *Auto-Flash*, press reset)
and `make yushakobo/ergo68:ergo68:flash` from a QMK checkout work too, as does avrdude on Linux right after a reset:

```bash
avrdude -p atmega32u4 -c avr109 -P /dev/ttyACM0 -U flash:w:yushakobo_ergo68_ergo68.hex:i
```

Once the firmware is on, `Boot` (top-left key on the MEDIA, NUMPAD and FUNC layers) also starts the bootloader. Connect
the halves with the TRS/TRRS cable and plug the left half into the computer.
[Remap's configure page](https://remap-keys.app/configure) can then edit the keymap live, as with Yushakobo's own
firmware.

## Bluetooth: BLE Micro Pro

### One-time setup with the Web Configurator

This installs the bootloader, the stock firmware and the Ergo68 config on the BMPs. You need Chrome or Edge (it uses Web
Serial). The [BMP getting started guide](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/getting_started) has
screenshots of every step (in Japanese).

1. Open the [BLE Micro Pro Web Configurator](https://sekigon-gonnoc.github.io/BLE-Micro-Pro-WebConfigurator/) and start
   the guided setup. Select **ergo68**. For Bluetooth in one half, also tick *Use with LPME-IO*.
2. Connect the **left** BMP with USB. The left half is the one that talks to your computer.
   1. Update the bootloader (latest version), then unplug and replug.
   2. Update the firmware (latest `ble_micro_pro_vial_1_4_x`).
   3. Write the config with *Is Slave* unchecked and *Is Left* checked.
3. Bluetooth in both halves only: swap to the **right** BMP and repeat. The wizard checks *Is Slave* and unchecks
   *Is Left* for you.

### Flashing this firmware

1. Put the BMP in bootloader mode, either:
   * switch the battery off, then hold the reset switch on the PCB while plugging in USB, or
   * with the keyboard on USB, press `Boot` (top-left key on the MEDIA, NUMPAD and FUNC layers).

   The BMP shows up as a USB drive that contains `INFO_UF2.TXT`.
2. Copy `ble_micro_pro_ergo68.uf2` onto the drive and wait for it to finish. Don't unplug it while it copies.
3. Unplug and replug. The drive now contains `VERSION.TXT`, and its `Build from:` line shows the `ergo68-<commit>` you
   just built.
4. Bluetooth in both halves: do the same for the right BMP. Only the left half uses the keymap, but keeping both halves on
   the same build avoids version mismatches between them.

The keymap is part of the firmware but is kept in the BMP's (emulated) EEPROM so that Vial can edit it. The firmware
reloads it from `keymap.c` on the first boot after you flash a changed keymap.

### Pairing, Bluetooth and USB

* **Both halves on Bluetooth** pair with each other during the first few seconds after both are switched on. Switch on
  the left half, then the right half within a few seconds.
* **Computers and phones:** the keyboard advertises for a few seconds after it starts, or press `Pair` (hold `;` + Q).
  It appears as *ergo68*. Pair with up to five hosts and switch between them with `Host1`–`Host5` (hold `;` + 1–5).
  To pair another device, turn Bluetooth off on the devices that are already paired first.
* **USB:** plug a USB cable into the left half. If no Bluetooth host is connected it switches to USB by itself;
  otherwise press `USB` (hold `;` + E). `BLE` (hold `;` + W) switches back, and unplugging USB returns to Bluetooth.

Pairing problems are usually fixed by removing the pairing on both sides and pairing again; see the
[BMP FAQ](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/FAQ).

### Batteries: AAA NiMH

Yushakobo's [AAA battery kit](https://shop.yushakobo.jp/products/8011) for the Ergo68 holds one AAA cell per half and
is meant for the [BMP Boost](https://github.com/sekigon-gonnoc/BLE-Micro-Pro/tree/master/bmp-boost), the BMP version with
a step-up converter that runs from a single cell (0.7 V and up). The original BMP needs at least 1.7 V and can't run from
one AAA cell. With an LPME-IO only the left half has a battery; the right half gets its power through the cable.

NiMH cells work fine. Things to know:

* **Nothing on the keyboard charges them.** Charge them in a NiMH charger.
* **Go by the voltage, not a percentage.** `Batt` (hold `;` + R) types the voltage of each half in millivolts, left half
  first. A NiMH cell reads about 1.3–1.4 V straight off the charger, stays around 1.2 V for most of its charge and is
  nearly empty around 1.1 V. Any percentage meant for 1.5 V alkaline cells will look low.
* **Use low-self-discharge cells** (Eneloop and similar). The keyboard draws so little that ordinary NiMH cells lose a
  noticeable part of their charge to self-discharge.
* **The left half runs down first.** The BMP docs give about 300 µA for the half that talks to the computer and about
  100 µA for the other half.
* **Switch off with the slide switch** when you're not using the keyboard, or press `Sleep` (hold `;` + left Ctrl).

Auto-sleep is off in the Ergo68 config from the Web Configurator. To have a half go to sleep after some time without a
key press while on battery, uncomment `BMP_AUTO_SLEEP_MINUTES` in `config.h`. It hasn't been tried on this keyboard yet,
so check that a key press wakes it up again.

## Changing the keymap

* **In `keymap.c` (recommended):** edit, build, flash. The same file is used for both firmwares; only the left half of
  MEDIA differs between them (`#ifdef PROTOCOL_BMP`).
* **Live, without flashing:** [Remap](https://remap-keys.app/) works with both firmwares. On the BMP,
  [VIA Custom UI for Vial](https://sekigon-gonnoc.github.io/via-custom-ui-for-vial/) and [Vial](https://vial.rocks/)
  work too. Live edits last until you flash a firmware whose layers in `keymap.c` differ, which replaces them. On the Pro
  Micro, VIA also resets them when you flash a build from a different day. Copy changes you want to keep into
  `keymap.c`, or define `KEYMAP_SYNC_DISABLE` in `config.h` to keep the keymap in Remap/Vial only.

`LAYOUT_ergo68` in `layout.h` takes its keys in the same order as the Ergo68's `LAYOUT` in mainline QMK, so keymaps can
be copied between them.

## What this builds on

I looked for other Ergo68 and BLE Micro Pro work to build on. This repo uses the official pieces, and borrows from others:

* **Pro Micro firmware:** Yushakobo's own keyboard definition in mainline QMK,
  [`yushakobo/ergo68`](https://github.com/qmk/qmk_firmware/tree/master/keyboards/yushakobo/ergo68). The indicator LEDs
  follow its default keymap. [`q4wk/qmk_firmware`](https://github.com/q4wk/qmk_firmware) (layer colours on the Ergo68)
  is where the note about LTO breaking Remap comes from.
* **BLE Micro Pro firmware and config:** sekigon-gonnoc's
  [`vial-qmk` fork](https://github.com/sekigon-gonnoc/vial-qmk/tree/dev/ble-micro-pro) and the Ergo68 configs in
  [`bmp-vial-config`](https://github.com/sekigon-gonnoc/BLE-Micro-Pro/tree/master/bmp-vial-config), which is what the
  Web Configurator installs.
* **Other Ergo68 keymaps** I found are personal QMK keymaps for the wired board, such as
  [`JanaUnderground/ergo68`](https://github.com/JanaUnderground/ergo68) (Miryoku-style) and
  [`kumbung01/my-ergo68-keymap`](https://github.com/kumbung01/my-ergo68-keymap). Neither covers Bluetooth.
* **ZMK, an alternative for the Bluetooth setups:**
  [`WisdomStoneWorshiper/zmk-config-Ergo68`](https://github.com/WisdomStoneWorshiper/zmk-config-Ergo68) is a ZMK
  config for the Ergo68 on nice!nano controllers, with the same pins as QMK's definition. sekigon-gonnoc's
  [`zmk-component-bmp-boost`](https://github.com/sekigon-gonnoc/zmk-component-bmp-boost) makes ZMK run on the BMP Boost,
  and its battery gauge is set up for a single NiMH cell by default. ZMK would need its own keymap file and has no
  LPME-IO support, so it would only replace the both-halves Bluetooth setup.

## Troubleshooting

* **BMP: the keymap doesn't change after flashing.** A saved default EEPROM image (`DEFAULT.BIN` on the BMP's drive)
  takes priority over `keymap.c`. Open the BMP's USB serial console (e.g. `screen /dev/ttyACM0` on Linux, TeraTerm on
  Windows), run `default erase`, then press `EEClr` (left key on the second row of the MEDIA or FUNC layer).
* **BMP: it doesn't show up as a drive in bootloader mode.** The battery was probably still on, so the BMP never
  restarted. Switch it off and try again.
* **BMP: the batteries run down much faster than expected.** The BMP Boost docs warn that LEDs powered from its VCC pin
  draw a lot of current. Compare `Batt` readings for the two halves over a day or two.
* **Pro Micro: only one half types.** Check the TRS/TRRS cable, and that both halves have the same firmware.

`vial.json` is copied unchanged from the stock BMP firmware (`keyboards/ble_micro_pro/keymaps/vial`), because the fork's
build needs it in the keymap folder.
