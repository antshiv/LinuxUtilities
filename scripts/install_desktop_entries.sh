#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APPS_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICON_PATH="${BASE_DIR}/assets/linuxterminal.png"

if ! mkdir -p "${APPS_DIR}" >/dev/null 2>&1; then
  APPS_DIR="${BASE_DIR}/build/desktop_entries"
  mkdir -p "${APPS_DIR}"
  echo "WARN: cannot write to user applications dir; using fallback: ${APPS_DIR}"
fi

if ! touch "${APPS_DIR}/.lcu_write_test.$$" >/dev/null 2>&1; then
  APPS_DIR="${BASE_DIR}/build/desktop_entries"
  mkdir -p "${APPS_DIR}"
  echo "WARN: applications dir not writable; using fallback: ${APPS_DIR}"
fi
rm -f "${APPS_DIR}/.lcu_write_test.$$" >/dev/null 2>&1 || true

write_entry() {
  local file_name="$1"
  local name="$2"
  local comment="$3"
  local exec_cmd="$4"
  cat > "${APPS_DIR}/${file_name}" <<EOF
[Desktop Entry]
Type=Application
Version=1.0
Name=${name}
Comment=${comment}
Exec=${exec_cmd}
Terminal=false
Categories=Utility;Development;
Icon=${ICON_PATH}
StartupNotify=true
EOF
}

write_entry "linuxutilities-control-center.desktop" \
  "LinuxUtilities Control Center" \
  "GTK4 control center for LinuxUtilities workflow" \
  "bash -lc 'if [ -x \"$HOME/Programs/bin/linux_control_center\" ]; then exec \"$HOME/Programs/bin/linux_control_center\"; elif [ -x \"${BASE_DIR}/build/bin/linux_control_center\" ]; then exec \"${BASE_DIR}/build/bin/linux_control_center\"; else echo \"linux_control_center (GTK4) not found. Build with ./build_linux_control_center.sh\"; fi'"

write_entry "linuxutilities-presenter-canvas.desktop" \
  "LinuxUtilities Presenter Canvas" \
  "Open presenter canvas whiteboard" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./launch_presenter_canvas.sh'"

write_entry "linuxutilities-storyboard.desktop" \
  "LinuxUtilities Storyboard DSL" \
  "Open storyboard DSL player/editor" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./launch_presenter_storyboard.sh'"

write_entry "linuxutilities-teleprompter.desktop" \
  "LinuxUtilities Teleprompter" \
  "Open local teleprompter window" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./launch_teleprompter.sh'"

write_entry "linuxutilities-present-live.desktop" \
  "LinuxUtilities Present Live" \
  "Launch reveal + presenter workflow" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./launch_present_live.sh'"

write_entry "linuxutilities-presentation-prep.desktop" \
  "LinuxUtilities Presentation Prep" \
  "Apply one-click presentation profile" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./scripts/presentation_mode.sh prep'"

write_entry "linuxutilities-presentation-live.desktop" \
  "LinuxUtilities Presentation Live Profile" \
  "Apply prep profile and launch live workflow" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./scripts/presentation_mode.sh live'"

write_entry "linuxutilities-manim-shell.desktop" \
  "LinuxUtilities Manim Shell" \
  "Open Manim workspace shell with virtualenv" \
  "bash -lc 'cd \"${BASE_DIR}\" && ./manim_tools.sh term-shell'"

write_entry "linuxutilities-shortcuts.desktop" \
  "LinuxUtilities Shortcuts Cheat Sheet" \
  "Open LinuxUtilities keyboard/mouse shortcut docs" \
  "bash -lc 'cd \"${BASE_DIR}\" && xdg-open SHORTCUTS_CHEATSHEET.md'"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${APPS_DIR}" >/dev/null 2>&1 || true
fi

echo "Installed desktop entries in: ${APPS_DIR}"
echo "Tip: open launcher (rofi drun / app menu) and search for: LinuxUtilities"
