#!/usr/bin/env bash
set -eo pipefail

echo "Configuring RPG"
emcmake cmake --preset RPG-WASM-RELEASE
echo "Configuring CuZn"
emcmake cmake --preset CuZn-WASM-RELEASE
echo "Configuring QNT"
emcmake cmake --preset QNT-WASM-RELEASE

echo "Building RPG"
cmake --build build/RPG-WASM --target m2 -j 8
echo "Building CuZn"
cmake --build build/CuZn-WASM --target m2 -j 8
echo "Building QNT"
cmake --build build/QNT-WASM --target m2 -j 8
