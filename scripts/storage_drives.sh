#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/linuxutilities"
WATCH_PID_FILE="${CACHE_DIR}/storage_drives_watch.pid"
WATCH_STATE_FILE="${CACHE_DIR}/storage_drives_watch.state"
WATCH_LOG_FILE="${CACHE_DIR}/storage_drives_watch.log"

SMB_530_HOST="${SMB_530_HOST:-10.0.0.119}"
SMB_530_SHARE="${SMB_530_SHARE:-shared}"
SMB_530_MOUNT="${SMB_530_MOUNT:-/mnt/w530}"
SMB_530_CREDENTIALS="${SMB_530_CREDENTIALS:-$HOME/.smbcredentials-530}"

declare -A BLOCK_TYPE=()
declare -A BLOCK_TRAN=()
declare -A BLOCK_HOTPLUG=()
declare -A BLOCK_RM=()
declare -A BLOCK_FSTYPE=()
declare -A BLOCK_SIZE=()
declare -A BLOCK_LABEL=()
declare -A BLOCK_MOUNT=()
declare -A BLOCK_PKNAME=()
declare -A BLOCK_MODEL=()
declare -A BLOCK_VENDOR=()

usage() {
  cat <<EOF
Usage: $0 <mode>

Modes:
  status                Show external-drive + Samba summary
  gui-summary           Compact summary for Linux Control Center
  mount-external        Mount all detected external partitions
  open-external         Mount external partitions, then open the first mountpoint
  unmount-external      Unmount mounted external partitions
  eject-external        Unmount + power off detected external USB disks
  auto-start            Start background USB auto-mount watcher
  auto-stop             Stop background USB auto-mount watcher
  auto-status           Show watcher status
  watch-loop            Internal: polling loop for auto-mount
  samba-status          Show Samba mount status only
  samba-probe           Run the Samba probe helper directly
  samba-probe-ui        Open Samba probe in a terminal
  samba-mount           Run the Samba mount helper directly
  samba-mount-ui        Open Samba mount in a terminal
  samba-unmount         Run the Samba unmount helper directly
  samba-unmount-ui      Open Samba unmount in a terminal
  samba-open            Open the Samba mountpoint
  help                  Show this help text
EOF
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing command: $1" >&2
    exit 1
  fi
}

notify_msg() {
  local title="$1"
  local body="$2"
  if command -v notify-send >/dev/null 2>&1; then
    notify-send "$title" "$body" >/dev/null 2>&1 || true
  fi
}

open_target() {
  local target="$1"
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$target" >/dev/null 2>&1 &
    return 0
  fi
  if command -v gio >/dev/null 2>&1; then
    gio open "$target" >/dev/null 2>&1 &
    return 0
  fi
  echo "Missing xdg-open/gio to open: $target" >&2
  return 1
}

launch_terminal_from_repo() {
  local relative_cmd="$1"
  local prompt="${2:-Press Enter to close...}"
  local shell_cmd

  if ! command -v x-terminal-emulator >/dev/null 2>&1; then
    echo "Missing x-terminal-emulator for interactive helper launch." >&2
    return 1
  fi

  shell_cmd=$(
    printf 'cd %q && %s; status=$?; printf "\\n%s\\n"; read -r _; exit "$status"' \
      "$ROOT_DIR" \
      "$relative_cmd" \
      "$prompt"
  )
  x-terminal-emulator -e bash -lc "$shell_cmd" >/dev/null 2>&1 &
}

load_block_db() {
  local line
  local NAME PATH PKNAME TYPE TRAN HOTPLUG RM FSTYPE SIZE LABEL MOUNTPOINT MODEL VENDOR

  BLOCK_TYPE=()
  BLOCK_TRAN=()
  BLOCK_HOTPLUG=()
  BLOCK_RM=()
  BLOCK_FSTYPE=()
  BLOCK_SIZE=()
  BLOCK_LABEL=()
  BLOCK_MOUNT=()
  BLOCK_PKNAME=()
  BLOCK_MODEL=()
  BLOCK_VENDOR=()

  while IFS= read -r line; do
    unset NAME PATH PKNAME TYPE TRAN HOTPLUG RM FSTYPE SIZE LABEL MOUNTPOINT MODEL VENDOR
    eval "$line"
    [[ -n "${PATH:-}" ]] || continue
    BLOCK_TYPE["$PATH"]="${TYPE:-}"
    BLOCK_TRAN["$PATH"]="${TRAN:-}"
    BLOCK_HOTPLUG["$PATH"]="${HOTPLUG:-0}"
    BLOCK_RM["$PATH"]="${RM:-0}"
    BLOCK_FSTYPE["$PATH"]="${FSTYPE:-}"
    BLOCK_SIZE["$PATH"]="${SIZE:-}"
    BLOCK_LABEL["$PATH"]="${LABEL:-}"
    BLOCK_MOUNT["$PATH"]="${MOUNTPOINT:-}"
    BLOCK_PKNAME["$PATH"]="${PKNAME:-}"
    BLOCK_MODEL["$PATH"]="${MODEL:-}"
    BLOCK_VENDOR["$PATH"]="${VENDOR:-}"
  done < <(lsblk -P -o NAME,PATH,PKNAME,TYPE,TRAN,HOTPLUG,RM,FSTYPE,SIZE,LABEL,MOUNTPOINT,MODEL,VENDOR 2>/dev/null)
}

mountpoint_for_device() {
  local device="$1"
  local target=""
  target="$(findmnt -nr -S "$device" -o TARGET 2>/dev/null | head -n 1 || true)"
  if [[ -n "$target" ]]; then
    printf '%s\n' "$target"
    return
  fi
  printf '%s\n' "${BLOCK_MOUNT[$device]:-}"
}

parent_disk_for() {
  local device="$1"
  local pkname="${BLOCK_PKNAME[$device]:-}"
  if [[ -n "$pkname" ]]; then
    printf '/dev/%s\n' "$pkname"
    return
  fi
  printf '%s\n' "$device"
}

is_external_partition() {
  local device="$1"
  local type="${BLOCK_TYPE[$device]:-}"
  local fstype="${BLOCK_FSTYPE[$device]:-}"
  local parent
  local transport
  local hotplug
  local removable

  [[ "$type" == "part" ]] || return 1
  [[ -n "$fstype" ]] || return 1

  parent="$(parent_disk_for "$device")"
  transport="${BLOCK_TRAN[$parent]:-${BLOCK_TRAN[$device]:-}}"
  hotplug="${BLOCK_HOTPLUG[$parent]:-${BLOCK_HOTPLUG[$device]:-0}}"
  removable="${BLOCK_RM[$parent]:-${BLOCK_RM[$device]:-0}}"

  [[ "$transport" == "usb" || "$hotplug" == "1" || "$removable" == "1" ]]
}

external_partition_records() {
  local device
  local parent
  local transport
  local mountpoint
  local label
  local vendor
  local model
  local size
  local fstype
  local records=()

  load_block_db
  for device in "${!BLOCK_TYPE[@]}"; do
    if ! is_external_partition "$device"; then
      continue
    fi
    parent="$(parent_disk_for "$device")"
    transport="${BLOCK_TRAN[$parent]:-${BLOCK_TRAN[$device]:-}}"
    mountpoint="$(mountpoint_for_device "$device")"
    label="${BLOCK_LABEL[$device]:-}"
    vendor="${BLOCK_VENDOR[$parent]:-}"
    model="${BLOCK_MODEL[$parent]:-}"
    size="${BLOCK_SIZE[$device]:-}"
    fstype="${BLOCK_FSTYPE[$device]:-}"
    records+=("${device}|${parent}|${transport}|${size}|${fstype}|${label}|${mountpoint}|${vendor}|${model}")
  done
  if ((${#records[@]})); then
    printf '%s\n' "${records[@]}" | sort
  fi
}

external_disk_list() {
  local record
  local disk
  declare -A seen=()
  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    disk="${record#*|}"
    disk="${disk%%|*}"
    if [[ -n "$disk" && -z "${seen[$disk]:-}" ]]; then
      printf '%s\n' "$disk"
      seen["$disk"]=1
    fi
  done < <(external_partition_records)
}

mount_external_partition() {
  local device="$1"
  local existing=""
  local output=""
  local mounted=""

  require_cmd udisksctl
  existing="$(mountpoint_for_device "$device")"
  if [[ -n "$existing" ]]; then
    printf 'Already mounted: %s -> %s\n' "$device" "$existing"
    return 0
  fi

  output="$(udisksctl mount -b "$device" 2>&1)" || {
    printf '%s\n' "$output" >&2
    return 1
  }
  mounted="$(mountpoint_for_device "$device")"
  if [[ -z "$mounted" ]]; then
    mounted="$(printf '%s\n' "$output" | sed -n 's/.* at \(.*\)\.$/\1/p' | head -n 1)"
  fi
  printf '%s\n' "$output"
  if [[ -n "$mounted" ]]; then
    notify_msg "External Drive Mounted" "$device -> $mounted"
  fi
}

mount_external() {
  local record
  local device
  local mounted_any=0
  local found=0

  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    found=1
    device="${record%%|*}"
    if [[ -z "$(mountpoint_for_device "$device")" ]]; then
      mount_external_partition "$device"
      mounted_any=1
    fi
  done < <(external_partition_records)

  if (( found == 0 )); then
    echo "No external partitions detected."
    return 0
  fi
  if (( mounted_any == 0 )); then
    echo "All detected external partitions are already mounted."
  fi
}

open_external() {
  local record
  local target=""

  mount_external >/dev/null
  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    target="$(mountpoint_for_device "${record%%|*}")"
    if [[ -n "$target" ]]; then
      open_target "$target"
      printf 'Opened: %s\n' "$target"
      return 0
    fi
  done < <(external_partition_records)

  echo "No mounted external drive found to open." >&2
  return 1
}

unmount_external() {
  local record
  local device
  local target
  local unmounted=0

  require_cmd udisksctl
  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    device="${record%%|*}"
    target="$(mountpoint_for_device "$device")"
    if [[ -n "$target" ]]; then
      udisksctl unmount -b "$device"
      notify_msg "External Drive Unmounted" "$device"
      unmounted=1
    fi
  done < <(external_partition_records)

  if (( unmounted == 0 )); then
    echo "No mounted external partitions found."
  fi
}

eject_external() {
  local disk
  local powered_off=0

  require_cmd udisksctl
  unmount_external >/dev/null || true
  while IFS= read -r disk; do
    [[ -n "$disk" ]] || continue
    udisksctl power-off -b "$disk"
    notify_msg "External USB Powered Off" "$disk"
    powered_off=1
  done < <(external_disk_list)

  if (( powered_off == 0 )); then
    echo "No external USB disks found to eject."
  fi
}

watch_signature() {
  local record
  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    printf '%s\n' "$record"
  done < <(external_partition_records)
}

watch_loop() {
  local current=""
  local previous=""

  mkdir -p "$CACHE_DIR"
  trap 'rm -f "$WATCH_PID_FILE"' EXIT
  printf '%s\n' "$$" > "$WATCH_PID_FILE"

  if [[ -f "$WATCH_STATE_FILE" ]]; then
    previous="$(cat "$WATCH_STATE_FILE" 2>/dev/null || true)"
  fi

  while true; do
    current="$(watch_signature)"
    if [[ "$current" != "$previous" ]]; then
      if [[ -n "$current" ]]; then
        mount_external >>"$WATCH_LOG_FILE" 2>&1 || true
        current="$(watch_signature)"
      fi
      printf '%s\n' "$current" > "$WATCH_STATE_FILE"
      previous="$current"
    fi
    sleep 3
  done
}

watch_running() {
  local pid=""
  pid="$(cat "$WATCH_PID_FILE" 2>/dev/null || true)"
  if [[ -n "$pid" ]] && kill -0 "$pid" >/dev/null 2>&1; then
    printf '%s\n' "$pid"
    return 0
  fi
  return 1
}

auto_start() {
  local pid=""
  mkdir -p "$CACHE_DIR"
  if pid="$(watch_running)"; then
    printf 'Storage auto-mount watcher already running (pid %s).\n' "$pid"
    return 0
  fi
  nohup "$0" watch-loop >>"$WATCH_LOG_FILE" 2>&1 </dev/null &
  sleep 0.2
  if pid="$(watch_running)"; then
    printf 'Started storage auto-mount watcher (pid %s).\n' "$pid"
    notify_msg "Storage Auto-Mount Enabled" "Watching for external USB drives."
    return 0
  fi
  echo "Failed to start storage auto-mount watcher." >&2
  return 1
}

auto_stop() {
  local pid=""
  if pid="$(watch_running)"; then
    kill "$pid" >/dev/null 2>&1 || true
    rm -f "$WATCH_PID_FILE"
    printf 'Stopped storage auto-mount watcher (pid %s).\n' "$pid"
    notify_msg "Storage Auto-Mount Disabled" "Stopped watching for external USB drives."
    return 0
  fi
  echo "Storage auto-mount watcher is not running."
}

auto_status() {
  local pid=""
  if pid="$(watch_running)"; then
    printf 'Storage auto-mount watcher: running (pid %s)\n' "$pid"
  else
    printf 'Storage auto-mount watcher: stopped\n'
  fi
  printf 'Watcher log: %s\n' "$WATCH_LOG_FILE"
}

samba_status() {
  echo "Samba 530"
  echo "---------"
  printf 'Share: //%s/%s\n' "$SMB_530_HOST" "$SMB_530_SHARE"
  printf 'Mountpoint: %s\n' "$SMB_530_MOUNT"
  if mountpoint -q "$SMB_530_MOUNT" 2>/dev/null; then
    echo "Mounted: yes"
  else
    echo "Mounted: no"
  fi
  if [[ -f "$SMB_530_CREDENTIALS" ]]; then
    printf 'Credentials: present (%s)\n' "$SMB_530_CREDENTIALS"
  else
    printf 'Credentials: missing (%s)\n' "$SMB_530_CREDENTIALS"
  fi
}

status_report() {
  local record
  local found=0
  local device
  local parent
  local transport
  local size
  local fstype
  local label
  local mountpoint
  local vendor
  local model

  auto_status
  echo
  echo "External Drives"
  echo "---------------"
  while IFS= read -r record; do
    [[ -n "$record" ]] || continue
    found=1
    IFS='|' read -r device parent transport size fstype label mountpoint vendor model <<<"$record"
    printf '%s  %s  %s\n' "$device" "${size:-unknown}" "${fstype:-unknown}"
    printf '  disk: %s\n' "$parent"
    if [[ -n "$vendor$model" ]]; then
      printf '  model: %s %s\n' "$vendor" "$model"
    fi
    printf '  transport: %s\n' "${transport:-unknown}"
    printf '  label: %s\n' "${label:-<none>}"
    if [[ -n "$mountpoint" ]]; then
      printf '  mounted: %s\n' "$mountpoint"
    else
      echo "  mounted: no"
    fi
  done < <(external_partition_records)
  if (( found == 0 )); then
    echo "No external USB partitions detected."
  fi
  echo
  samba_status
}

gui_summary() {
  status_report
}

main() {
  local mode="${1:-help}"
  case "$mode" in
    status) status_report ;;
    gui-summary) gui_summary ;;
    mount-external) mount_external ;;
    open-external) open_external ;;
    unmount-external) unmount_external ;;
    eject-external) eject_external ;;
    auto-start) auto_start ;;
    auto-stop) auto_stop ;;
    auto-status) auto_status ;;
    watch-loop) watch_loop ;;
    samba-status) samba_status ;;
    samba-probe) cd "$ROOT_DIR" && ./mount_530.sh --probe ;;
    samba-probe-ui) launch_terminal_from_repo "./mount_530.sh --probe" ;;
    samba-mount) cd "$ROOT_DIR" && ./mount_530.sh ;;
    samba-mount-ui) launch_terminal_from_repo "./mount_530.sh" ;;
    samba-unmount) cd "$ROOT_DIR" && ./umount_530.sh ;;
    samba-unmount-ui) launch_terminal_from_repo "./umount_530.sh" ;;
    samba-open) open_target "$SMB_530_MOUNT" ;;
    help|-h|--help) usage ;;
    *)
      echo "Unknown mode: $mode" >&2
      usage >&2
      exit 1
      ;;
  esac
}

main "$@"
