# m2

2D Game Engine written in C++

## Features

* Windows, macOS, and Linux support
* 2D physics in single player mode (top down or side scroller modes)
* Turn based (LAN) multi-player mode without physics (Physics will be added in future releases)
* Multi-layer sprite-based graphics (4 background, 1 foreground layers)
* Optional dynamic lightning
* Optional, 3D-like sprite "figurine" graphics (ex. Paper Mario)
* Stereo audio subsystem with sound synthesizer
* Fast arena allocator for game objects and their components
* A* Pathfinder
* Declarative UI subsystem
* Protobuf-based game resources and enums
* Level editor, sprite sheet editor, bulk sprite sheet editor

## Overview

m2 is a game engine written in C++.
It is designed for desktop computers, to develop games that use a mouse and a keyboard.

### Project Status

Although the testing isn't good enough for the engine to be considered production-ready,
I try to fix every instability and bug I encounter.

### Design Philosophy

* Sensibly-optimized. Readability is as important as performance.
* Sensibly-modern C++. Not over-engineered, not blatantly unsafe, somewhere in the middle.
* No opengl, directx, or metal pass-through.
* The engine is neither super generic nor super specific to a game genre. 

The repo contains some game prototypes to showcase the capabilities of the engine.

### Included Games

* RPG - Simulates a 2D Action RPG game
* MINE - Simulates a side-scroller game
* CuZn - Simulates a multi-player turn based top-down board game

### Provided Tooling

* Level editor - allows you to build a level using objects and sprites
* Sprite editor - allows you to specify the details of a sprite; the physics components, origin of the graphics, etc.
* Bulk Sprite Sheet editor - allows you to specify the boundaries of each sprite in a sprite sheet

## How to build

A C++ compiler, CMake, and Ninja is required for build. All other dependencies are built on the go.
Although the game code (`game/<game-name>`) is separated from the engine code (`include/`, `src/`), they are build into one statically linked binary.
`game/` directory contains the game code for different game prototypes. Which game to build is chosen at configure-time.

CMakePresets are provided for the included games:
```
macOS and Linux Debug build:
cmake -DCMAKE_BUILD_TYPE=Debug --preset <game-name>-POSIX-DEBUG
Windows Debug build:
cmake -DCMAKE_BUILD_TYPE=Debug --preset <game-name>-WIN-DEBUG
```
Replace `<game-name>` with either RPG, MINE, or CuZn. I haven't created CMake presets for release builds, but that should be fairly easy.

After configuring with CMake, go to the build directory and run Ninja to build the game (which includes the engine): 
```
ninja m2
```

### Dependencies

The following dependencies are included in the repo, and built automatically by the provided CMake script:
* box2d
* mt::expected
* freetype (required by SDL2_ttf)
* googletest (aka gtest)
* libpng (required by SDL2_image)
* protobuf
* SDL2
* SDL2_image
* SDL2_ttf
* libz (required by libpng)

## Who am I?

Çağrı Uslu (pronounced Cha-ry). Amsterdam based Electronics and Computer Engineer (MSc), GameDev and Photography enthusiast,
working in Embedded systems since 2016.
