#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/linuxutilities"
RECENTS_FILE="${CACHE_DIR}/folder_navigator_recent.txt"
THEME="${ROOT_DIR}/config/folder_navigator.rasi"

notify_error() {
  if command -v notify-send >/dev/null 2>&1; then
    notify-send "Folder Navigator" "$1"
  else
    printf '%s\n' "$1" >&2
  fi
}

record_recent() {
  local target="$1"
  local temporary
  mkdir -p "${CACHE_DIR}"
  temporary="$(mktemp)"
  {
    printf '%s\n' "${target}"
    if [[ -f "${RECENTS_FILE}" ]]; then
      grep -Fxv "${target}" "${RECENTS_FILE}" | head -n 39 || true
    fi
  } >"${temporary}"
  mv "${temporary}" "${RECENTS_FILE}"
}

list_candidates() {
  {
    printf '%s\n' \
      "${HOME}" \
      "${HOME}/Workspace" \
      "${HOME}/Downloads" \
      "${HOME}/Documents" \
      "${HOME}/Pictures" \
      "${HOME}/Screenshots" \
      "${HOME}/Programs" \
      "${ROOT_DIR}"

    [[ -f "${RECENTS_FILE}" ]] && head -n 40 "${RECENTS_FILE}"
    command -v zoxide >/dev/null 2>&1 && zoxide query --list 2>/dev/null | head -n 80

    for root in "${HOME}/Workspace" "${HOME}/Documents" "${HOME}/Programs" "/data" "/media/${USER}"; do
      [[ -d "${root}" ]] || continue
      find "${root}" -mindepth 1 -maxdepth 2 -type d \
        ! -path '*/.*' ! -path '*/node_modules/*' ! -path '*/build/*' \
        2>/dev/null
    done
  } | awk 'NF && !seen[$0]++' | while IFS= read -r path; do
    [[ -d "${path}" ]] && printf '%s\n' "${path}"
  done
}

normalize_path() {
  local selected="$1"
  selected="${selected/#\~/${HOME}}"
  if [[ "${selected}" != /* ]]; then
    selected="${HOME}/${selected}"
  fi
  realpath -m -- "${selected}"
}

display_path() {
  local target="$1"
  if [[ "${target}" == "${HOME}" ]]; then
    printf '~'
  elif [[ "${target}" == "${HOME}/"* ]]; then
    printf '~/%s' "${target#${HOME}/}"
  else
    printf '%s' "${target}"
  fi
}

candidate_label() {
  local target="$1"
  local name
  case "${target}" in
    "${HOME}") name="Home" ;;
    "${HOME}/Workspace") name="Workspace" ;;
    "${HOME}/Downloads") name="Downloads" ;;
    "${HOME}/Documents") name="Documents" ;;
    "${HOME}/Pictures") name="Pictures" ;;
    "${HOME}/Screenshots") name="Screenshots" ;;
    "${HOME}/Programs") name="Programs" ;;
    "${ROOT_DIR}") name="LinuxUtilities" ;;
    *) name="$(basename "${target}")" ;;
  esac
  printf '%s  -  %s' "${name}" "$(display_path "${target}")"
}

open_graphical() {
  local target="$1"
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "${target}" >/dev/null 2>&1 &
  elif command -v gio >/dev/null 2>&1; then
    gio open "${target}" >/dev/null 2>&1 &
  else
    notify_error "Install xdg-utils or gio to open folders."
    return 1
  fi
}

open_terminal() {
  local target="$1"
  local terminal
  for terminal in x-terminal-emulator kitty alacritty gnome-terminal terminator; do
    command -v "${terminal}" >/dev/null 2>&1 || continue
    "${terminal}" -e sh -lc 'cd "$1" && exec "${SHELL:-bash}"' sh "${target}" >/dev/null 2>&1 &
    return 0
  done
  notify_error "No supported terminal emulator was found."
  return 1
}

copy_path() {
  local target="$1"
  if command -v xclip >/dev/null 2>&1; then
    printf '%s' "${target}" | xclip -selection clipboard
  elif command -v xsel >/dev/null 2>&1; then
    printf '%s' "${target}" | xsel --clipboard --input
  else
    notify_error "Install xclip or xsel to copy paths."
    return 1
  fi
  command -v notify-send >/dev/null 2>&1 && notify-send "Path copied" "${target}"
}

run_rofi_provider() {
  local selected="${1:-}"
  local target

  if [[ "${ROFI_RETV:-0}" == "0" && -z "${selected}" ]]; then
    printf '\0prompt\x1fOpen\n'
    printf '\0message\x1fApplications, folders, windows, commands and LinuxUtilities actions\n'
    while IFS= read -r target; do
      printf '%s\0icon\x1ffolder\x1fmeta\x1f%s\x1finfo\x1f%s\n' \
        "$(candidate_label "${target}")" "${target}" "folder::${target}"
    done < <(list_candidates)
    return 0
  fi

  target="${ROFI_INFO:-}"
  target="${target#folder::}"
  if [[ -z "${target}" ]]; then
    target="$(normalize_path "${selected}")"
  fi
  if [[ ! -d "${target}" ]]; then
    notify_error "Directory does not exist: ${target}"
    return 1
  fi

  record_recent "${target}"
  open_graphical "${target}"
}

if [[ "${1:-}" == "--list" ]]; then
  list_candidates
  exit 0
fi

if [[ "${1:-}" == "--rofi" ]]; then
  shift
  run_rofi_provider "${1:-}"
  exit $?
fi

if ! command -v rofi >/dev/null 2>&1; then
  notify_error "Rofi is required for the folder navigator."
  exit 1
fi

set +e
selected="$(list_candidates | rofi -dmenu -i -matching fuzzy -sort \
  -p "Go to" \
  -mesg "Enter: open folder   Ctrl+O: terminal   Alt+C: copy path" \
  -kb-custom-1 "Control+o" \
  -kb-custom-2 "Alt+c" \
  -theme "${THEME}")"
result=$?
set -e

[[ -n "${selected}" ]] || exit 0
target="$(normalize_path "${selected}")"
if [[ ! -d "${target}" ]]; then
  notify_error "Directory does not exist: ${target}"
  exit 1
fi

record_recent "${target}"
case "${result}" in
  0) open_graphical "${target}" ;;
  10) open_terminal "${target}" ;;
  11) copy_path "${target}" ;;
  *) exit 0 ;;
esac
