#!/bin/bash
# ══════════════════════════════════════════════════════════════
# LinuxUtilities docs — build script
# Combines _partials/header + _pages/*.html + _partials/footer
# into build/*.html.  Also copies css/, js/, assets/ into build/.
#
# Output:  docs/build/   (gitignored)
# ══════════════════════════════════════════════════════════════
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARTIALS_DIR="$SCRIPT_DIR/_partials"
PAGES_DIR="$SCRIPT_DIR/_pages"
BUILD_DIR="$SCRIPT_DIR/build"

CURRENT_YEAR=$(date +%Y)
CURRENT_DATE=$(date +%Y-%m-%d)

echo "Building LinuxUtilities documentation..."
echo "  Date: $CURRENT_DATE"
echo "  Output: $BUILD_DIR"

# ── Clean + prepare build dir ─────────────────────────────────
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# ── Copy static assets ────────────────────────────────────────
echo "  Copying css/, js/, assets/..."
cp -r "$SCRIPT_DIR/css"    "$BUILD_DIR/css"
cp -r "$SCRIPT_DIR/js"     "$BUILD_DIR/js"
cp -r "$SCRIPT_DIR/assets" "$BUILD_DIR/assets"

# ── Process each page ─────────────────────────────────────────
for page in "$PAGES_DIR"/*.html; do
    [ -f "$page" ] || continue
    filename=$(basename "$page")

    # Skip partials / templates
    [[ "$filename" == _* ]] && { echo "  Skipping template: $filename"; continue; }

    echo "  Building: $filename"

    # Extract metadata from HTML comments
    page_title=$(grep -oP '<!--\s*TITLE:\s*\K[^-]+' "$page" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//' || echo "Documentation")
    nav_active=$(grep -oP '<!--\s*NAV:\s*\K\w+' "$page" || echo "")

    # Read partials
    header=$(cat "$PARTIALS_DIR/header.html")
    footer=$(cat "$PARTIALS_DIR/footer.html")

    # Read page content (strip metadata comments)
    content=$(sed '/^<!--.*-->$/d' "$page")

    # Replace template variables in header
    header="${header//\{\{PAGE_TITLE\}\}/$page_title}"

    # Clear all nav active states first
    header="${header//\{\{NAV_INDEX\}\}/}"
    header="${header//\{\{NAV_SETUP\}\}/}"
    header="${header//\{\{NAV_SHORTCUTS\}\}/}"
    header="${header//\{\{NAV_TROUBLESHOOTING\}\}/}"

    # Set active nav
    if [ -n "$nav_active" ]; then
        header="${header//\{\{NAV_${nav_active^^}\}\}/active}"
    fi

    # Replace date variables
    footer="${footer//\{\{YEAR\}\}/$CURRENT_YEAR}"
    footer="${footer//\{\{CURRENT_DATE\}\}/$CURRENT_DATE}"
    content="${content//\{\{YEAR\}\}/$CURRENT_YEAR}"
    content="${content//\{\{CURRENT_DATE\}\}/$CURRENT_DATE}"

    # Combine header + content + footer → build/
    output_file="$BUILD_DIR/$filename"
    echo "$header"  > "$output_file"
    echo "$content" >> "$output_file"
    echo "$footer"  >> "$output_file"
done

echo ""
echo "Build complete!"
echo "  Files in build/:"
ls -1 "$BUILD_DIR"/*.html 2>/dev/null | while read -r f; do
    echo "    $(basename "$f")  ($(wc -c < "$f") bytes)"
done
echo ""
echo "  To preview locally:"
echo "    cd $BUILD_DIR && python3 -m http.server 8080"
