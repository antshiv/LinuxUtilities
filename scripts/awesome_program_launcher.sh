#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/linuxutilities"
RECENTS_FILE="${CACHE_DIR}/awesome_launcher_recent.txt"
MAX_RECENTS=16
APPIMAGE_DIR="${LINUXUTILS_APPIMAGE_DIR:-}"
APPIMAGE_ONLY=0

if [[ -z "${APPIMAGE_DIR}" ]]; then
  if [[ -d "${HOME}/Programs/AppImage" ]]; then
    APPIMAGE_DIR="${HOME}/Programs/AppImage"
  else
    APPIMAGE_DIR="${HOME}/Programs/appimage"
  fi
fi

if [[ "${1:-}" == "--appimages" ]]; then
  APPIMAGE_ONLY=1
  shift
fi

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
    "${BASE_DIR}/build/bin/linux_control_center" \
    "${HOME}/Programs/bin/linux_control_center"; do
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

run_folder_bg() {
  local target="$1"
  local preferred="${LINUXUTILS_FOLDERS_APP:-auto}"
  mkdir -p "${target}"

  if [[ "${preferred}" != "gui" ]]; then
    if term="$(first_available_cmd x-terminal-emulator terminator gnome-terminal alacritty kitty)"; then
      if [[ "${preferred}" == "yazi" ]] && command -v yazi >/dev/null 2>&1; then
        run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec yazi \"$1\"' sh \"${target}\""
        return 0
      elif [[ "${preferred}" == "ranger" ]] && command -v ranger >/dev/null 2>&1; then
        run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec ranger \"$1\"' sh \"${target}\""
        return 0
      elif [[ "${preferred}" == "lf" ]] && command -v lf >/dev/null 2>&1; then
        run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec lf \"$1\"' sh \"${target}\""
        return 0
      elif [[ "${preferred}" == "auto" ]]; then
        if command -v yazi >/dev/null 2>&1; then
          run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec yazi \"$1\"' sh \"${target}\""
          return 0
        elif command -v ranger >/dev/null 2>&1; then
          run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec ranger \"$1\"' sh \"${target}\""
          return 0
        elif command -v lf >/dev/null 2>&1; then
          run_cmd_bg "${term} -e sh -lc 'cd \"$1\" && exec lf \"$1\"' sh \"${target}\""
          return 0
        fi
      fi
    fi
  fi

  if command -v xdg-open >/dev/null 2>&1; then
    run_cmd_bg "xdg-open \"${target}\""
  elif command -v gio >/dev/null 2>&1; then
    run_cmd_bg "gio open \"${target}\""
  else
    return 1
  fi
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

appimage_root() {
  mkdir -p "${APPIMAGE_DIR}"
  printf '%s\n' "${APPIMAGE_DIR}"
}

list_appimages() {
  find "$(appimage_root)" -maxdepth 3 -type f \( -iname '*.AppImage' -o -iname '*.appimage' \) | sort
}

appimage_label() {
  local path="$1"
  local root rel base dir
  root="$(appimage_root)"
  rel="${path#${root}/}"
  base="$(basename "${path}")"
  base="${base%.AppImage}"
  base="${base%.appimage}"
  dir="$(dirname "${rel}")"
  if [[ "${dir}" == "." ]]; then
    printf '%s\n' "${base}"
  else
    printf '%s  [%s]\n' "${base}" "${dir}"
  fi
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
    system_monitor) echo "System Monitor" ;;
    appimage_library) echo "AppImage Library" ;;
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
    system_monitor) echo "utilities-system-monitor" ;;
    appimage_library) echo "folder" ;;
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
    bluetooth_center network_connections audio_mixer system_monitor appimage_library screenshots_folder workspace_folder shortcuts_cheatsheet \
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
  if [[ "${action_id}" == appimage::* ]]; then
    local app_path="${action_id#appimage::}"
    [[ -f "${app_path}" ]] || return 0
    label="$(appimage_label "${app_path}")"
    printf '%s\0icon\x1f%s\0info\x1f%s\n' "${label}" "application-x-executable" "${action_id}"
    return 0
  fi
  label="$(action_label "${action_id}")" || return 0
  icon="$(action_icon "${action_id}")"
  printf '%s\0icon\x1f%s\0info\x1f%s\n' "${label}" "${icon}" "${action_id}"
}

emit_recent_rows() {
  local line count=0
  [[ -f "${RECENTS_FILE}" ]] || return 0
  while read -r line; do
    [[ -z "${line}" ]] && continue
    if [[ "${line}" == appimage::* ]]; then
      if [[ -f "${line#appimage::}" ]]; then
        emit_action_row "${line}"
        count=$((count + 1))
      fi
    elif known_action "${line}"; then
      emit_action_row "${line}"
      count=$((count + 1))
    fi
    [[ "${count}" -ge 8 ]] && break
  done < "${RECENTS_FILE}"
}

emit_appimage_rows() {
  local path label
  while read -r path; do
    [[ -n "${path}" ]] || continue
    label="$(appimage_label "${path}")"
    printf '%s\0icon\x1f%s\0info\x1f%s\n' "${label}" "application-x-executable" "appimage::${path}"
  done < <(list_appimages)
}

print_entries() {
  local id
  if [[ "${APPIMAGE_ONLY}" -eq 1 ]]; then
    emit_section "AppImages"
    emit_appimage_rows
    return 0
  fi
  emit_section "Favorites"
  for id in linux_control_center present_prep present_live presenter_canvas teleprompter; do
    emit_action_row "${id}"
  done
  emit_section "Recent"
  emit_recent_rows
  emit_section "AppImages"
  emit_appimage_rows
  emit_section "LinuxUtilities"
  for id in storyboard_dsl shortcuts_cheatsheet manim_workspace manim_version manim_smoke build_control_center build_cursor_spotlight; do
    emit_action_row "${id}"
  done
  emit_section "System"
  for id in bluetooth_center network_connections audio_mixer system_monitor appimage_library terminal files browser workspace_folder screenshots_folder; do
    emit_action_row "${id}"
  done
}

run_action() {
  local action_id="$1"
  if [[ "${action_id}" == appimage::* ]]; then
    local app_path="${action_id#appimage::}"
    if [[ -f "${app_path}" ]]; then
      chmod +x "${app_path}" >/dev/null 2>&1 || true
      run_cmd_bg "\"${app_path}\""
      record_recent "${action_id}"
      return 0
    fi
    return 1
  fi
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
    network_connections) run_cmd_bg "if command -v nm-connection-editor >/dev/null 2>&1; then nm-connection-editor; elif command -v nmtui-connect >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then x-terminal-emulator -e nmtui-connect; elif command -v nmtui >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then x-terminal-emulator -e nmtui; fi" ;;
    audio_mixer) run_cmd_bg "command -v pavucontrol >/dev/null 2>&1 && pavucontrol || true" ;;
    system_monitor)
      if term="$(first_available_cmd x-terminal-emulator terminator gnome-terminal alacritty kitty)"; then
        run_cmd_bg "if command -v btop >/dev/null 2>&1; then ${term} -e btop; elif command -v htop >/dev/null 2>&1; then ${term} -e htop; else ${term} -e top; fi"
      elif command -v btop >/dev/null 2>&1; then
        run_cmd_bg "btop"
      elif command -v htop >/dev/null 2>&1; then
        run_cmd_bg "htop"
      else
        run_cmd_bg "top"
      fi
      ;;
    appimage_library) run_folder_bg "$(appimage_root)" ;;
    screenshots_folder) run_folder_bg "${BASE_DIR}/Screenshots" ;;
    workspace_folder) run_folder_bg "${BASE_DIR}" ;;
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
    files) run_folder_bg "${HOME}" ;;
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
