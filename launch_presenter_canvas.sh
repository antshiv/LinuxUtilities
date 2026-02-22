#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PAGE="$ROOT_DIR/presenter_canvas.html"
PORT="${PRESENTER_CANVAS_PORT:-38947}"
HOST="127.0.0.1"
URL="http://${HOST}:${PORT}/presenter_canvas.html"
SERVER_LOG="${XDG_RUNTIME_DIR:-/tmp}/linuxutilities-presenter-canvas-server-${PORT}.log"

if [[ ! -f "$PAGE" ]]; then
    echo "Presenter canvas page not found: $PAGE" >&2
    exit 1
fi

if [[ -z "${DISPLAY:-}" ]]; then
    echo "DISPLAY is not set. Run this from your desktop session." >&2
    exit 1
fi

is_port_open() {
    local port="$1"
    bash -lc "cat < /dev/null > /dev/tcp/${HOST}/${port}" >/dev/null 2>&1
}

ensure_local_server() {
    if is_port_open "$PORT"; then
        return 0
    fi
    if ! command -v python3 >/dev/null 2>&1; then
        echo "python3 is required to serve Presenter Canvas ES modules." >&2
        exit 1
    fi

    nohup python3 -m http.server "$PORT" --bind "$HOST" --directory "$ROOT_DIR" >"$SERVER_LOG" 2>&1 &

    for _ in {1..30}; do
        if is_port_open "$PORT"; then
            return 0
        fi
        sleep 0.1
    done

    echo "Failed to start local presenter canvas server on ${HOST}:${PORT}." >&2
    echo "Check log: $SERVER_LOG" >&2
    exit 1
}

open_with_app_mode() {
    local browser="$1"
    "$browser" --new-window --app="$URL" >/dev/null 2>&1 &
}

ensure_local_server

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
    xdg-open "$URL" >/dev/null 2>&1 &
    exit 0
fi

echo "No supported launcher found. Install a browser or xdg-open." >&2
exit 1
