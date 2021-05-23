#!/bin/bash
set -e

clang   --std=c99   -c src/*/*.c src/*.c   -I/usr/local/include/SDL2
clang++ --std=c++11 -c src/*.cpp -I/usr/local/include/box2d
clang++ --std=c++11 -o bin/cgame *.o -lbox2d -lSDL2_image -lSDL2_ttf -lSDL2 -lm
rm *.o
