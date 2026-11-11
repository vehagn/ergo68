#!/usr/bin/env bash
# Build the BLE Micro Pro firmware (.uf2) with this Ergo68 keymap.
#
# Fetches sekigon-gonnoc/vial-qmk (the QMK/Vial fork for the BLE Micro Pro) into
# .bmp-qmk/, copies this keymap to keyboards/ble_micro_pro/keymaps/ergo68 and runs
# `make ble_micro_pro:ergo68:uf2`. The firmware ends up in build/.
#
# Needs the normal QMK build environment: git, make, arm-none-eabi-gcc and the qmk
# CLI (https://docs.qmk.fm/newbs_getting_started). Every setting below can be
# overridden from the environment, e.g. `BMP_QMK_REF=dev/ble-micro-pro ./build.sh`.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

KEYMAP="${KEYMAP:-ergo68}"
BMP_QMK_REPO="${BMP_QMK_REPO:-https://github.com/sekigon-gonnoc/vial-qmk.git}"
# dev/ble-micro-pro at release bmp-vial 1.4.1 (plus a build fix). The major.minor
# version has to match the bootloader installed by the BLE Micro Pro Web Configurator.
BMP_QMK_REF="${BMP_QMK_REF:-4de02c1aeec5a34f0744a97213499441c508d565}"
BMP_QMK_DIR="${BMP_QMK_DIR:-$here/.bmp-qmk}"

for tool in git make arm-none-eabi-gcc qmk; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "error: '$tool' not found, set up the QMK build environment first" >&2
        exit 1
    fi
done

if [ ! -d "$BMP_QMK_DIR/.git" ]; then
    git init -q "$BMP_QMK_DIR"
    git -C "$BMP_QMK_DIR" remote add origin "$BMP_QMK_REPO"
fi
if [ "$(git -C "$BMP_QMK_DIR" rev-parse -q --verify HEAD 2>/dev/null || true)" != "$BMP_QMK_REF" ]; then
    echo "Fetching $BMP_QMK_REPO ($BMP_QMK_REF)"
    git -C "$BMP_QMK_DIR" fetch -q --depth 1 origin "$BMP_QMK_REF"
    git -C "$BMP_QMK_DIR" checkout -q --force --detach FETCH_HEAD
fi
# printf is the only one of QMK's submodules a BLE Micro Pro build uses.
git -C "$BMP_QMK_DIR" submodule update -q --init --depth 1 lib/printf

keymap_dir="$BMP_QMK_DIR/keyboards/ble_micro_pro/keymaps/$KEYMAP"
rm -rf "$keymap_dir"
mkdir -p "$keymap_dir"
cp "$here"/{keymap.c,layers.h,layout.h,config.h,rules.mk,vial.json} "$keymap_dir/"

# Shown as "Build from:" in VERSION.TXT on the BLE Micro Pro's USB drive.
version="ergo68-$(git -C "$here" describe --always --dirty 2>/dev/null || echo local)_bmp-$(git -C "$BMP_QMK_DIR" rev-parse --short HEAD)"
jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

QMK_HOME="$BMP_QMK_DIR" make -C "$BMP_QMK_DIR" "ble_micro_pro:$KEYMAP:uf2" GIT_DESCRIBE="$version" -j"$jobs"

mkdir -p "$here/build"
cp "$BMP_QMK_DIR/.build/ble_micro_pro_$KEYMAP.uf2" "$here/build/"
echo
echo "Firmware: $here/build/ble_micro_pro_$KEYMAP.uf2 ($version)"
