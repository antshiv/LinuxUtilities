#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-status}"
SCRIPT_PATH="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/$(basename -- "${BASH_SOURCE[0]}")"
CACHE_DIR="${XDG_CACHE_HOME:-${HOME}/.cache}/linuxutilities"
PID_FILE="${CACHE_DIR}/audio_source_route.pid"
STATE_FILE="${CACHE_DIR}/audio_source_route.state"
LOG_FILE="${CACHE_DIR}/audio_source_route.log"
AUTO_INTERVAL_SEC="${AUDIO_AUTO_ROUTE_INTERVAL_SEC:-2}"
EXTERNAL_GAIN="${AUDIO_EXTERNAL_SOURCE_GAIN:-100}"
INTERNAL_GAIN="${AUDIO_INTERNAL_SOURCE_GAIN:-50}"
PREFERRED_EXTERNAL_SOURCE="${AUDIO_EXTERNAL_SOURCE:-}"
PREFERRED_INTERNAL_SOURCE="${AUDIO_INTERNAL_SOURCE:-}"
NOTIFY_ENABLED="${AUDIO_ROUTE_NOTIFY:-1}"

have() {
  command -v "$1" >/dev/null 2>&1
}

ensure_cache_dir() {
  mkdir -p "${CACHE_DIR}"
}

fail() {
  echo "$*" >&2
  exit 1
}

is_true() {
  case "${1:-}" in
    1|true|TRUE|yes|YES|on|ON)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

notify_user() {
  local title="$1"
  local body="$2"
  if is_true "${NOTIFY_ENABLED}" && have notify-send; then
    notify-send "${title}" "${body}" >/dev/null 2>&1 || true
  fi
}

require_audio_stack() {
  have pactl || fail "Missing pactl."
  pactl info >/dev/null 2>&1 || fail "Audio server unavailable."
}

list_sources_tsv() {
  pactl list sources | awk '
    function flush() {
      if (id != "" && name != "" && name !~ /\.monitor$/) {
        printf "%s\t%s\t%s\t%s\t%s\n", id, name, desc, state, mute
      }
    }
    /^Source #[0-9]+/ {
      flush()
      id = substr($2, 2)
      name = ""
      desc = ""
      state = ""
      mute = ""
      next
    }
    $1 == "State:" {
      state = $2
      next
    }
    $1 == "Name:" {
      name = $2
      next
    }
    $1 == "Description:" {
      sub(/^[[:space:]]*Description:[[:space:]]*/, "", $0)
      desc = $0
      next
    }
    $1 == "Mute:" {
      mute = $2
      next
    }
    END {
      flush()
    }
  '
}

list_recording_streams_tsv() {
  pactl list source-outputs | awk '
    function unquote(value) {
      gsub(/^"/, "", value)
      gsub(/"$/, "", value)
      return value
    }
    function flush() {
      if (id != "" && monitor != "true" && app_name != "" &&
          app_name != "PulseAudio Volume Control") {
        printf "%s\t%s\t%s\t%s\t%s\n", id, source, app_name, media_name, app_binary
      }
    }
    /^Source Output #[0-9]+/ {
      flush()
      id = substr($3, 2)
      source = ""
      app_name = ""
      media_name = ""
      app_binary = ""
      monitor = ""
      next
    }
    $1 == "Source:" {
      source = $2
      next
    }
    /application.name = / {
      split($0, parts, " = ")
      app_name = unquote(parts[2])
      next
    }
    /application.process.binary = / {
      split($0, parts, " = ")
      app_binary = unquote(parts[2])
      next
    }
    /media.name = / {
      split($0, parts, " = ")
      media_name = unquote(parts[2])
      next
    }
    /stream.monitor = / {
      split($0, parts, " = ")
      monitor = unquote(parts[2])
      next
    }
    END {
      flush()
    }
  '
}

source_name_by_id() {
  local source_id="$1"
  list_sources_tsv | awk -F'\t' -v source_id="${source_id}" '$1 == source_id {print $2; exit}'
}

source_desc_by_name() {
  local source_name="$1"
  list_sources_tsv | awk -F'\t' -v source_name="${source_name}" '$2 == source_name {print $3; exit}'
}

source_id_by_name() {
  local source_name="$1"
  list_sources_tsv | awk -F'\t' -v source_name="${source_name}" '$2 == source_name {print $1; exit}'
}

resolve_source_spec() {
  local spec="$1"
  [[ -n "${spec}" ]] || return 1
  list_sources_tsv | awk -F'\t' -v spec="${spec}" '
    $1 == spec || $2 == spec || $3 == spec {
      printf "%s\t%s\t%s\t%s\t%s\n", $1, $2, $3, $4, $5
      exit
    }
  '
}

pick_external_source() {
  local resolved=""
  if [[ -n "${PREFERRED_EXTERNAL_SOURCE}" ]]; then
    resolved="$(resolve_source_spec "${PREFERRED_EXTERNAL_SOURCE}" || true)"
    [[ -n "${resolved}" ]] && {
      printf "%s\n" "${resolved}"
      return 0
    }
  fi

  list_sources_tsv | awk -F'\t' '
    $3 ~ /(Headphones Stereo Microphone|Headset Microphone|External Microphone)/ {
      print; exit
    }
    $2 ~ /^alsa_input\.usb-/ || $3 ~ /USB/ {
      print; exit
    }
  '
}

pick_internal_source() {
  local resolved=""
  if [[ -n "${PREFERRED_INTERNAL_SOURCE}" ]]; then
    resolved="$(resolve_source_spec "${PREFERRED_INTERNAL_SOURCE}" || true)"
    [[ -n "${resolved}" ]] && {
      printf "%s\n" "${resolved}"
      return 0
    }
  fi

  list_sources_tsv | awk -F'\t' '
    $3 ~ /(Digital Microphone|Internal Microphone|Built-in Audio)/ {
      print; exit
    }
  '
}

default_source_name() {
  pactl info | awk -F': ' '/Default Source/ {print $2; exit}'
}

default_sink_name() {
  pactl info | awk -F': ' '/Default Sink/ {print $2; exit}'
}

set_default_source() {
  local source_name="$1"
  local source_id=""
  [[ -n "${source_name}" ]] || return 1
  pactl set-default-source "${source_name}" >/dev/null 2>&1 || return 1
  source_id="$(source_id_by_name "${source_name}" || true)"
  if [[ -n "${source_id}" ]] && have wpctl; then
    wpctl set-default "${source_id}" >/dev/null 2>&1 || true
  fi
}

prepare_source() {
  local source_name="$1"
  local volume_pct="$2"
  [[ -n "${source_name}" ]] || return 1
  pactl set-source-mute "${source_name}" 0 >/dev/null 2>&1 || true
  pactl set-source-volume "${source_name}" "${volume_pct}%" >/dev/null 2>&1 || true
}

move_recorders_to_source() {
  local source_name="$1"
  local stream_id=""
  [[ -n "${source_name}" ]] || return 1
  while IFS=$'\t' read -r stream_id _ _ _ _; do
    [[ -n "${stream_id}" ]] || continue
    pactl move-source-output "${stream_id}" "${source_name}" >/dev/null 2>&1 || true
  done < <(list_recording_streams_tsv)
}

format_source_line() {
  local tsv="$1"
  awk -F'\t' '{printf "%s (%s, state=%s, mute=%s)\n", $3, $2, $4, $5}' <<<"${tsv}"
}

running_pid() {
  if [[ -f "${PID_FILE}" ]]; then
    local pid
    pid="$(cat "${PID_FILE}" 2>/dev/null || true)"
    if [[ -n "${pid}" ]] && kill -0 "${pid}" >/dev/null 2>&1; then
      printf "%s\n" "${pid}"
      return 0
    fi
  fi
  return 1
}

write_state() {
  local target_name="$1"
  local target_desc="$2"
  ensure_cache_dir
  cat > "${STATE_FILE}" <<EOF
last_target_name=${target_name}
last_target_desc=${target_desc}
EOF
}

load_state_value() {
  local key="$1"
  if [[ -f "${STATE_FILE}" ]]; then
    awk -F'=' -v key="${key}" '$1 == key {print substr($0, index($0, "=") + 1); exit}' "${STATE_FILE}"
  fi
}

apply_target_source() {
  local source_tsv="$1"
  local notify_message="$2"
  local gain_pct="$3"
  local source_id source_name source_desc

  source_id="$(awk -F'\t' '{print $1}' <<<"${source_tsv}")"
  source_name="$(awk -F'\t' '{print $2}' <<<"${source_tsv}")"
  source_desc="$(awk -F'\t' '{print $3}' <<<"${source_tsv}")"

  [[ -n "${source_id}" && -n "${source_name}" ]] || return 1

  set_default_source "${source_name}"
  prepare_source "${source_name}" "${gain_pct}"
  move_recorders_to_source "${source_name}"

  if [[ "$(load_state_value last_target_name)" != "${source_name}" ]]; then
    notify_user "LinuxUtilities Audio" "${notify_message}"
  fi
  write_state "${source_name}" "${source_desc}"
  printf "Routed input to %s (%s)\n" "${source_desc}" "${source_name}"
}

use_external_source() {
  local source_tsv=""
  require_audio_stack
  source_tsv="$(pick_external_source || true)"
  [[ -n "${source_tsv}" ]] || fail "No external microphone source detected."
  apply_target_source "${source_tsv}" "External mic detected. Switched input." "${EXTERNAL_GAIN}"
}

use_internal_source() {
  local source_tsv=""
  require_audio_stack
  source_tsv="$(pick_internal_source || true)"
  [[ -n "${source_tsv}" ]] || fail "No internal microphone source detected."
  apply_target_source "${source_tsv}" "External mic removed. Reverted to internal mic." "${INTERNAL_GAIN}"
}

route_recorders() {
  local target_name=""
  require_audio_stack
  target_name="$(default_source_name || true)"
  [[ -n "${target_name}" ]] || fail "No default source available."
  move_recorders_to_source "${target_name}"
  printf "Moved active recording apps to %s\n" "${target_name}"
}

reconcile_sources() {
  local external_tsv=""
  local internal_tsv=""
  external_tsv="$(pick_external_source || true)"
  internal_tsv="$(pick_internal_source || true)"

  if [[ -n "${external_tsv}" ]]; then
    apply_target_source "${external_tsv}" "External mic detected. Switched input." "${EXTERNAL_GAIN}" >/dev/null
    return 0
  fi

  if [[ -n "${internal_tsv}" ]]; then
    apply_target_source "${internal_tsv}" "External mic removed. Reverted to internal mic." "${INTERNAL_GAIN}" >/dev/null
    return 0
  fi

  return 1
}

watch_loop() {
  ensure_cache_dir
  echo "$$" > "${PID_FILE}"
  trap 'rm -f "${PID_FILE}"' EXIT
  while true; do
    reconcile_sources || true
    sleep "${AUTO_INTERVAL_SEC}"
  done
}

auto_start() {
  local pid=""
  ensure_cache_dir
  if pid="$(running_pid || true)" && [[ -n "${pid}" ]]; then
    printf "Audio auto-route already running (pid %s)\n" "${pid}"
    return 0
  fi
  nohup "${SCRIPT_PATH}" watch-loop >> "${LOG_FILE}" 2>&1 &
  sleep 0.2
  pid="$(running_pid || true)"
  [[ -n "${pid}" ]] || fail "Failed to start audio auto-route watcher."
  printf "Started audio auto-route watcher (pid %s)\n" "${pid}"
}

auto_stop() {
  local pid=""
  pid="$(running_pid || true)"
  if [[ -z "${pid}" ]]; then
    echo "Audio auto-route watcher is not running."
    return 0
  fi
  kill "${pid}" >/dev/null 2>&1 || true
  rm -f "${PID_FILE}"
  echo "Stopped audio auto-route watcher."
}

auto_status() {
  local pid=""
  pid="$(running_pid || true)"
  if [[ -n "${pid}" ]]; then
    printf "Auto-route: running (pid %s)\n" "${pid}"
  else
    echo "Auto-route: stopped"
  fi
}

status_report() {
  local external_tsv internal_tsv current_source current_sink current_source_desc
  current_source="$(default_source_name || true)"
  current_sink="$(default_sink_name || true)"
  current_source_desc="$(source_desc_by_name "${current_source}" || true)"
  external_tsv="$(pick_external_source || true)"
  internal_tsv="$(pick_internal_source || true)"

  printf "Default sink: %s\n" "${current_sink:-unknown}"
  printf "Default source: %s\n" "${current_source_desc:-unknown}"
  [[ -n "${current_source}" ]] && printf "Default source node: %s\n" "${current_source}"
  auto_status
  echo
  if [[ -n "${external_tsv}" ]]; then
    printf "External mic candidate: %s" "$(format_source_line "${external_tsv}")"
  else
    echo "External mic candidate: not detected"
  fi
  if [[ -n "${internal_tsv}" ]]; then
    printf "Internal mic candidate: %s" "$(format_source_line "${internal_tsv}")"
  else
    echo "Internal mic candidate: not detected"
  fi
  echo
  echo "Recording apps:"
  if list_recording_streams_tsv | grep -q .; then
    while IFS=$'\t' read -r stream_id source_id app_name media_name app_binary; do
      local_source_name="$(source_name_by_id "${source_id}" || true)"
      local_source_desc="$(source_desc_by_name "${local_source_name}" || true)"
      printf -- "- %s [%s] via %s (%s)\n" \
        "${app_name}" \
        "${app_binary:-unknown}" \
        "${local_source_desc:-unknown source}" \
        "${stream_id}"
      [[ -n "${media_name}" ]] && printf "  media: %s\n" "${media_name}"
    done < <(list_recording_streams_tsv)
  else
    echo "- none"
  fi
}

gui_summary() {
  status_report | sed '/^$/N;/^\n$/D'
}

print_help() {
  cat <<'TXT'
audio_source_route.sh

Usage:
  ./scripts/audio_source_route.sh status
  ./scripts/audio_source_route.sh gui-summary
  ./scripts/audio_source_route.sh use-external
  ./scripts/audio_source_route.sh use-internal
  ./scripts/audio_source_route.sh route-recorders
  ./scripts/audio_source_route.sh auto-start
  ./scripts/audio_source_route.sh auto-stop
  ./scripts/audio_source_route.sh auto-status

Notes:
  - Prefers an external analog/USB microphone when detected.
  - Moves active recording apps (Audacity, Teams, browsers, etc.) to the target source.
  - Reverts to the internal mic when the external source disappears.
  - Set AUDIO_EXTERNAL_SOURCE or AUDIO_INTERNAL_SOURCE to force a specific source.
TXT
}

case "${MODE}" in
  status)
    require_audio_stack
    status_report
    ;;
  gui-summary)
    require_audio_stack
    gui_summary
    ;;
  use-external)
    use_external_source
    ;;
  use-internal)
    use_internal_source
    ;;
  route-recorders)
    route_recorders
    ;;
  auto-start)
    require_audio_stack
    auto_start
    ;;
  auto-stop)
    auto_stop
    ;;
  auto-status)
    auto_status
    ;;
  watch-loop)
    require_audio_stack
    watch_loop
    ;;
  help|-h|--help)
    print_help
    ;;
  *)
    echo "Unknown mode: ${MODE}" >&2
    print_help >&2
    exit 2
    ;;
esac
