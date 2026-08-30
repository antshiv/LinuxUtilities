#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TEMPLATE="${ROOT_DIR}/config/systemd/user/linuxutilities-bt-audio.service.in"
USER_UNIT_DIR="${XDG_CONFIG_HOME:-${HOME}/.config}/systemd/user"
UNIT_FILE="${USER_UNIT_DIR}/linuxutilities-bt-audio.service"

command -v systemctl >/dev/null 2>&1 || {
  echo "systemctl is required." >&2
  exit 1
}
[[ -f "${TEMPLATE}" ]] || {
  echo "Missing service template: ${TEMPLATE}" >&2
  exit 1
}

mkdir -p "${USER_UNIT_DIR}"
escaped_root="${ROOT_DIR//\\/\\\\}"
escaped_root="${escaped_root//&/\\&}"
escaped_root="${escaped_root//|/\\|}"
sed "s|@ROOT@|${escaped_root}|g" "${TEMPLATE}" > "${UNIT_FILE}"

systemctl --user daemon-reload
systemctl --user enable --now linuxutilities-bt-audio.service
echo "Installed and started: ${UNIT_FILE}"
systemctl --user --no-pager --full status linuxutilities-bt-audio.service | sed -n '1,12p'
