#!/bin/bash
set -e

gcc -g -fsanitize=address --std=c99   -c src/*/*.c src/*.c   -I/usr/include/SDL2
g++ -g -fsanitize=address --std=c++11 -c src/*.cpp -I/usr/local/include/box2d
g++ -g -fsanitize=address --std=c++11 -o bin/cgame *.o -lbox2d -lSDL2_image -lSDL2_ttf -lSDL2 -lm
rm *.o
