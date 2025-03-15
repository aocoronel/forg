#!/usr/bin/env bash

src_directory="$1"
output_directory="$2"
config_ext="$HOME/.local/share/organizer-extensions.conf"
config_dir="$HOME/.local/share/organizer-directories.conf"
logs="$HOME/.cache/organizer.logs"
time=$(date +%F)

if [ -z "$src_directory" ] || [ -z "$output_directory" ]; then
  echo "Usage: $0 [source-directory] [dest-directory]"
  exit 1
fi

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

if [ ! -f "$config_ext" ] || [ ! -f "$config_dir" ]; then
  echo "Error: extensions.conf not found.  Exiting." >&2
  exit 1
else
  source "$HOME/.local/share/organizer-extensions.conf"
  source "$HOME/.local/share/organizer-directories.conf"
  val=$?
  if [ "$val" -ne 0 ]; then
    echo "Failed to source configuration files"
    exit 1
  fi
fi

# Check if the associative array 'extension_map' is defined
if ! declare -p extension_map >/dev/null; then
  echo "Error: Associative array 'extension_map' not defined in extension_map.conf" >&2
  exit 1
fi

function directory_check() {
  for dir in "${directories[@]}"; do
    [[ -d "$dir" ]] || mkdir -p "$dir"
  done
}

function broad() {

  find "$src_directory" -type f -print0 | while IFS= read -r -d $'\0' file; do
    filename=$(basename "$file")
    extension="${filename##*.}" # Get extension
    destination_dir="${extension_map[$extension]}"

    # Handle special cases based on filename
    if [[ "$filename" == wppr-* || "$filename" == wallpaper-* || "$filename" == wall-* ]]; then
      destination_dir="$output_directory/media/wallpapers/"
    elif [[ "$filename" == screen-* || "$filename" == screenshot-* || "$filename" == scrsht-* ]]; then
      destination_dir="$output_directory/media/screenshots/"
    elif [[ "$filename" == meme-* ]]; then
      destination_dir="$output_directory/media/memes/"
    elif [[ "$filename" == legal-* || "$filename" == contract-* || "$filename" == agreement-* || "$filename" == law-* || "$filename" == case-* ]]; then
      destination_dir="$output_directory/docs/legal/"
    elif [[ "$filename" == backup-* || "$filename" == *.bak || "$filename" == *.bkp || "$filename" == *.old || "$filename" == *.sav ]]; then
      destination_dir="$output_directory/backups/"
    elif [[ "$filename" == project-* || "$filename" == *.xcf || "$filename" == *.workspace || "$filename" == *.sln || "$filename" == *.xcodeproj ]]; then
      destination_dir="$output_directory/projects/"
    elif [[ "$filename" == template-* || "$filename" == tpt-* ]]; then
      destination_dir="$output_directory/templates/"
    fi

    if [ -z "$destination_dir" ]; then
      destination_dir="$output_directory/others/"
    fi

    # Check if the file already exists in the destination
    if [[ -e "$destination_dir/$filename" ]]; then
      continue
    fi

    # Move the file
    if mv "$file" "$destination_dir"; then
      echo "$time Moved '$filename' to '$destination_dir'" >>"$logs"
      echo "Moved '$filename' to '$destination_dir'"
    else
      echo "$time Error moving '$filename' to '$destination_dir'" >>"$logs"
      echo "Error moving '$filename' to '$destination_dir'" >&2
    fi
  done
  echo "Complete!"
}

function main() {
  echo "Choose an organize method:"
  echo "broad - Apply changes based on filetype"
  echo "note - Apply note-taking rules"
  echo "gallery - Apply image rules"
  echo "music - Apply music rules"
  read method
  case "$method" in
  broad)
    broad
    ;;
  esac
}

directory_check
main
