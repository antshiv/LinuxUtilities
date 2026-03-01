#!/usr/bin/env bash
set -euo pipefail

resolve_invoker_home() {
  if [[ -n "${SMB_530_USER_HOME:-}" ]]; then
    printf '%s\n' "${SMB_530_USER_HOME}"
    return
  fi
  if [[ -n "${SUDO_USER:-}" && "${SUDO_USER}" != "root" ]]; then
    local home
    home="$(getent passwd "${SUDO_USER}" 2>/dev/null | cut -d: -f6 || true)"
    if [[ -n "${home}" ]]; then
      printf '%s\n' "${home}"
      return
    fi
  fi
  printf '%s\n' "${HOME:-/tmp}"
}

INVOKER_HOME="$(resolve_invoker_home)"
OWNER_UID="${SMB_530_UID:-${SUDO_UID:-$(id -u)}}"
OWNER_GID="${SMB_530_GID:-${SUDO_GID:-$(id -g)}}"

HOST="${SMB_530_HOST:-10.0.0.119}"
SHARE="${SMB_530_SHARE:-shared}"
MOUNT_POINT="${SMB_530_MOUNT:-/mnt/w530}"
CREDENTIALS_FILE="${SMB_530_CREDENTIALS:-$INVOKER_HOME/.smbcredentials-530}"
VERS="${SMB_530_VERS:-3.0}"
PROBE_ONLY=0

usage() {
  cat <<EOF
Usage: $0 [--probe] [--help]

Environment overrides:
  SMB_530_HOST          default: ${HOST}
  SMB_530_SHARE         default: ${SHARE}
  SMB_530_MOUNT         default: ${MOUNT_POINT}
  SMB_530_CREDENTIALS   default: ${CREDENTIALS_FILE}
  SMB_530_UID           default: ${OWNER_UID}
  SMB_530_GID           default: ${OWNER_GID}
  SMB_530_USER_HOME     default: ${INVOKER_HOME}
  SMB_530_VERS          default: ${VERS}
  SMB_USER / SMB_PASS / SMB_DOMAIN

Examples:
  $0 --probe
  $0
  SMB_530_SHARE=shared $0
EOF
}

while (($#)); do
  case "$1" in
    --probe) PROBE_ONLY=1 ;;
    --help|-h) usage; exit 0 ;;
    *) echo "Unknown argument: $1"; usage; exit 1 ;;
  esac
  shift
done

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing command: $1"
    exit 1
  fi
}

require_cmd ping
require_cmd smbclient

share_path="//${HOST}/${SHARE}"

echo "Checking host ${HOST}..."
if ping -c 1 -W 1 "${HOST}" >/dev/null 2>&1; then
  echo "Host reachable."
else
  echo "Ping failed (blocked or host not replying). Continuing with SMB probe..."
fi

echo "Probing SMB shares on //${HOST}..."
if ! smbclient -L "//${HOST}" -N 2>/dev/null | sed -n '1,60p'; then
  echo "SMB probe failed for //${HOST}. Check host/share and credentials."
  exit 1
fi

if (( PROBE_ONLY == 1 )); then
  exit 0
fi

require_cmd mount
if [[ "${EUID}" -eq 0 ]]; then
  AS_ROOT=1
else
  AS_ROOT=0
  require_cmd sudo
fi

if mountpoint -q "${MOUNT_POINT}"; then
  echo "Already mounted: ${MOUNT_POINT}"
  exit 0
fi

echo "Ensuring mount point exists: ${MOUNT_POINT}"
if (( AS_ROOT == 1 )); then
  mkdir -p "${MOUNT_POINT}"
else
  sudo mkdir -p "${MOUNT_POINT}"
fi

base_opts="rw,iocharset=utf8,uid=${OWNER_UID},gid=${OWNER_GID},file_mode=0664,dir_mode=0775,vers=${VERS},nounix,noserverino"
if (( AS_ROOT == 1 )); then
  mount_cmd=(mount -t cifs "${share_path}" "${MOUNT_POINT}")
else
  mount_cmd=(sudo mount -t cifs "${share_path}" "${MOUNT_POINT}")
fi

write_temp_creds() {
  local username="$1"
  local password="$2"
  tmp_creds="$(mktemp)"
  trap 'rm -f "${tmp_creds}"' EXIT
  {
    echo "username=${username}"
    echo "password=${password}"
    if [[ -n "${SMB_DOMAIN:-}" ]]; then
      echo "domain=${SMB_DOMAIN}"
    fi
  } > "${tmp_creds}"
  chmod 600 "${tmp_creds}"
  opts="${base_opts},credentials=${tmp_creds}"
}

if [[ -n "${SMB_USER:-}" && -n "${SMB_PASS:-}" ]]; then
  write_temp_creds "${SMB_USER}" "${SMB_PASS}"
elif [[ -f "${CREDENTIALS_FILE}" ]]; then
  opts="${base_opts},credentials=${CREDENTIALS_FILE}"
else
  if [[ ! -t 0 ]]; then
    echo "Credentials required for ${share_path}."
    echo "Run interactively to enter username/password, or create ${CREDENTIALS_FILE}."
    exit 1
  fi
  default_user="${SMB_USER:-${SUDO_USER:-${USER:-}}}"
  if [[ -n "${default_user}" ]]; then
    read -r -p "SMB username [${default_user}]: " prompt_user
    SMB_USER="${prompt_user:-${default_user}}"
  else
    read -r -p "SMB username: " SMB_USER
  fi
  if [[ -z "${SMB_USER}" ]]; then
    echo "SMB username cannot be empty."
    exit 1
  fi
  read -r -s -p "SMB password for ${SMB_USER}: " SMB_PASS
  echo
  if [[ -z "${SMB_PASS}" ]]; then
    echo "SMB password cannot be empty."
    exit 1
  fi
  write_temp_creds "${SMB_USER}" "${SMB_PASS}"
fi

echo "Mounting ${share_path} -> ${MOUNT_POINT}"
"${mount_cmd[@]}" -o "${opts}"
echo "Mounted successfully."
echo "Tip: run './umount_530.sh' or 'make umount-530' when done."
