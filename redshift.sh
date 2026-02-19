#!/usr/bin/env bash
set -euo pipefail

# Redshift helper with both auto (time-based) and manual controls.
#
# Usage:
#   ./redshift.sh auto                 # start auto mode (day/night schedule)
#   ./redshift.sh off                  # disable tint (reset color temperature)
#   ./redshift.sh set 4200             # force a specific temperature (Kelvin)
#   ./redshift.sh warm [step]          # make warmer (default step: 300K)
#   ./redshift.sh cool [step]          # make cooler (default step: 300K)
#   ./redshift.sh status               # show current manual state
#
# Optional env vars:
#   REDSHIFT_LOCATION="lat:lon"        # default: 4.65:-74.06
#   REDSHIFT_DAY_TEMP="5700"           # default day temp
#   REDSHIFT_NIGHT_TEMP="3600"         # default night temp
#   REDSHIFT_GAMMA="0.8"               # default gamma

REDSHIFT_LOCATION="${REDSHIFT_LOCATION:-4.65:-74.06}"
REDSHIFT_DAY_TEMP="${REDSHIFT_DAY_TEMP:-5700}"
REDSHIFT_NIGHT_TEMP="${REDSHIFT_NIGHT_TEMP:-3600}"
REDSHIFT_GAMMA="${REDSHIFT_GAMMA:-0.8}"
REDSHIFT_STEP_DEFAULT=300
REDSHIFT_MIN_TEMP=1000
REDSHIFT_MAX_TEMP=25000
STATE_FILE="${XDG_CACHE_HOME:-$HOME/.cache}/redshift_manual_temp"

have_redshift() {
    command -v redshift >/dev/null 2>&1
}

kill_redshift_processes() {
    pkill -x redshift >/dev/null 2>&1 || true
    pkill -x redshift-gtk >/dev/null 2>&1 || true
}

clamp_temp() {
    local temp="$1"
    if (( temp < REDSHIFT_MIN_TEMP )); then
        temp=$REDSHIFT_MIN_TEMP
    elif (( temp > REDSHIFT_MAX_TEMP )); then
        temp=$REDSHIFT_MAX_TEMP
    fi
    printf '%s\n' "$temp"
}

reset_tint() {
    redshift -x >/dev/null 2>&1 || true
}

set_manual_temp() {
    local temp="$1"
    temp="$(clamp_temp "$temp")"
    # Stop background auto controllers so manual mode persists.
    kill_redshift_processes
    reset_tint
    redshift -P -O "$temp" -m randr >/dev/null 2>&1
    mkdir -p "$(dirname "$STATE_FILE")"
    printf '%s\n' "$temp" > "$STATE_FILE"
    echo "Redshift manual temperature set to ${temp}K."
}

current_temp() {
    if [[ -f "$STATE_FILE" ]] && [[ "$(cat "$STATE_FILE")" =~ ^[0-9]+$ ]]; then
        cat "$STATE_FILE"
    else
        echo "4500"
    fi
}

start_auto_mode() {
    kill_redshift_processes
    reset_tint
    rm -f "$STATE_FILE"

    if command -v redshift-gtk >/dev/null 2>&1; then
        redshift-gtk >/dev/null 2>&1 &
        disown
        echo "Redshift auto mode started via redshift-gtk."
        return 0
    fi

    redshift \
        -l "$REDSHIFT_LOCATION" \
        -t "${REDSHIFT_DAY_TEMP}:${REDSHIFT_NIGHT_TEMP}" \
        -g "$REDSHIFT_GAMMA" \
        -m randr >/dev/null 2>&1 &
    disown
    echo "Redshift auto mode started (location ${REDSHIFT_LOCATION})."
}

turn_off() {
    kill_redshift_processes
    reset_tint
    rm -f "$STATE_FILE"
    echo "Redshift disabled and display reset."
}

print_status() {
    local running="not running"
    local mode="off"
    local local_time
    local probe line
    local solar_location=""
    local solar_period=""
    local solar_temp=""
    local solar_brightness=""

    local_time="$(date '+%Y-%m-%d %H:%M:%S %Z (%A)')"

    if pgrep -x redshift >/dev/null 2>&1 || pgrep -x redshift-gtk >/dev/null 2>&1; then
        running="running"
        mode="auto"
    fi

    if [[ -f "$STATE_FILE" ]]; then
        mode="manual"
        echo "Manual mode: $(cat "$STATE_FILE")K"
    else
        echo "Manual mode: not set"
    fi

    echo "Mode: $mode"
    echo "Auto/background process: $running"
    echo "Local time: $local_time"
    echo "Configured location: $REDSHIFT_LOCATION"
    echo "Schedule: day ${REDSHIFT_DAY_TEMP}K, night ${REDSHIFT_NIGHT_TEMP}K, gamma ${REDSHIFT_GAMMA}"

    probe="$(redshift -p -l "$REDSHIFT_LOCATION" 2>/dev/null || true)"
    while IFS= read -r line; do
        case "$line" in
            "Location:"*)
                solar_location="${line#Location: }"
                ;;
            "Period:"*)
                solar_period="${line#Period: }"
                ;;
            "Color temperature:"*)
                solar_temp="${line#Color temperature: }"
                ;;
            "Brightness:"*)
                solar_brightness="${line#Brightness: }"
                ;;
        esac
    done <<< "$probe"

    if [[ -n "$solar_location" ]]; then
        echo "Solar location: $solar_location"
    fi
    if [[ -n "$solar_period" ]]; then
        echo "Solar period: $solar_period"
    fi
    if [[ -n "$solar_temp" ]]; then
        echo "Solar color temperature: $solar_temp"
    fi
    if [[ -n "$solar_brightness" ]]; then
        echo "Solar brightness: $solar_brightness"
    fi
}

usage() {
    cat <<'EOF'
Usage:
  ./redshift.sh auto                 # start auto mode (day/night schedule)
  ./redshift.sh off                  # disable tint (reset color temperature)
  ./redshift.sh set 4200             # force a specific temperature (Kelvin)
  ./redshift.sh warm [step]          # make warmer (default step: 300K)
  ./redshift.sh cool [step]          # make cooler (default step: 300K)
  ./redshift.sh status               # show current manual state

Optional env vars:
  REDSHIFT_LOCATION="lat:lon"        # default: 4.65:-74.06
  REDSHIFT_DAY_TEMP="5700"           # default day temp
  REDSHIFT_NIGHT_TEMP="3600"         # default night temp
  REDSHIFT_GAMMA="0.8"               # default gamma
EOF
}

if ! have_redshift; then
    echo "Error: redshift is not installed."
    echo "Install with: sudo apt install redshift redshift-gtk"
    exit 1
fi

cmd="${1:-auto}"

case "$cmd" in
    auto)
        start_auto_mode
        ;;
    off|reset)
        turn_off
        ;;
    set)
        if [[ -z "${2:-}" || ! "${2:-}" =~ ^[0-9]+$ ]]; then
            echo "Usage: $0 set <kelvin>"
            exit 1
        fi
        set_manual_temp "$2"
        ;;
    warm)
        step="${2:-$REDSHIFT_STEP_DEFAULT}"
        if ! [[ "$step" =~ ^[0-9]+$ ]]; then
            echo "Usage: $0 warm [step_kelvin]"
            exit 1
        fi
        current="$(current_temp)"
        set_manual_temp "$(( current - step ))"
        ;;
    cool)
        step="${2:-$REDSHIFT_STEP_DEFAULT}"
        if ! [[ "$step" =~ ^[0-9]+$ ]]; then
            echo "Usage: $0 cool [step_kelvin]"
            exit 1
        fi
        current="$(current_temp)"
        set_manual_temp "$(( current + step ))"
        ;;
    status)
        print_status
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        echo "Unknown command: $cmd"
        usage
        exit 1
        ;;
esac
