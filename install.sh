#!/usr/bin/env bash

echo "Granting executable permission to forg"
chmod u+x forg.sh
echo "Moving files"
move ./config/* "$HOME/.local/share/" && echo "Moved config files to $HOME/.local/share/"
move ./src/file-organizer.sh "$HOME/.local/bin/" && echo "Moved script to $HOME/.local/bin/"
