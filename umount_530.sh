#!/usr/bin/env bash
set -euo pipefail

MOUNT_POINT="${SMB_530_MOUNT:-/mnt/w530}"

if ! command -v mountpoint >/dev/null 2>&1; then
  echo "Missing command: mountpoint"
  exit 1
fi

if mountpoint -q "${MOUNT_POINT}"; then
  echo "Unmounting ${MOUNT_POINT}..."
  sudo umount "${MOUNT_POINT}"
  echo "Unmounted ${MOUNT_POINT}."
else
  echo "Not mounted: ${MOUNT_POINT}"
fi

