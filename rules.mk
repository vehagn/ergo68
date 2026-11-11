ifeq ($(strip $(PLATFORM_KEY)), bmp)
    # BLE Micro Pro: same features as the stock firmware (keyboards/ble_micro_pro/keymaps/vial).
    # The keymap lives in EEPROM there, so VIA/Vial must stay enabled.
    VIA_ENABLE = yes
    VIAL_ENABLE = yes
    VIAL_INSECURE = yes
else
    # Pro Micro (yushakobo/ergo68 in mainline QMK): the keymap is compiled in, like on the Ergodox.
    # Link-time optimisation keeps the firmware well within the ATmega32U4's flash.
    LTO_ENABLE = yes
endif
