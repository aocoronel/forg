#!/usr/bin/env bash

config="$HOME/.local/share/forg.conf" # Sources directories
output_directory="$2"
mode=$1

if [ -z "$1" ]; then
  echo "Usage: $0 list"
  echo "Usage: $0 create [directory]"
fi

if [[ ! -f "$config" ]]; then
  echo "Error: extensions.conf or directories.conf not found.  Exiting." >&2
  exit 1
else
  # shellcheck disable=SC1090
  source "$config"
  val=$?
  if [ "$val" -ne 0 ]; then
    echo "Failed to source configuration files"
    exit 1
  fi
fi

mapfile -t directories < <(awk -F'[])=]' '/\[[^]]+\]=/ {print $NF}' "$config" | sort -u)

function main() {
  if [ "$mode" = "create" ]; then
    for dest_dir in "${directories[@]}"; do
      [[ -d "$dest_dir" ]] || mkdir -p "$output_directory/$dest_dir"
    done
  elif [ "$mode" = "list" ]; then
    for dir in "${directories[@]}"; do
      echo "$dir"
    done
  fi
}

main
