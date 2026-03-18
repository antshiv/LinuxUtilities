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

echo "[awesome-test] Runtime smoke"
bash "$ROOT_DIR/tests/awesomewm_runtime_smoke.sh" >/dev/null

echo "[awesome-test] Staged XDG config tree syntax"
mkdir -p "$TMP_DIR/awesome/linuxutils" "$TMP_DIR/awesome/icons"
install -m 0644 rc.lua "$TMP_DIR/awesome/rc.lua"
install -m 0644 linuxutils/*.lua "$TMP_DIR/awesome/linuxutils/"
if [[ -d "$ROOT_DIR/icons" ]]; then
    cp -R "$ROOT_DIR/icons"/. "$TMP_DIR/awesome/icons/"
fi
XDG_CONFIG_HOME="$TMP_DIR" awesome -k >/dev/null

echo "[awesome-test] Module inventory"
require_file "$ROOT_DIR/linuxutils/common.lua"
require_file "$ROOT_DIR/linuxutils/brightness.lua"
require_file "$ROOT_DIR/linuxutils/icons.lua"
require_file "$ROOT_DIR/linuxutils/audio.lua"
require_file "$ROOT_DIR/linuxutils/presenter.lua"
require_file "$ROOT_DIR/linuxutils/launchers.lua"
require_file "$ROOT_DIR/linuxutils/calendar.lua"
require_file "$ROOT_DIR/linuxutils/widgets.lua"
require_file "$ROOT_DIR/linuxutils/bindings.lua"
require_file "$ROOT_DIR/icons/LinuxUtilitiesStatus/index.theme"
require_file "$ROOT_DIR/tests/awesomewm_bindings_smoke_rc.lua"
require_file "$ROOT_DIR/tests/awesomewm_module_smoke_rc.lua"
require_file "$ROOT_DIR/tests/awesomewm_runtime_smoke.sh"
require_grep 'require("linuxutils.common")' "$ROOT_DIR/rc.lua" "common module require"
require_grep 'require("linuxutils.brightness")' "$ROOT_DIR/rc.lua" "brightness module require"
require_grep 'require("linuxutils.audio")' "$ROOT_DIR/rc.lua" "audio module require"
require_grep 'require("linuxutils.presenter")' "$ROOT_DIR/rc.lua" "presenter module require"
require_grep 'require("linuxutils.launchers")' "$ROOT_DIR/rc.lua" "launchers module require"
require_grep 'require("linuxutils.calendar")' "$ROOT_DIR/rc.lua" "calendar module require"
require_grep 'require("linuxutils.widgets")' "$ROOT_DIR/rc.lua" "widgets module require"
require_grep 'require("linuxutils.bindings")' "$ROOT_DIR/rc.lua" "bindings module require"
require_grep 'require("linuxutils.icons")' "$ROOT_DIR/linuxutils/widgets.lua" "icons module require"
require_grep 'widgets.new requires calendar controller' "$ROOT_DIR/linuxutils/widgets.lua" "calendar injection assertion"
require_grep 'command -v obsidian' "$ROOT_DIR/linuxutils/launchers.lua" "obsidian notes preference"
require_grep '${EDITOR:-vi}' "$ROOT_DIR/linuxutils/launchers.lua" "vi notes fallback"

echo "[awesome-test] Brightness + presenter bindings"
require_grep '"XF86MonBrightnessUp"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness up binding"
require_grep '"XF86MonBrightnessDown"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness down binding"
require_grep '"F12"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness fallback binding"
require_grep '"F6"' "$ROOT_DIR/linuxutils/bindings.lua" "gromit binding"
require_grep '"F7"' "$ROOT_DIR/linuxutils/bindings.lua" "spotlight binding"
require_grep '"F11"' "$ROOT_DIR/linuxutils/bindings.lua" "presenter dash binding"

echo "[awesome-test] Make deploy dry-run coverage"
make -n awesome-user-update >"$TMP_DIR/awesome-user-update.txt"
make -n awesome-update >"$TMP_DIR/awesome-update.txt"
require_grep 'linuxutils/*.lua' "$TMP_DIR/awesome-user-update.txt" "user module install step"
require_grep 'linuxutils/*.lua' "$TMP_DIR/awesome-update.txt" "system module install step"
require_grep 'Installed icons/' "$TMP_DIR/awesome-user-update.txt" "user icon install step"
require_grep 'Installed icons/' "$TMP_DIR/awesome-update.txt" "system icon install step"
require_grep 'config/local.mk' "$ROOT_DIR/Makefile" "local override include"
require_grep 'config/host/' "$ROOT_DIR/Makefile" "host override include"
require_grep 'symbolic-status-icons false' "$ROOT_DIR/rc.lua" "blueman full-color startup setting"
require_grep 'GTK_ICON_THEME=LinuxUtilitiesStatus' "$ROOT_DIR/rc.lua" "custom tray icon theme"
if rg -q --fixed-strings 'nm-applet --indicator' "$ROOT_DIR/rc.lua"; then
    fail "legacy nm-applet --indicator launch is still present"
fi

echo "[awesome-test] PASS"
