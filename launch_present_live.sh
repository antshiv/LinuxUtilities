#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVEAL_URL="${REVEAL_URL:-http://127.0.0.1:8000}"
CODE_URL="${CODE_URL:-}"
PRESENT_LIVE_DELAY_SEC="${PRESENT_LIVE_DELAY_SEC:-0.35}"
PRESENT_WACOM_MODE="${PRESENT_WACOM_MODE:-none}"

if [[ -z "${DISPLAY:-}" ]]; then
    echo "DISPLAY is not set. Run this from your desktop session." >&2
    exit 1
fi

if [[ ! -x "$ROOT_DIR/launch_presenter_canvas.sh" ]]; then
    echo "Missing launcher: $ROOT_DIR/launch_presenter_canvas.sh" >&2
    exit 1
fi

open_url_window() {
    local url="$1"
    if [[ -z "$url" ]]; then
        return 0
    fi

    if command -v google-chrome >/dev/null 2>&1; then
        google-chrome --new-window --app="$url" >/dev/null 2>&1 &
        return 0
    fi
    if command -v chromium-browser >/dev/null 2>&1; then
        chromium-browser --new-window --app="$url" >/dev/null 2>&1 &
        return 0
    fi
    if command -v chromium >/dev/null 2>&1; then
        chromium --new-window --app="$url" >/dev/null 2>&1 &
        return 0
    fi
    if command -v xdg-open >/dev/null 2>&1; then
        xdg-open "$url" >/dev/null 2>&1 &
        return 0
    fi

    echo "No supported launcher found. Install a browser or xdg-open." >&2
    return 1
}

apply_wacom_mode() {
    local mode="$1"
    if [[ "$mode" == "none" ]]; then
        return 0
    fi

    echo "Applying Wacom mode: $mode"
    case "$mode" in
        default)
            make --no-print-directory wacom || echo "WARN: Wacom default mapping failed; continuing."
            ;;
        external)
            make --no-print-directory wacom-external || echo "WARN: Wacom external mapping failed; continuing."
            ;;
        switch)
            make --no-print-directory wacom-switch || echo "WARN: Wacom switch mapping failed; continuing."
            ;;
        *)
            make --no-print-directory wacom-set-screen OUTPUT="$mode" || echo "WARN: Wacom mapping to $mode failed; continuing."
            ;;
    esac
}

apply_wacom_mode "$PRESENT_WACOM_MODE"

echo "Opening reveal window: $REVEAL_URL"
open_url_window "$REVEAL_URL"

sleep "$PRESENT_LIVE_DELAY_SEC"

echo "Opening presenter canvas window..."
"$ROOT_DIR/launch_presenter_canvas.sh"

if [[ -n "$CODE_URL" ]]; then
    sleep "$PRESENT_LIVE_DELAY_SEC"
    echo "Opening code window: $CODE_URL"
    open_url_window "$CODE_URL"
fi

echo "Live presentation windows launched."
echo "Tip: run make wacom-switch if pen mapping is on the wrong monitor."
