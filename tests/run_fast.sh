#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT_DIR"

echo "[fast] Bash syntax checks"
for script in \
    build_linux_control_center.sh \
    build_cursor_spotlight.sh \
    presenter_dash.sh \
    test_linux_control_center_smoke.sh \
    import_screenshots.sh \
    redshift.sh \
    install_workspace_shortcuts.sh \
    setup_git_guardrails.sh; do
    if [[ -f "$script" ]]; then
        bash -n "$script"
    fi
done

echo "[fast] Build + GUI smoke"
"$ROOT_DIR/test_linux_control_center_smoke.sh"

echo "[fast] PASS"
