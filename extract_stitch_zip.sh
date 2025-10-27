#!/bin/bash

# A utility to extract content from a zip file, specifically looking for
# files/folders starting with 'stitch_' and moving them to a destination.

# --- Configuration ---
DOWNLOADS_DIR="$HOME/Downloads"
DEFAULT_BASE_DEST_DIR="." # Current directory for base feature folders

# --- Functions ---

# Function to display help message
display_help() {
    echo "Usage: ./extract_stitch_zip.sh [zip_file_path]"
    echo ""
    echo "If no zip_file_path is provided, the script will search for 'stitch_*.zip' files"
    echo "in your Downloads directory, prompt for confirmation, and process them."
    echo ""
    echo "For each 'stitch_*.zip' file:"
    echo "  1. It extracts the feature name (e.g., 'state_estimation_view' from 'stitch_state_estimation_view(1).zip')."
    echo "  2. Creates a main folder based on the feature name (e.g., './state_estimation_view/')."
    echo "  3. If a sequence number is found (e.g., '(1)'), it creates a nested folder (e.g., './state_estimation_view/1/')."
    echo "     If the nested folder already exists, it finds the next available sequential number."
    echo "  4. Extracts the zip file contents directly into this nested folder."
    echo ""
    echo "Arguments:"
    echo "  [zip_file_path]        Optional. The path to a specific zip file to extract."
    echo "                         If provided, only this file will be processed."
    echo ""
    echo "Options:"
    echo "  --help, -h             Displays this help message."
    exit 0
}

# Function to process a single zip file
process_zip_file() {
    local zip_file="$1"
    local base_dest_dir="$2"

    echo "\n--- Processing: '$zip_file' ---"

    # Extract feature name and sequence number
    filename=$(basename "$zip_file")
    # Remove 'stitch_' prefix
    feature_part="${filename#stitch_}"
    # Remove '.zip' suffix
    feature_part="${feature_part%.zip}"

    local sequence_num=""
    # Check for (number) pattern at the end
    if [[ "$feature_part" =~ \(([0-9]+)\)$ ]]; then
        sequence_num="${BASH_REMATCH[1]}"
        feature_name="${feature_part%(*)}" # Remove (number) from feature name
    else
        feature_name="$feature_part"
    fi

    # Clean up feature name (e.g., remove trailing underscores or spaces)
    feature_name=$(echo "$feature_name" | sed 's/[_-]*$//')

    local final_dest_dir=""
    if [ -n "$sequence_num" ]; then
        # Find the next available sequence number if the folder already exists
        local current_sequence="$sequence_num"
        while [ -d "$base_dest_dir/$feature_name/$current_sequence" ]; do
            echo "Warning: Destination folder '$base_dest_dir/$feature_name/$current_sequence' already exists."
            read -p "Do you want to extract into this existing folder (y) or find the next available sequence (n)? (y/n) " -n 1 -r
            echo ""
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                break # Use the existing folder
            else
                current_sequence=$((current_sequence + 1))
                echo "Attempting to use sequence number: $current_sequence"
            fi
        done
        final_dest_dir="$base_dest_dir/$feature_name/$current_sequence"
    else
        final_dest_dir="$base_dest_dir/$feature_name"
    fi

    mkdir -p "$final_dest_dir"

    if [ ! -d "$final_dest_dir" ]; then
        echo "Error: Failed to create destination directory '$final_dest_dir'."
        return 1
    fi

    echo "  -> Destination: '$final_dest_dir'"
    echo "  -> Extracting '$zip_file'..."

    unzip -q "$zip_file" -d "$final_dest_dir"

    if [ $? -ne 0 ]; then
        echo "Error: Failed to extract zip file '$zip_file'."
        return 1
    fi

    echo "✅ Successfully extracted to '$final_dest_dir'."

    # Optional: Remove original zip file
    read -p "Do you want to remove the original zip file '$zip_file'? (y/n) " -n 1 -r
    echo "" # Move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm "$zip_file"
        echo "✅ Original zip file '$zip_file' removed."
    else
        echo "Original zip file '$zip_file' kept."
    fi
    return 0
}

# --- Main Logic ---

# Check for help flag
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    display_help
fi

# If a specific zip file is provided as an argument
if [ -n "$1" ]; then
    ZIP_FILE="$1"
    if [ ! -f "$ZIP_FILE" ]; then
        echo "Error: Zip file '$ZIP_FILE' not found."
        exit 1
    fi
    process_zip_file "$ZIP_FILE" "$DEFAULT_BASE_DEST_DIR"
    exit 0
fi

# If no arguments, search Downloads directory
echo "🔍 Searching for 'stitch_*.zip' files in '$DOWNLOADS_DIR'..."

found_zips=()
while IFS= read -r -d $'' file; do
    found_zips+=("$file")
done < <(find "$DOWNLOADS_DIR" -maxdepth 1 -name "stitch_*.zip" -print0)

if [ ${#found_zips[@]} -eq 0 ]; then
    echo "✅ No 'stitch_*.zip' files found in '$DOWNLOADS_DIR'."
    exit 0
fi

echo "Found ${#found_zips[@]} zip file(s) to process:"
for i in "${!found_zips[@]}"; do
    echo "  $((i+1))) ${found_zips[$i]}"
done

for zip_file_to_process in "${found_zips[@]}"; do
    read -p "Process '$zip_file_to_process'? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        process_zip_file "$zip_file_to_process" "$DEFAULT_BASE_DEST_DIR"
    else
        echo "Skipping '$zip_file_to_process'."
    fi
done

exit 0