#!/usr/bin/env bash
set -Euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

run_runtime_smoke() {
    local name="$1"
    local config_rel="$2"
    local config_abs="$ROOT_DIR/$config_rel"
    local log_file="$TMP_DIR/${name}.log"
    local rc

    timeout 5s xvfb-run -a awesome -c "$config_abs" >"$log_file" 2>&1
    rc=$?

    if [[ "$rc" -eq 0 ]]; then
        return 0
    fi

    if [[ -f "$log_file" ]] && grep -Eqi "cannot open display|failed to open display|unable to open display" "$log_file"; then
        echo "[awesome-runtime] SKIP: display server unavailable for ${name}" >&2
        return 2
    fi

    echo "[awesome-runtime] ${name} failed (exit $rc)" >&2
    tail -n 60 "$log_file" >&2 || true
    return 1
}

if ! command -v xvfb-run >/dev/null 2>&1; then
    echo "[awesome-runtime] SKIP: xvfb-run not found" >&2
    exit 0
fi

skipped=0
for item in \
    "module-smoke tests/awesomewm_module_smoke_rc.lua" \
    "bindings-smoke tests/awesomewm_bindings_smoke_rc.lua"; do
    name="${item%% *}"
    config_rel="${item#* }"
    run_runtime_smoke "$name" "$config_rel"
    rc=$?
    if [[ "$rc" -eq 2 ]]; then
        skipped=1
        continue
    fi
    if [[ "$rc" -ne 0 ]]; then
        exit "$rc"
    fi
done

if [[ "$skipped" -eq 1 ]]; then
    echo "[awesome-runtime] SKIP"
else
    echo "[awesome-runtime] PASS"
fi
