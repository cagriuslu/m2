#!/bin/bash

clang --std=c99 -o cgame src/*.c -I/usr/local/include/SDL2 -lSDL2_image -lSDL2_ttf -lSDL2 -lm