#!/bin/bash
set -euo pipefail

# Configuration
STATE_FILE="${XDG_CACHE_HOME:-$HOME/.cache}/window-dock-state"
LAPTOP_SCREEN="${LAPTOP_SCREEN:-LVDS-1}"  # Set to your laptop screen name (eDP-1, LVDS-1, etc.)

# Usage information
show_usage() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS]

Window management script for dock/undock scenarios.

OPTIONS:
    --move-to-laptop        Move all windows to laptop screen
    --move-to-external      Move all windows to external monitor
    --spread-all            Spread windows evenly across all screens
    --auto                  Auto-detect dock/undock and move windows (default)
    --rescue                Rescue any off-screen windows
    -h, --help              Show this help message

If no option is provided, an interactive menu will be shown.

EXAMPLES:
    $(basename "$0")                      # Interactive menu
    $(basename "$0") --move-to-laptop     # Move everything to laptop
    $(basename "$0") --auto               # Auto dock/undock detection

EOF
    exit 0
}

# Get screen information
get_screens() {
    xrandr --query | awk '/ connected/ {
        for(i=1;i<=NF;i++) {
            if ($i ~ /^[0-9]+x[0-9]+\+[0-9]+\+[0-9]+$/) {
                print $1 ":" $i
                break
            }
        }
    }'
}

# Parse geometry string (WIDTHxHEIGHT+X+Y)
parse_geom() {
    local geom="$1"
    local w=${geom%%x*}
    local rest=${geom#*x}
    local h=${rest%%+*}
    rest=${rest#*+}
    local x=${rest%%+*}
    local y=${rest#*+}
    echo "$w $h $x $y"
}

# Check if point is inside rectangle
point_in_rect() {
    local px=$1 py=$2 rx=$3 ry=$4 rw=$5 rh=$6
    [ "$px" -ge "$rx" ] && [ "$py" -ge "$ry" ] && \
    [ "$px" -lt $((rx+rw)) ] && [ "$py" -lt $((ry+rh)) ]
}

# Get Awesome WM tab groups using awesome-client
get_awesome_tabs() {
    if ! command -v awesome-client &> /dev/null; then
        return 1
    fi

    awesome-client 'local awful = require("awful")
local result = {}
for s in screen do
    for _, c in ipairs(s.clients) do
        local master = awful.client.getmaster(s)
        if c.first_tag then
            local siblings = {}
            for _, sib in ipairs(c.first_tag:clients()) do
                if sib.window ~= c.window then
                    table.insert(siblings, sib.window)
                end
            end
            result[tostring(c.window)] = {
                tag = c.first_tag.name,
                siblings = siblings,
                screen = s.index
            }
        end
    end
end
for wid, data in pairs(result) do
    print(wid .. "|" .. data.tag .. "|" .. table.concat(data.siblings, ",") .. "|" .. data.screen)
end' 2>/dev/null || return 1
}

# Restore Awesome WM tab groups
restore_awesome_tabs() {
    local state_file="$1"
    [ ! -f "$state_file" ] && return 0

    if ! command -v awesome-client &> /dev/null; then
        return 0
    fi

    # This is complex and would require significant Lua code
    # For now, we'll skip automatic tab restoration and just move windows
    # Users can manually re-tab windows or we can enhance this later
    return 0
}

# Movement functions
move_all_to_screen() {
    local target_name="$1"
    local target_geom="$2"

    read -r tw th tx ty <<< "$(parse_geom "$target_geom")"

    echo "==> Moving all windows to $target_name"

    # Store all window info in arrays to avoid subshell issues
    local -a wids=()
    local -a positions=()
    local -a titles=()

    while read -r wid ws x y w h host title; do
        # Skip if already on target screen
        if point_in_rect "$x" "$y" "$tx" "$ty" "$tw" "$th"; then
            continue
        fi

        wids+=("$wid")
        positions+=("$x:$y")
        titles+=("$title")
    done < <(wmctrl -lG)

    # Now move all windows
    cascade_offset=0
    for i in "${!wids[@]}"; do
        local wid="${wids[$i]}"
        local pos="${positions[$i]}"
        local title="${titles[$i]}"
        local x="${pos%%:*}"
        local y="${pos#*:}"

        # Find which screen window is currently on
        local moved=0
        for screen_info in $(get_screens); do
            screen_geom="${screen_info#*:}"
            read -r sw sh sx sy <<< "$(parse_geom "$screen_geom")"

            if point_in_rect "$x" "$y" "$sx" "$sy" "$sw" "$sh"; then
                # Calculate relative position on current screen
                rel_x=$((x - sx))
                rel_y=$((y - sy))

                # Map to target screen proportionally
                new_x=$((tx + rel_x * tw / sw + cascade_offset))
                new_y=$((ty + rel_y * th / sh + cascade_offset))
                cascade_offset=$((cascade_offset + 30))

                # Bounds check
                [ $new_x -lt $tx ] && new_x=$tx
                [ $new_y -lt $ty ] && new_y=$ty
                [ $new_x -gt $((tx + tw - 100)) ] && new_x=$((tx + tw - 100))
                [ $new_y -gt $((ty + th - 100)) ] && new_y=$((ty + th - 100))

                echo "  Moving: $title"
                wmctrl -i -r "$wid" -e 0,$new_x,$new_y,-1,-1
                moved=1
                break
            fi
        done

        # If not on any screen (shouldn't happen), move it anyway
        if [ $moved -eq 0 ]; then
            new_x=$((tx + 50 + cascade_offset))
            new_y=$((ty + 50 + cascade_offset))
            cascade_offset=$((cascade_offset + 30))
            echo "  Moving (orphaned): $title"
            wmctrl -i -r "$wid" -e 0,$new_x,$new_y,-1,-1
        fi
    done
}

spread_windows_across_all() {
    echo "==> Spreading windows across all screens"

    # Get all screens
    local -a screen_list=()
    while IFS=: read -r name geom; do
        screen_list+=("$name:$geom")
    done < <(get_screens)

    local screen_count=${#screen_list[@]}
    [ $screen_count -eq 0 ] && return

    # Collect all windows first
    local -a wids=()
    local -a titles=()
    while read -r wid ws x y w h host title; do
        wids+=("$wid")
        titles+=("$title")
    done < <(wmctrl -lG)

    # Now spread them across screens
    local screen_index=0
    for i in "${!wids[@]}"; do
        local wid="${wids[$i]}"
        local title="${titles[$i]}"

        # Get target screen (round-robin)
        local target="${screen_list[$screen_index]}"
        local target_name="${target%%:*}"
        local target_geom="${target#*:}"

        read -r tw th tx ty <<< "$(parse_geom "$target_geom")"

        # Position in center of target screen with some offset
        local offset=$((screen_index * 50))
        local new_x=$((tx + tw / 4 + offset))
        local new_y=$((ty + th / 4 + offset))

        echo "  Moving '$title' to $target_name"
        wmctrl -i -r "$wid" -e 0,$new_x,$new_y,-1,-1

        # Next screen
        screen_index=$(((screen_index + 1) % screen_count))
    done
}

rescue_offscreen_windows() {
    echo "==> Rescuing off-screen windows"

    local all_geoms=$(get_screens | cut -d: -f2)

    # Use laptop screen as rescue target
    local laptop_geom=$(get_screens | grep "^${LAPTOP_SCREEN}:" | cut -d: -f2)
    read -r lw lh lx ly <<< "$(parse_geom "$laptop_geom")"

    # Collect windows to rescue
    local -a rescue_wids=()
    local -a rescue_titles=()

    while read -r wid ws x y w h host title; do
        inside=0
        while read -r g; do
            [ -z "$g" ] && continue
            read -r gw gh gx gy <<< "$(parse_geom "$g")"
            if point_in_rect "$x" "$y" "$gx" "$gy" "$gw" "$gh"; then
                inside=1
                break
            fi
        done <<< "$all_geoms"

        if [ "$inside" -eq 0 ]; then
            rescue_wids+=("$wid")
            rescue_titles+=("$title")
        fi
    done < <(wmctrl -lG)

    # Now rescue them
    cascade_offset=0
    for i in "${!rescue_wids[@]}"; do
        local wid="${rescue_wids[$i]}"
        local title="${rescue_titles[$i]}"

        echo "  Rescuing: $title"
        new_x=$((lx + 50 + cascade_offset))
        new_y=$((ly + 50 + cascade_offset))
        cascade_offset=$((cascade_offset + 30))
        wmctrl -i -r "$wid" -e 0,$new_x,$new_y,-1,-1
    done
}

# Parse command-line arguments
MODE=""
case "${1:-}" in
    --move-to-laptop)
        MODE="move-to-laptop"
        ;;
    --move-to-external)
        MODE="move-to-external"
        ;;
    --spread-all)
        MODE="spread-all"
        ;;
    --auto)
        MODE="auto"
        ;;
    --rescue)
        MODE="rescue"
        ;;
    -h|--help)
        show_usage
        ;;
    "")
        # No argument, show interactive menu
        MODE="interactive"
        ;;
    *)
        echo "Error: Unknown option '$1'"
        echo "Use --help for usage information"
        exit 1
        ;;
esac

# Main logic
screens=$(get_screens)
external_screens=$(echo "$screens" | grep -v "^${LAPTOP_SCREEN}:" || true)
laptop_screen=$(echo "$screens" | grep "^${LAPTOP_SCREEN}:" || true)

if [ -z "$laptop_screen" ]; then
    echo "Error: Laptop screen $LAPTOP_SCREEN not found"
    echo "Available screens:"
    echo "$screens"
    echo "Set LAPTOP_SCREEN environment variable to your laptop screen name"
    exit 1
fi

laptop_geom=$(echo "$laptop_screen" | cut -d: -f2)
laptop_name="${laptop_screen%%:*}"

# Display current screen configuration
echo "=== Screen Configuration ==="
echo "Laptop: $laptop_name ($laptop_geom)"
if [ -n "$external_screens" ]; then
    echo "External monitors:"
    echo "$external_screens" | while IFS=: read -r name geom; do
        echo "  - $name ($geom)"
    done
else
    echo "External monitors: None"
fi
echo ""

# Interactive menu if no mode specified
if [ "$MODE" = "interactive" ]; then
    echo "What would you like to do?"
    options=("Move all windows to laptop screen")

    # Add external monitor options if available
    if [ -n "$external_screens" ]; then
        while IFS=: read -r name geom; do
            options+=("Move all windows to $name")
        done <<< "$external_screens"
    fi

    options+=("Spread windows across all screens")
    options+=("Auto-detect dock/undock state")
    options+=("Rescue off-screen windows only")
    options+=("Quit")

    select opt in "${options[@]}"; do
        case $REPLY in
            1)
                MODE="move-to-laptop"
                break
                ;;
            2)
                if [ -n "$external_screens" ]; then
                    MODE="move-to-external"
                    break
                fi
                ;;
            3)
                if [ -n "$external_screens" ]; then
                    MODE="spread-all"
                    break
                else
                    MODE="auto"
                    break
                fi
                ;;
            4)
                if [ -n "$external_screens" ]; then
                    MODE="auto"
                    break
                else
                    MODE="rescue"
                    break
                fi
                ;;
            5)
                if [ -n "$external_screens" ]; then
                    MODE="rescue"
                    break
                else
                    echo "Goodbye!"
                    exit 0
                fi
                ;;
            6)
                if [ -n "$external_screens" ]; then
                    echo "Goodbye!"
                    exit 0
                fi
                ;;
            *)
                echo "Invalid option. Please try again."
                ;;
        esac
    done
    echo ""
fi

# Execute based on mode
case "$MODE" in
    move-to-laptop)
        move_all_to_screen "$laptop_name" "$laptop_geom"
        ;;

    move-to-external)
        if [ -z "$external_screens" ]; then
            echo "Error: No external monitor detected"
            exit 1
        fi
        # Use first external screen
        external_info=$(echo "$external_screens" | head -n1)
        external_name="${external_info%%:*}"
        external_geom="${external_info#*:}"
        move_all_to_screen "$external_name" "$external_geom"
        ;;

    spread-all)
        spread_windows_across_all
        ;;

    rescue)
        rescue_offscreen_windows
        ;;

    auto)
        # Original auto-detect behavior
        is_docked=0
        if [ -n "$external_screens" ]; then
            is_docked=1
        fi

        prev_docked=0
        if [ -f "$STATE_FILE" ]; then
            prev_docked=$(cat "$STATE_FILE")
        fi

        echo "Current state: $([ $is_docked -eq 1 ] && echo "DOCKED" || echo "UNDOCKED")"
        echo "Previous state: $([ $prev_docked -eq 1 ] && echo "DOCKED" || echo "UNDOCKED")"
        echo ""

        if [ $is_docked -eq 1 ] && [ $prev_docked -eq 0 ]; then
            # Docking detected
            external_info=$(echo "$external_screens" | head -n1)
            external_name="${external_info%%:*}"
            external_geom="${external_info#*:}"
            echo "==> Docking detected: Moving windows to $external_name"
            move_all_to_screen "$external_name" "$external_geom"
        elif [ $is_docked -eq 0 ] && [ $prev_docked -eq 1 ]; then
            # Undocking detected
            echo "==> Undocking detected: Moving windows to $laptop_name"
            move_all_to_screen "$laptop_name" "$laptop_geom"
        else
            # No change, just rescue
            echo "==> No dock state change"
            rescue_offscreen_windows
        fi

        # Save current state
        echo "$is_docked" > "$STATE_FILE"
        ;;
esac

echo ""
echo "Done!"
