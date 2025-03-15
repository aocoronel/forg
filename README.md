<h1 align="center">Forg</h1>

`forg.sh` is a file organizer written in shell, that allows to organize and move files based on filetype or any other rule, such as naming conventions.

It's a CLI tool that tries to make the process of organizing your files much easier and configured to you own liking, is it a hierarchical structure or fully based on filetype.

## Requirements

- shell (bash, zsh, fish)

## Features

- Organize files by filetype
- Organize files by naming conventions
- Preview the process
- Remove duplicate files
- Create the whole directory structure

## Usage

```
Usage: $0 [options] <source-directory> <dest-directory> <method> ...
Options:
  -d, --dry       Preview actions
  -r, --rm        Remove duplicate files
  -h, --help      Show this message
```

### Examples

- Example 1: Remove duplicates

```bash
forg.sh -r /home/user/Downloads /home/user/Files ftp
```

This will move all files from Downloads/ to Files/, organize them by filetype and remove any duplicates from the Downloads/ directory.

- Example 2: Preview

```bash
forg.sh -d gallery /home/user/Downloads /home/user/Files ftp
```

This will preview move for all files from Downloads/ to Files/ and organize them by the gallery rules.

- Example 3: Multiple methods

```bash
forg.sh -dr docs /home/user/Downloads /home/user/Files ftp gallery docs
```

This will preview move for all files from Downloads/ to Files/, organize them by filetype, them by gallery rules, them by docs rules and deduplicate files.

## Configuration

This script reads the `forg.conf` which contains arrays. There is a big array called `ftp` and two extra arrays to serve as a template. One of them looks like this:

```shell
declare -A gallery=(
  [wppr]=media/wallpapers/
  [wallpaper]=media/wallpapers/
  [wall]=media/wallpapers/
  [screen]=media/screenshots/
  [screenshot]=media/screenshots/
  [scrsht]=media/screenshots/
  [meme]=media/memes/
)
```

When the method `gallery` is used, all files that start with any of those patterns are moved to the corresponding directory.

By default, only `ftp` reads the end of the files, looking for filetypes. All the other arrays will have their pattern to the beginning of the filename, for example: `wppr%-cat.png`. The **percentage symbol** declares the end of the pattern, in this case `wppr`.

There is no hardcode in the script, so the array could be called anything and infinitely modifiable. The `%` symbol can be changed to something else, by changing this line in the code:

```bash
file_tag="${filename%%\%*}"
```

Directories are generated on demand. Only the required directories are created, but if you want to see or create all configured directories, you can use the `forg-config.sh`

## Installation

Make sure to add the $HOME/.local/bin/ to your `$PATH`, as the `install.sh` sends the script there.

```
git clone https://github.com/aocoronel/forg
cd forg
sh install.sh
```

## Notes

This script has been only tested in a Linux Machine.

## License

This repository is licensed under the MIT License, a very permissive license that allows you to use, modify, copy, distribute and more.
