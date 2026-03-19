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
require_grep 'keyboardlayout_widget.new()' "$ROOT_DIR/rc.lua" "per-screen keyboard widget creation"
require_grep 'build_screen_profile' "$ROOT_DIR/rc.lua" "screen width profile helper"
require_grep 'menu_linuxutilities' "$ROOT_DIR/rc.lua" "LinuxUtilities right-click submenu"
require_grep '"LinuxUtilities"' "$ROOT_DIR/rc.lua" "LinuxUtilities menu label"
require_grep 'Wi-Fi picker (nmtui)' "$ROOT_DIR/rc.lua" "Wi-Fi picker menu label"
require_grep 'command -v obsidian' "$ROOT_DIR/linuxutils/launchers.lua" "obsidian notes preference"
require_grep '${EDITOR:-vi}' "$ROOT_DIR/linuxutils/launchers.lua" "vi notes fallback"
require_grep 'obsidian://open?vault=' "$ROOT_DIR/linuxutils/launchers.lua" "Obsidian vault URI launcher"
require_grep 'obsidian://open?path=' "$ROOT_DIR/linuxutils/launchers.lua" "Obsidian note URI launcher"
require_grep 'linked/LinuxUtilities' "$ROOT_DIR/linuxutils/launchers.lua" "managed linked markdown mirror"
require_grep 'gio open "$notes_dir"' "$ROOT_DIR/linuxutils/launchers.lua" "notes folder gio fallback"
require_grep 'open_tasks_note' "$ROOT_DIR/linuxutils/launchers.lua" "tasks note launcher"
require_grep 'show_world_clock_popup' "$ROOT_DIR/linuxutils/launchers.lua" "world clock launcher"
require_grep 'set_timezone_mumbai' "$ROOT_DIR/linuxutils/launchers.lua" "Mumbai timezone launcher"
require_grep 'set_timezone_vancouver' "$ROOT_DIR/linuxutils/launchers.lua" "Vancouver timezone launcher"
require_grep 'Programs/AppImage' "$ROOT_DIR/linuxutils/launchers.lua" "AppImage directory default"
require_grep 'open_appimage_palette' "$ROOT_DIR/linuxutils/launchers.lua" "AppImage palette launcher"
require_grep 'open_network_tui' "$ROOT_DIR/linuxutils/launchers.lua" "network tui launcher"
require_grep 'x-terminal-emulator -e nmtui-connect' "$ROOT_DIR/linuxutils/launchers.lua" "network tui terminal launch"
require_grep 'create_applications_widget' "$ROOT_DIR/linuxutils/widgets.lua" "applications widget"
require_grep 'APPIMAGE_ONLY=1' "$ROOT_DIR/scripts/awesome_program_launcher.sh" "AppImage-only launcher mode"

echo "[awesome-test] Brightness + presenter bindings"
require_grep '"XF86MonBrightnessUp"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness up binding"
require_grep '"XF86MonBrightnessDown"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness down binding"
require_grep '"F12"' "$ROOT_DIR/linuxutils/bindings.lua" "brightness fallback binding"
require_grep '"h"' "$ROOT_DIR/linuxutils/bindings.lua" "system monitor binding"
require_grep '"t"' "$ROOT_DIR/linuxutils/bindings.lua" "world clock binding"
require_grep '"i"' "$ROOT_DIR/linuxutils/bindings.lua" "Mumbai timezone binding"
require_grep '"v"' "$ROOT_DIR/linuxutils/bindings.lua" "Vancouver timezone binding"
require_grep '"F6"' "$ROOT_DIR/linuxutils/bindings.lua" "gromit binding"
require_grep '"F7"' "$ROOT_DIR/linuxutils/bindings.lua" "spotlight binding"
require_grep '"F11"' "$ROOT_DIR/linuxutils/bindings.lua" "presenter dash binding"
require_grep 'open_system_monitor' "$ROOT_DIR/linuxutils/launchers.lua" "system monitor launcher"
require_grep 'open_home_folder' "$ROOT_DIR/linuxutils/launchers.lua" "home folder launcher"
require_grep 'create_folder_widget' "$ROOT_DIR/linuxutils/widgets.lua" "folder widget"

echo "[awesome-test] Dynamic AppImage launcher inventory"
APPIMAGE_TEST_DIR="$TMP_DIR/appimages"
mkdir -p "$APPIMAGE_TEST_DIR"
touch "$APPIMAGE_TEST_DIR/FreeCAD_1.0.2-conda-Linux-x86_64-py311.AppImage"
LINUXUTILS_APPIMAGE_DIR="$APPIMAGE_TEST_DIR" bash "$ROOT_DIR/scripts/awesome_program_launcher.sh" --appimages >"$TMP_DIR/appimages.txt"
require_grep 'FreeCAD_1.0.2-conda-Linux-x86_64-py311' "$TMP_DIR/appimages.txt" "AppImage launcher row"
require_grep 'appimage::' "$TMP_DIR/appimages.txt" "AppImage launcher action id"

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
