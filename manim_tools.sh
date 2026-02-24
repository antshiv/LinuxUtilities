#!/usr/bin/env bash
set -euo pipefail

MANIM_DIR="${MANIM_DIR:-$HOME/Workspace/manim}"
MANIM_FILE="${MANIM_FILE:-smoke.py}"
MANIM_SCENE="${MANIM_SCENE:-Smoke}"
MANIM_QUALITY="${MANIM_QUALITY:-ql}"
MANIM_VENV="${MANIM_VENV:-$MANIM_DIR/.venv/bin/activate}"

usage() {
    cat <<'EOF'
Usage: ./manim_tools.sh <mode>

Modes:
  version       Print manim --version in current shell
  smoke         Run manim smoke render with defaults
  scene         Run manim render using MANIM_FILE/MANIM_SCENE
  shell         Open an interactive shell with manim venv activated
  term-version  Open terminal and run manim --version
  term-smoke    Open terminal and run smoke render
  term-scene    Open terminal and run scene render
  term-shell    Open terminal in manim workspace with venv activated

Environment overrides:
  MANIM_DIR      (default: ~/Workspace/manim)
  MANIM_VENV     (default: $MANIM_DIR/.venv/bin/activate)
  MANIM_FILE     (default: smoke.py)
  MANIM_SCENE    (default: Smoke)
  MANIM_QUALITY  (default: ql, passed as -p<quality>)
EOF
}

die() {
    echo "Error: $*" >&2
    exit 1
}

check_env() {
    [[ -d "$MANIM_DIR" ]] || die "MANIM_DIR not found: $MANIM_DIR"
    [[ -f "$MANIM_VENV" ]] || die "Manim virtualenv activate script not found: $MANIM_VENV"
}

activate_env() {
    check_env
    # shellcheck disable=SC1090
    source "$MANIM_VENV"
}

run_version() {
    activate_env
    manim --version
}

run_scene() {
    activate_env
    cd "$MANIM_DIR"
    manim "-p${MANIM_QUALITY}" "$MANIM_FILE" "$MANIM_SCENE"
}

run_shell() {
    activate_env
    cd "$MANIM_DIR"
    exec "${SHELL:-bash}" -i
}

terminal_cmd() {
    local action="${1:-}"
    local q_dir q_venv q_file q_scene q_quality

    q_dir="$(printf '%q' "$MANIM_DIR")"
    q_venv="$(printf '%q' "$MANIM_VENV")"
    q_file="$(printf '%q' "$MANIM_FILE")"
    q_scene="$(printf '%q' "$MANIM_SCENE")"
    q_quality="$(printf '%q' "$MANIM_QUALITY")"

    case "$action" in
        version)
            echo "set -euo pipefail; cd $q_dir; source $q_venv; manim --version"
            ;;
        smoke|scene)
            echo "set -euo pipefail; cd $q_dir; source $q_venv; manim -p$q_quality $q_file $q_scene"
            ;;
        shell)
            echo "set -euo pipefail; cd $q_dir; source $q_venv"
            ;;
        *)
            die "Unknown terminal action: $action"
            ;;
    esac
}

open_terminal() {
    local action="${1:-}"
    local inner=""
    local final=""

    check_env
    inner="$(terminal_cmd "$action")"
    final="$inner; echo; echo 'Manim workspace: $MANIM_DIR'; exec bash"

    if command -v terminator >/dev/null 2>&1; then
        terminator -x bash -lc "$final" >/dev/null 2>&1 &
        return 0
    fi
    if command -v x-terminal-emulator >/dev/null 2>&1; then
        x-terminal-emulator -e bash -lc "$final" >/dev/null 2>&1 &
        return 0
    fi
    if command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal -- bash -lc "$final" >/dev/null 2>&1 &
        return 0
    fi
    die "No supported terminal launcher found (tried: terminator, x-terminal-emulator, gnome-terminal)."
}

main() {
    local mode="${1:-}"
    case "$mode" in
        version)
            run_version
            ;;
        smoke)
            MANIM_FILE="${MANIM_FILE:-smoke.py}"
            MANIM_SCENE="${MANIM_SCENE:-Smoke}"
            run_scene
            ;;
        scene)
            run_scene
            ;;
        shell)
            run_shell
            ;;
        term-version)
            open_terminal "version"
            ;;
        term-smoke)
            MANIM_FILE="${MANIM_FILE:-smoke.py}"
            MANIM_SCENE="${MANIM_SCENE:-Smoke}"
            open_terminal "smoke"
            ;;
        term-scene)
            open_terminal "scene"
            ;;
        term-shell)
            open_terminal "shell"
            ;;
        -h|--help|help|"")
            usage
            ;;
        *)
            die "Unknown mode: $mode (run './manim_tools.sh --help')"
            ;;
    esac
}

main "${1:-}"
