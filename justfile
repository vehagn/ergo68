# Build the Ergo68 firmware from this keymap. Run `just` to list the recipes.
#
#   BLE Micro Pro (.uf2): sekigon-gonnoc/vial-qmk (the BMP fork)  make ble_micro_pro:ergo68:uf2
#   Pro Micro (.hex):     qmk/qmk_firmware                         make yushakobo/ergo68:ergo68
#
# Each build fetches its QMK tree at a pinned version, copies this keymap into it as "ergo68" and
# runs make. Needs git, make, the qmk CLI, arm-none-eabi-gcc and avr-gcc; the devcontainer in
# .devcontainer/ has all of them. The settings below can be overridden from the environment or the
# command line, e.g. `just bmp_ref=dev/ble-micro-pro bmp`.

set shell := ["bash", "-euo", "pipefail", "-c"]
set positional-arguments

keymap := "ergo68"
build_dir := justfile_directory() / "build"

bmp_repo := env_var_or_default("BMP_QMK_REPO", "https://github.com/sekigon-gonnoc/vial-qmk.git")
# dev/ble-micro-pro at release bmp-vial 1.4.1 (plus a build fix). The major.minor version has to
# match the bootloader installed by the BLE Micro Pro Web Configurator.
bmp_ref := env_var_or_default("BMP_QMK_REF", "4de02c1aeec5a34f0744a97213499441c508d565")
bmp_dir := env_var_or_default("BMP_QMK_DIR", justfile_directory() / ".qmk/bmp")

mainline_repo := env_var_or_default("MAINLINE_QMK_REPO", "https://github.com/qmk/qmk_firmware.git")
# Release 0.34.7
mainline_ref := env_var_or_default("MAINLINE_QMK_REF", "180f394b489c238cb231c2bf8b080262155e03c0")
mainline_dir := env_var_or_default("MAINLINE_QMK_DIR", justfile_directory() / ".qmk/mainline")

bmp_target := "ble_micro_pro:" + keymap + ":uf2"
promicro_target := "yushakobo/ergo68:" + keymap
uf2 := build_dir / "ble_micro_pro_" + keymap + ".uf2"
hex := build_dir / "yushakobo_ergo68_" + keymap + ".hex"

# List the recipes
default:
    @just --list --unsorted

# Build both firmwares into build/
build: bmp promicro

# Build the BLE Micro Pro firmware: build/ble_micro_pro_ergo68.uf2
bmp: (_fetch bmp_dir bmp_repo bmp_ref "lib/printf") (_install-keymap bmp_dir "ble_micro_pro") (_make bmp_dir bmp_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(bmp_dir / ".build" / "ble_micro_pro_" + keymap + ".uf2") }} {{ quote(uf2) }}
    @echo "Built {{ uf2 }}"

# Build the Pro Micro firmware: build/yushakobo_ergo68_ergo68.hex
promicro: (_fetch mainline_dir mainline_repo mainline_ref "lib/lufa" "lib/printf") (_install-keymap mainline_dir "yushakobo/ergo68") (_make mainline_dir promicro_target)
    @mkdir -p {{ quote(build_dir) }}
    @cp {{ quote(mainline_dir / ".build" / "yushakobo_ergo68_" + keymap + ".hex") }} {{ quote(hex) }}
    @echo "Built {{ hex }}"

# Flash the Pro Micro firmware with avrdude, on your own machine (not in the devcontainer)
flash-promicro port="/dev/ttyACM0":
    #!/usr/bin/env bash
    set -euo pipefail
    echo "Press the reset switch on the half you want to flash (waiting for $1)"
    until [ -e "$1" ]; do sleep 0.2; done
    sleep 0.5
    avrdude -p atmega32u4 -c avr109 -P "$1" -U flash:w:{{ quote(hex) }}:i

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

# Copy this keymap into the QMK tree in DIR as keyboards/KEYBOARD/keymaps/ergo68
_install-keymap dir keyboard:
    @rm -rf "$1/keyboards/$2/keymaps/{{ keymap }}"
    @mkdir -p "$1/keyboards/$2/keymaps/{{ keymap }}"
    @cp {{ quote(justfile_directory()) }}/{keymap.c,layers.h,layout.h,config.h,rules.mk,vial.json} "$1/keyboards/$2/keymaps/{{ keymap }}/"

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
    version="{{ keymap }}-$(git -C {{ quote(justfile_directory()) }} describe --always --dirty 2>/dev/null || echo local)_$(git -C "$dir" rev-parse --short HEAD)"
    qmk_env make -C "$dir" "$@" GIT_DESCRIBE="$version" -j{{ num_cpus() }}
