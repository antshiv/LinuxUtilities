#!/bin/bash
# ============================================================================
# Workspace Interactive Directory Browser
# ============================================================================
# Browse Workspace directories interactively with contextual help
#
# USAGE:
#   ./workspace_browse.sh [directory]
#   browse                              # Launch from shortcuts
#
# FEATURES:
#   - Arrow key navigation (↑/↓)
#   - Enter to navigate into directories
#   - Backspace/Left to go up
#   - 'h' for help on current item
#   - 'q' to quit
# ============================================================================

# Terminal colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
GRAY='\033[0;90m'
BOLD='\033[1m'
DIM='\033[2m'
NC='\033[0m' # No Color

# Base Workspace directory
WORKSPACE_ROOT="/home/antshiv/Workspace"

# ============================================================================
# File/Directory Descriptions
# ============================================================================

get_description() {
    local item="$1"
    local item_name="$(basename "$item")"

    # Directory-based descriptions
    if [ -d "$item" ]; then
        case "$item_name" in
            "antsand.com")
                echo "🐜 ANTSAND main web project repository."
                ;;
            "C-Kernel-Engine")
                echo "⚙️  Custom C Kernel Engine project."
                ;;
            "C-Transformer")
                echo "🧠 C language Transformer model implementation."
                ;;
            "AeroDynControlRig")
                echo "✈️ Aerodynamics and Control Systems Simulation Rig."
                ;;
            "AttitudeMathLib")
                echo "📐 C++ library for attitude and orientation math (quaternions, etc.)."
                ;;
            "DronelinuxSystem")
                echo "🐧 Drone-focused Linux system configuration and tools."
                ;;
            "Generative-AI-with-LLMs")
                echo "🤖 Coursework and projects for Generative AI with LLMs."
                ;;
            "LinuxUtilities")
                echo "🔧 Custom Linux utility scripts and tools (like this one!)."
                ;;
            ".git")
                echo "🙈 Git directory - version control metadata."
                ;;
            *)
                local readme_path="$item/README.md"
                if [ -f "$readme_path" ]; then
                    # Show first line of README if it exists
                    head -n 1 "$readme_path"
                else
                    echo "📁 Directory - Navigate with Enter, back with Backspace"
                fi
                ;;
        esac
    else
        # Generic file description based on extension
        case "${item_name##*.}" in
            "md")
                echo "📝 Markdown documentation file."
                ;;
            "sh")
                echo "⚙️  Shell script for automation."
                ;;
            "py")
                echo "🐍 Python script."
                ;;
            "cpp"|"c"|"h"|"hpp")
                echo "🔷 C/C++ source or header file."
                ;;
            "js"|"ts")
                echo "📜 JavaScript/TypeScript source file."
                ;;
            "json")
                echo "📋 JSON configuration or data file."
                ;;
            "pdf")
                echo "📕 PDF document."
                ;;
            "git"*)
                echo "🙈 Git-related file."
                ;;
            *)
                echo "📄 File - Press 'h' for help"
                ;;
        esac
    fi
}

# ============================================================================
# UI Functions
# ============================================================================

clear_screen() {
    clear
}

hide_cursor() {
    tput civis
}

show_cursor() {
    tput cnorm
}

move_cursor() {
    tput cup "$1" "$2"
}

print_header() {
    local current_path="$1"
    local rel_path="${current_path#$WORKSPACE_ROOT}"
    [ -z "$rel_path" ] && rel_path="/"

    echo -e "${BOLD}${CYAN}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}${CYAN}║${NC} ${BOLD}Workspace Interactive Browser${NC}                                            ${BOLD}${CYAN}║${NC}"
    echo -e "${BOLD}${CYAN}╠════════════════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${BOLD}${CYAN}║${NC} ${BLUE}📂 ${WHITE}${rel_path}${NC}"
    echo -e "${BOLD}${CYAN}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_footer() {
    echo ""
    echo -e "${GRAY}───────────────────────────────────────────────────────────────────────────────${NC}"
    echo -e "${DIM}↑/↓: Navigate  Enter: Open  ←/Backspace: Back  h: Help  q: Quit${NC}"
}

get_item_icon() {
    local item="$1"
    if [ -d "$item" ]; then
        echo "📁"
    else
        case "${item##*.}" in
            "md") echo "📝" ;;
            "sh") echo "⚙️ " ;;
            "py") echo "🐍" ;;
            "c"|"cpp"|"h"|"hpp") echo "🔷" ;;
            "js"|"ts") echo "📜" ;;
            "json") echo "📋" ;;
            "pdf") echo "📕" ;;
            *) echo "📄" ;;
        esac
    fi
}

# ============================================================================
# Directory Browser
# ============================================================================

browse_directory() {
    local current_dir="$1"
    local selected_index=0
    local scroll_offset=0
    local max_visible=20

    while true; do
        # Get items in current directory
        local items=()
        if [ "$current_dir" != "$WORKSPACE_ROOT" ]; then
            items+=("..")
        fi

        # Use ls to sort directories first, then files, ignoring case
        while IFS= read -r item; do
            items+=("$item")
        done < <(ls -aF --group-directories-first "$current_dir" | grep -v '^\.$' | sed "s#^#$current_dir/#" | sed 's/\/\//\//')


        # If no items, show message
        if [ ${#items[@]} -eq 0 ]; then
            clear_screen
            print_header "$current_dir"
            echo -e "${YELLOW}Empty directory${NC}"
            print_footer
            read -rsn1 key
            return
        fi

        # Adjust scroll offset
        if [ $selected_index -ge $((scroll_offset + max_visible)) ]; then
            scroll_offset=$((selected_index - max_visible + 1))
        elif [ $selected_index -lt $scroll_offset ]; then
            scroll_offset=$selected_index
        fi

        # Draw screen
        clear_screen
        hide_cursor
        print_header "$current_dir"

        # Display items
        local visible_end=$((scroll_offset + max_visible))
        [ $visible_end -gt ${#items[@]} ] && visible_end=${#items[@]}

        for i in $(seq $scroll_offset $((visible_end - 1))); do
            local item_path="${items[$i]}"
            # Correctly handle ".." entry
            if [[ "$(basename "$item_path")" == ".." ]]; then
                 item_path=$(dirname "$current_dir")
                 display_name="../ (go up)"
            else
                 display_name="$(basename "$item_path")"
            fi
            
            local icon=$(get_item_icon "$item_path")

            if [ $i -eq $selected_index ]; then
                # Highlight selected
                if [ -d "$item_path" ]; then
                    echo -e "  ${BOLD}${GREEN}▶ $icon $display_name${NC}"
                else
                    echo -e "  ${BOLD}${WHITE}▶ $icon $display_name${NC}"
                fi

                # Show description
                local desc
                desc=$(get_description "$item_path")
                echo -e "    ${DIM}${CYAN}$desc${NC}"
            else
                # Normal display
                if [ -d "$item_path" ]; then
                    echo -e "    ${BLUE}$icon $display_name${NC}"
                else
                    echo -e "    ${GRAY}$icon $display_name${NC}"
                fi
            fi
        done

        # Scroll indicators
        if [ $scroll_offset -gt 0 ]; then
            move_cursor 7 70
            echo -e "${YELLOW}▲ More above${NC}"
        fi
        if [ $visible_end -lt ${#items[@]} ]; then
            move_cursor $((8 + max_visible)) 70
            echo -e "${YELLOW}▼ More below${NC}"
        fi

        print_footer

        # Read input
        read -rsn1 -t 1 key
        if [ "$key" = $'\x1b' ]; then
            read -rsn2 -t 0.1 key
            case "$key" in
                '[A') # Up arrow
                    [ $selected_index -gt 0 ] && ((selected_index--))
                    ;;
                '[B') # Down arrow
                    [ $selected_index -lt $((${#items[@]} - 1)) ] && ((selected_index++))
                    ;;
                '[D') # Left arrow (back)
                    if [ "$current_dir" != "$WORKSPACE_ROOT" ]; then
                        show_cursor
                        return
                    fi
                    ;;
            esac
        else
            case "$key" in
                '') # Enter
                    local selected="${items[$selected_index]}"
                    if [[ "$(basename "$selected")" == ".." ]]; then
                        show_cursor
                        return
                    elif [ -d "$selected" ]; then
                        browse_directory "$selected"
                        selected_index=0
                        scroll_offset=0
                    else
                        show_file_info "$selected"
                    fi
                    ;;
                'q'|'Q') # Quit
                    show_cursor
                    clear_screen
                    echo "👋 Goodbye!"
                    exit 0
                    ;;
                'h'|'H'|'?') # Help
                    show_file_info "${items[$selected_index]}"
                    ;;
                $'\x7f') # Backspace
                    if [ "$current_dir" != "$WORKSPACE_ROOT" ]; then
                        show_cursor
                        return
                    fi
                    ;;
            esac
        fi
    done
}

# ============================================================================
# File Info Display
# ============================================================================

show_file_info() {
    local file="$1"
    local filename
    filename="$(basename "$file")"
     if [[ "$filename" == ".." ]]; then
        file=$(dirname "$(dirname "$file")")
        filename="$(basename "$file")"
    fi


    clear_screen
    echo -e "${BOLD}${CYAN}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}${CYAN}║${NC} ${BOLD}Item Information${NC}                                                          ${BOLD}${CYAN}║${NC}"
    echo -e "${BOLD}${CYAN}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""

    echo -e "${BOLD}${WHITE}📄 Name:${NC} $filename"
    echo -e "${BOLD}${WHITE}📂 Path:${NC} ${file#$WORKSPACE_ROOT}"
    echo ""

    if [ -d "$file" ]; then
        echo -e "${BOLD}${WHITE}📊 Type:${NC} Directory"
        local item_count
        item_count=$(find "$file" -maxdepth 1 -mindepth 1 | wc -l)
        echo -e "${BOLD}${WHITE}📁 Items:${NC} $item_count"
    else
        echo -e "${BOLD}${WHITE}📊 Type:${NC} File"
        echo -e "${BOLD}${WHITE}💾 Size:${NC} $(du -h "$file" | cut -f1)"
        echo -e "${BOLD}${WHITE}📅 Modified:${NC} $(stat -c %y "$file" | cut -d. -f1)"
    fi

    echo ""
    echo -e "${BOLD}${WHITE}ℹ️  Description:${NC}"
    local desc
    desc=$(get_description "$file")
    echo -e "${CYAN}$desc${NC}"

    echo ""
    echo -e "${GRAY}───────────────────────────────────────────────────────────────────────────────${NC}"

    if [ -f "$file" ]; then
        echo -e "${DIM}v: View file (less)  e: Edit (vim)  Enter: Back${NC}"

        read -rsn1 action
        case "$action" in
            'v'|'V')
                show_cursor
                less "$file"
                hide_cursor
                ;;
            'e'|'E')
                show_cursor
                vim "$file"
                hide_cursor
                ;;
        esac
    else
        echo -e "${DIM}Press any key to go back${NC}"
        read -rsn1
    fi
}

# ============================================================================
# Main Entry Point
# ============================================================================

main() {
    local start_dir="${1:-$WORKSPACE_ROOT}"

    # Validate directory
    if [ ! -d "$start_dir" ]; then
        # Try to resolve as a shortcut from ws_nav
        local nav_output
        nav_output=$(ws_nav "$start_dir" 2>&1)
        if [ $? -eq 0 ]; then
            start_dir=$(echo "$nav_output" | sed -n 's/📂 //p')
        else
            echo "Error: Directory not found: $start_dir"
            exit 1
        fi
    fi

    # Setup terminal
    trap show_cursor EXIT
    hide_cursor

    # Start browsing
    browse_directory "$start_dir"

    # Cleanup
    show_cursor
    clear_screen
}

# Run if executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
