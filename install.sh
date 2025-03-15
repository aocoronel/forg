#!/usr/bin/env bash

echo "Granting executable permission to forg"
chmod u+x forg
echo "Moving files"
mv ./config/forg.conf "$HOME/.local/share/" && echo "Moved config files to $HOME/.local/share/"
mv ./src/forg "$HOME/.local/bin/" && echo "Moved script to $HOME/.local/bin/"
