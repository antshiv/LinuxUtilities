#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_CFG_COMPAT="$ROOT_DIR/config/gromit-mpx.cfg"
SRC_CFG_ADVANCED="$ROOT_DIR/config/gromit-mpx-advanced.cfg"
CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
DEST_CFG="$CONFIG_HOME/gromit-mpx.cfg"
PROFILE_MODE="${GROMIT_PROFILE_MODE:-auto}"

choose_profile() {
    local mode="$1"
    local deb_ver=""
    local doc_path="/usr/share/doc/gromit-mpx/README.md.gz"

    case "$mode" in
        compat)
            printf 'compat'
            return
            ;;
        advanced)
            printf 'advanced'
            return
            ;;
        auto|"")
            ;;
        *)
            echo "Warning: unknown GROMIT_PROFILE_MODE='$mode', using auto." >&2
            ;;
    esac

    if command -v dpkg-query >/dev/null 2>&1; then
        deb_ver="$(dpkg-query -W -f='${Version}' gromit-mpx 2>/dev/null || true)"
        if [[ "$deb_ver" =~ ^1\.5\. ]]; then
            printf 'compat'
            return
        fi
    fi

    if [[ -f "$doc_path" ]] && zgrep -Eq 'CIRCLE[[:space:]-]+tool|RECT[[:space:]-]+tool|ORTHOGONAL[[:space:]-]+tool' "$doc_path" 2>/dev/null; then
        printf 'advanced'
        return
    fi

    printf 'compat'
}

PROFILE_CHOICE="$(choose_profile "$PROFILE_MODE")"
if [[ "$PROFILE_CHOICE" == "advanced" ]]; then
    SRC_CFG="$SRC_CFG_ADVANCED"
else
    SRC_CFG="$SRC_CFG_COMPAT"
fi

if [[ ! -f "$SRC_CFG" ]]; then
    echo "Error: source config not found at: $SRC_CFG" >&2
    exit 1
fi

mkdir -p "$CONFIG_HOME"

if [[ -f "$DEST_CFG" ]] && ! cmp -s "$SRC_CFG" "$DEST_CFG"; then
    BACKUP_CFG="$DEST_CFG.backup.$(date +%Y-%m-%d_%H-%M-%S)"
    cp "$DEST_CFG" "$BACKUP_CFG"
    echo "Backed up existing config: $BACKUP_CFG"
fi

install -m 0644 "$SRC_CFG" "$DEST_CFG"
echo "Installed Gromit profile ($PROFILE_CHOICE): $DEST_CFG"
if [[ "$PROFILE_CHOICE" == "compat" ]]; then
    echo "Note: using compatibility profile (pen/marker/eraser/arrow pen) to avoid parser issues on older distro builds."
    echo "To force advanced profile: GROMIT_PROFILE_MODE=advanced ./install_gromit_profile.sh"
fi

if command -v gromit-mpx >/dev/null 2>&1; then
    echo "Restart overlay to load profile:"
    echo "  gromit-mpx -q >/dev/null 2>&1 || true"
    echo "  gromit-mpx --key F6 --undo-key F5 >/dev/null 2>&1 &"
else
    echo "gromit-mpx is not installed. Install with:"
    echo "  sudo apt install gromit-mpx"
fi
