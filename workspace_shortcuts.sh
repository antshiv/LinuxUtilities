#!/bin/bash
# ============================================================================
# Workspace Navigation Shortcuts
# ============================================================================
# Quick navigation to common directories in the /home/antshiv/Workspace
#
# USAGE:
#   Source this file in your .bashrc or .bash_aliases:
#   source /home/antshiv/Workspace/LinuxUtilities/workspace_shortcuts.sh
#
# EXAMPLES:
#   ck        → cd /home/antshiv/Workspace/C-Kernel-Engine
#   ct        → cd /home/antshiv/Workspace/C-Transformer
#   ant       → cd /home/antshiv/Workspace/antsand.com
#   browse    → Launch context-aware interactive browser
#
# ============================================================================

# Base Workspace directory
export WORKSPACE_ROOT="/home/antshiv/Workspace"

# ============================================================================
# Core Navigation Function
# ============================================================================
ws_nav() {
    local shortcut="$1"
    local target_dir=""

    # --- Static mapping of shortcuts to directories ---
    case "$shortcut" in
        "aero") target_dir="AeroDynControlRig" ;;
        "aml") target_dir="AttitudeMathLib" ;;
        "ant") target_dir="antsand.com" ;;
        "ck") target_dir="C-Kernel-Engine" ;;
        "ct") target_dir="C-Transformer" ;;
        "dls") target_dir="DronelinuxSystem" ;;
        "dtr") target_dir="DroneTestRig" ;;
        "genai") target_dir="Generative-AI-with-LLMs" ;;
        "lu") target_dir="LinuxUtilities" ;;
        "sn") target_dir="ShivasNotes" ;;

        # Add other shortcuts here

        *)
            echo "Unknown shortcut: $shortcut"
            echo "Use 'ws_help' for available shortcuts"
            return 1
            ;;
    esac

    if [ -d "$WORKSPACE_ROOT/$target_dir" ]; then
        cd "$WORKSPACE_ROOT/$target_dir" || return 1
        echo "📂 $(pwd)"
    else
        echo "❌ Directory not found: $WORKSPACE_ROOT/$target_dir"
        return 1
    fi
}

# ============================================================================
# Individual Shortcut Functions
# ============================================================================

aero() { ws_nav "aero" "$1"; }
aml() { ws_nav "aml" "$1"; }
ant() { ws_nav "ant" "$1"; }
ck() { ws_nav "ck" "$1"; }
ct() { ws_nav "ct" "$1"; }
dls() { ws_nav "dls" "$1"; }
dtr() { ws_nav "dtr" "$1"; }
genai() { ws_nav "genai" "$1"; }
lu() { ws_nav "lu" "$1"; }
sn() { ws_nav "sn" "$1"; }
ws() { cd "$WORKSPACE_ROOT" || return 1; echo "📂 $(pwd)"; }

# ============================================================================
# Simple Help Function
# ============================================================================
ws_help() {
    echo "Workspace Navigation Shortcuts"
    echo "=============================="
    echo ""
    echo "Usage: <shortcut>"
    echo ""
    echo "Shortcuts:"
    echo "  aero         → /home/antshiv/Workspace/AeroDynControlRig"
    echo "  aml          → /home/antshiv/Workspace/AttitudeMathLib"
    echo "  ant          → /home/antshiv/Workspace/antsand.com"
    echo "  ck           → /home/antshiv/Workspace/C-Kernel-Engine"
    echo "  ct           → /home/antshiv/Workspace/C-Transformer"
    echo "  dls          → /home/antshiv/Workspace/DronelinuxSystem"
    echo "  dtr          → /home/antshiv/Workspace/DroneTestRig"
    echo "  genai        → /home/antshiv/Workspace/Generative-AI-with-LLMs"
    echo "  lu           → /home/antshiv/Workspace/LinuxUtilities"
    echo "  sn           → /home/antshiv/Workspace/ShivasNotes"
    echo "  ws           → /home/antshiv/Workspace (Root)"
    echo ""
    echo "Special Commands:"
    echo "  browse       → Interactive context-aware browser"
    echo "  project_find → Simple command-line project search"
    echo "  ws_help      → Display this help message"
    echo ""
}

# ============================================================================
# Context-Aware Interactive Browser
# ============================================================================
browse() {
    local current_dir
    current_dir="$(pwd)"
    local antsand_project_dir="$WORKSPACE_ROOT/antsand.com"
    local workspace_browse_script="$WORKSPACE_ROOT/LinuxUtilities/workspace_browse.sh"
    local antsand_browse_script="$antsand_project_dir/antsand_browse.sh"

    # Check if we are inside the antsand.com project directory
    if [[ -f "$antsand_browse_script" && "$current_dir" == "$antsand_project_dir"* ]]; then
        echo -e "\033[1;33mInside ANTSAND project. Using specialized browser...\033[0m"
        "$antsand_browse_script" "$current_dir"
    elif [ -f "$workspace_browse_script" ]; then
        echo -e "\033[1;36mUsing workspace browser...\033[0m"
        "$workspace_browse_script" "$current_dir"
    else
        echo -e "\033[0;31mError: Browsing script not found.\033[0m"
        echo "Please ensure 'workspace_browse.sh' is in '$WORKSPACE_ROOT/LinuxUtilities/'"
        return 1
    fi
}

# ============================================================================
# Simple Project Search
# ============================================================================
project_find() {
    if [ -z "$1" ]; then
        echo "Usage: project_find <pattern>"
        echo "Example: project_find kernel"
        return 1
    fi
    echo "Searching for '$1' in workspace projects:"
    find "$WORKSPACE_ROOT" -maxdepth 1 -mindepth 1 -type d -printf '%P\n' | grep -i "$1" --color=always
}


# ============================================================================
# Auto-completion (REMOVED - User preferred simplicity)
# ============================================================================


# ============================================================================
# Status Function
# ============================================================================

ws_status() {
    echo "🚀 Workspace Navigation Shortcuts Loaded"
    echo "   Use 'ws_help' for a full list of shortcuts."
    echo "   Use 'browse' for interactive navigation."
    echo "   Use 'project_find <pattern>' for simple search."
}

# ============================================================================
# Detect if being sourced or executed
# ============================================================================

if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
    # Being sourced - load functions and show status
    ws_status
else
    # Being executed directly - show help
    ws_help
fi
