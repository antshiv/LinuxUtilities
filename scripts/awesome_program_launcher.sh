#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/linuxutilities"
RECENTS_FILE="${CACHE_DIR}/awesome_launcher_recent.txt"
MAX_RECENTS=16

if ! mkdir -p "${CACHE_DIR}" >/dev/null 2>&1 || ! touch "${CACHE_DIR}/.lcu_write_test.$$" >/dev/null 2>&1; then
  CACHE_DIR="${BASE_DIR}/build/cache"
  mkdir -p "${CACHE_DIR}"
fi
rm -f "${CACHE_DIR}/.lcu_write_test.$$" >/dev/null 2>&1 || true
RECENTS_FILE="${CACHE_DIR}/awesome_launcher_recent.txt"

run_repo_bg() {
  local snippet="$1"
  (
    cd "${BASE_DIR}"
    bash -lc "${snippet}"
  ) >/tmp/lcu_launcher.log 2>&1 &
  disown || true
}

control_center_cmd() {
  local candidate
  for candidate in \
    "${HOME}/Programs/bin/linux_control_center" \
    "${BASE_DIR}/build/bin/linux_control_center"; do
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done
  return 1
}

run_cmd_bg() {
  local cmd="$1"
  bash -lc "${cmd}" >/tmp/lcu_launcher.log 2>&1 &
  disown || true
}

first_available_cmd() {
  local candidate
  for candidate in "$@"; do
    if command -v "${candidate}" >/dev/null 2>&1; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done
  return 1
}

action_label() {
  case "$1" in
    linux_control_center) echo "LinuxUtilities Control Center" ;;
    presenter_canvas) echo "Presenter Canvas" ;;
    storyboard_dsl) echo "Storyboard DSL" ;;
    teleprompter) echo "Teleprompter" ;;
    present_prep) echo "Presentation Prep (audio+bluetooth+wacom)" ;;
    present_live) echo "Presentation Live (launch workflow)" ;;
    present_live_code) echo "Presentation Live + Code Window" ;;
    bluetooth_center) echo "Bluetooth Manager" ;;
    network_connections) echo "Network Connections" ;;
    audio_mixer) echo "Audio Mixer" ;;
    screenshots_folder) echo "Screenshots Folder" ;;
    workspace_folder) echo "LinuxUtilities Folder" ;;
    shortcuts_cheatsheet) echo "Shortcut Cheat Sheet" ;;
    manim_workspace) echo "Manim Workspace" ;;
    manim_version) echo "Manim Version" ;;
    manim_smoke) echo "Manim Smoke" ;;
    build_control_center) echo "Build Linux Control Center" ;;
    build_cursor_spotlight) echo "Build Cursor Spotlight" ;;
    terminal) echo "Terminal" ;;
    files) echo "Files" ;;
    browser) echo "Browser" ;;
    *) return 1 ;;
  esac
}

action_icon() {
  case "$1" in
    linux_control_center) echo "applications-system" ;;
    presenter_canvas|storyboard_dsl|teleprompter) echo "applications-graphics" ;;
    present_prep|present_live|present_live_code) echo "video-display" ;;
    bluetooth_center) echo "bluetooth" ;;
    network_connections) echo "network-wireless" ;;
    audio_mixer) echo "audio-card" ;;
    screenshots_folder|workspace_folder|files) echo "folder" ;;
    shortcuts_cheatsheet) echo "text-x-markdown" ;;
    manim_workspace|manim_version|manim_smoke) echo "applications-science" ;;
    build_control_center|build_cursor_spotlight) echo "applications-development" ;;
    terminal) echo "utilities-terminal" ;;
    browser) echo "web-browser" ;;
    *) echo "application-x-executable" ;;
  esac
}

known_action() {
  action_label "$1" >/dev/null 2>&1
}

label_to_action() {
  local label="$1"
  local id
  for id in \
    linux_control_center presenter_canvas storyboard_dsl teleprompter present_prep present_live present_live_code \
    bluetooth_center network_connections audio_mixer screenshots_folder workspace_folder shortcuts_cheatsheet \
    manim_workspace manim_version manim_smoke build_control_center build_cursor_spotlight terminal files browser; do
    if [[ "$(action_label "${id}")" == "${label}" ]]; then
      printf '%s\n' "${id}"
      return 0
    fi
  done
  return 1
}

record_recent() {
  local action_id="$1"
  local tmp_file
  mkdir -p "${CACHE_DIR}"
  tmp_file="$(mktemp)"
  {
    printf '%s\n' "${action_id}"
    if [[ -f "${RECENTS_FILE}" ]]; then
      grep -Fxv "${action_id}" "${RECENTS_FILE}" | head -n $((MAX_RECENTS - 1)) || true
    fi
  } > "${tmp_file}"
  mv "${tmp_file}" "${RECENTS_FILE}"
}

emit_section() {
  local title="$1"
  printf '%s\0nonselectable\x1ftrue\n' "==== ${title} ===="
}

emit_action_row() {
  local action_id="$1"
  local label icon
  label="$(action_label "${action_id}")" || return 0
  icon="$(action_icon "${action_id}")"
  printf '%s\0icon\x1f%s\0info\x1f%s\n' "${label}" "${icon}" "${action_id}"
}

emit_recent_rows() {
  local line count=0
  [[ -f "${RECENTS_FILE}" ]] || return 0
  while read -r line; do
    [[ -z "${line}" ]] && continue
    if known_action "${line}"; then
      emit_action_row "${line}"
      count=$((count + 1))
      [[ "${count}" -ge 8 ]] && break
    fi
  done < "${RECENTS_FILE}"
}

print_entries() {
  local id
  emit_section "Favorites"
  for id in linux_control_center present_prep present_live presenter_canvas teleprompter; do
    emit_action_row "${id}"
  done
  emit_section "Recent"
  emit_recent_rows
  emit_section "LinuxUtilities"
  for id in storyboard_dsl shortcuts_cheatsheet manim_workspace manim_version manim_smoke build_control_center build_cursor_spotlight; do
    emit_action_row "${id}"
  done
  emit_section "System"
  for id in bluetooth_center network_connections audio_mixer terminal files browser workspace_folder screenshots_folder; do
    emit_action_row "${id}"
  done
}

run_action() {
  local action_id="$1"
  case "${action_id}" in
    linux_control_center)
      if cc_bin="$(control_center_cmd)"; then
        run_cmd_bg "${cc_bin}"
      fi
      ;;
    presenter_canvas) run_repo_bg "./launch_presenter_canvas.sh" ;;
    storyboard_dsl) run_repo_bg "./launch_presenter_storyboard.sh" ;;
    teleprompter) run_repo_bg "./launch_teleprompter.sh" ;;
    present_prep) run_repo_bg "./scripts/presentation_mode.sh prep" ;;
    present_live) run_repo_bg "./scripts/presentation_mode.sh live" ;;
    present_live_code) run_repo_bg "CODE_URL=https://github.com/ ./scripts/presentation_mode.sh live" ;;
    bluetooth_center) run_cmd_bg "command -v blueman-manager >/dev/null 2>&1 && blueman-manager || true" ;;
    network_connections) run_cmd_bg "command -v nm-connection-editor >/dev/null 2>&1 && nm-connection-editor || true" ;;
    audio_mixer) run_cmd_bg "command -v pavucontrol >/dev/null 2>&1 && pavucontrol || true" ;;
    screenshots_folder) run_repo_bg "xdg-open Screenshots" ;;
    workspace_folder) run_repo_bg "xdg-open ." ;;
    shortcuts_cheatsheet) run_repo_bg "xdg-open SHORTCUTS_CHEATSHEET.md" ;;
    manim_workspace) run_repo_bg "./manim_tools.sh term-shell" ;;
    manim_version) run_repo_bg "./manim_tools.sh term-version" ;;
    manim_smoke) run_repo_bg "./manim_tools.sh term-smoke" ;;
    build_control_center) run_repo_bg "./build_linux_control_center.sh" ;;
    build_cursor_spotlight) run_repo_bg "./build_cursor_spotlight.sh" ;;
    terminal)
      if term="$(first_available_cmd x-terminal-emulator terminator gnome-terminal alacritty kitty)"; then
        run_cmd_bg "${term}"
      fi
      ;;
    files) run_cmd_bg "xdg-open \"${HOME}\"" ;;
    browser) run_cmd_bg "xdg-open https://www.google.com" ;;
    *)
      return 1
      ;;
  esac
  record_recent "${action_id}"
  return 0
}

if [[ $# -eq 0 ]]; then
  print_entries
  exit 0
fi

choice="$*"
action_id="${ROFI_INFO:-}"
if [[ -z "${action_id}" ]]; then
  if mapped="$(label_to_action "${choice}" 2>/dev/null)"; then
    action_id="${mapped}"
  fi
fi

if [[ -n "${action_id}" ]]; then
  if run_action "${action_id}"; then
    exit 0
  fi
fi

# If not a known action, allow direct command execution.
if [[ "${choice}" == "==== "* ]]; then
  exit 0
fi
if [[ -z "${choice// }" ]]; then
  exit 0
fi
run_cmd_bg "${choice}"
