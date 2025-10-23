#!/usr/bin/env bash
# Identify files named like "name (n).ext" and remove them if they duplicate "name.ext".

set -euo pipefail

usage() {
  cat <<'EOF'
Usage: remove_duplicate_assets.sh [--root DIR] [--apply] [--case-insensitive]

Search DIR (default: ./assets) for files whose names end with " (number)" before
the extension, compare them with their unnumbered counterpart, and delete the
duplicate copies when --apply is supplied. Without --apply the script performs a
dry run and only reports the files it would delete.

Options:
  --root DIR            Directory to scan (default: ./assets)
  --apply               Delete duplicates instead of reporting them
  --case-insensitive    Match originals ignoring case differences
  -h, --help            Show this help
EOF
}

ROOT="assets"
APPLY=false
IGNORE_CASE=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --root)
      [[ $# -ge 2 ]] || { usage >&2; exit 1; }
      ROOT="$2"
      shift 2
      ;;
    --apply)
      APPLY=true
      shift
      ;;
    --case-insensitive)
      IGNORE_CASE=true
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      printf "error: unknown option '%s'\n" "$1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if command -v realpath >/dev/null 2>&1; then
  ROOT_PATH=$(realpath "$ROOT")
else
  ROOT_PATH=$ROOT
fi

if [[ ! -d "$ROOT_PATH" ]]; then
  printf "error: '%s' is not a directory\n" "$ROOT_PATH" >&2
  exit 2
fi

file_size() {
  local path=$1 size
  if size=$(stat -c %s "$path" 2>/dev/null); then
    printf '%s\n' "$size"
    return 0
  fi
  if size=$(stat -f %z "$path" 2>/dev/null); then
    printf '%s\n' "$size"
    return 0
  fi
  return 1
}

find_case_insensitive() {
  local dir=$1
  local target=$2
  local target_lower=${target,,}
  local match=""
  local entry

  shopt -s nullglob
  for entry in "$dir"/*; do
    local name=${entry##*/}
    if [[ ${name,,} == "$target_lower" ]]; then
      match=$entry
      break
    fi
  done
  shopt -u nullglob

  printf '%s\n' "$match"
}

confirmed=0
removed=0
skipped=0

while IFS= read -r -d '' duplicate; do
  name=$(basename "$duplicate")
  dir=$(dirname "$duplicate")

  if [[ $name =~ ^(.*)\ \(([0-9]+)\)(\..*)$ ]]; then
    base=${BASH_REMATCH[1]}
    ext=${BASH_REMATCH[3]}
  elif [[ $name =~ ^(.*)\ \(([0-9]+)\)$ ]]; then
    base=${BASH_REMATCH[1]}
    ext=""
  else
    continue
  fi

  original_candidate="${base}${ext}"

  if [[ "$IGNORE_CASE" == true ]]; then
    original=$(find_case_insensitive "$dir" "$original_candidate")
  else
    original="$dir/$original_candidate"
    [[ -f "$original" ]] || original=""
  fi

  if [[ -z "$original" ]]; then
    printf "SKIP (original missing): %s\n" "$duplicate"
    skipped=$((skipped + 1))
    continue
  fi

  size_duplicate=$(file_size "$duplicate") || size_duplicate=""
  size_original=$(file_size "$original") || size_original=""
  if [[ -z "$size_duplicate" || -z "$size_original" || "$size_duplicate" != "$size_original" ]]; then
    printf "SKIP (size mismatch): %s vs %s\n" "$duplicate" "$original"
    skipped=$((skipped + 1))
    continue
  fi

  if cmp -s -- "$duplicate" "$original"; then
    confirmed=$((confirmed + 1))
    if [[ "$APPLY" == true ]]; then
      if rm -- "$duplicate"; then
        printf "DELETE: %s (kept %s)\n" "$duplicate" "$original"
        removed=$((removed + 1))
      else
        printf "error: failed to delete '%s'\n" "$duplicate" >&2
      fi
    else
      printf "DELETE (dry run): %s (original %s)\n" "$duplicate" "$original"
    fi
  else
    printf "SKIP (content differs): %s vs %s\n" "$duplicate" "$original"
    skipped=$((skipped + 1))
  fi
done < <(find "$ROOT_PATH" -type f -name '* (*' -print0)

if [[ "$APPLY" == true ]]; then
  printf "\nSummary: %d duplicate(s) removed, %d skipped.\n" "$removed" "$skipped"
else
  printf "\nSummary: %d duplicate(s) identified, %d skipped.\n" "$confirmed" "$skipped"
fi

if [[ $confirmed -gt 0 && "$APPLY" == false ]]; then
  printf "Run again with --apply to remove the duplicates.\n"
fi
