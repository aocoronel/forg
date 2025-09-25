# A simple file organizer

`forg` is a file organizer, that allows to organize and move files based on filetype or any other rule, such as naming conventions.

It's a CLI tool that tries to make the process of organizing your files much easier and configured to you own liking, is it a hierarchical structure or fully based on filetype.

## Requirements

- `cmake`
- `just` (optional)

## Features

- Create the whole directory structure
- Organize files by filetype
- Organize files by naming conventions (tags)
- Preview the process
- Remove duplicate files

## Usage

```
File Organizer
Usage: forg [options] <src> <dest> <mode>
Options:
  -d, --dry       Preview actions
  -r, --rm        Remove duplicate files
  -h, --help      Show this message
  -V, --verbose   Enable verbosity
```

### Examples

**Example 1:** Remove duplicates

```bash
forg -r /home/user/Downloads /home/user/Files
```

This will move all files from `Downloads/` to `Files/`, organize them by filetype and remove any duplicates from the `Downloads/` directory.

**Example 2:** Preview

```bash
forg.sh -V /home/user/Downloads /home/user/Files
```

This will preview move for all files from `Downloads/` to `Files/`.

## Configuration

This script reads the `forg.conf` at `~/.local/share`, by default.

```conf
# Tags
tag:agreement=docs/legal/
tag:backup=backups/
tag:case=docs/legal/

# Extensions
ext:7z=archives/compressed/
ext:aa=media/audiobooks/
ext:aac=media/music/
```

By default, the auto mode is set where tags precede extensions. In this case, files starting with `agreement-myfile.docx` will be moved to `docs/legal/`. However, if the agreement tag was not set, and an extension is set it's going to be moved to the extension's configured path.

## Installation

```bash
git clone https://github.com/aocoronel/forg
cd forg && just release
cd build && sudo make install
```

### Compiling

You can either run `just build` or:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build .
cmake --build build
```

## Notes

This script has been only tested in a Linux Machine.

## License

This repository is licensed under the MIT License, a very permissive license that allows you to use, modify, copy, distribute and more.
