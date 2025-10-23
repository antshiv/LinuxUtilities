#!/bin/bash

# A utility to find new screenshots, move them to the project,
# and generate a prompt for the Gemini CLI.
# Includes interactive previews, history, cleaning, and auto-copy to clipboard.

# --- Configuration ---
DOWNLOADS_DIR="$HOME/Downloads"
DEFAULT_DEST_DIR="Screenshots"
SCREENSHOT_PATTERN="Screenshot*.png"

# --- Main Logic ---

# Check for flags
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    echo "Usage: ./import_screenshots.sh [option]"
    echo ""
    echo "A utility to manage screenshots and photos for sharing with the AI."
    echo ""
    echo "Default behavior (no options):"
    echo "  - Imports new desktop screenshots to the 'Screenshots/' directory."
    echo "  - If no new files, interactively prompts from recent screenshots."
    echo ""
    echo "Options:"
    echo "  --photo, --pxl [dest]  Imports new phone photos (PXL_*.jpg). Defaults to ~/Pictures/Phone/."
    echo "                     [dest] can be a folder path or '.' for the current directory."
    echo "  --clean          Prompts to delete all .png files from the 'Screenshots/' directory."
    echo "  --history, -l    Lists the 5 most recently imported screenshots."
    echo "  --help, -h       Displays this help message."
    exit 0
fi

# Function to output the final prompt
output_prompt() {
    PROMPT_TEXT="Please analyze these files:$1"
    if command -v xclip &> /dev/null; then
        echo -n "$PROMPT_TEXT" | xclip -selection primary
        echo ""
        echo "✅ Prompt automatically copied to primary selection (middle-click to paste)!"
        echo ""
    else
        echo ""
        echo "--- (xclip not found, please install it for auto-copy) ---"
        echo "Copy the full prompt below:"
        echo ""
        echo "$PROMPT_TEXT"
        echo "---"
    fi
}

# Handle --photo flag
if [[ "$1" == "--photo" || "$1" == "--pxl" ]]; then
    PHOTO_DEST_DIR="$HOME/Pictures/Phone"
    if [ -n "$2" ]; then
        if [ "$2" == "." ]; then
            PHOTO_DEST_DIR=$(pwd)
        else
            PHOTO_DEST_DIR="$2"
        fi
    fi
    PHOTO_PATTERN="PXL_*.jp*g"
    echo "Mode: Importing Phone Photos"
    mkdir -p "$PHOTO_DEST_DIR"
    echo "Destination folder is '$PHOTO_DEST_DIR'"
    echo "🔍 Searching for files matching '$PHOTO_PATTERN' in $DOWNLOADS_DIR..."
    
    MOVED_COUNT=0
    PROMPT_BODY=""
    
    while IFS= read -r filepath; do
        filename=$(basename "$filepath")
        if [ ! -f "$PHOTO_DEST_DIR/$filename" ]; then
            echo "  -> Moving '$filename'..."
            mv "$filepath" "$PHOTO_DEST_DIR/"
            PROMPT_BODY+=" @$PHOTO_DEST_DIR/$filename"
            MOVED_COUNT=$((MOVED_COUNT + 1))
        fi
    done < <(find "$DOWNLOADS_DIR" -maxdepth 1 -name "$PHOTO_PATTERN")

    if [ "$MOVED_COUNT" -gt 0 ]; then
        echo "✅ Moved $MOVED_COUNT new photo(s)."
        output_prompt "$PROMPT_BODY"
    else
        echo "✅ No new phone photos found in Downloads."
    fi
    exit 0
fi

if [[ "$1" == "--history" || "$1" == "-l" ]]; then
    echo "--- Most Recent 5 Files in '$DEFAULT_DEST_DIR' ---"
    ls -t "$DEFAULT_DEST_DIR"/*.png 2>/dev/null | head -n 5 | nl
    exit 0
fi

if [[ "$1" == "--clean" ]]; then
    echo "This will permanently delete all .png files in the '$DEFAULT_DEST_DIR' directory."
    read -p "Are you sure you want to continue? (y/n) " -n 1 -r
    echo "" # Move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Cleaning directory: $DEFAULT_DEST_DIR..."
        find "$DEFAULT_DEST_DIR" -maxdepth 1 -name "*.png" -delete
        echo "✅ Directory cleaned."
    else
        echo "Clean operation cancelled."
    fi
    exit 0
fi

# --- Default Screenshot Logic ---

mkdir -p "$DEFAULT_DEST_DIR"
echo "Image destination folder is '$DEFAULT_DEST_DIR'"
echo "🔍 Searching for new screenshots in $DOWNLOADS_DIR..."
MOVED_COUNT=0
PROMPT_BODY=""

while IFS= read -r filepath; do
  filename=$(basename "$filepath")
  if [ ! -f "$DEFAULT_DEST_DIR/$filename" ]; then
    echo "  -> Moving '$filename'..."
    mv "$filepath" "$DEFAULT_DEST_DIR/"
    PROMPT_BODY+=" @$DEFAULT_DEST_DIR/$filename"
    MOVED_COUNT=$((MOVED_COUNT + 1))
  fi
done < <(find "$DOWNLOADS_DIR" -maxdepth 1 -name "$SCREENSHOT_PATTERN")

if [ "$MOVED_COUNT" -gt 0 ]; then
  echo "✅ Moved $MOVED_COUNT new screenshot(s)."
  output_prompt "$PROMPT_BODY"
else
  echo "✅ No new screenshots found."
  echo ""
  read -p "Generate a prompt from recent files instead? (y/n) " -n 1 -r
  echo ""
  if [[ $REPLY =~ ^[Yy]$ ]]; then
    
    CURRENT_DIR=$(pwd)
    cd "$DEFAULT_DEST_DIR" || { echo "Error: Could not enter $DEFAULT_DEST_DIR"; exit 1; }
    mapfile -t RECENT_FILES < <(ls -t *.png 2>/dev/null | head -n 5)
    
    if [ ${#RECENT_FILES[@]} -eq 0 ]; then
      echo "No existing screenshots found in $DEFAULT_DEST_DIR."
      cd - > /dev/null
      exit 0
    fi

    echo ""
    echo "Displaying recent screenshots with thumbnails..."
    i=1
    declare -a FILE_LINKS

    for f in "${RECENT_FILES[@]}"; do
      ENCODED_F=$(echo "$f" | sed 's/ /%20/g')
      FILE_PATH="file://$CURRENT_DIR/$DEFAULT_DEST_DIR/$ENCODED_F"
      FILE_LINKS+=("$FILE_PATH")
      
      echo "========================================"
      echo "  $i) $f"
      echo "========================================"
      chafa --symbols braille --size 60x20 "$f"
      echo ""
      ((i++))
    done

    cd - > /dev/null

    # --- Simplified Summary Block ---
    echo "--- File Links Summary ---"
    i=0
    while [ $i -lt ${#RECENT_FILES[@]} ]; do
        filename="${RECENT_FILES[$i]}"
        link="${FILE_LINKS[$i]}"
        echo "Image $((i+1)): $filename"
        echo "Link: $link"
        ((i++))
    done
    echo "------------------------"
    echo ""

    echo "Enter the numbers of the files to include, separated by spaces (e.g., 1 3 4), then press Enter:"
    read -ra choices

    PROMPT_BODY=""
    for choice in "${choices[@]}"; do
      if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#RECENT_FILES[@]}" ]; then
        index=$((choice - 1))
        filename="${RECENT_FILES[$index]}"
        PROMPT_BODY+=" @$DEFAULT_DEST_DIR/$filename"
      fi
    done

    if [ -z "$PROMPT_BODY" ]; then
      echo "No valid files selected."
    else
      output_prompt "$PROMPT_BODY"
    fi
  fi
fi