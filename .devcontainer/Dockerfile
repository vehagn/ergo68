# QMK's build image: the qmk CLI, arm-none-eabi-gcc, avr-gcc and the Python packages QMK needs.
FROM ghcr.io/qmk/qmk_cli:latest

# Non-root user for VS Code; Dev Containers changes its UID/GID to match yours. Created here
# rather than with the common-utils devcontainer feature, because features run apt-get and the
# image's apt source for cli.github.com fails its signature check.
RUN useradd --create-home --shell /bin/bash --user-group vscode \
    && echo 'vscode ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode
