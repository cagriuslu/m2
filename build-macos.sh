#!/bin/bash

clang -o cgame src/main.c -I/usr/local/include/SDL2 -lSDL2_image -lSDL2_ttf -lSDL2