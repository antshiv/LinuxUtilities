#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="/tmp/linux_control_center_smoke.log"
APP_BIN="$ROOT_DIR/build/bin/linux_control_center"
EMPTY_CONTEXT="$(mktemp -d)"

trap 'rm -rf "$EMPTY_CONTEXT"' EXIT

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

run_launch_case() {
    local name="$1"
    local context_dir="$2"
    local case_log="${LOG_FILE%.log}_${name}.log"
    local rc=1

    for attempt in 1 2; do
        set +e
        GDK_BACKEND=x11 timeout 5s xvfb-run -a "$APP_BIN" "$context_dir" >"$case_log" 2>&1
        rc=$?
        set -e

        if [[ "$rc" -eq 124 || "$rc" -eq 143 || "$rc" -eq 0 ]]; then
            break
        fi

        if grep -Eqi "cannot open display|failed to open display" "$case_log"; then
            sleep 1
            continue
        fi

        break
    done

    if [[ "$rc" -eq 124 || "$rc" -eq 143 ]]; then
        echo "Smoke test passed ($name): app stayed running under Xvfb."
        return 0
    fi

    if grep -Eqi "cannot open display|failed to open display" "$case_log"; then
        echo "Smoke test skipped: display server unavailable in this environment."
        exit 0
    fi

    echo "Smoke test failed ($name): app exited before timeout (exit $rc)."
    tail -n 60 "$case_log" || true
    return 1
}

run_launch_case "populated" "$ROOT_DIR"
run_launch_case "empty" "$EMPTY_CONTEXT"

if [[ ! -d "$EMPTY_CONTEXT/Screenshots" ]]; then
    echo "Smoke test failed: empty context did not initialize its Screenshots directory."
    exit 1
fi
