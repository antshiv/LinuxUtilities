#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PAGE="$ROOT_DIR/teleprompter.html"

if [[ ! -f "$PAGE" ]]; then
    echo "Teleprompter page not found: $PAGE" >&2
    exit 1
fi

if [[ -z "${DISPLAY:-}" ]]; then
    echo "DISPLAY is not set. Run this from your desktop session." >&2
    exit 1
fi

URL="file://$(realpath "$PAGE")"

open_with_app_mode() {
    local browser="$1"
    "$browser" --new-window --app="$URL" >/dev/null 2>&1 &
}

if command -v google-chrome >/dev/null 2>&1; then
    open_with_app_mode google-chrome
    exit 0
fi

if command -v chromium-browser >/dev/null 2>&1; then
    open_with_app_mode chromium-browser
    exit 0
fi

if command -v chromium >/dev/null 2>&1; then
    open_with_app_mode chromium
    exit 0
fi

if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$PAGE" >/dev/null 2>&1 &
    exit 0
fi

echo "No supported launcher found. Install a browser or xdg-open." >&2
exit 1
