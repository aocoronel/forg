#!/usr/bin/env bash

# Use `set -euo pipefail` to catch errors early and prevent unexpected behavior.
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error.
# -o pipefail: If a command in a pipeline fails, the pipeline's exit status is that of the failing command.
set -euo pipefail

src_directory="$1"
output_directory="$2"
config_ext="$HOME/.local/share/organizer-extensions.conf"  # Sources filenames and extensions
config_dir="$HOME/.local/share/organizer-directories.conf" # Sources directories
logs="$HOME/.cache/organizer.logs"
KILLDUPLICATE=false
DRYMODE=false

readonly config_ext
readonly config_dir

function help() {
  cat <<EOF
File Organizer
Usage: $0 [source-directory] [dest-directory] [options]
Available Options:
- dry                - Give a preview of the command
- rmdup              - Remove duplicate files
- help               - Display this message and exits
EOF
}

function directory_check() {
  # directories is sourced from config_dir
  for dest_dir in "${directories[@]}"; do
    [[ -d "$dest_dir" ]] || mkdir -p "$output_directory/$dest_dir"
  done
}

move() {
  find "$src_directory" -type f -print0 | while IFS= read -r -d $'\0' file; do
    filename=$(basename "$file")
    extension="${filename##*.}"
    destination_subpath=""

    case "$method" in
    broad)
      destination_subpath="${extension_map[$extension]}"
      ;;
    gallery)
      for pattern in "${!gallery_map[@]}"; do
        if [[ "$filename" == $pattern ]]; then
          destination_subpath="${gallery_map[$pattern]}"
          break
        fi
      done
      ;;
    esac

    # Ensure destination_dir is valid
    if [[ -n "$destination_subpath" ]]; then
      destination_dir="$output_directory/$destination_subpath"
    else
      destination_dir="$output_directory/others"
    fi

    # Check if the file already exists in the destination
    if [[ -e "$destination_dir/$filename" ]]; then
      echo "File '$filename' already exists in '$destination_dir'! Possible duplicate?"
      if [ "$KILLDUPLICATE" = "true" ]; then
        rm "$src_directory/$filename"
      fi
      continue
    fi

    # Move the file
    if [ "$DRYMODE" = "true" ]; then
      echo "Move file: $file ==> $destination_dir"
    else
      if mv -n "$file" "$destination_dir" &>"$logs"; then
        echo "Moved '$filename' to '$destination_dir'"
      else
        echo "Error moving '$filename' to '$destination_dir'.  Check '$logs' for details." >&2
      fi
    fi
  done
  echo "Complete!"
}

function main() {
  echo "Choose an organize method:"
  echo "broad - Apply changes based on filetype"
  echo "gallery - Apply image rules"
  echo "music - Apply music rules"
  echo ""
  read method

  if [ "$DRYMODE" = "true" ]; then
    echo "Dry Mode:"
  fi

  move
}

# Check for optional arguments
case "$3" in
dry)
  DRYMODE=true
  ;;
rmdup)
  KILLDUPLICATE=true
  ;;
help)
  help
  exit 0
  ;;
esac

# Check required arguments
if [ -z "$src_directory" ] || [ -z "$output_directory" ]; then
  echo "Error: Missing required arguments"
  help
  exit 1
fi

# Same directory warning
if [ "$src_directory" == "$output_directory" ]; then
  echo "Be careful! You are running this script in the same directory!"
  echo "If this directory is overpopulated, it may take longer"
  select choice_continue in "Yes" "No"; do
    case $choice_continue in
    Yes) break ;;
    No) exit 0 ;;
    esac
  done
fi

# Check for config files and source them
if [[ ! -f "$config_ext" ]] || [[ ! -f "$config_dir" ]]; then
  echo "Error: extensions.conf or directories.conf not found.  Exiting." >&2
  exit 1
else
  # shellcheck disable=SC1090
  source "$config_ext"
  # shellcheck disable=SC1090
  source "$config_dir"
  val=$?
  if [ "$val" -ne 0 ]; then
    echo "Failed to source configuration files"
    exit 1
  fi
fi

directory_check
main
