# Both builds keep the keymap in EEPROM so it can also be edited live: with Vial/Remap on the
# BLE Micro Pro, with Remap/VIA on the Pro Micro. keymap.c reloads it when its layers change.
ifeq ($(strip $(PLATFORM_KEY)), bmp)
    # BLE Micro Pro: same features as the stock firmware (keyboards/ble_micro_pro/keymaps/vial).
    VIA_ENABLE = yes
    VIAL_ENABLE = yes
    VIAL_INSECURE = yes

    # Which hardware config to build in (bmp_config.c): left or right for a BMP in each half, lpme
    # for one BMP with an LPME-IO. Leave it empty to use the config the Web Configurator stored.
    SRC += bmp_config.c
    ifeq ($(strip $(ERGO68_BMP_CONFIG)), left)
        OPT_DEFS += -DERGO68_BMP_LEFT
    else ifeq ($(strip $(ERGO68_BMP_CONFIG)), right)
        OPT_DEFS += -DERGO68_BMP_RIGHT
    else ifeq ($(strip $(ERGO68_BMP_CONFIG)), lpme)
        OPT_DEFS += -DERGO68_BMP_LPME
    else ifneq ($(strip $(ERGO68_BMP_CONFIG)),)
        $(error ERGO68_BMP_CONFIG must be left, right, lpme or empty, not "$(ERGO68_BMP_CONFIG)")
    endif
else
    # Pro Micro (yushakobo/ergo68 in mainline QMK). Others with Ergo68 VIA builds report that
    # link-time optimisation breaks Remap, so LTO stays off. VIA and the RGB matrix nearly fill the
    # ATmega32U4, so leave out QMK features this keymap doesn't use (docs/squeezing_avr.md).
    VIA_ENABLE = yes
    MAGIC_ENABLE = no
    SPACE_CADET_ENABLE = no
    GRAVE_ESC_ENABLE = no
endif
