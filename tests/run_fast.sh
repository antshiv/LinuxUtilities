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
    scripts/audio_bt_profile.sh \
    scripts/audio_source_route.sh \
    scripts/install_audio_service.sh \
    scripts/install_device_event_service.sh \
    scripts/device_event_bridge.sh \
    scripts/folder_navigator.sh \
    scripts/storage_drives.sh \
    scripts/bluetooth_refresh.sh \
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
echo "[fast] Skipping make -n wacom-switch (target performs live X11/tablet probing even under make -n)"

echo "[fast] AwesomeWM config tests"
"$ROOT_DIR/tests/awesomewm_config_test.sh"

echo "[fast] Folder navigator provider tests"
"$ROOT_DIR/tests/folder_navigator_test.sh"

if command -v node >/dev/null 2>&1; then
    echo "[fast] Presenter canvas JS tests"
    node --test "$ROOT_DIR/tests/presenter_canvas_logic.test.mjs"
else
    echo "[fast] Skipping presenter canvas JS tests (node not installed)"
fi

echo "[fast] Docs build"
bash "$ROOT_DIR/docs/build.sh" >/dev/null

echo "[fast] Build + GUI smoke"
"$ROOT_DIR/test_linux_control_center_smoke.sh"

echo "[fast] PASS"
