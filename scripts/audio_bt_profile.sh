#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-help}"
PREFERRED_BT_CARD="${BT_CARD:-}"
PREFERRED_OUTPUT_SINK="${AUDIO_OUTPUT_SINK:-}"

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

list_bt_cards() {
  pactl list cards short | awk '$2 ~ /^bluez_card\./ {print $2}'
}

pick_bt_card() {
  local first_card=""
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

first_bt_source() {
  pactl list short sources | awk '$2 ~ /^bluez_input\./ {print $2; exit}'
}

first_non_bt_source() {
  pactl list short sources | awk '$2 !~ /^bluez_input\./ && $2 !~ /\.monitor$/ {print $2; exit}'
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

print_help() {
  cat <<'TXT'
audio_bt_profile.sh

Usage:
  ./scripts/audio_bt_profile.sh help
  ./scripts/audio_bt_profile.sh status
  ./scripts/audio_bt_profile.sh mic-mode
  ./scripts/audio_bt_profile.sh music-mode

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

Notes:
  - A2DP = good music quality, no headset mic.
  - HFP/HSP = mic enabled, lower speaker quality (phone/call profile).
  - Set BT_CARD=<bluez_card...> to target a specific paired headset.
TXT
}

print_status() {
  local card="${1:-}"
  local active_profile=""

  echo "=== Bluetooth Audio Status ==="
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

require_audio_stack() {
  have_cmd pactl || fail "Missing pactl (install PipeWire/PulseAudio tools)."
  pactl_server_available || fail "pactl found, but audio server is unavailable. Start PipeWire/PulseAudio user services."
}

mic_mode() {
  local card=""
  local bt_source=""
  local sink=""

  require_audio_stack
  card="$(pick_bt_card || true)"
  [[ -n "${card}" ]] || fail "No Bluetooth card found. Pair/connect headset first."

  if ! set_profile_first_supported "${card}" \
      headset-head-unit-msbc \
      headset-head-unit \
      hfp_hf \
      hsp_hs; then
    fail "No mic-capable headset profile found on ${card}."
  fi

  bt_source="$(first_bt_source || true)"
  set_default_source_if_present "${bt_source}"

  if [[ -n "${PREFERRED_OUTPUT_SINK}" ]]; then
    sink="${PREFERRED_OUTPUT_SINK}"
  else
    sink="$(first_non_bt_sink || true)"
  fi
  set_default_sink_if_present "${sink}"

  echo "Bluetooth mic mode enabled."
  [[ -n "${bt_source}" ]] && echo "Default source -> ${bt_source}"
  [[ -n "${sink}" ]] && echo "Default sink   -> ${sink}"
  echo "Note: speaker quality is lower in HFP/HSP by design."
}

music_mode() {
  local card=""
  local sink=""
  local source=""

  require_audio_stack
  card="$(pick_bt_card || true)"
  [[ -n "${card}" ]] || fail "No Bluetooth card found. Pair/connect headset first."

  if ! set_profile_first_supported "${card}" a2dp-sink a2dp_sink; then
    fail "A2DP profile not available on ${card}."
  fi

  sink="$(first_bt_sink || true)"
  set_default_sink_if_present "${sink}"

  source="$(first_non_bt_source || true)"
  set_default_source_if_present "${source}"

  echo "Bluetooth music mode enabled."
  [[ -n "${sink}" ]] && echo "Default sink   -> ${sink}"
  [[ -n "${source}" ]] && echo "Default source -> ${source}"
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
  mic-mode)
    mic_mode
    ;;
  music-mode)
    music_mode
    ;;
  *)
    echo "Unknown mode: ${MODE}" >&2
    print_help >&2
    exit 2
    ;;
esac
