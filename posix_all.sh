#!/usr/bin/env bash
set -eo pipefail

# Host OS name (Darwin/Linux). Mirrors the ${hostSystemName} macro used in the
# POSIX presets' binaryDir, so the build paths below match what CMake configured.
HOST="$(uname -s)"

echo "Configuring RPG"
cmake --preset RPG-POSIX-DEBUG
echo "Configuring CuZn"
cmake --preset CuZn-POSIX-DEBUG
echo "Configuring PINBALL"
cmake --preset PINBALL-POSIX-DEBUG
echo "Configuring WAR"
cmake --preset WAR-POSIX-DEBUG
echo "Configuring QNT"
cmake --preset QNT-POSIX-DEBUG

echo "Building RPG"
cmake --build "build/RPG-$HOST-DEBUG" --target m2 -j 8
echo "Building CuZn"
cmake --build "build/CuZn-$HOST-DEBUG" --target m2 -j 8
echo "Building PINBALL"
cmake --build "build/PINBALL-$HOST-DEBUG" --target m2 -j 8
echo "Building WAR"
cmake --build "build/WAR-$HOST-DEBUG" --target m2 -j 8
echo "Building QNT"
cmake --build "build/QNT-$HOST-DEBUG" --target m2 -j 8
