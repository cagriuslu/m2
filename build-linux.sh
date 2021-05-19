#!/bin/bash

gcc --std=c99 -o cgame src/Camera.c src/Player.c src/Object.c src/Array.c src/main.c -I/usr/include/SDL2 -lSDL2_image -lSDL2_ttf -lSDL2