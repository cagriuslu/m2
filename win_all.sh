#!/usr/bin/env bash
set -e

# Windows counterpart of build_all.sh: configures & builds every game in Debug.
# Run from Git Bash.

# Use the CMake bundled inside the CLion installation. The path carries no version
# number, so it stays valid across CLion updates. Override with:
#   CMAKE=/c/path/to/cmake.exe ./win_all.sh
CMAKE="${CMAKE:-/c/Users/Uslu/AppData/Local/Programs/CLion/bin/cmake/win/x64/bin/cmake.exe}"
if [ ! -x "$CMAKE" ]; then
    echo "error: CLion's bundled cmake.exe not found at '$CMAKE'." >&2
    echo "       Set the CMAKE environment variable to the correct path and retry." >&2
    exit 1
fi

# Run from the repository root (this script's directory) so the preset names and the
# build/<GAME>-DEBUG paths resolve correctly regardless of the caller's directory.
cd "$(dirname "$0")"

echo "Configuring RPG"
"$CMAKE" --preset RPG-WIN-DEBUG
echo "Configuring CuZn"
"$CMAKE" --preset CuZn-WIN-DEBUG
echo "Configuring PINBALL"
"$CMAKE" --preset PINBALL-WIN-DEBUG
echo "Configuring WAR"
"$CMAKE" --preset WAR-WIN-DEBUG
echo "Configuring QNT"
"$CMAKE" --preset QNT-WIN-DEBUG

# The *-WIN-DEBUG presets use the default generator (Visual Studio 17 2022), which is
# multi-config, so the Debug configuration is selected here at build time via --config.
echo "Building RPG"
"$CMAKE" --build build/RPG-DEBUG --target m2 --config Debug -j 8
echo "Building CuZn"
"$CMAKE" --build build/CuZn-DEBUG --target m2 --config Debug -j 8
echo "Building PINBALL"
"$CMAKE" --build build/PINBALL-DEBUG --target m2 --config Debug -j 8
echo "Building WAR"
"$CMAKE" --build build/WAR-DEBUG --target m2 --config Debug -j 8
echo "Building QNT"
"$CMAKE" --build build/QNT-DEBUG --target m2 --config Debug -j 8
