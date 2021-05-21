#!/bin/bash

clang++ --std=c++11 -o bin/Box2DWrapper.o -c src/Box2DWrapper.cpp
clang --std=c99 -o bin/cgame src/*.c bin/Box2DWrapper.o -I/usr/local/include/SDL2 -lSDL2_image -lSDL2_ttf -lSDL2 -lm