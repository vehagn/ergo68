# Stonegarden keyboards

My [ergodox_stonegarden](https://github.com/vehagn/ergodox_stonegarden) layout for the keyboards I use, with one build
setup for all of them: a devcontainer, [just](https://github.com/casey/just) recipes and GitHub Actions.

| Keyboard                                         | Controllers                         | Firmware       | Build with              | File                                            | Flash with                                         |
|--------------------------------------------------|-------------------------------------|----------------|-------------------------|-------------------------------------------------|----------------------------------------------------|
| [Ergo68](keyboards/ergo68/README.md)             | Pro Micro (wired)                   | QMK            | `just ergo68-promicro`  | `ergo68_pro_micro.hex`                          | Remap, QMK Toolbox or `just flash-ergo68-promicro` |
| [Ergo68](keyboards/ergo68/README.md)             | BLE Micro Pro or BMP Boost          | QMK (BMP fork) | `just ergo68-bmp`       | `ergo68_ble_micro_pro.uf2`                      | Copy it onto the BMP's USB drive                   |
| [Ergo68](keyboards/ergo68/README.md)             | BMP Boost                           | ZMK            | GitHub Actions          | `ergo68_left/right-bmp_boost-zmk.uf2`           | Copy it onto the BMP's USB drive                   |
| [ErgoDox Infinity](keyboards/ergodox/README.md)  | Built in (Kinetis MK20)             | QMK            | `just ergodox-infinity` | `ergodox_infinity.bin`                          | `just flash-ergodox-infinity` (dfu-util)           |
| [ErgoDox EZ](keyboards/ergodox/README.md)        | Built in (ATmega32U4)               | QMK            | `just ergodox-ez`       | `ergodox_ez_base.hex`                           | Keymapp, QMK Toolbox or `just flash-ergodox-ez`    |

## Building

`just build` builds every QMK firmware into `build/`, and `just` on its own lists all recipes. Each build fetches its QMK
tree at a pinned version, copies the keymap from `keyboards/` into it as `stonegarden`, and runs QMK's `make`.

### In the devcontainer (recommended)

The [devcontainer](.devcontainer/devcontainer.json) is based on QMK's own build image and has just installed, so you
don't need QMK or the compilers on your machine. You need Docker and VS Code with the
[Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension, or a
GitHub Codespace.

1. Open the repo in VS Code and run **Dev Containers: Reopen in Container**.
2. Run `just build` (or a single recipe such as `just ergodox-infinity`) in the terminal, or **Terminal → Run Build
   Task** (Ctrl+Shift+B).
3. The firmware appears in `build/` in the repo folder on your own machine. In a Codespace, right-click the file in the
   Explorer and choose **Download**.

The QMK trees live in a Docker volume mounted at `/qmk`, so container rebuilds don't download them again. The BLE Micro
Pro fork is based on an older QMK that wants Python packages the current QMK image lacks; the build installs those in a
virtualenv inside the fork's folder the first time. Flashing needs the USB device, so run the `flash-*` recipes (or
Remap, QMK Toolbox, …) on your own machine, not in the container.

### On GitHub

Every push runs [the build workflow](.github/workflows/build.yml). It builds the QMK firmwares in the same devcontainer
(artifact `stonegarden-qmk-firmware`) and the Ergo68's ZMK firmware with ZMK's own workflow (artifact
`ergo68-zmk-firmware`). It also checks that the generated ZMK keymap matches `keyboards/ergo68/keymap.c`.

### Without the devcontainer

With the [QMK build environment](https://docs.qmk.fm/newbs_getting_started) and just (`brew install just`,
`apt install just`, …) installed, the recipes also work directly on your machine; they put the QMK trees in `.qmk/`.

Or do it by hand, like ergodox_stonegarden: link a keymap folder into a QMK checkout and run `make`. For example, for
the ErgoDox in your normal `qmk_firmware` checkout, with this repo cloned to `~/stonegarden`:

```bash
ln -s ~/stonegarden/keyboards/ergodox keyboards/input_club/ergodox_infinity/keymaps/stonegarden
```

```bash
make input_club/ergodox_infinity:stonegarden:dfu-util
```

The other targets are `yushakobo/ergo68:stonegarden` and `ergodox_ez/base:stonegarden` (with `keyboards/ergo68` and
`keyboards/ergodox` linked into those keyboards' `keymaps/`), and `ble_micro_pro:stonegarden:uf2` in a checkout of the
[BLE Micro Pro fork](https://github.com/sekigon-gonnoc/vial-qmk/tree/dev/ble-micro-pro).

## Repository layout

```
keyboards/ergo68/     Ergo68 keymap (QMK), for Pro Micros and BLE Micro Pros
keyboards/ergodox/    ErgoDox keymap (QMK), for the ErgoDox Infinity and ErgoDox EZ
zmk-config/           ZMK config for the Ergo68 on BMP Boosts; ergo68.keymap is generated
scripts/              qmk_to_zmk_keymap.py, which generates the ZMK keymap from keyboards/ergo68
justfile              build and flash recipes
.devcontainer/        the build environment
.github/workflows/    CI
```

Pinned versions: mainline QMK 0.34.7 and the BLE Micro Pro fork at bmp-vial 1.4.1 (in the `justfile`); ZMK, the
`bmp_boost` board and its battery module (in `zmk-config/west.yml` and `.github/workflows/build.yml`).
