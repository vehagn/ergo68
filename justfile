# Firmware for the stonegarden layout on the Ergo68 and the ErgoDox. Run `just` to list the recipes.
#
#   Ergo68, BLE Micro Pro       sekigon-gonnoc/vial-qmk  make ble_micro_pro:stonegarden:uf2
#   Ergo68, Pro Micro           qmk/qmk_firmware         make yushakobo/ergo68:stonegarden
#   ErgoDox Infinity            qmk/qmk_firmware         make input_club/ergodox_infinity:stonegarden
#   ErgoDox EZ                  qmk/qmk_firmware         make ergodox_ez/base:stonegarden
#   Ergo68, BMP Boost with ZMK  built by CI with ZMK's own workflow, from zmk-config/
#
# Each QMK build fetches its QMK tree at a pinned version, copies the keymap from keyboards/ into it
# as "stonegarden" and runs make. Needs git, make, the qmk CLI, arm-none-eabi-gcc and avr-gcc; the
# devcontainer in .devcontainer/ has all of them. Flashing needs the USB device, so run the flash-*
# recipes on your own machine. The settings below can be overridden from the environment or the
# command line, e.g. `just ez_variant=glow ergodox-ez`.

set shell := ["bash", "-euo", "pipefail", "-c"]
set positional-arguments

keymap := "stonegarden"
root := justfile_directory()
build_dir := root / "build"

bmp_repo := env_var_or_default("BMP_QMK_REPO", "https://github.com/sekigon-gonnoc/vial-qmk.git")
# dev/ble-micro-pro at release bmp-vial 1.4.1 (plus a build fix). The major.minor version has to
# match the bootloader installed by the BLE Micro Pro Web Configurator.
bmp_ref := env_var_or_default("BMP_QMK_REF", "4de02c1aeec5a34f0744a97213499441c508d565")
bmp_dir := env_var_or_default("BMP_QMK_DIR", root / ".qmk/bmp")

mainline_repo := env_var_or_default("MAINLINE_QMK_REPO", "https://github.com/qmk/qmk_firmware.git")
# Release 0.34.7
mainline_ref := env_var_or_default("MAINLINE_QMK_REF", "180f394b489c238cb231c2bf8b080262155e03c0")
mainline_dir := env_var_or_default("MAINLINE_QMK_DIR", root / ".qmk/mainline")

# ErgoDox EZ model: base, glow (RGB matrix) or shine (RGB underglow).
ez_variant := env_var_or_default("ERGODOX_EZ_VARIANT", "base")

ergo68_bmp_target := "ble_micro_pro:" + keymap + ":uf2"
ergo68_promicro_target := "yushakobo/ergo68:" + keymap
infinity_target := "input_club/ergodox_infinity:" + keymap
ez_target := "ergodox_ez/" + ez_variant + ":" + keymap

ergo68_uf2 := build_dir / "ergo68_ble_micro_pro.uf2"
ergo68_hex := build_dir / "ergo68_pro_micro.hex"
infinity_bin := build_dir / "ergodox_infinity.bin"
ez_hex := build_dir / "ergodox_ez_" + ez_variant + ".hex"
zmk_keymap := root / "zmk-config/ergo68.keymap"

# List the recipes
default:
    @just --list --unsorted

# Build every QMK firmware into build/
build: ergo68-bmp ergo68-promicro ergodox-infinity ergodox-ez

# Ergo68 with BLE Micro Pros: build/ergo68_ble_micro_pro.uf2
ergo68-bmp: (_fetch bmp_dir bmp_repo bmp_ref "lib/printf") (_install-keymap bmp_dir "ble_micro_pro" "ergo68") (_make bmp_dir ergo68_bmp_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(bmp_dir / ".build" / "ble_micro_pro_" + keymap + ".uf2") }} {{ quote(ergo68_uf2) }}
    @echo "Built {{ ergo68_uf2 }}"

# Ergo68 with Pro Micros (the kit's controllers): build/ergo68_pro_micro.hex
ergo68-promicro: (_fetch mainline_dir mainline_repo mainline_ref "lib/lufa" "lib/printf") (_install-keymap mainline_dir "yushakobo/ergo68" "ergo68") (_make mainline_dir ergo68_promicro_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(mainline_dir / ".build" / "yushakobo_ergo68_" + keymap + ".hex") }} {{ quote(ergo68_hex) }}
    @echo "Built {{ ergo68_hex }}"

# ErgoDox Infinity: build/ergodox_infinity.bin
ergodox-infinity: (_fetch mainline_dir mainline_repo mainline_ref "lib/chibios" "lib/chibios-contrib" "lib/printf") (_install-keymap mainline_dir "input_club/ergodox_infinity" "ergodox") (_make mainline_dir infinity_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(mainline_dir / ".build" / "input_club_ergodox_infinity_" + keymap + ".bin") }} {{ quote(infinity_bin) }}
    @echo "Built {{ infinity_bin }}"

# ErgoDox EZ: build/ergodox_ez_base.hex (set ez_variant=glow or shine for those models)
ergodox-ez: (_fetch mainline_dir mainline_repo mainline_ref "lib/lufa" "lib/printf") (_install-keymap mainline_dir "ergodox_ez" "ergodox") (_make mainline_dir ez_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(mainline_dir / ".build" / "ergodox_ez_" + ez_variant + "_" + keymap + ".hex") }} {{ quote(ez_hex) }}
    @echo "Built {{ ez_hex }}"

# Regenerate the ZMK keymap (zmk-config/ergo68.keymap) from keyboards/ergo68/keymap.c
ergo68-zmk-keymap:
    #!/usr/bin/env bash
    set -euo pipefail
    tmp="$(mktemp)"
    python3 {{ quote(root / "scripts/qmk_to_zmk_keymap.py") }} {{ quote(root / "keyboards/ergo68") }} > "$tmp"
    mv "$tmp" {{ quote(zmk_keymap) }}
    echo "Wrote {{ zmk_keymap }}"

# Check that zmk-config/ergo68.keymap is up to date with keyboards/ergo68/keymap.c
check-zmk-keymap:
    #!/usr/bin/env bash
    set -euo pipefail
    if ! python3 {{ quote(root / "scripts/qmk_to_zmk_keymap.py") }} {{ quote(root / "keyboards/ergo68") }} | diff -u {{ quote(zmk_keymap) }} -; then
        echo "zmk-config/ergo68.keymap is out of date: run 'just ergo68-zmk-keymap'" >&2
        exit 1
    fi

# Flash build/ergo68_pro_micro.hex with avrdude; waits for you to press the reset switch
flash-ergo68-promicro port="/dev/ttyACM0":
    #!/usr/bin/env bash
    set -euo pipefail
    echo "Press the reset switch on the half you want to flash (waiting for $1)"
    until [ -e "$1" ]; do sleep 0.2; done
    sleep 0.5
    avrdude -p atmega32u4 -c avr109 -P "$1" -U flash:w:{{ quote(ergo68_hex) }}:i

# Flash build/ergodox_infinity.bin with dfu-util; waits for you to press a half's program button
flash-ergodox-infinity:
    #!/usr/bin/env bash
    set -euo pipefail
    echo "Press the program button on the back of the half you want to flash"
    until dfu-util -l 2>/dev/null | grep -q "Found DFU"; do sleep 0.5; done
    dfu-util -D {{ quote(infinity_bin) }}

# Flash build/ergodox_ez_base.hex with teensy_loader_cli; waits for you to press the reset button
flash-ergodox-ez:
    @echo "Press the reset button on the ErgoDox EZ"
    teensy_loader_cli -mmcu=atmega32u4 -w -v {{ quote(ez_hex) }}

# Delete build/ (the QMK trees in .qmk/ stay, so the next build doesn't fetch them again)
clean:
    rm -rf {{ quote(build_dir) }}

# Shallow checkout of REF from REPO in DIR, plus the given submodules
_fetch dir repo ref *submodules:
    #!/usr/bin/env bash
    set -euo pipefail
    dir="$1" repo="$2" ref="$3"
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

# Copy keyboards/SOURCE into the QMK tree in DIR as keyboards/KEYBOARD/keymaps/stonegarden
_install-keymap dir keyboard source:
    @rm -rf "$1/keyboards/$2/keymaps/{{ keymap }}"
    @mkdir -p "$1/keyboards/$2/keymaps/{{ keymap }}"
    @cp -R {{ quote(root / "keyboards") }}/"$3"/. "$1/keyboards/$2/keymaps/{{ keymap }}/"

# Run make in the QMK tree in DIR
_make dir *args:
    #!/usr/bin/env bash
    set -euo pipefail
    dir="$1"
    shift

    # Run a command for this tree, with its own virtualenv (if any) first on PATH. QMK_USERSPACE is
    # unset because the QMK container image points it at a folder that doesn't exist, and mainline
    # QMK would copy firmware there.
    qmk_env() {
        local path="$PATH"
        if [ -x "$dir/.venv/bin/qmk" ]; then
            path="$dir/.venv/bin:$PATH"
        fi
        env -u QMK_USERSPACE PATH="$path" QMK_HOME="$dir" "$@"
    }

    # The qmk CLI refuses to run unless every package in the tree's requirements.txt can be
    # imported. The BMP fork is based on an older QMK and lists packages that current qmk CLI
    # installs (including QMK's container image) no longer have. In that case set up a virtualenv
    # in the tree with the qmk CLI and those requirements, rather than changing the qmk CLI's own
    # environment.
    if ! qmk_env qmk hello >/dev/null 2>&1; then
        echo "The qmk CLI lacks Python packages $dir/requirements.txt needs; installing them in $dir/.venv"
        python="$(sed -n '1s/^#!//p' "$(command -v qmk)")" # the qmk CLI's own interpreter
        ${python:-python3} -m venv "$dir/.venv"
        "$dir/.venv/bin/python" -m pip install --quiet qmk -r "$dir/requirements.txt"
        qmk_env qmk hello >/dev/null
    fi

    # Shown as "Build from:" in VERSION.TXT on a BLE Micro Pro's USB drive (mainline ignores it).
    version="{{ keymap }}-$(git -C {{ quote(root) }} describe --always --dirty 2>/dev/null || echo local)_$(git -C "$dir" rev-parse --short HEAD)"
    qmk_env make -C "$dir" "$@" GIT_DESCRIBE="$version" -j{{ num_cpus() }}
