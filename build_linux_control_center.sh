#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${ROOT_DIR}/linux_control_center.c"
BUILD_DIR="${ROOT_DIR}/build/bin"
OUT="${BUILD_DIR}/linux_control_center"
INSTALL_DIR="${HOME}/Programs/bin"
INSTALL_OUT="${INSTALL_DIR}/linux_control_center"
INSTALL_BIN=0

usage() {
    cat <<'EOF'
Usage: ./build_linux_control_center.sh [--install]

Build output:
  build/bin/linux_control_center

Options:
  --install  Copy the built binary to ~/Programs/bin/linux_control_center
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --install)
            INSTALL_BIN=1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

if ! command -v pkg-config >/dev/null 2>&1; then
    echo "Error: pkg-config is required."
    echo "Install with: sudo apt install pkg-config"
    exit 1
fi

if ! pkg-config --exists gtk+-3.0; then
    echo "Error: GTK 3 development headers are missing."
    echo "Install with: sudo apt install libgtk-3-dev"
    exit 1
fi

mkdir -p "${BUILD_DIR}"

cc -O2 -g -Wall -Wextra -Wpedantic \
   -o "${OUT}" "${SRC}" \
   $(pkg-config --cflags --libs gtk+-3.0) \
   -lm

if [[ "${INSTALL_BIN}" -eq 1 ]]; then
    mkdir -p "${INSTALL_DIR}"
    install -m 755 "${OUT}" "${INSTALL_OUT}"
    echo "Installed: ${INSTALL_OUT}"
fi

echo "Built: ${OUT}"
echo "Run with: ${OUT}"

if [[ "${INSTALL_BIN}" -eq 0 ]]; then
    echo "Tip: install to ~/Programs/bin with: ./build_linux_control_center.sh --install"
fi
