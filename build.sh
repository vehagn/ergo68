#!/usr/bin/env bash
# Build the Ergo68 firmware from this keymap.
#
#   ./build.sh            both firmwares
#   ./build.sh bmp        BLE Micro Pro (.uf2): Bluetooth in one or both halves, or USB
#   ./build.sh promicro   Pro Micro (.hex): wired, TRRS cable between the halves
#
# Each target fetches its QMK tree at a pinned version, copies this keymap into it as "ergo68"
# and runs make. The firmware files end up in build/.
#   bmp       sekigon-gonnoc/vial-qmk (BLE Micro Pro fork)  make ble_micro_pro:ergo68:uf2
#   promicro  qmk/qmk_firmware                               make yushakobo/ergo68:ergo68
#
# Needs the normal QMK build environment: git, make, arm-none-eabi-gcc, avr-gcc and the qmk CLI.
# The devcontainer in .devcontainer/ has all of it. Every setting below can be overridden from
# the environment, e.g. `BMP_QMK_REF=dev/ble-micro-pro ./build.sh bmp`.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

KEYMAP="${KEYMAP:-ergo68}"

BMP_QMK_REPO="${BMP_QMK_REPO:-https://github.com/sekigon-gonnoc/vial-qmk.git}"
# dev/ble-micro-pro at release bmp-vial 1.4.1 (plus a build fix). The major.minor version has
# to match the bootloader installed by the BLE Micro Pro Web Configurator.
BMP_QMK_REF="${BMP_QMK_REF:-4de02c1aeec5a34f0744a97213499441c508d565}"
BMP_QMK_DIR="${BMP_QMK_DIR:-$here/.qmk/bmp}"

MAINLINE_QMK_REPO="${MAINLINE_QMK_REPO:-https://github.com/qmk/qmk_firmware.git}"
MAINLINE_QMK_REF="${MAINLINE_QMK_REF:-180f394b489c238cb231c2bf8b080262155e03c0}" # 0.34.7
MAINLINE_QMK_DIR="${MAINLINE_QMK_DIR:-$here/.qmk/mainline}"

jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

require() {
    for tool in "$@"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            echo "error: '$tool' not found, set up the QMK build environment (or use the devcontainer)" >&2
            exit 1
        fi
    done
}

# fetch DIR REPO REF SUBMODULE...: shallow checkout of REF in DIR, plus the given submodules.
fetch() {
    local dir="$1" repo="$2" ref="$3"
    shift 3
    if [ ! -d "$dir/.git" ]; then
        mkdir -p "$dir"
        git init -q "$dir"
        git -C "$dir" remote add origin "$repo"
    fi
    if [ "$(git -C "$dir" rev-parse -q --verify HEAD 2>/dev/null || true)" != "$ref" ]; then
        echo "Fetching $repo ($ref)"
        git -C "$dir" fetch -q --depth 1 origin "$ref"
        git -C "$dir" checkout -q --force --detach FETCH_HEAD
    fi
    git -C "$dir" submodule update -q --init --depth 1 "$@"
}

# install_keymap QMK_DIR KEYBOARD: copy this keymap to keyboards/KEYBOARD/keymaps/$KEYMAP.
install_keymap() {
    local dest="$1/keyboards/$2/keymaps/$KEYMAP"
    rm -rf "$dest"
    mkdir -p "$dest"
    cp "$here"/{keymap.c,layers.h,layout.h,config.h,rules.mk,vial.json} "$dest/"
}

# qmk_env QMK_DIR COMMAND...: run COMMAND for the QMK tree in QMK_DIR, with that tree's own
# virtualenv first on PATH if it has one (see ensure_python_env). QMK_USERSPACE is unset because
# the QMK container image points it at a folder that doesn't exist here, and mainline QMK would
# copy firmware there.
qmk_env() {
    local dir="$1"
    shift
    local path="$PATH"
    if [ -x "$dir/.venv/bin/qmk" ]; then
        path="$dir/.venv/bin:$PATH"
    fi
    env -u QMK_USERSPACE PATH="$path" QMK_HOME="$dir" "$@"
}

# ensure_python_env QMK_DIR: the qmk CLI refuses to run unless every package in the tree's
# requirements.txt can be imported. The BLE Micro Pro fork is based on an older QMK and lists
# packages that current qmk CLI installs (including QMK's container image) no longer have. In that
# case set up a virtualenv in QMK_DIR/.venv with the qmk CLI and the tree's requirements, rather
# than changing the qmk CLI's own environment.
ensure_python_env() {
    local dir="$1"
    if qmk_env "$dir" qmk hello >/dev/null 2>&1; then
        return
    fi
    echo "The qmk CLI lacks Python packages $dir/requirements.txt needs; installing them in $dir/.venv"
    # The interpreter the qmk CLI runs on, from its shebang line.
    local python
    python="$(sed -n '1s/^#!//p' "$(command -v qmk)")"
    ${python:-python3} -m venv "$dir/.venv"
    "$dir/.venv/bin/python" -m pip install --quiet qmk -r "$dir/requirements.txt"
    qmk_env "$dir" qmk hello >/dev/null
}

# qmk_make QMK_DIR ARGS...: run make in QMK_DIR.
qmk_make() {
    local dir="$1"
    shift
    ensure_python_env "$dir"
    qmk_env "$dir" make -C "$dir" "$@" -j"$jobs"
}

build_bmp() {
    require git make qmk arm-none-eabi-gcc
    fetch "$BMP_QMK_DIR" "$BMP_QMK_REPO" "$BMP_QMK_REF" lib/printf
    install_keymap "$BMP_QMK_DIR" ble_micro_pro
    # Shown as "Build from:" in VERSION.TXT on the BLE Micro Pro's USB drive.
    local version
    version="ergo68-$(git -C "$here" describe --always --dirty 2>/dev/null || echo local)_bmp-$(git -C "$BMP_QMK_DIR" rev-parse --short HEAD)"
    qmk_make "$BMP_QMK_DIR" "ble_micro_pro:$KEYMAP:uf2" GIT_DESCRIBE="$version"
    cp "$BMP_QMK_DIR/.build/ble_micro_pro_$KEYMAP.uf2" "$here/build/"
    built+=("build/ble_micro_pro_$KEYMAP.uf2  (BLE Micro Pro, $version)")
}

build_promicro() {
    require git make qmk avr-gcc
    fetch "$MAINLINE_QMK_DIR" "$MAINLINE_QMK_REPO" "$MAINLINE_QMK_REF" lib/lufa lib/printf
    install_keymap "$MAINLINE_QMK_DIR" yushakobo/ergo68
    qmk_make "$MAINLINE_QMK_DIR" "yushakobo/ergo68:$KEYMAP"
    cp "$MAINLINE_QMK_DIR/.build/yushakobo_ergo68_$KEYMAP.hex" "$here/build/"
    built+=("build/yushakobo_ergo68_$KEYMAP.hex  (Pro Micro)")
}

targets=("$@")
[ ${#targets[@]} -gt 0 ] || targets=(bmp promicro)
for target in "${targets[@]}"; do
    case "$target" in
        bmp | promicro) ;;
        *)
            echo "usage: $0 [bmp] [promicro]" >&2
            exit 2
            ;;
    esac
done

mkdir -p "$here/build"
built=()
for target in "${targets[@]}"; do
    "build_$target"
done

echo
printf 'Built %s\n' "${built[@]}"
