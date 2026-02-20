#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_CFG="$ROOT_DIR/config/gromit-mpx.cfg"
CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
DEST_CFG="$CONFIG_HOME/gromit-mpx.cfg"

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
echo "Installed Gromit profile: $DEST_CFG"

if command -v gromit-mpx >/dev/null 2>&1; then
    echo "Restart overlay to load profile:"
    echo "  gromit-mpx -q >/dev/null 2>&1 || true"
    echo "  gromit-mpx --key F6 --undo-key F5 >/dev/null 2>&1 &"
else
    echo "gromit-mpx is not installed. Install with:"
    echo "  sudo apt install gromit-mpx"
fi
