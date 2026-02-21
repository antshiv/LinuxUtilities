#!/usr/bin/env bash
set -euo pipefail

STATE_FILE="${XDG_RUNTIME_DIR:-/tmp}/presenter_dash_anchor"
COLOR="${PRESENTER_DASH_COLOR:-#00d4ff}"
THICKNESS="${PRESENTER_DASH_THICKNESS:-6}"
FRAME_SLEEP="${PRESENTER_DASH_FRAME_SLEEP:-0.012}"

notify_info() {
    if command -v notify-send >/dev/null 2>&1; then
        notify-send -t 1200 "Presenter Dash" "$1" >/dev/null 2>&1 || true
    fi
}

notify_warn() {
    if command -v notify-send >/dev/null 2>&1; then
        notify-send -u normal -t 1600 "Presenter Dash" "$1" >/dev/null 2>&1 || true
    fi
}

usage() {
    cat <<'USAGE'
Usage:
  presenter_dash.sh anchor     # set anchor at current cursor
  presenter_dash.sh dash       # animated dashed segment: anchor -> cursor
  presenter_dash.sh dot        # animated dotted segment: anchor -> cursor
  presenter_dash.sh solid      # animated solid segment: anchor -> cursor
  presenter_dash.sh arrow      # animated solid segment with arrow head
  presenter_dash.sh clear      # clear all gromit strokes
  presenter_dash.sh undo       # undo last stroke
  presenter_dash.sh redo       # redo last undone stroke
  presenter_dash.sh reset      # clear saved anchor

Environment variables:
  PRESENTER_DASH_COLOR         default: #00d4ff
  PRESENTER_DASH_THICKNESS     default: 6
  PRESENTER_DASH_FRAME_SLEEP   default: 0.012
USAGE
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        notify_warn "Required command missing: $1"
        echo "Missing command: $1" >&2
        exit 1
    fi
}

display_hint() {
    if [[ -z "${DISPLAY:-}" ]]; then
        printf '%s' "DISPLAY is not set. Run from your desktop session."
        return
    fi

    if [[ "${XDG_SESSION_TYPE:-}" == "wayland" ]]; then
        printf '%s' "Wayland session detected; ensure XWayland is enabled for gromit-mpx/xdotool."
        return
    fi

    printf '%s' "Verify cursor access with: xdotool getmouselocation --shell"
}

read_cursor_xy() {
    local out=""
    local key=""
    local value=""
    local x=""
    local y=""
    local hint=""

    if ! out="$(xdotool getmouselocation --shell 2>&1)"; then
        hint="$(display_hint)"
        out="$(tr '\n' ' ' <<< "$out" | sed 's/[[:space:]]\+/ /g; s/[[:space:]]*$//')"
        if [[ -n "$hint" ]]; then
            echo "xdotool failed: ${out:-no output}. ${hint}" >&2
        else
            echo "xdotool failed: ${out:-no output}." >&2
        fi
        return 1
    fi

    if [[ -z "$out" ]]; then
        hint="$(display_hint)"
        if [[ -n "$hint" ]]; then
            echo "xdotool returned empty cursor output. ${hint}" >&2
        else
            echo "xdotool returned empty cursor output." >&2
        fi
        return 1
    fi

    while IFS='=' read -r key value; do
        case "$key" in
            X) x="$value" ;;
            Y) y="$value" ;;
        esac
    done <<< "$out"

    if [[ ! "$x" =~ ^-?[0-9]+$ || ! "$y" =~ ^-?[0-9]+$ ]]; then
        echo "Unexpected xdotool output: ${out}" >&2
        return 1
    fi

    printf '%s %s\n' "$x" "$y"
}

ensure_gromit_running() {
    require_command gromit-mpx

    if ! pgrep -u "$USER" -x gromit-mpx >/dev/null 2>&1; then
        gromit-mpx --key F6 --undo-key F5 >/dev/null 2>&1 &
        sleep 0.25
    fi

    if ! pgrep -u "$USER" -x gromit-mpx >/dev/null 2>&1; then
        local hint=""
        hint="$(display_hint)"
        if [[ -n "$hint" ]]; then
            notify_warn "Could not start gromit-mpx. ${hint}"
            echo "Presenter Dash: Could not start gromit-mpx. ${hint}" >&2
        else
            notify_warn "Could not start gromit-mpx."
            echo "Presenter Dash: Could not start gromit-mpx." >&2
        fi
        return 1
    fi

    return 0
}

line_draw() {
    local x1="$1"
    local y1="$2"
    local x2="$3"
    local y2="$4"

    gromit-mpx -l "$x1" "$y1" "$x2" "$y2" "$COLOR" "$THICKNESS" >/dev/null 2>&1 || true
}

line_draw_solid_animated() {
    local x1="$1"
    local y1="$2"
    local x2="$3"
    local y2="$4"
    local dx=$((x2 - x1))
    local dy=$((y2 - y1))
    local dist=""
    local steps=12
    local px="$x1"
    local py="$y1"

    dist="$(awk -v dx="$dx" -v dy="$dy" 'BEGIN { printf "%d", sqrt(dx*dx + dy*dy) }')"
    if (( dist <= 0 )); then
        return
    fi

    steps=$(( dist / 30 + 6 ))
    if (( steps < 6 )); then
        steps=6
    elif (( steps > 28 )); then
        steps=28
    fi

    for ((i = 1; i <= steps; ++i)); do
        local nx=$(( x1 + (dx * i) / steps ))
        local ny=$(( y1 + (dy * i) / steps ))
        line_draw "$px" "$py" "$nx" "$ny"
        px="$nx"
        py="$ny"
        sleep "$FRAME_SLEEP"
    done
}

line_draw_dashed_animated() {
    local x1="$1"
    local y1="$2"
    local x2="$3"
    local y2="$4"
    local on_len="$5"
    local off_len="$6"
    local dx=$((x2 - x1))
    local dy=$((y2 - y1))
    local dist=""
    local start_off=0

    dist="$(awk -v dx="$dx" -v dy="$dy" 'BEGIN { printf "%d", sqrt(dx*dx + dy*dy) }')"
    if (( dist <= 0 )); then
        return
    fi

    while (( start_off < dist )); do
        local end_off=$(( start_off + on_len ))
        if (( end_off > dist )); then
            end_off=$dist
        fi

        local sx=$(( x1 + (dx * start_off) / dist ))
        local sy=$(( y1 + (dy * start_off) / dist ))
        local ex=$(( x1 + (dx * end_off) / dist ))
        local ey=$(( y1 + (dy * end_off) / dist ))

        line_draw "$sx" "$sy" "$ex" "$ey"
        sleep "$FRAME_SLEEP"

        start_off=$(( end_off + off_len ))
    done
}

line_draw_arrow_head() {
    local x1="$1"
    local y1="$2"
    local x2="$3"
    local y2="$4"
    local head_len=$(( THICKNESS * 4 ))
    local p1x=""
    local p1y=""
    local p2x=""
    local p2y=""

    if (( head_len < 18 )); then
        head_len=18
    fi

    read -r p1x p1y p2x p2y < <(
        awk -v x1="$x1" -v y1="$y1" -v x2="$x2" -v y2="$y2" -v len="$head_len" '
            BEGIN {
                dx = x2 - x1;
                dy = y2 - y1;
                dist = sqrt(dx*dx + dy*dy);
                if (dist < 1) {
                    printf "%d %d %d %d\n", x2, y2, x2, y2;
                    exit;
                }
                ux = dx / dist;
                uy = dy / dist;
                px = -uy;
                py = ux;
                ax = x2 - ux * len + px * len * 0.45;
                ay = y2 - uy * len + py * len * 0.45;
                bx = x2 - ux * len - px * len * 0.45;
                by = y2 - uy * len - py * len * 0.45;
                printf "%d %d %d %d\n", ax, ay, bx, by;
            }
        '
    )

    line_draw "$x2" "$y2" "$p1x" "$p1y"
    line_draw "$x2" "$y2" "$p2x" "$p2y"
}

set_anchor_here() {
    local cursor_xy=""
    local cx=""
    local cy=""

    if ! cursor_xy="$(read_cursor_xy 2>&1)"; then
        notify_warn "$cursor_xy"
        echo "Presenter Dash: $cursor_xy" >&2
        return 1
    fi

    read -r cx cy <<< "$cursor_xy"
    printf '%s %s\n' "$cx" "$cy" > "$STATE_FILE"
    notify_info "Anchor set at (${cx}, ${cy})."
}

reset_anchor() {
    rm -f "$STATE_FILE"
    notify_info "Anchor reset."
}

draw_from_anchor() {
    local style="$1"
    local cursor_xy=""
    local sx=""
    local sy=""
    local ex=""
    local ey=""
    local on_len=0
    local off_len=0

    if ! cursor_xy="$(read_cursor_xy 2>&1)"; then
        notify_warn "$cursor_xy"
        echo "Presenter Dash: $cursor_xy" >&2
        return 1
    fi

    read -r ex ey <<< "$cursor_xy"
    if [[ ! -f "$STATE_FILE" ]]; then
        printf '%s %s\n' "$ex" "$ey" > "$STATE_FILE"
        notify_info "Anchor was missing, set now. Trigger again to draw."
        return 0
    fi

    if ! read -r sx sy < "$STATE_FILE"; then
        printf '%s %s\n' "$ex" "$ey" > "$STATE_FILE"
        notify_info "Anchor refreshed. Trigger again to draw."
        return 0
    fi

    if ! ensure_gromit_running; then
        return 1
    fi

    case "$style" in
        solid)
            line_draw_solid_animated "$sx" "$sy" "$ex" "$ey"
            ;;
        dash)
            on_len=$(( THICKNESS * 5 ))
            off_len=$(( THICKNESS * 3 ))
            if (( on_len < 18 )); then on_len=18; fi
            if (( off_len < 10 )); then off_len=10; fi
            line_draw_dashed_animated "$sx" "$sy" "$ex" "$ey" "$on_len" "$off_len"
            ;;
        dot)
            on_len=$(( THICKNESS ))
            off_len=$(( THICKNESS * 3 ))
            if (( on_len < 3 )); then on_len=3; fi
            if (( off_len < 8 )); then off_len=8; fi
            line_draw_dashed_animated "$sx" "$sy" "$ex" "$ey" "$on_len" "$off_len"
            ;;
        arrow)
            line_draw_solid_animated "$sx" "$sy" "$ex" "$ey"
            line_draw_arrow_head "$sx" "$sy" "$ex" "$ey"
            ;;
        *)
            notify_warn "Unknown style: ${style}"
            return 1
            ;;
    esac

    printf '%s %s\n' "$ex" "$ey" > "$STATE_FILE"
}

main() {
    local cmd="${1:-dash}"

    if [[ ! "$THICKNESS" =~ ^[0-9]+$ ]]; then
        THICKNESS=6
    fi

    case "$cmd" in
        anchor|dash|dot|solid|arrow)
            require_command xdotool
            ;;
    esac

    case "$cmd" in
        anchor)
            set_anchor_here
            ;;
        reset)
            reset_anchor
            ;;
        clear)
            if ensure_gromit_running; then
                gromit-mpx -c >/dev/null 2>&1 || true
                notify_info "Drawings cleared."
            fi
            ;;
        undo)
            if ensure_gromit_running; then
                gromit-mpx -z >/dev/null 2>&1 || true
                notify_info "Undid last stroke."
            fi
            ;;
        redo)
            if ensure_gromit_running; then
                gromit-mpx -y >/dev/null 2>&1 || true
                notify_info "Redid stroke."
            fi
            ;;
        dash|dot|solid|arrow)
            draw_from_anchor "$cmd"
            ;;
        -h|--help|help)
            usage
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

main "$@"
