#!/usr/bin/env python3
"""Generate the ZMK keymap for the Ergo68 from its QMK keymap.

Reads keyboards/ergo68/keymap.c (the BLE Micro Pro version of each layer, i.e. the
`#ifdef PROTOCOL_BMP` branch) and layers.h, and prints a ZMK .keymap with the same layers.
The keys stay in LAYOUT_ergo68 order, which zmk-config/boards/shields/ergo68 matches.

    python3 scripts/qmk_to_zmk_keymap.py keyboards/ergo68 > zmk-config/ergo68.keymap

Fails with an error for any QMK keycode it doesn't know how to translate.
"""

import re
import sys
from pathlib import Path

ROW_LENGTHS = [12, 14, 14, 14, 14]  # keys per row in LAYOUT_ergo68

# QMK keycode -> ZMK key name (dt-bindings/zmk/keys.h), used with &kp.
KEYS = {
    **{f"KC_{c}": c for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},
    **{f"KC_{d}": f"N{d}" for d in "0123456789"},
    **{f"KC_F{n}": f"F{n}" for n in range(1, 25)},
    **{f"KC_P{d}": f"KP_N{d}" for d in "0123456789"},
    "KC_ESC": "ESC", "KC_TAB": "TAB", "KC_BSPC": "BSPC", "KC_DEL": "DEL", "KC_ENT": "RET", "KC_SPC": "SPACE",
    "KC_BSLS": "BSLH", "KC_SLSH": "FSLH", "KC_QUOT": "SQT", "KC_GRV": "GRAVE", "KC_EQL": "EQUAL",
    "KC_MINS": "MINUS", "KC_LBRC": "LBKT", "KC_RBRC": "RBKT", "KC_COMM": "COMMA", "KC_DOT": "DOT",
    "KC_SCLN": "SEMI", "KC_COLN": "COLON", "KC_TILD": "TILDE", "KC_DLR": "DOLLAR", "KC_LPRN": "LPAR",
    "KC_RPRN": "RPAR",
    "KC_LSFT": "LSHFT", "KC_RSFT": "RSHFT", "KC_LCTL": "LCTRL", "KC_RCTL": "RCTRL", "KC_LGUI": "LGUI",
    "KC_RGUI": "RGUI", "KC_LALT": "LALT", "KC_RALT": "RALT", "KC_MENU": "K_MENU", "KC_APP": "K_APP",
    "KC_LEFT": "LEFT", "KC_DOWN": "DOWN", "KC_UP": "UP", "KC_RGHT": "RIGHT", "KC_INS": "INS",
    "KC_HOME": "HOME", "KC_END": "END", "KC_PGUP": "PG_UP", "KC_PGDN": "PG_DN", "KC_PSCR": "PSCRN",
    "KC_NUM": "KP_NUM", "KC_PSLS": "KP_SLASH", "KC_PAST": "KP_MULTIPLY", "KC_PMNS": "KP_MINUS",
    "KC_PPLS": "KP_PLUS", "KC_PDOT": "KP_DOT", "KC_PCMM": "KP_COMMA",
    "KC_MPRV": "C_PREV", "KC_MNXT": "C_NEXT", "KC_MUTE": "C_MUTE", "KC_VOLD": "C_VOL_DN",
    "KC_VOLU": "C_VOL_UP", "KC_MRWD": "C_RW", "KC_MFFD": "C_FF", "KC_MPLY": "C_PP", "KC_MSTP": "C_STOP",
    "KC_WBAK": "C_AC_BACK", "KC_WFWD": "C_AC_FORWARD",
}

# QMK keycode -> complete ZMK binding.
BINDINGS = {
    "_______": "&trans", "KC_TRNS": "&trans", "XXXXXXX": "&none", "KC_NO": "&none",
    "QK_BOOT": "&bootloader",
    "EE_CLR": "&none",  # ZMK keeps no EEPROM keymap; flash settings_reset to clear its settings
    "MS_BTN1": "&mkp LCLK", "MS_BTN2": "&mkp RCLK", "MS_BTN3": "&mkp MCLK", "MS_BTN4": "&mkp MB4",
    "MS_BTN5": "&mkp MB5",
    "MS_UP": "&mmv MOVE_UP", "MS_DOWN": "&mmv MOVE_DOWN", "MS_LEFT": "&mmv MOVE_LEFT",
    "MS_RGHT": "&mmv MOVE_RIGHT",
    "MS_WHLU": "&msc SCRL_UP", "MS_WHLD": "&msc SCRL_DOWN", "MS_WHLL": "&msc SCRL_LEFT",
    "MS_WHLR": "&msc SCRL_RIGHT",
    "MS_ACL0": "&none", "MS_ACL1": "&none", "MS_ACL2": "&none",  # ZMK has no acceleration presets
    # BLE Micro Pro wireless keys -> ZMK Bluetooth/output behaviors. ZMK's profiles count from 0.
    **{f"ADV_ID{n}": f"&bt BT_SEL {n - 1}" for n in range(1, 6)},
    "AD_WO_L": "&bt BT_CLR",  # forget the current profile's pairing, so it can pair a new host
    "SEL_BLE": "&out OUT_BLE", "SEL_USB": "&out OUT_USB",
    "BATT_LV": "&none",  # ZMK reports the battery to the computer instead
    "ENT_SLP": "&none",  # ZMK sleeps on its own after CONFIG_ZMK_IDLE_SLEEP_TIMEOUT
}

# QMK modifier wrappers -> ZMK modifier functions.
MODS = {"LSFT": "LS", "RSFT": "RS", "LCTL": "LC", "RCTL": "RC", "LALT": "LA", "RALT": "RA", "LGUI": "LG", "RGUI": "RG"}

# Mod-tap hold modifiers -> the ZMK key that is held.
MOD_TAPS = {"LCA_T": "LC(LALT)", "LCTL_T": "LCTRL", "LSFT_T": "LSHFT", "LALT_T": "LALT", "LGUI_T": "LGUI"}


def strip_comments(src):
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    return re.sub(r"//[^\n]*", "", src)


def layer_names(layers_h):
    body = re.search(r"enum\s+\w+\s*{(.*?)}", strip_comments(layers_h), re.S).group(1)
    return [n.strip() for n in body.split(",") if n.strip()]


def bmp_source(src):
    """Keep the PROTOCOL_BMP side of every `#ifdef PROTOCOL_BMP ... #else ... #endif`."""
    out, stack = [], []
    for line in src.split("\n"):
        t = line.strip()
        if t.startswith("#if"):
            stack.append(("bmp" if t.startswith("#ifdef PROTOCOL_BMP") else "other", True))
            continue
        if t.startswith("#else") and stack:
            kind, _ = stack[-1]
            stack[-1] = (kind, kind != "bmp")
            continue
        if t.startswith("#endif") and stack:
            stack.pop()
            continue
        if all(keep for _, keep in stack):
            out.append(line)
    return "\n".join(out)


def split_args(text):
    args, depth, cur = [], 0, ""
    for ch in text:
        if ch == "," and depth == 0:
            args.append(cur.strip())
            cur = ""
            continue
        depth += {"(": 1, ")": -1}.get(ch, 0)
        cur += ch
    if cur.strip():
        args.append(cur.strip())
    return args


def layers(src):
    for m in re.finditer(r"\[(\w+)\]\s*=\s*LAYOUT_ergo68\(", src):
        i, depth = m.end(), 1
        while depth:
            depth += {"(": 1, ")": -1}.get(src[i], 0)
            i += 1
        yield m.group(1), split_args(src[m.end() : i - 1])


class Translator:
    def __init__(self, defines, layer_index):
        self.defines = defines
        self.layer_index = layer_index

    def layer(self, name):
        if name.isdigit():
            return name
        return str(self.layer_index[name])

    def key(self, token):
        """A QMK keycode -> a ZMK key name (for &kp and as a hold-tap parameter)."""
        token = self.defines.get(token, token)
        if token in KEYS:
            return KEYS[token]
        m = re.fullmatch(r"(\w+)\((.*)\)", token)
        if m and m.group(1) in MODS:
            return f"{MODS[m.group(1)]}({self.key(m.group(2).strip())})"
        raise ValueError(f"no ZMK key for QMK keycode {token!r}")

    def binding(self, token):
        if token in BINDINGS:
            return BINDINGS[token]
        if token in self.defines:
            return self.binding(self.defines[token])
        m = re.fullmatch(r"(\w+)\((.*)\)", token)
        if m:
            fn, args = m.group(1), split_args(m.group(2))
            if fn == "TO":
                return f"&to {self.layer(args[0])}"
            if fn == "TG":
                return f"&tog {self.layer(args[0])}"
            if fn == "MO":
                return f"&mo {self.layer(args[0])}"
            if fn == "LT":
                return f"&lt {self.layer(args[0])} {self.key(args[1])}"
            if fn in MOD_TAPS:
                return f"&mt {MOD_TAPS[fn]} {self.key(args[0])}"
        return f"&kp {self.key(token)}"


def main():
    folder = Path(sys.argv[1] if len(sys.argv) > 1 else "keyboards/ergo68")
    names = layer_names((folder / "layers.h").read_text())
    src = bmp_source(strip_comments((folder / "keymap.c").read_text()))
    defines = dict(re.findall(r"^\s*#\s*define\s+(\w+)\s+(.+?)\s*$", src, re.M))
    tr = Translator(defines, {n: i for i, n in enumerate(names)})

    out = [
        "/*",
        " * Generated from keyboards/ergo68/keymap.c by scripts/qmk_to_zmk_keymap.py. Don't edit it by hand:",
        " * change keymap.c and run `just ergo68-zmk-keymap`.",
        " */",
        "",
        "#include <behaviors.dtsi>",
        "#include <dt-bindings/zmk/bt.h>",
        "#include <dt-bindings/zmk/keys.h>",
        "#include <dt-bindings/zmk/outputs.h>",
        "#include <dt-bindings/zmk/pointing.h>",
        "",
        "/ {",
        "    keymap {",
        '        compatible = "zmk,keymap";',
    ]
    seen = []
    for name, tokens in layers(src):
        if len(tokens) != sum(ROW_LENGTHS):
            raise ValueError(f"{name}: {len(tokens)} keys, expected {sum(ROW_LENGTHS)}")
        bindings = [tr.binding(t) for t in tokens]
        width = max(len(b) for b in bindings) + 1
        label = name.lstrip("_")
        out += ["", f"        {label.lower()} {{", f'            display-name = "{label}";', "            bindings = <"]
        i = 0
        for n in ROW_LENGTHS:
            out.append("                " + " ".join(b.ljust(width) for b in bindings[i : i + n]).rstrip())
            i += n
        out += ["            >;", "        };"]
        seen.append(name)
    if seen != names:
        raise ValueError(f"layers in keymap.c {seen} don't match layers.h {names}")
    out += ["    };", "};", ""]
    sys.stdout.write("\n".join(out))


if __name__ == "__main__":
    try:
        main()
    except ValueError as e:
        sys.exit(f"error: {e}")
