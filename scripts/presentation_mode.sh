#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="${1:-prep}"
PROFILE_NAME="${2:-${PRESENT_PROFILE:-present}}"
PROFILE_FILE="${PRESENT_PROFILE_FILE:-${ROOT_DIR}/config/presentation_profiles.json}"

# Profile-derived values (optional; loaded from JSON)
PROFILE_VOLUME=""
PROFILE_WACOM_MODE=""
PROFILE_LIVE_DELAY_SEC=""
PROFILE_REVEAL_URL=""
PROFILE_CODE_URL=""
PROFILE_BLUETOOTH_REFRESH=""
PROFILE_LAUNCH_CONTROL_CENTER=""
PROFILE_LAUNCH_TELEPROMPTER=""
PROFILE_LAUNCH_PRESENTER_CANVAS=""
PROFILE_NOTIFY=""

have() {
  command -v "$1" >/dev/null 2>&1
}

is_true() {
  case "${1:-}" in
    1|true|TRUE|yes|YES|on|ON) return 0 ;;
    *) return 1 ;;
  esac
}

log() {
  printf '[present-mode] %s\n' "$1"
}

load_profile_from_json() {
  local file="$1"
  local profile="$2"
  local row key value

  if [[ ! -f "${file}" ]]; then
    return 0
  fi
  if ! have python3; then
    log "python3 not found; skipping profile file ${file}."
    return 0
  fi

  while IFS= read -r row; do
    [[ -z "${row}" ]] && continue
    key="${row%%=*}"
    value="${row#*=}"
    case "${key}" in
      volume) PROFILE_VOLUME="${value}" ;;
      wacom_mode) PROFILE_WACOM_MODE="${value}" ;;
      live_delay_sec) PROFILE_LIVE_DELAY_SEC="${value}" ;;
      reveal_url) PROFILE_REVEAL_URL="${value}" ;;
      code_url) PROFILE_CODE_URL="${value}" ;;
      bluetooth_refresh) PROFILE_BLUETOOTH_REFRESH="${value}" ;;
      launch_control_center) PROFILE_LAUNCH_CONTROL_CENTER="${value}" ;;
      launch_teleprompter) PROFILE_LAUNCH_TELEPROMPTER="${value}" ;;
      launch_presenter_canvas) PROFILE_LAUNCH_PRESENTER_CANVAS="${value}" ;;
      notify) PROFILE_NOTIFY="${value}" ;;
    esac
  done < <(python3 - "${file}" "${profile}" <<'PY'
import json
import sys
from pathlib import Path

path = Path(sys.argv[1])
profile_name = sys.argv[2]

try:
    data = json.loads(path.read_text())
except Exception:
    sys.exit(0)

profiles = data.get("profiles") or {}
profile = profiles.get(profile_name) or {}
if not isinstance(profile, dict):
    sys.exit(0)

keys = [
    "volume", "wacom_mode", "live_delay_sec", "reveal_url", "code_url",
    "bluetooth_refresh", "launch_control_center", "launch_teleprompter",
    "launch_presenter_canvas", "notify",
]

for key in keys:
    if key not in profile:
        continue
    value = profile[key]
    if isinstance(value, bool):
        value = "1" if value else "0"
    elif value is None:
        value = ""
    print(f"{key}={value}")
PY
)
}

load_profile_from_json "${PROFILE_FILE}" "${PROFILE_NAME}"

# Final values: env var overrides profile file, profile overrides hard defaults.
PRESENT_VOLUME="${PRESENT_VOLUME:-${PROFILE_VOLUME:-35}}"
PRESENT_WACOM_MODE="${PRESENT_WACOM_MODE:-${PROFILE_WACOM_MODE:-external}}"
PRESENT_LIVE_DELAY_SEC="${PRESENT_LIVE_DELAY_SEC:-${PROFILE_LIVE_DELAY_SEC:-0.35}}"
REVEAL_URL="${REVEAL_URL:-${PROFILE_REVEAL_URL:-http://127.0.0.1:8000}}"
CODE_URL="${CODE_URL:-${PROFILE_CODE_URL:-}}"
PRESENT_BLUETOOTH_REFRESH="${PRESENT_BLUETOOTH_REFRESH:-${PROFILE_BLUETOOTH_REFRESH:-1}}"
PRESENT_LAUNCH_CONTROL_CENTER="${PRESENT_LAUNCH_CONTROL_CENTER:-${PROFILE_LAUNCH_CONTROL_CENTER:-1}}"
PRESENT_LAUNCH_TELEPROMPTER="${PRESENT_LAUNCH_TELEPROMPTER:-${PROFILE_LAUNCH_TELEPROMPTER:-1}}"
PRESENT_LAUNCH_PRESENTER_CANVAS="${PRESENT_LAUNCH_PRESENTER_CANVAS:-${PROFILE_LAUNCH_PRESENTER_CANVAS:-0}}"
PRESENT_NOTIFY="${PRESENT_NOTIFY:-${PROFILE_NOTIFY:-1}}"

notify_user() {
  local msg="$1"
  if is_true "${PRESENT_NOTIFY}" && have notify-send; then
    notify-send "Presentation Mode (${PROFILE_NAME})" "${msg}" >/dev/null 2>&1 || true
  fi
}

apply_audio_profile() {
  if ! have pactl; then
    log "pactl not found; skipping audio profile."
    return 0
  fi
  log "Setting audio profile (unmute + volume ${PRESENT_VOLUME}%)."
  pactl set-sink-mute @DEFAULT_SINK@ 0 >/dev/null 2>&1 || true
  pactl set-sink-volume @DEFAULT_SINK@ "${PRESENT_VOLUME}%" >/dev/null 2>&1 || true
}

apply_bluetooth_profile() {
  if ! is_true "${PRESENT_BLUETOOTH_REFRESH}"; then
    log "Bluetooth refresh disabled by profile."
    return 0
  fi
  if [[ ! -x "${ROOT_DIR}/scripts/bluetooth_refresh.sh" ]]; then
    log "Bluetooth helper not found; skipping."
    return 0
  fi
  log "Refreshing Bluetooth trust/reconnect."
  "${ROOT_DIR}/scripts/bluetooth_refresh.sh" trust >/dev/null 2>&1 || true
}

apply_wacom_profile() {
  if [[ -z "${DISPLAY:-}" ]]; then
    log "DISPLAY not set; skipping Wacom mapping."
    return 0
  fi
  if ! have make; then
    log "make not found; skipping Wacom mapping."
    return 0
  fi
  if [[ "${PRESENT_WACOM_MODE}" == "none" ]]; then
    log "Wacom mapping skipped (PRESENT_WACOM_MODE=none)."
    return 0
  fi
  log "Applying Wacom mode: ${PRESENT_WACOM_MODE}"
  (
    cd "${ROOT_DIR}"
    case "${PRESENT_WACOM_MODE}" in
      default)
        make --no-print-directory wacom >/dev/null 2>&1 || true
        ;;
      external)
        make --no-print-directory wacom-external >/dev/null 2>&1 || true
        ;;
      switch)
        make --no-print-directory wacom-switch >/dev/null 2>&1 || true
        ;;
      *)
        make --no-print-directory wacom-set-screen OUTPUT="${PRESENT_WACOM_MODE}" >/dev/null 2>&1 || true
        ;;
    esac
  )
}

launch_support_apps() {
  log "Launching support apps."
  (
    cd "${ROOT_DIR}"
    if is_true "${PRESENT_LAUNCH_CONTROL_CENTER}"; then
      if [[ -x ./build/bin/linux_control_center ]]; then
        ./build/bin/linux_control_center >/dev/null 2>&1 &
      elif [[ -x "${HOME}/Programs/bin/linux_control_center" ]]; then
        "${HOME}/Programs/bin/linux_control_center" >/dev/null 2>&1 &
      fi
    fi

    if is_true "${PRESENT_LAUNCH_TELEPROMPTER}" && [[ -x ./launch_teleprompter.sh ]]; then
      ./launch_teleprompter.sh >/dev/null 2>&1 &
    fi

    if is_true "${PRESENT_LAUNCH_PRESENTER_CANVAS}" && [[ -x ./launch_presenter_canvas.sh ]]; then
      ./launch_presenter_canvas.sh >/dev/null 2>&1 &
    fi
  )
}

prep_mode() {
  log "Applying profile '${PROFILE_NAME}' (${PROFILE_FILE})."
  apply_audio_profile
  apply_bluetooth_profile
  apply_wacom_profile
  launch_support_apps
  notify_user "Prep complete (audio, bluetooth, wacom, support apps)."
  log "Presentation prep complete."
}

live_mode() {
  prep_mode
  log "Launching present-live workflow."
  (
    cd "${ROOT_DIR}"
    REVEAL_URL="${REVEAL_URL}" \
    CODE_URL="${CODE_URL}" \
    PRESENT_WACOM_MODE="none" \
    PRESENT_LIVE_DELAY_SEC="${PRESENT_LIVE_DELAY_SEC}" \
    ./launch_present_live.sh >/tmp/lcu_present_live.log 2>&1 &
  )
  notify_user "Live presentation workflow launched."
}

list_profiles() {
  if [[ ! -f "${PROFILE_FILE}" ]] || ! have python3; then
    echo "present"
    echo "work"
    echo "record"
    return 0
  fi
  python3 - "${PROFILE_FILE}" <<'PY'
import json
import sys
from pathlib import Path

path = Path(sys.argv[1])
try:
    data = json.loads(path.read_text())
except Exception:
    sys.exit(0)
profiles = data.get("profiles") or {}
for name in sorted(profiles.keys()):
    print(name)
PY
}

case "${MODE}" in
  prep)
    prep_mode
    ;;
  live)
    live_mode
    ;;
  list)
    list_profiles
    ;;
  *)
    echo "Usage: $0 [prep|live|list] [profile]"
    exit 1
    ;;
esac
