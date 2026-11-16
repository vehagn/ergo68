# Ergo68

Keymap for a [Yushakobo Ergo68](https://shop.yushakobo.jp/en/products/ergo68), ported from the
[ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) layout. It builds for the kit's Pro Micros (wired),
for [BLE Micro Pros](https://github.com/sekigon-gonnoc/BLE-Micro-Pro) with QMK (Bluetooth), and for BMP Boosts with
[ZMK](https://zmk.dev/) (Bluetooth). See the [top-level README](../../README.md) for the build environment.

## Hardware setups

| Setup                          | Left half     | Right half                                              | Between the halves | To the computer               | Firmware (in `build/` or the CI artifacts)                        |
|--------------------------------|---------------|---------------------------------------------------------|--------------------|-------------------------------|-------------------------------------------------------------------|
| Wired (the kit's chips)        | Pro Micro     | Pro Micro                                               | TRS or TRRS cable  | USB                           | `ergo68_pro_micro.hex` on both halves                             |
| Bluetooth in one half          | BLE Micro Pro | [LPME-IO](https://shop.yushakobo.jp/products/lpme-io2a) | 4-pole TRRS cable  | Bluetooth, or USB (left half) | `ergo68_lpme_ble_micro_pro.uf2` on the left half                  |
| Bluetooth in both halves       | BLE Micro Pro | BLE Micro Pro                                           | Bluetooth          | Bluetooth, or USB (left half) | `ergo68_left_ble_micro_pro.uf2` and `ergo68_right_ble_micro_pro.uf2` |
| Bluetooth in both halves (ZMK) | BMP Boost     | BMP Boost                                               | Bluetooth          | Bluetooth, or USB (left half) | `ergo68_left-bmp_boost-zmk.uf2` and `ergo68_right-bmp_boost-zmk.uf2` |

* The controllers sit in con-through sockets, so you switch setups by swapping them. Yushakobo's
  [build guide](https://github.com/yushakobo/build-documents/tree/master/Ergo68) (section EX1) shows how to fit a BLE
  Micro Pro (BMP) and the jumper an LPME-IO needs.
* For Bluetooth in one half, the other half must have an LPME-IO. The BMP firmware only supports a cable to the other
  half through an LPME-IO, not to a Pro Micro or a second BMP. ZMK doesn't support the LPME-IO.
* Never unplug the cable between the halves while the keyboard is connected to the computer. Yushakobo warns that this
  can damage it.
* The per-key LEDs work in the wired setup, including the indicator LEDs. With an original BLE Micro Pro they only
  light on USB power (and the BMP's Ergo68 config leaves them off); with a BMP Boost they can't light at all, see
  [The LEDs](#the-leds).

The BMP is an nRF52840 that mainline QMK doesn't support, so its QMK firmware is built from sekigon-gonnoc's BLE Micro
Pro fork of Vial-QMK ([`sekigon-gonnoc/vial-qmk`, branch `dev/ble-micro-pro`](https://github.com/sekigon-gonnoc/vial-qmk/tree/dev/ble-micro-pro)).
The Pro Micro firmware is built from mainline QMK ([`yushakobo/ergo68`](https://github.com/qmk/qmk_firmware/tree/master/keyboards/yushakobo/ergo68)),
and the ZMK firmware from [`zmk-config/`](../../zmk-config) with a keymap generated from `keymap.c`.

## Layout

Four layers, ported from a Vial layout:

* `NAV` is held: either `NAV` thumb key, or the 1u thumb key next to Space on the right.
* `FUNC` and `NUMPD` latch their layer on. `BASE`, the inner key in the same spot on every other layer, returns to the base
  layer. Holding `;` gives FUNC momentarily.
* Blank keys fall through to the base layer, and `·` keys do nothing.
* The left half of FUNC has Bluetooth controls in the BMP and ZMK firmware, and RGB controls in the Pro Micro firmware.

### BASE

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
|  \   |  1   |  2   |  3   |  4   |  5   |                |  6   |  7   |  8   |  9   |  0   |  /   |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
| Tab  |  Q   |  W   |  E   |  R   |  T   | Esc  |  | Esc  |  Y   |  U   |  I   |  O   |  P   |  '   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|  `   |  A   |  S   |  D   |  F   |  G   | FUNC |  |NUMPD |  H   |  J   |  K   |  L   |;/FUNC|  =   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|Shift |  Z   |  X   |  C   |  V   |  B   |  [   |  |  ]   |  N   |  M   |  ,   |  .   |  :   |  -   |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
| Ctrl | Alt  | GUI  |  ~   | NAV  | Bspc | Del  |  |Enter |Space | NAV  |  $   |  §   | Menu | RAlt |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

`§` sends the ISO key next to left Shift (`KC_NUBS`), so what it types depends on the host layout: § on layouts that put
it there, `\` or `<` on others. `RAlt` doubles as the Compose key with the XKB option `compose:ralt`.

### NAV (hold `NAV`)

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
|      |      |      |      |      |      |                |      |      |      |      |      |      |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|      |      | Back | Btn1 | Btn2 | Fwd  |      |  |      |      |      |      |      |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |  M←  |  M↓  |  M↑  |  M→  | BASE |  | BASE | Left | Down |  Up  |Right |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |  W←  |  W↓  |  W↑  |  W→  | Btn3 |  |      | Home | PgDn | PgUp | End  |      |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

### FUNC, BLE Micro Pro and ZMK firmware (tap FUNC or hold `;`)

`Host1`–`Host5` switch between paired hosts, `Pair` advertises to a new host, `BLE`/`USB` choose where key presses go,
`Batt` types the battery voltage, `Sleep` puts the keyboard in deep sleep.

These keys are all on the left half, and so is the `FUNC` key on the base layer (inner column, middle row). Use that
one to reach them: tap `FUNC`, press the key, then tap `BASE` (same spot) to go back. This README writes it as
"`FUNC`, A". Holding `;` is quicker, but `;` is on the right half, so it only works while the right half is connected
to the left: not when the halves haven't found each other, or the right half is off or out of battery.

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
|      |  F1  |  F2  |  F3  |  F4  |  F5  |                |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|      |Host1 |Host2 |Host3 |Host4 |Host5 |      |  |      | Ins  |ScrLk |PrtSc |Pause |      | F12  |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      | Pair | BLE  | USB  | Batt |      | BASE |  | BASE | Prev | Vol- | Vol+ | Next |      | Play |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      | Rew  | Mute |      |  FF  |      | Stop |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|Sleep |      |      |      |      |      |      |  |      |      |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

### FUNC, Pro Micro firmware

`Mode+`/`Mode-` step through the RGB effects, `RGB` toggles the lighting and `On`/`Off` switch it on and off.

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Sat+ |  F1  |  F2  |  F3  |  F4  |  F5  |                |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
| Sat- |Mode+ |      |      |      |      |      |  |      | Ins  |ScrLk |PrtSc |Pause |      | F12  |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
| Bri+ | RGB  |Mode+ |Mode- | Hue+ | Hue- | BASE |  | BASE | Prev | Vol- | Vol+ | Next |      | Play |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
| Bri- | Off  |      |      |      |      |      |  |      | Rew  | Mute |      |  FF  |      | Stop |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |  On  | Off  |  |      |      |      |      |      |      |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

In the Pro Micro firmware the three LEDs at the top of each half are indicators. On the left they show Caps Lock, Num
Lock and Scroll Lock. On the right they show which of NAV (purple), FUNC (orange) and NUMPAD (green) are on.

### NUMPAD (tap NUMPD)

```
.------.------.------.------.------.------.                .------.------.------.------.------.------.
| Boot |      |      |      |      |      |                |      |NumLk |  /   |  *   |  -   |      |
|------+------+------+------+------+------+------.  .------+------+------+------+------+------+------|
|Reboot|      |      |      |      |      |      |  |      |      |  7   |  8   |  9   |  +   |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|EEClr |      |      |      |      |      | BASE |  | BASE |      |  4   |  5   |  6   |  ,   |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |      |  1   |  2   |  3   |  =   |      |
|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
|      |      |      |      |      |      |      |  |      |      |      |  0   |  .   |  ,   |      |
'------'------'------'------'------|      |      |  |      |      |------'------'------'------'------'
                                   '------'------'  '------'------'
```

The `,` and `=` beside the numbers are the keypad's own comma and equals (`KC_PCMM`, `KC_PEQL`); the `,` on the bottom
row is the normal comma.

## Building

`just ergo68-promicro` and `just ergo68-bmp` build the QMK firmwares; `just build` builds them along with the ErgoDox
ones. The BLE Micro Pro firmware has the hardware config built in (see [Setup](#setup-bootloader-and-config)):
`just ergo68-bmp` builds one file per half for Bluetooth in both halves, `just ergo68-bmp-lpme` the file for the
LPME-IO setup. `just ergo68-bmp-stored` builds `ergo68_ble_micro_pro.uf2`, which uses the config the Web Configurator
stored instead. `just ergo68-zmk` builds the ZMK firmware (see [ZMK](#bluetooth-with-zmk-bmp-boost) below). After changing
`keymap.c`, run `just ergo68-zmk-keymap` to update the ZMK keymap; `just ergo68-zmk` and CI fail if you forget.

## Wired: Pro Micro

These are the controllers that come with the kit (ATmega32U4 Pro Micros with the Caterina bootloader). Both halves get
the same `ergo68_pro_micro.hex`; the PCB tells each half which side it is. Connect one half at a time with USB.

The easiest way to flash is [Remap](https://remap-keys.app/), the web page Yushakobo's build guide uses (Chrome or Edge):

1. Click **Flash Firmware** on Remap's front page and drop `ergo68_pro_micro.hex` into the dialog.
2. Choose the **caterina** bootloader and click **Flash**.
3. When it asks for the serial port, press the half's reset switch, pick the port that appears and click **Connect**.
   The Pro Micro only waits about 8 seconds after a reset, so be quick; Remap shows *successfully* when it's done.

QMK Toolbox ([download](https://github.com/qmk/qmk_toolbox/releases): open the `.hex`, tick *Auto-Flash*, press reset)
works too. On Linux with avrdude installed, `just flash-ergo68-promicro` waits for you to press the reset switch and
flashes `build/ergo68_pro_micro.hex` (give the serial port as `just flash-ergo68-promicro /dev/ttyACM1` if it isn't
`/dev/ttyACM0`).

Once the firmware is on, `Boot` (top-left key on the NUMPAD layer) also starts the bootloader. Connect
the halves with the TRS/TRRS cable and plug the left half into the computer.
[Remap's configure page](https://remap-keys.app/configure) can then edit the keymap live, as with Yushakobo's own
firmware.

## Bluetooth: BLE Micro Pro with QMK

### Setup: bootloader and config

The BMP firmware needs a bootloader and the Ergo68's hardware config (matrix, pins, which half, split or LPME-IO).

The config is built into the firmware (`bmp_config.c`, with the values from sekigon-gonnoc's Ergo68 configs):
`ergo68_left_ble_micro_pro.uf2` and `ergo68_right_ble_micro_pro.uf2` for Bluetooth in both halves,
`ergo68_lpme_ble_micro_pro.uf2` for the LPME-IO setup. Any config stored on the BMP is ignored, so you only need a
bootloader. BMP Boosts come with one: if yours shows a `BLEMICROPRO` drive in bootloader mode, go straight to
[Flashing this firmware](#flashing-this-firmware).

The Web Configurator is still the tool for installing or updating the bootloader. A new bootloader is only needed if
the firmware moves to another major.minor version of the BMP fork (see `bmp_ref` in the `justfile`).

**With the Web Configurator.** This installs the bootloader, the stock firmware and the Ergo68 config. With this
firmware only the bootloader step matters, but the wizard runs all three. You need Chrome
or Edge (it uses Web Serial). The [BMP getting started guide](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/getting_started)
has screenshots of every step (in Japanese).

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
   * with the keyboard on USB, press `Boot` (top-left key on the NUMPAD layer).

   The BMP shows up as a USB drive that contains `INFO_UF2.TXT`.
2. Copy the firmware onto the drive and wait for it to finish; don't unplug it while it copies. Bluetooth in both
   halves: `ergo68_left_ble_micro_pro.uf2` on the left half and `ergo68_right_ble_micro_pro.uf2` on the right. LPME-IO:
   `ergo68_lpme_ble_micro_pro.uf2` on the left half.
3. Unplug and replug. To check which build it runs, open the BMP's USB serial console (e.g. `screen /dev/ttyACM0` on
   Linux, TeraTerm on Windows) and run `version`; it shows the `stonegarden-<commit>` you just built.
4. Bluetooth in both halves: do the same for the right BMP with its own file. Only the left half uses the keymap, but
   keeping both halves on the same build avoids version mismatches between them.

The keymap is part of the firmware but is kept in the BMP's (emulated) EEPROM so that Vial can edit it. The firmware
reloads it from `keymap.c` on the first boot after you flash a changed keymap.

The stock firmware also shows a USB drive called *ergo68* (with `VERSION.TXT` and the config files) whenever the
keyboard is plugged in. `DISABLE_MSC` in `config.h` turns it off; remove that line to get it back. The bootloader's
`BLEMICROPRO` drive, which you flash with, is unaffected.

### Pairing, Bluetooth and USB

* **Both halves on Bluetooth** pair with each other during the first few seconds after both are switched on. Switch on
  the left half, then the right half within a few seconds. If they don't find each other, switch both off and on again
  the same way. No key helps here: until the halves are connected, the right half's keys don't reach the left.
* **Computers and phones:** the keyboard advertises for a few seconds after it starts, or press `Pair` (`FUNC`, A).
  It appears as *ergo68*. Pair with up to five hosts and switch between them with `Host1`–`Host5` (`FUNC`, Q–T).
  To pair another device, turn Bluetooth off on the devices that are already paired first.
* **USB:** plug a USB cable into the left half. If no Bluetooth host is connected it switches to USB by itself;
  otherwise press `USB` (`FUNC`, D). `BLE` (`FUNC`, S) switches back, and unplugging USB returns to Bluetooth.

Pairing problems are usually fixed by removing the pairing on both sides and pairing again; see the
[BMP FAQ](https://sekigon-gonnoc.github.io/BLE-Micro-Pro/#/FAQ).

### Batteries: AAA NiMH

Yushakobo's [AAA battery kit](https://shop.yushakobo.jp/products/8011) for the Ergo68 holds two AAA cells per half and
is meant for the [BMP Boost](https://github.com/sekigon-gonnoc/BLE-Micro-Pro/tree/master/bmp-boost), the BMP version with
a step-up converter that runs from very low voltages (0.7 V and up). The two cells are in series (a half stops when you
take one out), so the BMP Boost's `BT` input gets about 2.6 V from fresh cells, less what the kit's diode and switch
drop. With an LPME-IO only the left half's cells are
used; the right half gets its power through the cable, so you can take the right half's cells out.

NiMH cells work fine. Things to know:

* **Nothing on the keyboard charges them.** Charge them in a NiMH charger.
* **Go by the voltage, not a percentage.** A NiMH cell reads about 1.3–1.4 V straight off the charger, stays around
  1.2 V for most of its charge and is nearly empty around 1.1 V, so two in series go from about 2.6 V to 2.2 V. Any
  percentage meant for 1.5 V alkaline cells will look low.
* **`Batt` (`FUNC`, F)** types a voltage for each half in millivolts, left half first. By default the BMP measures its
  supply, which on a BMP Boost is its `VCC` output, not the `BT` battery input. Measured on both halves with fresh
  cells, before this firmware pointed it elsewhere:

  | Cells (2 in series) | `BT` to `G` | `VCC` to `GND` (pins 21, 23) | `Batt` |
  |---------------------|-------------|------------------------------|--------|
  | 2.59 V              | 2.27 V      | 2.20 V                       | 2200 mV |

  So the kit's diode and switch drop about 0.32 V before the BMP Boost. `VCC` is the step-up converter's output: it
  follows the cells while they're above its set point and stays flat below it, so it can't show low cells. This
  firmware therefore points the measurement at `BT` (`BMP_BATTERY_AIN` in `config.h`; comment it out on an original
  BLE Micro Pro). **Not calibrated yet:** with fresh cells `Batt` should now show either about 2270 mV (the `BT`
  voltage) or about 756 mV (the voltage at the pin, a third of it, after the board's divider). Which one tells how to
  set `BMP_BATTERY_MIN_MV`/`MAX_MV`, the range of the percentage and the voltage below which the BMP switches the half
  off; until then they're set so that neither case can switch a half off. ZMK measures `BT` too.
* **Use low-self-discharge cells** (Eneloop and similar). The keyboard draws so little that ordinary NiMH cells lose a
  noticeable part of their charge to self-discharge.
* **The left half runs down first.** The BMP docs give about 300 µA for the half that talks to the computer and about
  100 µA for the other half.
* **Switch off with the slide switch** when you're not using the keyboard, or press `Sleep` (`FUNC`, left Ctrl).

Auto-sleep is off in the Ergo68 config from the Web Configurator, so this firmware turns it on: on battery, a half goes
to sleep after 30 minutes without a key press (`BMP_AUTO_SLEEP_MINUTES` in `config.h`; comment it out to turn it off).
It never sleeps on USB. It hasn't been tried on this keyboard yet, so check that a key press wakes it up again. Whether
the BMP Boost also cuts the LEDs' supply while asleep is unknown (see [The LEDs](#the-leds)).

## Bluetooth with ZMK (BMP Boost)

[ZMK](https://zmk.dev/) is firmware made for wireless and split keyboards. It runs on the BMP Boost (not the original
BLE Micro Pro) using sekigon-gonnoc's [`bmp_boost` board](https://github.com/sekigon-gonnoc/zmk-component-bmp-boost), and
replaces the QMK firmware for the both-halves Bluetooth setup. The shield in
[`zmk-config/boards/shields/ergo68`](../../zmk-config/boards/shields/ergo68) uses the same pins as QMK's Ergo68, and
[`zmk-config/ergo68.keymap`](../../zmk-config/ergo68.keymap) is generated from the BMP version of `keymap.c`, so both
firmwares have the same layers.

**Getting it:** `just ergo68-zmk` builds `ergo68_left-bmp_boost-zmk.uf2`, `ergo68_right-bmp_boost-zmk.uf2` and
`settings_reset-bmp_boost-zmk.uf2` into `build/`, in the devcontainer or anywhere with west and the Zephyr SDK. Every
push also builds them on GitHub with ZMK's own build workflow (artifact `ergo68-zmk-firmware`).

**Flashing:** the BMP Boost's bootloader takes ZMK's `.uf2` the same way as the QMK one. Put each half in bootloader
mode (battery off, hold the reset switch while plugging in USB, or `Boot` on the NUMPAD layer) and copy
`ergo68_left-bmp_boost-zmk.uf2` onto the left half's drive and `ergo68_right-bmp_boost-zmk.uf2` onto the right's. To
go back to QMK, flash `ergo68_left_ble_micro_pro.uf2` and `ergo68_right_ble_micro_pro.uf2` again.

**How the keys behave in ZMK:**

* `Host1`–`Host5` select Bluetooth profiles 1–5. `Pair` forgets the current profile's pairing so a new device can pair
  with it. `BLE` and `USB` choose where key presses go, as in QMK.
* `Batt`, `Sleep` and `EEClr` do nothing: ZMK reports the battery to the computer instead, goes to sleep by itself
  (see below), and doesn't keep the keymap in EEPROM.
* The mouse keys on NAV (movement, scroll wheel, buttons 1–3) work as in QMK, over Bluetooth and USB. Browser Back and
  Forward are ordinary keys.
* The halves pair with each other on their own. If they don't, or to forget all pairings, flash
  `settings_reset-bmp_boost-zmk.uf2` onto both halves, then the normal firmware again.

**Battery (AAA NiMH):** ZMK reports both halves' battery level to the computer. Out of the box it would treat the BMP
Boost's cell as lithium and always show 0%, so this config uses sekigon-gonnoc's
[NiMH battery module](https://github.com/sekigon-gonnoc/zmk-feature-non-lipo-battery-management) instead
([`boards/bmp_boost.overlay`](../../zmk-config/boards/shields/ergo68/boards/bmp_boost.overlay) and
[`.conf`](../../zmk-config/boards/shields/ergo68/boards/bmp_boost.conf) in the shield):

| Per cell | At `BT` | Reported                                                                        |
|----------|---------|---------------------------------------------------------------------------------|
| 1.30 V+  | 2.28 V  | 100%                                                                            |
| 1.20 V   | 2.08 V  | 50% (where a NiMH cell spends most of its life)                                 |
| 1.10 V   | 1.88 V  | 0%: time to charge                                                              |
| 1.05 V   | 1.78 V  | The half switches itself off to protect the cells, unless it's on USB           |

The two cells are in series and the battery kit's diode and switch drop about 0.32 V before the BMP Boost's `BT` input
(measured on both halves: 2.59 V across the cells, 2.27 V at `BT`), hence the `BT` column. The module measures `BT`
through the board's 1:3 voltage divider, so the thresholds in the config are a third of those. If the low-battery
protection switches a half off, charge or replace its cells, or plug it into USB.

**Sleep:** a half goes to sleep after 15 minutes without a key press, or after a minute of looking for a connection it
doesn't get (the computer or the other half is off). A key press wakes it.

## From an LPME-IO to two BMP Boosts

This is the switch from a BMP Boost on the left with an LPME-IO on the right, to a BMP Boost in each half that talk to
each other over Bluetooth, with the left half paired to your computer.

**Hardware**

1. Put the second BMP Boost in the right half, in place of the LPME-IO, the same way as on the left (13-pin
   con-through, or the kit's 12-pin ones offset by one pin).
2. The right half needs its own battery now. Yushakobo's guide skips the right-hand battery parts for an LPME-IO, so
   solder the AAA holder, Schottky diodes, capacitor and slide switch onto the right bottom plate. Also fit the 2-pin
   con-through between that plate and the PCB (section EX1-2 to EX1-4 of the
   [build guide](https://github.com/yushakobo/build-documents/tree/master/Ergo68)). The AAA battery kit has parts for
   both halves.
3. Keep the TRRS cable, but leave it unplugged in normal use (see [Running out of battery](#running-out-of-battery)).

**Firmware.** Either works and both have the same keymap. ZMK is the better fit for two BMP Boosts on NiMH cells: it
reads the battery properly, sleeps and wakes by itself and was made for Bluetooth splits. QMK has live editing in
Vial/Remap and works with the LPME-IO.

* **QMK (BLE Micro Pro firmware):** flash `ergo68_left_ble_micro_pro.uf2` onto the left half and
  `ergo68_right_ble_micro_pro.uf2` onto the right ([Flashing this firmware](#flashing-this-firmware)). The split config
  is built in, so the left half's stored LPME-IO config doesn't matter and the Web Configurator isn't needed.
* **ZMK:** flash `ergo68_left-bmp_boost-zmk.uf2` onto the left half and `ergo68_right-bmp_boost-zmk.uf2` onto the right
  ([Bluetooth with ZMK](#bluetooth-with-zmk-bmp-boost)). It doesn't use the Web Configurator's config.

**Pairing**

1. *The halves with each other.* With QMK, switch on the left half, then the right half within a few seconds; if they
   don't find each other, switch both off and try again. With ZMK they pair by themselves the first time both are on. If not, flash `settings_reset-bmp_boost-zmk.uf2` onto both and then the normal firmware again.
2. *The computer.* Pair it with the left half: it appears as *ergo68* in the computer's Bluetooth settings, right after
   switching on or after `Pair` (QMK, `FUNC`, A). `Host1`–`Host5` (`FUNC`, Q–T) switch between paired computers.

### Running out of battery

Plug the **left** half into the computer with USB. It runs from USB and sends key presses over it. QMK switches to USB
by itself unless a Bluetooth host is connected (then press `USB`: `FUNC`, D); ZMK prefers USB whenever it's
plugged in.

The halves keep talking over Bluetooth, so the right half only needs power, not a data connection. That's where the TRRS
cable may help: today it carries the left BMP Boost's supply to your LPME-IO. Whether a BMP Boost on the right will run
from that same supply I couldn't confirm. On the BMP Boost that pin is its own roughly 2.4 V output, and its documentation
doesn't say whether it can also be powered through it. To try it safely:

1. Unplug USB and switch **both** halves' batteries off.
2. Connect the TRRS cable. Only plug or unplug it with everything switched off and USB disconnected; Yushakobo warns
   that doing it live can damage the keyboard.
3. Plug the left half into the computer and type on the right half.

If the right half types, that's your fallback; leave the right battery switched off while it's powered over the cable.
If it doesn't, the dependable fallback is a USB charger or power bank on the right half (or swapping in fresh cells).

With ZMK, this clashes with its low-battery switch-off. Powered over the cable, the right half isn't on USB and reads
an empty or switched-off cell, so it switches itself off at once. Setting `CONFIG_ZMK_NON_LIPO_LOW_MV=0` in the
shield's `bmp_boost.conf` avoids that, but then nothing protects the cells. A USB
charger on the right half works either way.

### The LEDs

Unfortunately the LEDs can't be made to work with a BMP Boost, whatever the firmware does. The Ergo68 powers them from
the controller's VCC pin. Yushakobo notes that with a BLE Micro Pro they only light on USB, and the original BLE Micro
Pro deliberately puts about 4.3 V from USB on that pin for this kind of LED. On a BMP Boost that pin is its roughly 2.4 V
supply, on battery and on USB alike. That's below what these addressable LEDs need (typically 3.5 V or more).

They also cost battery even when dark. sekigon-gonnoc's BMP Boost notes warn that LEDs on its VCC output increase power
use considerably, and suggest pulling that pin out of the con-through if the LEDs are already fitted. Doing that also
disconnects the TRRS supply above, so it's a choice between the two. If your left cell already runs down much faster
than expected, this is the likely reason.

The LEDs do work with the kit's Pro Micros (wired), including the indicator LEDs.

## Changing the keymap

* **In `keymap.c` (recommended):** edit, build, flash. The same file is used for every firmware; only the left half of
  FUNC differs between the BMP and Pro Micro builds (`#ifdef PROTOCOL_BMP`). Run `just ergo68-zmk-keymap` afterwards
  to carry the change over to ZMK.
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
* **ZMK:** the shield follows
  [`WisdomStoneWorshiper/zmk-config-Ergo68`](https://github.com/WisdomStoneWorshiper/zmk-config-Ergo68) (a ZMK config
  for the Ergo68 on nice!nano controllers, with the same pins as QMK's definition), and the board comes from
  sekigon-gonnoc's [`zmk-component-bmp-boost`](https://github.com/sekigon-gonnoc/zmk-component-bmp-boost) and its
  [sample config](https://github.com/sekigon-gonnoc/zmk-bmp-boost-sample).

## Troubleshooting

* **BMP: the keymap doesn't change after flashing.** A saved default EEPROM image (`DEFAULT.BIN` on the *ergo68*
  drive, hidden by `DISABLE_MSC`) takes priority over `keymap.c`. Open the BMP's USB serial console (e.g. `screen /dev/ttyACM0` on Linux, TeraTerm on
  Windows), run `default erase`, then press `EEClr` (left key on the third row of the NUMPAD layer).
* **BMP: it doesn't show up as a drive in bootloader mode.** The battery was probably still on, so the BMP never
  restarted. Switch it off and try again.
* **BMP: the batteries run down much faster than expected.** The BMP Boost docs warn that LEDs powered from its VCC pin
  draw a lot of current; see [The LEDs](#the-leds). Compare `Batt` readings for the two halves over a day or two.
* **Pro Micro: only one half types.** Check the TRS/TRRS cable, and that both halves have the same firmware.
* **ZMK: the right half doesn't type.** Flash `settings_reset-bmp_boost-zmk.uf2` onto both halves, then the normal
  firmware, and switch both halves on together so they pair again.

`vial.json` is copied unchanged from the stock BMP firmware (`keyboards/ble_micro_pro/keymaps/vial`), because the fork's
build needs it in the keymap folder.
