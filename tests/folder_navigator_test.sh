#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_ROOT="$(mktemp -d)"
TEST_HOME="${TEST_ROOT}/home"
TEST_BIN="${TEST_ROOT}/bin"
OPEN_LOG="${TEST_ROOT}/opened.txt"
trap 'rm -rf "${TEST_ROOT}"' EXIT

mkdir -p "${TEST_HOME}/Workspace/Project" "${TEST_HOME}/Documents" "${TEST_BIN}"

cat >"${TEST_BIN}/xdg-open" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' "$1" >"${LINUXUTILS_FOLDER_TEST_LOG}"
EOF
chmod +x "${TEST_BIN}/xdg-open"

HOME="${TEST_HOME}" XDG_CACHE_HOME="${TEST_ROOT}/cache" \
  "${ROOT_DIR}/scripts/folder_navigator.sh" --list >"${TEST_ROOT}/folders.txt"
grep -Fxq "${TEST_HOME}/Workspace" "${TEST_ROOT}/folders.txt"
grep -Fxq "${TEST_HOME}/Workspace/Project" "${TEST_ROOT}/folders.txt"

HOME="${TEST_HOME}" XDG_CACHE_HOME="${TEST_ROOT}/cache" \
  ROFI_RETV=0 "${ROOT_DIR}/scripts/folder_navigator.sh" --rofi >"${TEST_ROOT}/provider.bin"
grep -aFq 'Workspace  -  ~/Workspace' "${TEST_ROOT}/provider.bin"
grep -aFq "folder::${TEST_HOME}/Workspace" "${TEST_ROOT}/provider.bin"

HOME="${TEST_HOME}" XDG_CACHE_HOME="${TEST_ROOT}/cache" \
  PATH="${TEST_BIN}:${PATH}" LINUXUTILS_FOLDER_TEST_LOG="${OPEN_LOG}" \
  ROFI_RETV=1 ROFI_INFO="folder::${TEST_HOME}/Workspace" \
  "${ROOT_DIR}/scripts/folder_navigator.sh" --rofi 'Workspace'

for _ in $(seq 1 20); do
  [[ -f "${OPEN_LOG}" ]] && break
  sleep 0.05
done
grep -Fxq "${TEST_HOME}/Workspace" "${OPEN_LOG}"
grep -Fxq "${TEST_HOME}/Workspace" "${TEST_ROOT}/cache/linuxutilities/folder_navigator_recent.txt"

echo "[folder-navigator-test] PASS"
