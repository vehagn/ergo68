# QMK's build image: the qmk CLI, arm-none-eabi-gcc, avr-gcc and the Python packages QMK needs.
FROM ghcr.io/qmk/qmk_cli:latest

# just runs the build recipes in ../justfile. The image's apt source for cli.github.com fails its
# signature check and breaks apt-get update, and gh isn't needed here, so drop it first.
RUN rm -f /etc/apt/sources.list.d/github-cli.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends just \
    && rm -rf /var/lib/apt/lists/*

# Non-root user for VS Code; Dev Containers changes its UID/GID to match yours.
RUN useradd --create-home --shell /bin/bash --user-group vscode \
    && echo 'vscode ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode
