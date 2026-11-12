# Ergo68

Keymap for a [Yushakobo Ergo68](https://shop.yushakobo.jp/en/products/ergo68), ported from the
[ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) layout. It builds for the kit's Pro Micros (wired),
for [BLE Micro Pros](https://github.com/sekigon-gonnoc/BLE-Micro-Pro) with QMK (Bluetooth), and for BMP Boosts with
[ZMK](https://zmk.dev/) (Bluetooth). See the [top-level README](../../README.md) for the build environment.

## Hardware setups

| Setup                          | Left half     | Right half                                              | Between the halves | To the computer               | Firmware (in `build/` or the CI artifacts)                        |
|--------------------------------|---------------|---------------------------------------------------------|--------------------|-------------------------------|-------------------------------------------------------------------|
| Wired (the kit's chips)        | Pro Micro     | Pro Micro                                               | TRS or TRRS cable  | USB                           | `ergo68_pro_micro.hex` on both halves                             |
| Bluetooth in one half          | BLE Micro Pro | [LPME-IO](https://shop.yushakobo.jp/products/lpme-io2a) | 4-pole TRRS cable  | Bluetooth, or USB (left half) | `ergo68_ble_micro_pro.uf2` on the left half                       |
| Bluetooth in both halves       | BLE Micro Pro | BLE Micro Pro                                           | Bluetooth          | Bluetooth, or USB (left half) | `ergo68_ble_micro_pro.uf2` on both halves                         |
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

### MEDIA, BLE Micro Pro and ZMK firmware (hold `;` or tap MEDIA)

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

`just ergo68-promicro` and `just ergo68-bmp` build the two QMK firmwares; `just build` builds them along with the
ErgoDox ones. The ZMK firmware is built by CI only (see [ZMK](#bluetooth-with-zmk-bmp-boost) below); after changing
`keymap.c`, run `just ergo68-zmk-keymap` to update its keymap. CI fails if you forget.

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

Once the firmware is on, `Boot` (top-left key on the MEDIA, NUMPAD and FUNC layers) also starts the bootloader. Connect
the halves with the TRS/TRRS cable and plug the left half into the computer.
[Remap's configure page](https://remap-keys.app/configure) can then edit the keymap live, as with Yushakobo's own
firmware.

## Bluetooth: BLE Micro Pro with QMK

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
2. Copy `ergo68_ble_micro_pro.uf2` onto the drive and wait for it to finish. Don't unplug it while it copies.
3. Unplug and replug. The drive now contains `VERSION.TXT`, and its `Build from:` line shows the `stonegarden-<commit>`
   you just built.
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

## Bluetooth with ZMK (BMP Boost)

[ZMK](https://zmk.dev/) is firmware made for wireless and split keyboards. It runs on the BMP Boost (not the original
BLE Micro Pro) using sekigon-gonnoc's [`bmp_boost` board](https://github.com/sekigon-gonnoc/zmk-component-bmp-boost), and
replaces the QMK firmware for the both-halves Bluetooth setup. The shield in
[`zmk-config/boards/shields/ergo68`](../../zmk-config/boards/shields/ergo68) uses the same pins as QMK's Ergo68, and
[`zmk-config/ergo68.keymap`](../../zmk-config/ergo68.keymap) is generated from the BMP version of `keymap.c`, so both
firmwares have the same layers.

**Getting it:** every push builds it on GitHub with ZMK's own build workflow. Download the `ergo68-zmk-firmware`
artifact from the run. It isn't built in the devcontainer.

**Flashing:** the BMP Boost's bootloader takes ZMK's `.uf2` the same way as the QMK one. Put each half in bootloader
mode (battery off, hold the reset switch while plugging in USB, or `Boot` on the MEDIA/NUMPAD/FUNC layers) and copy
`ergo68_left-bmp_boost-zmk.uf2` onto the left half's drive and `ergo68_right-bmp_boost-zmk.uf2` onto the right's. To
go back to QMK, flash `ergo68_ble_micro_pro.uf2` again; if the keyboard doesn't come back up, redo the Web Configurator
setup.

**How the keys behave in ZMK:**

* `Host1`–`Host5` select Bluetooth profiles 1–5. `Pair` forgets the current profile's pairing so a new device can pair
  with it. `BLE` and `USB` choose where key presses go, as in QMK.
* `Batt`, `Sleep` and `EEClr` do nothing: ZMK reports the battery to the computer instead, goes to sleep by itself
  after 15 minutes without a key press (a key press wakes it), and doesn't keep the keymap in EEPROM. The mouse
  acceleration keys do nothing either.
* The halves pair with each other on their own. If they don't, or to forget all pairings, flash
  `settings_reset-bmp_boost-zmk.uf2` onto both halves, then the normal firmware again.

**Battery level:** ZMK reports both halves' battery to the computer, but on the BMP Boost it treats the cell as lithium
(0% at 3.45 V and below), so a NiMH AAA cell always shows **0%**. That's only the display; the keyboard keeps working.
sekigon-gonnoc's NiMH battery module would read it properly, but it can't be used on the BMP Boost as it is: it doesn't
account for the board's voltage divider, so it would read a third of the real voltage and switch the keyboard off.

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

**Firmware.** Pick one; both have the same keymap.

* **QMK (BLE Micro Pro firmware):** in the Web Configurator, select **ergo68** *without* *Use with LPME-IO*. Rewrite the
  left half's config with *Is Slave* unchecked and *Is Left* checked; it's currently set up for the LPME-IO. Then set
  up the right BMP Boost (bootloader, firmware, config with *Is Slave* checked and *Is Left* unchecked). Finally flash
  `ergo68_ble_micro_pro.uf2` onto both halves ([Flashing this firmware](#flashing-this-firmware)).
* **ZMK:** flash `ergo68_left-bmp_boost-zmk.uf2` onto the left half and `ergo68_right-bmp_boost-zmk.uf2` onto the right
  ([Bluetooth with ZMK](#bluetooth-with-zmk-bmp-boost)). It doesn't use the Web Configurator's config.

**Pairing**

1. *The halves with each other.* With QMK, switch on the left half, then the right half within a few seconds. If they
   don't find each other, press `Pair` (hold `;` + Q) with both switched on. With ZMK they pair by themselves the first
   time both are on. If not, flash `settings_reset-bmp_boost-zmk.uf2` onto both and then the normal firmware again.
2. *The computer.* Pair it with the left half: it appears as *ergo68* in the computer's Bluetooth settings, right after
   switching on or after `Pair` (QMK). `Host1`–`Host5` (hold `;` + 1–5) switch between paired computers.

### Running out of battery

Plug the **left** half into the computer with USB. It runs from USB and sends key presses over it. QMK switches to USB
by itself unless a Bluetooth host is connected (then press `USB`, hold `;` + E); ZMK prefers USB whenever it's
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
  MEDIA differs between the BMP and Pro Micro builds (`#ifdef PROTOCOL_BMP`). Run `just ergo68-zmk-keymap` afterwards
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

* **BMP: the keymap doesn't change after flashing.** A saved default EEPROM image (`DEFAULT.BIN` on the BMP's drive)
  takes priority over `keymap.c`. Open the BMP's USB serial console (e.g. `screen /dev/ttyACM0` on Linux, TeraTerm on
  Windows), run `default erase`, then press `EEClr` (left key on the second row of the MEDIA or FUNC layer).
* **BMP: it doesn't show up as a drive in bootloader mode.** The battery was probably still on, so the BMP never
  restarted. Switch it off and try again.
* **BMP: the batteries run down much faster than expected.** The BMP Boost docs warn that LEDs powered from its VCC pin
  draw a lot of current; see [The LEDs](#the-leds). Compare `Batt` readings for the two halves over a day or two.
* **Pro Micro: only one half types.** Check the TRS/TRRS cable, and that both halves have the same firmware.
* **ZMK: the right half doesn't type.** Flash `settings_reset-bmp_boost-zmk.uf2` onto both halves, then the normal
  firmware, and switch both halves on together so they pair again.

`vial.json` is copied unchanged from the stock BMP firmware (`keyboards/ble_micro_pro/keymaps/vial`), because the fork's
build needs it in the keymap folder.
