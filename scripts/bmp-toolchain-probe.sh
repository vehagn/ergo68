#!/usr/bin/env bash
# Temporary diagnostics: the BLE Micro Pro firmware built with QMK's current GCC 15 doesn't start.
# Build sekigon's stock 1.4.0 firmware with several Arm GCC releases, compare each with the official
# 1.4.0 binary to find the compiler he used, and build the stonegarden firmware with each.
set -euo pipefail
cd "$(dirname "$0")/.."
out=build/bmp-toolchains
tc_root="${BMP_TOOLCHAINS_DIR:-/qmk/toolchains}"
mkdir -p "$out" "$tc_root"

declare -A toolchains=(
    [gcc-10.3]="https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2"
    [gcc-12.3]="https://developer.arm.com/-/media/Files/downloads/gnu/12.3.rel1/binrel/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz"
    [gcc-13.3]="https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz"
    [gcc-14.2]="https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz"
)

curl -sSfL -o "$out/official_1_4_0.bin" \
    https://raw.githubusercontent.com/sekigon-gonnoc/BLE-Micro-Pro-WebConfigurator/master/public/application/ble_micro_pro_vial_1_4_0.bin

bmp_dir="${BMP_QMK_DIR:-.qmk/bmp}"
bmp140_dir="$bmp_dir-1.4.0"
just _fetch "$bmp_dir" https://github.com/sekigon-gonnoc/vial-qmk.git 4de02c1aeec5a34f0744a97213499441c508d565 lib/printf
just _fetch "$bmp140_dir" https://github.com/sekigon-gonnoc/vial-qmk.git be3adb0a11ab751d2cdf35ba26f5b9011d937ec2 lib/printf
just _install-keymap "$bmp_dir" ble_micro_pro ergo68

for name in "${!toolchains[@]}"; do
    dir="$tc_root/$name"
    if [ ! -x "$dir/bin/arm-none-eabi-gcc" ]; then
        echo "Downloading $name"
        mkdir -p "$dir"
        url="${toolchains[$name]}"
        case "$url" in *.bz2) z=-j ;; *) z=-J ;; esac
        curl -sSfL "$url" | tar -x "$z" -C "$dir" --strip-components=1 -f -
    fi
    prefix="$dir/bin/arm-none-eabi-"
    "${prefix}gcc" --version | head -1

    # One toolchain failing to build shouldn't hide the others' results.
    rm -rf "$bmp140_dir/.build"
    if just _make "$bmp140_dir" ble_micro_pro:vial:uf2 "TOOLCHAIN=$prefix"; then
        "${prefix}objcopy" -O binary "$bmp140_dir/.build/ble_micro_pro_vial.elf" "$out/stock_1_4_0_$name.bin"
        cp "$bmp140_dir/.build/ble_micro_pro_vial.uf2" "$out/stock_1_4_0_$name.uf2" || true
    else
        echo "::warning::stock 1.4.0 failed to build with $name"
    fi

    rm -rf "$bmp_dir/.build"
    if just _make "$bmp_dir" ble_micro_pro:stonegarden:uf2 "TOOLCHAIN=$prefix"; then
        cp "$bmp_dir/.build/ble_micro_pro_stonegarden.uf2" "$out/ergo68_ble_micro_pro_$name.uf2"
    else
        echo "::warning::stonegarden failed to build with $name"
    fi
done

python3 -I - "$out" <<'PY'
import pathlib, sys
out = pathlib.Path(sys.argv[1])
official = (out / "official_1_4_0.bin").read_bytes()
print(f"official 1.4.0: {len(official)} bytes")
for p in sorted(out.glob("stock_1_4_0_*.bin")):
    b = p.read_bytes()
    diff = sum(x != y for x, y in zip(b, official)) + abs(len(b) - len(official))
    print(f"{p.name}: {len(b)} bytes, {diff} bytes differ from official")
PY
