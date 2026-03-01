#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-refresh}"

have() {
  command -v "$1" >/dev/null 2>&1
}

restart_blueman_ui() {
  if have blueman-applet; then
    pkill -x blueman-applet >/dev/null 2>&1 || true
    nohup blueman-applet >/dev/null 2>&1 &
  fi
  if have blueman-manager; then
    nohup blueman-manager >/dev/null 2>&1 &
  fi
}

reconnect_paired_audio_devices() {
  local mac
  local info

  if ! have bluetoothctl; then
    return 0
  fi

  while read -r mac; do
    [ -z "${mac}" ] && continue
    info="$(bluetoothctl info "${mac}" 2>/dev/null || true)"
    if [ -z "${info}" ]; then
      continue
    fi
    if ! printf '%s\n' "${info}" | grep -Eq "UUID: (Audio Sink|Headset|A/V Remote Control)"; then
      continue
    fi
    if printf '%s\n' "${info}" | grep -q "Connected: yes"; then
      continue
    fi
    bluetoothctl connect "${mac}" >/dev/null 2>&1 || true
  done < <(bluetoothctl paired-devices | awk '{print $2}')
}

trust_paired_audio_devices() {
  local mac
  local info

  if ! have bluetoothctl; then
    return 0
  fi

  while read -r mac; do
    [ -z "${mac}" ] && continue
    info="$(bluetoothctl info "${mac}" 2>/dev/null || true)"
    if [ -z "${info}" ]; then
      continue
    fi
    if ! printf '%s\n' "${info}" | grep -Eq "UUID: (Audio Sink|Headset|A/V Remote Control)"; then
      continue
    fi
    bluetoothctl trust "${mac}" >/dev/null 2>&1 || true
  done < <(bluetoothctl paired-devices | awk '{print $2}')
}

soft_refresh() {
  if ! have bluetoothctl; then
    echo "Missing bluetoothctl (install bluez)."
    exit 1
  fi
  bluetoothctl power off >/dev/null 2>&1 || true
  sleep 1
  bluetoothctl power on >/dev/null 2>&1 || true
  bluetoothctl scan on >/dev/null 2>&1 || true
  sleep 2
  bluetoothctl scan off >/dev/null 2>&1 || true
  trust_paired_audio_devices
  reconnect_paired_audio_devices
  restart_blueman_ui
  echo "Bluetooth soft refresh complete."
}

case "${MODE}" in
  refresh)
    soft_refresh
    ;;
  ui)
    restart_blueman_ui
    echo "Bluetooth UI reset complete."
    ;;
  trust)
    trust_paired_audio_devices
    reconnect_paired_audio_devices
    echo "Trusted/reconnected paired audio devices."
    ;;
  *)
    echo "Usage: $0 [refresh|ui|trust]"
    exit 1
    ;;
esac
