#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE="${ROOT_DIR}/config/systemd/user/linuxutilities-device-events.service.in"
TARGET_DIR="${XDG_CONFIG_HOME:-${HOME}/.config}/systemd/user"
TARGET="${TARGET_DIR}/linuxutilities-device-events.service"

mkdir -p "${TARGET_DIR}"
sed "s|@ROOT_DIR@|${ROOT_DIR//|/\\|}|g" "${SOURCE}" > "${TARGET}"
chmod 0644 "${TARGET}"
chmod +x "${ROOT_DIR}/scripts/device_event_bridge.sh"
systemctl --user daemon-reload
systemctl --user enable --now linuxutilities-device-events.service
echo "Installed and started ${TARGET}"
