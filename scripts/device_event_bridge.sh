#!/usr/bin/env bash
set -Eeuo pipefail

MODE="${1:-monitor}"
BUS_PATH="/com/antshiv/LinuxUtilities"
BUS_INTERFACE="com.antshiv.LinuxUtilities"
CACHE_DIR="${XDG_CACHE_HOME:-${HOME}/.cache}/linuxutilities"
EVENT_FILE="${CACHE_DIR}/device-events.tsv"
EVENT_FIFO=""

cleanup() {
  jobs -pr | xargs -r kill 2>/dev/null || true
  [[ -z "${EVENT_FIFO}" ]] || rm -f "${EVENT_FIFO}"
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

emit_event() {
  local domain="${1:-all}"
  local detail="${2:-changed}"
  local timestamp=""

  mkdir -p "${CACHE_DIR}"
  timestamp="$(date --iso-8601=seconds)"
  printf '%s\t%s\t%s\n' "${timestamp}" "${domain}" "${detail}" > "${EVENT_FILE}.tmp"
  mv "${EVENT_FILE}.tmp" "${EVENT_FILE}"

  if have_cmd dbus-send && [[ -n "${DBUS_SESSION_BUS_ADDRESS:-}" ]]; then
    dbus-send --session --type=signal "${BUS_PATH}" \
      "${BUS_INTERFACE}.DeviceChanged" \
      string:"${domain}" string:"${detail}" >/dev/null 2>&1 || true
  fi
}

monitor_command() {
  local domain="$1"
  shift

  while true; do
    if have_cmd "$1"; then
      "$@" 2>&1 | while IFS= read -r line; do
        [[ -n "${line}" ]] || continue
        printf '%s\t%s\n' "${domain}" "${line}"
      done
    fi
    sleep 2
  done
}

monitor() {
  local fifo=""
  local domain=""
  local detail=""
  local last_domain=""
  local last_emit_ms=0
  local now_ms=0

  mkdir -p "${CACHE_DIR}"
  fifo="$(mktemp -u "${CACHE_DIR}/device-events.XXXXXX.fifo")"
  EVENT_FIFO="${fifo}"
  mkfifo "${fifo}"

  trap cleanup EXIT
  trap 'exit 0' INT TERM

  monitor_command network nmcli monitor > "${fifo}" &
  monitor_command bluetooth dbus-monitor --system "type='signal',sender='org.bluez'" > "${fifo}" &
  monitor_command audio pactl subscribe > "${fifo}" &
  emit_event all started

  while IFS=$'\t' read -r domain detail; do
    [[ -n "${domain}" ]] || continue
    now_ms="${EPOCHREALTIME/./}"
    if [[ "${domain}" == "${last_domain}" ]] && (( now_ms - last_emit_ms < 250000 )); then
      continue
    fi
    emit_event "${domain}" "${detail:0:240}"
    last_domain="${domain}"
    last_emit_ms="${now_ms}"
  done < "${fifo}"
}

case "${MODE}" in
  emit)
    emit_event "${2:-all}" "${3:-manual}"
    ;;
  monitor)
    monitor
    ;;
  status)
    if [[ -f "${EVENT_FILE}" ]]; then
      cat "${EVENT_FILE}"
    else
      echo "No device event has been recorded yet."
    fi
    ;;
  *)
    echo "Usage: $0 {monitor|status|emit [domain] [detail]}" >&2
    exit 2
    ;;
esac
