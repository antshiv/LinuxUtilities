#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

fail() {
    echo "[awesome-test] FAIL: $1" >&2
    exit 1
}

require_file() {
    local file="$1"
    [[ -f "$file" ]] || fail "missing file: $file"
}

require_grep() {
    local pattern="$1"
    local file="$2"
    local desc="$3"
    if ! rg -q --fixed-strings "$pattern" "$file"; then
        fail "missing ${desc}: ${pattern} in ${file}"
    fi
}

cd "$ROOT_DIR"

echo "[awesome-test] Repo config syntax"
awesome -k -c rc.lua >/dev/null

echo "[awesome-test] Staged XDG config tree syntax"
mkdir -p "$TMP_DIR/awesome/linuxutils"
install -m 0644 rc.lua "$TMP_DIR/awesome/rc.lua"
install -m 0644 linuxutils/*.lua "$TMP_DIR/awesome/linuxutils/"
XDG_CONFIG_HOME="$TMP_DIR" awesome -k >/dev/null

echo "[awesome-test] Module inventory"
require_file "$ROOT_DIR/linuxutils/common.lua"
require_file "$ROOT_DIR/linuxutils/brightness.lua"
require_grep 'require("linuxutils.common")' "$ROOT_DIR/rc.lua" "common module require"
require_grep 'require("linuxutils.brightness")' "$ROOT_DIR/rc.lua" "brightness module require"

echo "[awesome-test] Brightness + presenter bindings"
require_grep '"XF86MonBrightnessUp"' "$ROOT_DIR/rc.lua" "brightness up binding"
require_grep '"XF86MonBrightnessDown"' "$ROOT_DIR/rc.lua" "brightness down binding"
require_grep '"F12"' "$ROOT_DIR/rc.lua" "brightness fallback binding"
require_grep '"F6"' "$ROOT_DIR/rc.lua" "gromit binding"
require_grep '"F7"' "$ROOT_DIR/rc.lua" "spotlight binding"
require_grep '"F11"' "$ROOT_DIR/rc.lua" "presenter dash binding"

echo "[awesome-test] Make deploy dry-run coverage"
make -n awesome-user-update >"$TMP_DIR/awesome-user-update.txt"
make -n awesome-update >"$TMP_DIR/awesome-update.txt"
require_grep 'linuxutils/*.lua' "$TMP_DIR/awesome-user-update.txt" "user module install step"
require_grep 'linuxutils/*.lua' "$TMP_DIR/awesome-update.txt" "system module install step"
require_grep 'config/local.mk' "$ROOT_DIR/Makefile" "local override include"
require_grep 'config/host/' "$ROOT_DIR/Makefile" "host override include"

echo "[awesome-test] PASS"
