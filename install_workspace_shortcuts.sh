#!/bin/bash
# ============================================================================
# Workspace Shortcuts Installer
# ============================================================================
# This script installs the workspace navigation shortcuts by adding a 'source'
# command to your .bashrc file.
#
# It creates a backup of your .bashrc before making any changes.
# ============================================================================

# --- Configuration ---
BASHRC_FILE="$HOME/.bashrc"
SHORTCUTS_FILE="/home/antshiv/Workspace/LinuxUtilities/workspace_shortcuts.sh"
SOURCE_LINE="source $SHORTCUTS_FILE"
BACKUP_FILE="$HOME/.bashrc_backup_$(date +%Y-%m-%d_%H-%M-%S)"

# --- Colors for output ---
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# --- Main Logic ---
echo -e "${CYAN}Starting installation of Workspace Shortcuts...${NC}"

# 1. Check if the shortcuts file exists
if [ ! -f "$SHORTCUTS_FILE" ]; then
    echo -e "${RED}ERROR: Shortcuts file not found at '$SHORTCUTS_FILE'.${NC}"
    echo "Please ensure the script is in the correct location."
    exit 1
fi

echo "Shortcuts file found."

# 2. Check if .bashrc exists
if [ ! -f "$BASHRC_FILE" ]; then
    echo -e "${YELLOW}WARNING: '$BASHRC_FILE' not found.${NC}"
    echo "A new .bashrc file will be created."
    touch "$BASHRC_FILE"
fi

# 3. Check if the line is already in .bashrc
if grep -qF -- "$SOURCE_LINE" "$BASHRC_FILE"; then
    echo -e "${GREEN}Shortcuts are already installed in '$BASHRC_FILE'. Nothing to do.${NC}"
    echo "To apply changes, you can run:"
    echo "  source ~/.bashrc"
    exit 0
fi

echo "Shortcuts not yet installed. Proceeding with installation."

# 4. Create a backup
echo -e "Creating a backup of your .bashrc at:${CYAN} $BACKUP_FILE${NC}"
cp "$BASHRC_FILE" "$BACKUP_FILE"
if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Failed to create backup. Aborting installation.${NC}"
    exit 1
fi
echo "Backup successful."

# 5. Add the source line to .bashrc
echo -e "\n# Load Workspace Navigation Shortcuts" >> "$BASHRC_FILE"
echo "$SOURCE_LINE" >> "$BASHRC_FILE"
if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Failed to write to '$BASHRC_FILE'.${NC}"
    echo "Please check file permissions."
    # Restore from backup
    cp "$BACKUP_FILE" "$BASHRC_FILE"
    exit 1
fi

echo -e "${GREEN}✅ Success! Workspace shortcuts have been installed.${NC}"
echo ""
echo -e "${YELLOW}To activate the shortcuts in your CURRENT terminal session, run this command:${NC}"
echo "  source ~/.bashrc"
echo ""
echo "New terminal sessions will have the shortcuts available automatically."
echo "Your original .bashrc was backed up to '$BACKUP_FILE'."
