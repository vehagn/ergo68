# Build environment for every firmware in this repo.
#
# Ubuntu 24.04 rather than QMK's own qmk_cli image: the BLE Micro Pro firmware only starts when it's
# built with Ubuntu 24.04's gcc-arm-none-eabi (GCC 13.2.1), the compiler sekigon builds the official
# releases with. Built with the GCC 15 in QMK's image (or Arm's own 10.3 to 14.2 releases), the
# bootloader accepts it but it never starts. These are the packages QMK's own install script uses on
# Debian and Ubuntu, so mainline QMK builds the same way.
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates curl git make just sudo unzip zip \
        python3 python3-venv python3-pip \
        gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi \
        gcc-avr binutils-avr avr-libc avrdude \
        dfu-util dfu-programmer teensy-loader-cli \
    && rm -rf /var/lib/apt/lists/*

# The qmk CLI, in its own virtualenv (Ubuntu doesn't allow pip installs into the system Python).
RUN python3 -m venv /opt/qmk \
    && /opt/qmk/bin/pip install --no-cache-dir qmk
ENV PATH=/opt/qmk/bin:$PATH

# Non-root user for VS Code; Dev Containers changes its UID/GID to match yours. Ubuntu's image
# already has a user with UID 1000, which would take the UID Dev Containers wants to give vscode.
RUN userdel --remove ubuntu 2>/dev/null; \
    useradd --create-home --shell /bin/bash --user-group vscode \
    && echo 'vscode ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode
