#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${ROOT_DIR}/cursor_spotlight.c"
OUT="${ROOT_DIR}/cursor_spotlight"

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

cc -O2 -g -Wall -Wextra -Wpedantic \
   -o "${OUT}" "${SRC}" \
   $(pkg-config --cflags --libs x11 xext xfixes xrender cairo) \
   -lm

echo "Built: ${OUT}"
echo "Run with: ${OUT} --radius 180 --dim 0.68 --fps 50"
