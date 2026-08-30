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

if [[ "${1:-}" == "--list" ]]; then
  list_candidates
  exit 0
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
