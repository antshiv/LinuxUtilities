#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${ROOT_DIR}/gtk_dsl_demo.c"
OUT="${ROOT_DIR}/gtk_dsl_demo"

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

cc -O2 -g -Wall -Wextra -Wpedantic \
   -o "${OUT}" "${SRC}" \
   $(pkg-config --cflags --libs gtk+-3.0)

echo "Built: ${OUT}"
echo "Run with: ${OUT} dsl/workbench.gdsl"
