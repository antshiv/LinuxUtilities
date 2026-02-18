#!/usr/bin/env bash
set -Eeuo pipefail

# Import screenshots/photos from common folders and build a prompt containing file refs.

DOWNLOADS_DIR="${HOME}/Downloads"
PICTURES_DIR="${HOME}/Pictures"
FLAMESHOT_CONFIG="${HOME}/.config/flameshot/flameshot.ini"
FLAMESHOT_FALLBACK_DIR="${HOME}/Screenshots"
DEFAULT_DEST_DIR="Screenshots"
SCREENSHOT_PATTERN="Screenshot*.png"
PHONE_PATTERN="PXL_*.jp*g"

have_cmd() {
    command -v "$1" >/dev/null 2>&1
}

abs_path() {
    readlink -f -- "$1"
}

print_help() {
    cat <<'EOF'
Usage: ./import_screenshots.sh [option]

Default behavior (no options):
  - Imports new desktop screenshots from ~/Downloads, ~/Pictures, and Flameshot's save path to ./Screenshots
  - If no new files are found, offers interactive selection from recent files

Options:
  --photo, --pxl [dest]  Import new phone photos (PXL_*.jpg) to ~/Pictures/Phone
                         [dest] can be a folder path or '.' for current directory.
  --clean                Delete all .png files in ./Screenshots after confirmation.
  --history, -l          List the 5 most recently imported screenshots.
  --help, -h             Show this help.
EOF
}

expand_home_path() {
    local path="$1"
    if [[ -z "$path" ]]; then
        printf '%s\n' ""
        return 0
    fi
    if [[ "$path" == "~" ]]; then
        printf '%s\n' "$HOME"
        return 0
    fi
    if [[ "$path" == "~/"* ]]; then
        printf '%s\n' "${HOME}${path:1}"
        return 0
    fi
    printf '%s\n' "$path"
}

detect_flameshot_save_dir() {
    local save_dir=""
    local config_value=""

    if [[ -f "$FLAMESHOT_CONFIG" ]]; then
        config_value="$(sed -n 's/^savePath=\(.*\)$/\1/p' "$FLAMESHOT_CONFIG" | tail -n 1)"
        config_value="$(expand_home_path "$config_value")"
        if [[ -n "$config_value" && -d "$config_value" ]]; then
            save_dir="$config_value"
        fi
    fi

    if [[ -z "$save_dir" ]]; then
        save_dir="$FLAMESHOT_FALLBACK_DIR"
    fi

    printf '%s\n' "$save_dir"
}

copy_prompt_to_clipboard() {
    local prompt_text="$1"
    local session_type="${XDG_SESSION_TYPE:-}"
    local wl_primary_ok=0
    local clip_ok=1
    local primary_ok=1

    copy_with_x11_tools() {
        # CLIPBOARD: prefer xclip, fallback xsel
        if have_cmd xclip; then
            if ! printf '%s' "$prompt_text" | xclip -selection clipboard -in; then
                clip_ok=0
            fi
        elif have_cmd xsel; then
            if ! printf '%s' "$prompt_text" | xsel --clipboard --input; then
                clip_ok=0
            fi
        else
            clip_ok=0
        fi

        # PRIMARY (middle-click): prefer xsel on X11, fallback xclip
        if have_cmd xsel; then
            if ! printf '%s' "$prompt_text" | xsel --primary --input; then
                primary_ok=0
            fi
        elif have_cmd xclip; then
            if ! printf '%s' "$prompt_text" | xclip -selection primary -in; then
                primary_ok=0
            fi
        else
            primary_ok=0
        fi

        if (( clip_ok == 1 && primary_ok == 1 )); then
            echo "Copied prompt to clipboard and primary selection via X11 tools."
            return 0
        fi
        if (( clip_ok == 1 && primary_ok == 0 )); then
            echo "Copied prompt to clipboard, but primary selection could not be set."
            return 0
        fi
        if (( clip_ok == 0 && primary_ok == 1 )); then
            echo "Copied prompt to primary selection, but clipboard could not be set."
            return 0
        fi
        return 1
    }

    copy_with_wlcopy() {
        if ! have_cmd wl-copy; then
            return 1
        fi
        if printf '%s' "$prompt_text" | wl-copy; then
            if printf '%s' "$prompt_text" | wl-copy --primary >/dev/null 2>&1; then
                wl_primary_ok=1
            fi
            if (( wl_primary_ok == 1 )); then
                echo "Copied prompt to clipboard and primary selection via wl-copy."
            else
                echo "Copied prompt to clipboard via wl-copy (primary selection unavailable in this session)."
            fi
            return 0
        fi
        return 1
    }

    # X11: prefer xclip/xsel for PRIMARY selection reliability.
    if [[ "$session_type" == "x11" ]]; then
        copy_with_x11_tools && return 0
        copy_with_wlcopy && return 0
    else
        copy_with_wlcopy && return 0
        copy_with_x11_tools && return 0
    fi

    return 1
}

output_prompt() {
    local prompt_body="$1"
    local prompt_text="Please analyze these files:${prompt_body}"

    echo
    if ! copy_prompt_to_clipboard "$prompt_text"; then
        if [[ "${XDG_SESSION_TYPE:-}" == "x11" ]]; then
            echo "Clipboard helper not found. Install one: sudo apt install xclip xsel"
        else
            echo "Clipboard helper not found. Install one: sudo apt install wl-clipboard xclip xsel"
        fi
    fi
    echo
    echo "$prompt_text"
    echo
}

unique_target_path() {
    local dest_dir="$1"
    local filename="$2"
    local candidate="$dest_dir/$filename"
    local base ext idx

    if [[ ! -e "$candidate" ]]; then
        printf '%s\n' "$candidate"
        return 0
    fi

    base="${filename%.*}"
    ext="${filename##*.}"
    if [[ "$base" == "$filename" ]]; then
        ext=""
    fi

    idx=1
    while true; do
        if [[ -n "$ext" ]]; then
            candidate="$dest_dir/${base}_$idx.$ext"
        else
            candidate="$dest_dir/${base}_$idx"
        fi
        if [[ ! -e "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
        idx=$((idx + 1))
    done
}

list_recent_pngs() {
    local dir="$1"
    local limit="${2:-5}"

    if [[ ! -d "$dir" ]]; then
        return 0
    fi

    find "$dir" -maxdepth 1 -type f -name "*.png" -printf '%T@ %f\n' \
        | sort -nr \
        | head -n "$limit" \
        | cut -d' ' -f2-
}

import_matching_files() {
    local dest_dir="$1"
    local pattern="$2"
    local mode_name="$3"
    shift 3
    local source_dirs=("$@")
    local moved_count=0
    local prompt_body=""
    local abs_dest_dir abs_source_dir source_dir file_path file_name target_path target_name
    local seen
    local -a active_sources=()

    mkdir -p "$dest_dir"
    abs_dest_dir="$(abs_path "$dest_dir")"

    echo "Mode: $mode_name"
    echo "Destination: $dest_dir"
    for source_dir in "${source_dirs[@]}"; do
        [[ -d "$source_dir" ]] || continue
        abs_source_dir="$(abs_path "$source_dir")"
        if [[ "$abs_source_dir" == "$abs_dest_dir" ]]; then
            continue
        fi

        seen=0
        for source_dir in "${active_sources[@]}"; do
            if [[ "$(abs_path "$source_dir")" == "$abs_source_dir" ]]; then
                seen=1
                break
            fi
        done
        if (( seen == 0 )); then
            active_sources+=("$abs_source_dir")
        fi
    done

    echo "Sources:"
    for source_dir in "${active_sources[@]}"; do
        echo "  - $source_dir"
    done
    echo "Pattern: $pattern"

    for source_dir in "${active_sources[@]}"; do
        while IFS= read -r -d '' file_path; do
            file_name="$(basename "$file_path")"
            target_path="$(unique_target_path "$dest_dir" "$file_name")"
            target_name="$(basename "$target_path")"

            echo "  -> Moving '$file_name' -> '$target_name'"
            mv -- "$file_path" "$target_path"
            prompt_body+=$'\n'" @$abs_dest_dir/$target_name"
            moved_count=$((moved_count + 1))
        done < <(find "$source_dir" -maxdepth 1 -type f -name "$pattern" -print0)
    done

    if (( moved_count > 0 )); then
        echo "Moved $moved_count file(s)."
        output_prompt "$prompt_body"
        return 0
    fi

    echo "No new files found."
    return 1
}

prompt_from_recent_screenshots() {
    local dest_dir="$1"
    local abs_dest_dir idx file choice prompt_body
    local -a recent_files

    abs_dest_dir="$(abs_path "$dest_dir")"
    mapfile -t recent_files < <(list_recent_pngs "$dest_dir" 5)

    if (( ${#recent_files[@]} == 0 )); then
        echo "No existing screenshots found in $dest_dir."
        return 0
    fi

    echo
    echo "Recent screenshots:"
    for idx in "${!recent_files[@]}"; do
        file="${recent_files[$idx]}"
        printf '%d) %s\n' "$((idx + 1))" "$file"
        if have_cmd chafa; then
            chafa --symbols braille --size 60x20 "$dest_dir/$file" || true
        fi
        echo
    done

    read -r -p "Enter screenshot numbers (e.g. 1 3 4), then Enter: " selection_line

    prompt_body=""
    for choice in $selection_line; do
        if [[ "$choice" =~ ^[0-9]+$ ]] && (( choice >= 1 && choice <= ${#recent_files[@]} )); then
            file="${recent_files[$((choice - 1))]}"
            prompt_body+=$'\n'" @$abs_dest_dir/$file"
        fi
    done

    if [[ -z "$prompt_body" ]]; then
        echo "No valid files selected."
        return 0
    fi

    output_prompt "$prompt_body"
}

# Option handling
case "${1:-}" in
    --help|-h)
        print_help
        exit 0
        ;;
    --photo|--pxl)
        photo_dest_dir="${HOME}/Pictures/Phone"
        if [[ -n "${2:-}" ]]; then
            if [[ "$2" == "." ]]; then
                photo_dest_dir="$(pwd)"
            else
                photo_dest_dir="$(abs_path "$2")"
            fi
        fi
        import_matching_files "$photo_dest_dir" "$PHONE_PATTERN" "Importing phone photos" "$DOWNLOADS_DIR"
        exit 0
        ;;
    --history|-l)
        echo "--- Most Recent 5 Files in '$DEFAULT_DEST_DIR' ---"
        mapfile -t history_files < <(list_recent_pngs "$DEFAULT_DEST_DIR" 5)
        if (( ${#history_files[@]} == 0 )); then
            echo "No screenshots found."
            exit 0
        fi
        for idx in "${!history_files[@]}"; do
            printf '%d) %s\n' "$((idx + 1))" "${history_files[$idx]}"
        done
        exit 0
        ;;
    --clean)
        if [[ ! -d "$DEFAULT_DEST_DIR" ]]; then
            echo "Directory '$DEFAULT_DEST_DIR' does not exist. Nothing to clean."
            exit 0
        fi
        echo "This will permanently delete all .png files in '$DEFAULT_DEST_DIR'."
        read -r -p "Continue? (y/n) " confirm
        if [[ "$confirm" =~ ^[Yy]$ ]]; then
            find "$DEFAULT_DEST_DIR" -maxdepth 1 -type f -name "*.png" -delete
            echo "Directory cleaned."
        else
            echo "Clean operation cancelled."
        fi
        exit 0
        ;;
    "")
        ;;
    *)
        echo "Unknown option: $1"
        print_help
        exit 1
        ;;
esac

# Default screenshot import mode
FLAMESHOT_SAVE_DIR="$(detect_flameshot_save_dir)"
mkdir -p "$DEFAULT_DEST_DIR"
if import_matching_files "$DEFAULT_DEST_DIR" "$SCREENSHOT_PATTERN" "Importing screenshots" "$DOWNLOADS_DIR" "$PICTURES_DIR" "$FLAMESHOT_SAVE_DIR"; then
    exit 0
fi

echo
read -r -p "Generate a prompt from recent screenshots instead? (y/n) " confirm
if [[ "$confirm" =~ ^[Yy]$ ]]; then
    prompt_from_recent_screenshots "$DEFAULT_DEST_DIR"
fi
