# List (default)
list:
  just --list

# Build (debug)
build:
  zig build

# Build (release)
release:
  zig build -Doptimize=ReleaseFast

# Clean files
clean:
  rm -rf .zig-cache/
  rm -rf zig-out/

# Generate test sample to run forg << test-clean
test: test-clean
  cp -r test/ zig-out/bin/src
  mkdir zig-out/bin/dst/

# Removes test sample
test-clean:
  rm -rf zig-out/bin/src/
  rm -rf zig-out/bin/dst/
