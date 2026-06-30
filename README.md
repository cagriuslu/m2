# m2

2D Game Engine written in C++

## Features

* Windows, macOS, Linux and web browser support
* 2D physics in single player mode (top down or side scroller modes)
* Turn based (LAN) multi-player mode without physics
* Deterministic lockstep multi-player mode with simple physics
* Multi-layer sprite-based graphics
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

### Design Philosophy

* Sensibly-optimized. Readability is as important as performance.
* Modern C++. Not over-engineered, not blatantly unsafe, somewhere in the middle.
* No opengl, directx, or metal pass-through.
* The engine is neither super generic nor super specific to a game genre. 

The repo contains some game prototypes to showcase the capabilities of the engine.

### Included Games

* RPG - Simulates a 2D Action RPG game
* CuZn - Simulates a multi-player turn based top-down board game

### Provided Tooling

* Level editor - allows you to build a level using objects and sprites
* Sprite editor - allows you to specify the details of a sprite; the physics components, origin of the graphics, etc.
* Bulk Sprite Sheet editor - allows you to specify the boundaries of each sprite in a sprite sheet

## How to build for desktop

A C++ compiler, CMake, and Ninja is required for build. All other dependencies are built on the go.
Although the game code (`game/<game>`) is separated from the engine code (`include/`, `src/`), they are build into one statically linked binary.
`game/` directory contains the game code for different game prototypes. Which game to build is chosen at configure-time.

CMakePresets are provided for the included games. macOS and Linux build:
```
cmake --preset <game>-POSIX-DEBUG
```
Windows build:
```
cmake --preset <game>-WIN-DEBUG
```
Replace `<game>` with either RPG or CuZn.

After configuring with CMake, go to the build directory and run Ninja to build the game (which includes the engine): 
```
ninja m2
```

Alternatively, run `mac_all.sh` or `win_all.sh` to build all games.

### How to build for web

The engine, dependencies, and the games can be cross-compiled to WebAssembly with Emscripten and run as a static web page in a desktop browser.
While single-player games are supported as-is, multiplayer games are only playable with local bots.

Prerequisites:

* The Emscripten SDK. Install and activate it so that `emcc` and `emcmake` are on your `PATH`.
* Native build of the game is required as they also compile the code generators that are needed during web build.

The web version can be built with:
```
emcmake cmake --preset <game>-WASM-RELEASE
cmake --build build/<game>-WASM --target m2
```
This cross-compiles the runtime dependencies and the engine for wasm and writes `m2.html`, `m2.js`, `m2.wasm`, and `m2.data` into `build/<game>-WASM`.

Alternatively, run `web_all.sh` to build all games.

To play it, serve that directory over HTTP and open `m2.html` in a desktop browser:
```
cd build/<game>-WASM
python3 -m http.server 8000
# open http://localhost:8000/m2.html
```

### Dependencies

The following dependencies are included in the repo, and built automatically by the provided CMake script:
* box2d
* freetype (required by SDL2_ttf)
* googletest (aka gtest)
* libpng
* protobuf
* SDL3
* SDL3_image
* SDL3_ttf
* sqlite3
* libz

## Who am I?

Çağrı Uslu (pronounced Cha-ry). Amsterdam based Electronics and Computer Engineer (MSc), GameDev and Photography enthusiast, working in Embedded systems since 2016.
