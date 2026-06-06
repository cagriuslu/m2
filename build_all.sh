#!/usr/bin/env bash
set -e

echo "Configuring RPG"
cmake -DCMAKE_BUILD_TYPE=Debug --preset RPG-POSIX-DEBUG
echo "Configuring CuZn"
cmake -DCMAKE_BUILD_TYPE=Debug --preset CuZn-POSIX-DEBUG
echo "Configuring PINBALL"
cmake -DCMAKE_BUILD_TYPE=Debug --preset PINBALL-POSIX-DEBUG
echo "Configuring WAR"
cmake -DCMAKE_BUILD_TYPE=Debug --preset WAR-POSIX-DEBUG
echo "Configuring QNT"
cmake -DCMAKE_BUILD_TYPE=Debug --preset QNT-POSIX-DEBUG

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
