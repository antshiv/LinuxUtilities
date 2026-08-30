#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-help}"
PREFERRED_BT_CARD="${BT_CARD:-}"
PREFERRED_BT_DEVICE="${BT_DEVICE:-}"
PREFERRED_OUTPUT_SINK="${AUDIO_OUTPUT_SINK:-}"
CACHE_DIR="${XDG_CACHE_HOME:-${HOME}/.cache}/linuxutilities"
PID_FILE="${CACHE_DIR}/audio_bt_profile.pid"
STATE_FILE="${CACHE_DIR}/audio_bt_profile.state"
LOG_FILE="${CACHE_DIR}/audio_bt_profile.log"
AUTO_RESTORE_GRACE_SEC="${AUDIO_BT_RESTORE_GRACE_SEC:-3}"
NOTIFY_ENABLED="${AUDIO_BT_NOTIFY:-1}"
SYSTEMD_UNIT="linuxutilities-bt-audio.service"

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

pactl_server_available() {
  pactl info >/dev/null 2>&1
}

fail() {
  echo "$*" >&2
  exit 1
}

is_true() {
  case "${1:-}" in
    1|true|TRUE|yes|YES|on|ON) return 0 ;;
    *) return 1 ;;
  esac
}

ensure_cache_dir() {
  mkdir -p "${CACHE_DIR}"
}

notify_mode() {
  local mode="$1"
  local detail="$2"
  if is_true "${NOTIFY_ENABLED}" && have_cmd notify-send; then
    notify-send -a LinuxUtilities "Sony headset: ${mode}" "${detail}" >/dev/null 2>&1 || true
  fi
}

write_mode_state() {
  local mode="$1"
  ensure_cache_dir
  printf 'mode=%s\n' "${mode}" > "${STATE_FILE}"
}

read_mode_state() {
  if [[ -f "${STATE_FILE}" ]]; then
    awk -F= '$1 == "mode" {print $2; exit}' "${STATE_FILE}"
  fi
}

list_bt_cards() {
  pactl list cards short | awk '$2 ~ /^bluez_card\./ {print $2}'
}

is_audio_device_info() {
  grep -Eq 'UUID: (Audio Sink|Headset|Handsfree|Advanced Audio Distribu|A/V Remote Control)'
}

device_info() {
  local mac="$1"
  bluetoothctl info "${mac}" 2>/dev/null || true
}

mac_to_card_name() {
  local mac="$1"
  echo "bluez_card.${mac//:/_}"
}

card_name_to_mac() {
  local card="$1"
  card="${card#bluez_card.}"
  echo "${card//_/:}"
}

first_audio_device_from() {
  local scope="$1"
  local cmd=(bluetoothctl devices "${scope}")
  local mac=""

  if [[ "${scope}" == "Paired" ]]; then
    cmd=(bluetoothctl devices Paired)
  fi

  while read -r _ mac _; do
    [[ -n "${mac}" ]] || continue
    if device_info "${mac}" | is_audio_device_info; then
      echo "${mac}"
      return 0
    fi
  done < <("${cmd[@]}" 2>/dev/null || true)
  return 1
}

pick_bt_device() {
  local mac=""

  if [[ -n "${PREFERRED_BT_DEVICE}" ]]; then
    echo "${PREFERRED_BT_DEVICE^^}"
    return 0
  fi

  if [[ -n "${PREFERRED_BT_CARD}" ]]; then
    card_name_to_mac "${PREFERRED_BT_CARD^^}"
    return 0
  fi

  mac="$(first_audio_device_from Connected || true)"
  if [[ -n "${mac}" ]]; then
    echo "${mac}"
    return 0
  fi

  mac="$(first_audio_device_from Paired || true)"
  [[ -n "${mac}" ]] || return 1
  echo "${mac}"
}

pick_bt_card() {
  local first_card=""
  if [[ -n "${PREFERRED_BT_DEVICE}" ]]; then
    mac_to_card_name "${PREFERRED_BT_DEVICE^^}"
    return 0
  fi
  if [[ -n "${PREFERRED_BT_CARD}" ]]; then
    echo "${PREFERRED_BT_CARD}"
    return 0
  fi
  first_card="$(list_bt_cards | head -n 1 || true)"
  [[ -n "${first_card}" ]] || return 1
  echo "${first_card}"
}

set_profile_first_supported() {
  local card="$1"
  shift
  local profile
  for profile in "$@"; do
    if pactl set-card-profile "${card}" "${profile}" >/dev/null 2>&1; then
      echo "Set ${card} -> ${profile}"
      return 0
    fi
  done
  return 1
}

first_non_bt_sink() {
  pactl list short sinks | awk '$2 !~ /^bluez_output\./ {print $2; exit}'
}

first_bt_sink() {
  pactl list short sinks | awk '$2 ~ /^bluez_output\./ {print $2; exit}'
}

bt_sink_for_device() {
  local mac="$1"
  local prefix="bluez_output.${mac//:/_}."
  pactl list short sinks | awk -v prefix="${prefix}" '$2 ~ ("^" prefix) {print $2; exit}'
}

bt_source_for_device() {
  local mac="$1"
  local prefix="bluez_input.${mac//:/_}."
  pactl list short sources | awk -v prefix="${prefix}" '$2 ~ ("^" prefix) {print $2; exit}'
}

first_bt_source() {
  pactl list short sources | awk '$2 ~ /^bluez_input\./ {print $2; exit}'
}

first_non_bt_source() {
  pactl list short sources | awk '$2 !~ /^bluez_input\./ && $2 !~ /\.monitor$/ {print $2; exit}'
}

move_all_outputs_to_source() {
  local source="$1"
  local output_id=""
  [[ -n "${source}" ]] || return 0
  while read -r output_id _; do
    [[ -n "${output_id}" ]] || continue
    pactl move-source-output "${output_id}" "${source}" >/dev/null 2>&1 || true
  done < <(pactl list short source-outputs 2>/dev/null || true)
}

active_recording_stream_ids() {
  pactl list source-outputs 2>/dev/null | awk '
    function unquote(value) {
      gsub(/^"|"$/, "", value)
      return value
    }
    function flush() {
      if (id != "" && monitor != "true" && app != "PulseAudio Volume Control" && app != "pavucontrol") {
        print id
      }
    }
    /^Source Output #[0-9]+/ {
      flush()
      id = substr($3, 2)
      app = ""
      monitor = ""
      next
    }
    /application.name = / {
      split($0, parts, " = ")
      app = unquote(parts[2])
      next
    }
    /stream.monitor = / {
      split($0, parts, " = ")
      monitor = unquote(parts[2])
      next
    }
    END { flush() }
  '
}

active_recording_count() {
  local count
  count="$(active_recording_stream_ids | wc -l)"
  printf '%s\n' "${count//[[:space:]]/}"
}

set_default_sink_if_present() {
  local sink="$1"
  [[ -n "${sink}" ]] || return 0
  pactl set-default-sink "${sink}" >/dev/null 2>&1 || true
}

set_default_source_if_present() {
  local source="$1"
  [[ -n "${source}" ]] || return 0
  pactl set-default-source "${source}" >/dev/null 2>&1 || true
}

unmute_sink_if_present() {
  local sink="$1"
  [[ -n "${sink}" ]] || return 0
  pactl set-sink-mute "${sink}" 0 >/dev/null 2>&1 || true
}

move_all_inputs_to_sink() {
  local sink="$1"
  local input_id=""
  [[ -n "${sink}" ]] || return 0
  while read -r input_id _; do
    [[ -n "${input_id}" ]] || continue
    pactl move-sink-input "${input_id}" "${sink}" >/dev/null 2>&1 || true
  done < <(pactl list short sink-inputs 2>/dev/null || true)
}

wait_for_pactl_server() {
  local attempt=0
  for ((attempt = 0; attempt < 20; attempt++)); do
    if pactl_server_available; then
      return 0
    fi
    sleep 0.5
  done
  return 1
}

wait_for_bt_card() {
  local card="$1"
  local attempt=0
  for ((attempt = 0; attempt < 20; attempt++)); do
    if list_bt_cards | grep -Fxq "${card}"; then
      return 0
    fi
    sleep 0.5
  done
  return 1
}

wait_for_bt_sink() {
  local mac="$1"
  local attempt=0
  for ((attempt = 0; attempt < 20; attempt++)); do
    if [[ -n "$(bt_sink_for_device "${mac}" || true)" ]]; then
      return 0
    fi
    sleep 0.5
  done
  return 1
}

wait_for_bt_source() {
  local mac="$1"
  local attempt=0
  for ((attempt = 0; attempt < 20; attempt++)); do
    if [[ -n "$(bt_source_for_device "${mac}" || true)" ]]; then
      return 0
    fi
    sleep 0.25
  done
  return 1
}

device_alias() {
  local mac="$1"
  device_info "${mac}" | awk -F': ' '/Alias:/ {print $2; exit}'
}

switch_card_to_music_mode() {
  local card="$1"
  local mac="${2:-}"
  local sink=""
  local source=""

  if ! set_profile_first_supported "${card}" \
      a2dp-sink-ldac \
      a2dp-sink-aac \
      a2dp-sink-sbc_xq \
      a2dp-sink-sbc \
      a2dp-sink \
      a2dp_sink; then
    fail "A2DP profile not available on ${card}."
  fi

  if [[ -n "${mac}" ]]; then
    wait_for_bt_sink "${mac}" || true
    sink="$(bt_sink_for_device "${mac}" || true)"
  else
    sink="$(first_bt_sink || true)"
  fi
  set_default_sink_if_present "${sink}"
  unmute_sink_if_present "${sink}"
  move_all_inputs_to_sink "${sink}"

  source="$(first_non_bt_source || true)"
  set_default_source_if_present "${source}"

  echo "Bluetooth music mode enabled."
  [[ -n "${sink}" ]] && echo "Default sink   -> ${sink}"
  [[ -n "${source}" ]] && echo "Default source -> ${source}"
  write_mode_state "music"
}

require_bluetooth_recovery_stack() {
  have_cmd pactl || fail "Missing pactl (install PipeWire/PulseAudio tools)."
  have_cmd bluetoothctl || fail "Missing bluetoothctl (install bluez)."
  have_cmd systemctl || fail "Missing systemctl."
}

recover_playback() {
  local mac=""
  local card=""
  local sink=""
  local alias=""

  require_bluetooth_recovery_stack
  mac="$(pick_bt_device || true)"
  [[ -n "${mac}" ]] || fail "No Bluetooth audio device found. Pair or connect the headset first, or set BT_DEVICE=<MAC>."
  card="$(mac_to_card_name "${mac}")"
  alias="$(device_alias "${mac}" || true)"

  echo "Recovering Bluetooth playback for ${alias:-${mac}}..."
  echo "Restarting PipeWire user services..."
  systemctl --user restart pipewire pipewire-pulse wireplumber >/dev/null 2>&1 || \
    fail "Failed to restart pipewire, pipewire-pulse, and wireplumber."

  wait_for_pactl_server || fail "Audio server did not come back after restarting PipeWire user services."

  echo "Reconnecting ${alias:-${mac}}..."
  bluetoothctl disconnect "${mac}" >/dev/null 2>&1 || true
  sleep 1
  bluetoothctl connect "${mac}" >/dev/null 2>&1 || fail "Failed to reconnect ${mac}."

  wait_for_bt_card "${card}" || fail "Bluetooth card ${card} did not reappear after reconnect."

  if ! switch_card_to_music_mode "${card}" "${mac}"; then
    fail "Bluetooth playback recovery failed while enabling A2DP on ${card}."
  fi

  wait_for_bt_sink "${mac}" || fail "Bluetooth sink for ${mac} did not appear after recovery."
  sink="$(bt_sink_for_device "${mac}" || true)"
  set_default_sink_if_present "${sink}"
  unmute_sink_if_present "${sink}"
  move_all_inputs_to_sink "${sink}"

  echo "Recovery complete."
  [[ -n "${sink}" ]] && echo "Recovered sink -> ${sink}"
}

print_help() {
  cat <<'TXT'
audio_bt_profile.sh

Usage:
  ./scripts/audio_bt_profile.sh help
  ./scripts/audio_bt_profile.sh status
  ./scripts/audio_bt_profile.sh mic-mode
  ./scripts/audio_bt_profile.sh music-mode
  ./scripts/audio_bt_profile.sh auto-start
  ./scripts/audio_bt_profile.sh auto-stop
  ./scripts/audio_bt_profile.sh auto-status
  ./scripts/audio_bt_profile.sh repair
  ./scripts/audio_bt_profile.sh recover

Modes:
  mic-mode    Switch BT headset card to HFP/HSP (mic enabled).
              Tries profiles in this order:
                headset-head-unit-msbc, headset-head-unit, hfp_hf, hsp_hs
              Also sets default source to bluez_input.* when present.
              If AUDIO_OUTPUT_SINK is set, default sink is forced to it.
              Otherwise first non-Bluetooth sink is preferred (e.g. HDMI).

  music-mode  Switch BT headset card back to A2DP (high quality playback).
              Tries profiles:
                a2dp-sink, a2dp_sink
              Also sets default sink to bluez_output.* when present.

  auto-start  Watch recording streams. Enter call mode when recording starts,
              then restore A2DP after recording stops.

  auto-stop   Stop automatic Bluetooth profile switching.

  auto-status Show watcher and last selected profile mode.

  repair      One-command meeting recovery. Restart the audio stack, reconnect
              the headset, restore call/music routing, and restart auto-switch.

  recover     Recover a BT headset after PipeWire/WirePlumber restart or OOM.
              Restarts user audio services, reconnects the headset, forces A2DP,
              restores the default sink, and moves active app streams to it.

Notes:
  - A2DP = good music quality, no headset mic.
  - HFP/HSP = mic enabled, lower speaker quality (phone/call profile).
  - Set BT_CARD=<bluez_card...> to target a specific paired headset.
  - Set BT_DEVICE=<MAC> to target a specific paired headset by Bluetooth address.
TXT
}

print_status() {
  local card="${1:-}"
  local mac=""
  local alias=""
  local connected="no"
  local active_profile=""

  echo "=== Bluetooth Audio Status ==="
  mac="$(pick_bt_device || true)"
  if [[ -n "${mac}" ]]; then
    alias="$(device_alias "${mac}" || true)"
    if device_info "${mac}" | grep -q 'Connected: yes'; then
      connected="yes"
    fi
    echo "Headset: ${alias:-${mac}}"
    echo "Connected: ${connected}"
  else
    echo "Headset: not paired"
    echo "Connected: no"
  fi
  auto_status
  echo
  echo "BT cards:"
  list_bt_cards || true
  echo

  if [[ -n "${card}" ]]; then
    active_profile="$(pactl list cards | awk -v c="${card}" '
      index($0, "Name: " c) {in_card=1; next}
      in_card && $0 ~ /^[[:space:]]*Active Profile:/ {
        sub(/^[[:space:]]*Active Profile:[[:space:]]*/, "", $0);
        print; exit
      }
      in_card && $0 ~ /^Card #[0-9]+/ {in_card=0}
    ')"
    echo "Selected card: ${card}"
    echo "Active profile: ${active_profile:-unknown}"
    echo
  fi

  echo "Default devices:"
  pactl info | awk -F': ' '/Default Sink|Default Source/ {print $1 ": " $2}'
  echo

  echo "Sinks:"
  pactl list short sinks | awk '{print "- " $2}'
  echo

  echo "Sources:"
  pactl list short sources | awk '{print "- " $2}'
}

gui_summary() {
  local card=""
  local mac=""
  local alias=""
  local connected="no"
  local profile="unavailable"
  local mode=""
  local pid=""
  local sink=""
  local source=""

  mac="$(pick_bt_device || true)"
  card="$(pick_bt_card || true)"
  if [[ -n "${mac}" ]]; then
    alias="$(device_alias "${mac}" || true)"
    device_info "${mac}" | grep -q 'Connected: yes' && connected="yes"
  fi
  if [[ -n "${card}" ]]; then
    profile="$(pactl list cards | awk -v c="${card}" '
      index($0, "Name: " c) {inside=1; next}
      inside && /^[[:space:]]*Active Profile:/ {
        sub(/^[[:space:]]*Active Profile:[[:space:]]*/, "", $0); print; exit
      }
      inside && /^Card #[0-9]+/ {inside=0}
    ')"
  fi
  mode="$(read_mode_state || true)"
  pid="$(running_auto_pid || true)"
  sink="$(pactl info | awk -F': ' '/Default Sink/ {print $2; exit}')"
  source="$(pactl info | awk -F': ' '/Default Source/ {print $2; exit}')"

  printf 'HEADSET    %s\n' "${alias:-not paired}"
  printf 'CONNECTION %s\n' "$([[ "${connected}" == yes ]] && echo CONNECTED || echo DISCONNECTED)"
  printf 'AUTOMATIC  %s\n' "$([[ -n "${pid}" ]] && echo ENABLED || echo DISABLED)"
  [[ -n "${mode}" ]] || mode="unselected"
  printf 'MODE       %s\n' "${mode^^}"
  printf 'PROFILE    %s\n' "${profile:-unavailable}"
  printf 'SPEAKER    %s\n' "${sink:-unknown}"
  printf 'MICROPHONE %s\n' "${source:-unknown}"
  printf 'RECORDING  %s active stream(s)\n' "$(active_recording_count 2>/dev/null || echo unknown)"
}

require_audio_stack() {
  have_cmd pactl || fail "Missing pactl (install PipeWire/PulseAudio tools)."
  pactl_server_available || fail "pactl found, but audio server is unavailable. Start PipeWire/PulseAudio user services."
}

mic_mode() {
  local card=""
  local mac=""
  local bt_source=""
  local sink=""

  require_audio_stack
  card="$(pick_bt_card || true)"
  [[ -n "${card}" ]] || fail "No Bluetooth card found. Pair/connect headset first."
  mac="$(card_name_to_mac "${card}")"

  if ! set_profile_first_supported "${card}" \
      headset-head-unit-msbc \
      handsfree-head-unit-msbc \
      headset-head-unit \
      handsfree-head-unit \
      headset-head-unit-cvsd \
      hfp_hf \
      hsp_hs; then
    fail "No mic-capable headset profile found on ${card}."
  fi

  wait_for_bt_source "${mac}" || true
  bt_source="$(bt_source_for_device "${mac}" || true)"
  [[ -n "${bt_source}" ]] || bt_source="$(first_bt_source || true)"
  set_default_source_if_present "${bt_source}"
  move_all_outputs_to_source "${bt_source}"

  if [[ -n "${PREFERRED_OUTPUT_SINK}" ]]; then
    sink="${PREFERRED_OUTPUT_SINK}"
  else
    wait_for_bt_sink "${mac}" || true
    sink="$(bt_sink_for_device "${mac}" || true)"
  fi
  set_default_sink_if_present "${sink}"
  unmute_sink_if_present "${sink}"
  move_all_inputs_to_sink "${sink}"

  echo "Bluetooth mic mode enabled."
  [[ -n "${bt_source}" ]] && echo "Default source -> ${bt_source}"
  [[ -n "${sink}" ]] && echo "Default sink   -> ${sink}"
  echo "Note: speaker quality is lower in HFP/HSP by design."
  write_mode_state "call"
}

music_mode() {
  local card=""

  require_audio_stack
  card="$(pick_bt_card || true)"
  [[ -n "${card}" ]] || fail "No Bluetooth card found. Pair/connect headset first."

  switch_card_to_music_mode "${card}" "$(card_name_to_mac "${card}")"
}

running_auto_pid() {
  local pid=""
  if [[ -f "${PID_FILE}" ]]; then
    pid="$(cat "${PID_FILE}" 2>/dev/null || true)"
    if [[ -n "${pid}" ]] && kill -0 "${pid}" >/dev/null 2>&1; then
      printf '%s\n' "${pid}"
      return 0
    fi
  fi
  return 1
}

systemd_service_installed() {
  have_cmd systemctl && systemctl --user cat "${SYSTEMD_UNIT}" >/dev/null 2>&1
}

apply_auto_policy() {
  local count=0
  local card=""
  local mode=""

  card="$(pick_bt_card || true)"
  if [[ -z "${card}" ]]; then
    AUTO_QUIET_CYCLES=0
    return 0
  fi

  count="$(active_recording_count)"
  mode="$(read_mode_state || true)"
  if (( count > 0 )); then
    if [[ "${mode}" != "call" ]]; then
      if mic_mode; then
        notify_mode "CALL" "Microphone active. Sony input and call-quality output selected."
      fi
    else
      move_all_outputs_to_source "$(bt_source_for_device "$(card_name_to_mac "${card}")" || true)"
    fi
  elif [[ "${mode}" == "call" ]]; then
    if music_mode; then
      notify_mode "MUSIC" "Recording ended. High-quality Bluetooth playback restored."
    fi
  fi
}

auto_watch_loop() {
  local event=""

  ensure_cache_dir
  echo "$$" > "${PID_FILE}"
  trap 'rm -f "${PID_FILE}"' EXIT

  while true; do
    if ! pactl_server_available; then
      sleep 2
      continue
    fi

    apply_auto_policy
    while IFS= read -r event; do
      case "${event}" in
        *"on source-output"*)
          # Let conferencing applications finish opening/closing their stream.
          # This sleep also provides the restoration grace period without polling.
          sleep "${AUTO_RESTORE_GRACE_SEC}"
          apply_auto_policy
          ;;
        *"on card"*|*"on source"*|*"on sink"*|*"on server"*)
          apply_auto_policy
          ;;
      esac
    done < <(pactl subscribe 2>/dev/null)
    sleep 2
  done
}

auto_start() {
  local pid=""
  ensure_cache_dir
  if pid="$(running_auto_pid || true)" && [[ -n "${pid}" ]]; then
    echo "Bluetooth audio auto-switch is already running (pid ${pid})."
    return 0
  fi
  if systemd_service_installed; then
    systemctl --user enable --now "${SYSTEMD_UNIT}" >/dev/null
    sleep 0.3
    pid="$(running_auto_pid || true)"
    [[ -n "${pid}" ]] || fail "Bluetooth audio service did not start."
    echo "Started persistent Bluetooth audio auto-switch (pid ${pid})."
    return 0
  fi
  nohup "${BASH_SOURCE[0]}" auto-watch >> "${LOG_FILE}" 2>&1 &
  sleep 0.2
  pid="$(running_auto_pid || true)"
  [[ -n "${pid}" ]] || fail "Failed to start Bluetooth audio auto-switch."
  echo "Started Bluetooth audio auto-switch (pid ${pid})."
}

auto_stop() {
  local pid=""
  pid="$(running_auto_pid || true)"
  if systemd_service_installed && systemctl --user is-active --quiet "${SYSTEMD_UNIT}"; then
    systemctl --user disable --now "${SYSTEMD_UNIT}" >/dev/null
    rm -f "${PID_FILE}"
    echo "Stopped and disabled persistent Bluetooth audio auto-switch."
    return 0
  fi
  if [[ -z "${pid}" ]]; then
    echo "Bluetooth audio auto-switch is not running."
    return 0
  fi
  kill "${pid}" >/dev/null 2>&1 || true
  rm -f "${PID_FILE}"
  echo "Stopped Bluetooth audio auto-switch."
}

auto_status() {
  local pid=""
  local mode=""
  pid="$(running_auto_pid || true)"
  mode="$(read_mode_state || true)"
  if [[ -n "${pid}" ]]; then
    echo "Auto-switch: running (pid ${pid})"
  else
    echo "Auto-switch: stopped"
  fi
  echo "Last mode: ${mode:-unselected}"
  echo "Active recording streams: $(active_recording_count 2>/dev/null || echo unknown)"
}

repair_now() {
  local had_recording=0
  had_recording="$(active_recording_count 2>/dev/null || echo 0)"
  auto_stop >/dev/null 2>&1 || true
  recover_playback
  if (( had_recording > 0 )); then
    mic_mode
    notify_mode "CALL" "Audio recovered. Sony microphone and call output selected."
  else
    notify_mode "MUSIC" "Audio recovered. High-quality Sony playback selected."
  fi
  auto_start
  echo "Headset repair complete."
}

case "${MODE}" in
  help|-h|--help)
    print_help
    ;;
  status)
    have_cmd pactl || fail "Missing pactl (install PipeWire/PulseAudio tools)."
    if ! pactl_server_available; then
      echo "Audio server unavailable. Start PipeWire/PulseAudio user services, then retry."
      exit 0
    fi
    print_status "$(pick_bt_card || true)"
    ;;
  gui-summary)
    require_audio_stack
    gui_summary
    ;;
  mic-mode)
    mic_mode
    ;;
  music-mode)
    music_mode
    ;;
  auto-start)
    require_audio_stack
    auto_start
    ;;
  auto-stop)
    auto_stop
    ;;
  auto-status)
    require_audio_stack
    auto_status
    ;;
  auto-watch)
    require_audio_stack
    auto_watch_loop
    ;;
  repair)
    require_bluetooth_recovery_stack
    repair_now
    ;;
  recover)
    recover_playback
    ;;
  *)
    echo "Unknown mode: ${MODE}" >&2
    print_help >&2
    exit 2
    ;;
esac
