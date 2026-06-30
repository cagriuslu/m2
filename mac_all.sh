#!/usr/bin/env bash
set -eo pipefail

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
cmake --build /Users/uslu/m2/build/RPG-DEBUG --target m2 -j 8
echo "Building CuZn"
cmake --build /Users/uslu/m2/build/CuZn-DEBUG --target m2 -j 8
echo "Building PINBALL"
cmake --build /Users/uslu/m2/build/PINBALL-DEBUG --target m2 -j 8
echo "Building WAR"
cmake --build /Users/uslu/m2/build/WAR-DEBUG --target m2 -j 8
echo "Building QNT"
cmake --build /Users/uslu/m2/build/QNT-DEBUG --target m2 -j 8
