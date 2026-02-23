#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT_DIR"

echo "[fast] Bash syntax checks"
for script in \
    build_linux_control_center.sh \
    build_cursor_spotlight.sh \
    presenter_dash.sh \
    launch_presenter_canvas.sh \
    launch_presenter_storyboard.sh \
    launch_teleprompter.sh \
    test_linux_control_center_smoke.sh \
    import_screenshots.sh \
    install_gromit_profile.sh \
    redshift.sh \
    install_workspace_shortcuts.sh \
    setup_git_guardrails.sh; do
    if [[ -f "$script" ]]; then
        bash -n "$script"
    fi
done

echo "[fast] Make target dry checks"
make help >/dev/null
make wacom-help >/dev/null
make wacom-status >/dev/null
make -n docs >/dev/null
make -n docs-serve >/dev/null
make -n wacom >/dev/null
make -n wacom-hdmi >/dev/null
if [[ -n "${DISPLAY:-}" ]] && command -v xrandr >/dev/null 2>&1 && xrandr --query >/dev/null 2>&1; then
    make -n wacom-switch >/dev/null
else
    echo "[fast] Skipping make -n wacom-switch (no active X11 display)"
fi

if command -v node >/dev/null 2>&1; then
    echo "[fast] Presenter canvas JS tests"
    node --test "$ROOT_DIR/tests/presenter_canvas_logic.test.mjs"
else
    echo "[fast] Skipping presenter canvas JS tests (node not installed)"
fi

echo "[fast] Build + GUI smoke"
"$ROOT_DIR/test_linux_control_center_smoke.sh"

echo "[fast] PASS"
