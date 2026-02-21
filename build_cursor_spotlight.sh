#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${ROOT_DIR}/cursor_spotlight.c"
BUILD_DIR="${ROOT_DIR}/build/bin"
OUT="${BUILD_DIR}/cursor_spotlight"
INSTALL_DIR="${HOME}/Programs/bin"
INSTALL_OUT="${INSTALL_DIR}/cursor_spotlight"
INSTALL_BIN=0

usage() {
    cat <<'EOF'
Usage: ./build_cursor_spotlight.sh [--install]

Build output:
  build/bin/cursor_spotlight

Options:
  --install  Copy the built binary to ~/Programs/bin/cursor_spotlight
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

if ! pkg-config --exists x11 xext xfixes xrender cairo; then
    echo "Error: required X11/Cairo development headers are missing."
    echo "Install with: sudo apt install libx11-dev libxext-dev libxfixes-dev libxrender-dev libcairo2-dev"
    exit 1
fi

mkdir -p "${BUILD_DIR}"

cc -O2 -g -Wall -Wextra -Wpedantic \
   -o "${OUT}" "${SRC}" \
   $(pkg-config --cflags --libs x11 xext xfixes xrender cairo) \
   -lm

if [[ "${INSTALL_BIN}" -eq 1 ]]; then
    mkdir -p "${INSTALL_DIR}"
    install -m 755 "${OUT}" "${INSTALL_OUT}"
    echo "Installed: ${INSTALL_OUT}"
fi

echo "Built: ${OUT}"
echo "Run with: ${OUT} --radius 180 --dim 0.68 --fps 50"

if [[ "${INSTALL_BIN}" -eq 0 ]]; then
    echo "Tip: install to ~/Programs/bin with: ./build_cursor_spotlight.sh --install"
fi
