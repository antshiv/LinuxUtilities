#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="/tmp/linux_control_center_smoke.log"
APP_BIN="$ROOT_DIR/build/bin/linux_control_center"

cd "$ROOT_DIR"

if [[ ! -x "$ROOT_DIR/build_linux_control_center.sh" ]]; then
    echo "Missing build script: $ROOT_DIR/build_linux_control_center.sh"
    exit 1
fi

"$ROOT_DIR/build_linux_control_center.sh" >/dev/null

if [[ ! -x "$APP_BIN" ]]; then
    echo "Expected binary not found after build: $APP_BIN"
    exit 1
fi

if ! command -v xvfb-run >/dev/null 2>&1; then
    echo "xvfb-run not found. Install with: sudo apt install xvfb"
    echo "Build succeeded; smoke launch was skipped."
    exit 0
fi

set +e
timeout 5s xvfb-run -a "$APP_BIN" "$ROOT_DIR" >"$LOG_FILE" 2>&1
rc=$?
set -e

if [[ "$rc" -eq 124 || "$rc" -eq 143 ]]; then
    echo "Smoke test passed: app launched and stayed running under Xvfb."
    exit 0
fi

if [[ "$rc" -eq 0 ]]; then
    echo "Smoke test warning: app exited cleanly before timeout."
    echo "Check if this is expected behavior."
    exit 0
fi

if [[ -f "$LOG_FILE" ]] && grep -qi "cannot open display" "$LOG_FILE"; then
    echo "Smoke test skipped: display server unavailable in this environment."
    exit 0
fi

echo "Smoke test failed (exit $rc). Recent log:"
tail -n 60 "$LOG_FILE" || true
exit 1
