# Overview

- This repo contains a game engine (written in C++, configured with CMake) and some games written for the engine.
- ./game directory contains the code for several games. Everywhere else contains the engine code.
- The game to build is chosen during configuration.
- The engine and the chosen game are compiled into a statically linked binary.
- All games must be maintained.

## Configure

```sh
cmake --preset <GAME>-POSIX-DEBUG
```
where <GAME> is the name of the directory of a game. Build directory will be `build/<GAME>-DEBUG`.

## Build

```sh
cd build/<GAME>-DEBUG && ninja m2
```
or
```sh
./build_all.sh
```
to configure & build all games all at once.

## Architecture Overview

For lifecycle hooks, and UI blueprints, the game must provide a class called `m2g::Proxy`, which must be derived from
`m2::Proxy`. During initialization, an instance of `m2g::Proxy` will be created as a part of the `m2::Game` singleton.

- Proxy instance can be accessed via `M2G_PROXY` macro.
- `m2::Game` singleton instance can be accessed via `M2_GAME` macro. It owns the `Level`, resources (sprites, blueprints), multiplayer state, and more.
- The currently running level (`m2::Level`) can be accessed via `M2_LEVEL` macro. It owns all runtime objects and component pools for presently loaded level.

### Component System

- `m2::Object` represents an interactive game object. It contains the IDs of the components belonging to that object.
- Components are stored in `Pool`s, owned by the `Level`. The main loop iterates on these component pools rather than objects.

### Component types

- `Character` — game logic and state. Must implements `CharacterImpl` concept.
- `Graphic` — sprite rendering.
- `Physique` — Box2D rigid body.
- `SoundEmitter` — positional audio.

User-defined data not iterated by the main loop goes in `Object::impl`.

## Multiplayer

Two multiplayer modes:

- Turn-based (`src/multiplayer/turnbased/`): server/client architecture over TCP. Character state is serialized via `Store`/`Load` on each `CharacterImpl`.
- Lockstep (`src/multiplayer/lockstep/`): deterministic lockstep simulation. Enabled by setting `GAME_IS_DETERMINISTIC ON` in the game's CMakeLists. Lockstep games use `pb-deterministic/` instead of `pb-undeterministic/`, and `m2::Exact` instead of floating point types.

## Platform Layer (`platform/`)

Headers in `include/` that require platform-specific implementations are provided under `platform/macos/`, `platform/linux/`, `platform/windows/`, and `platform/posix/` (shared by macOS and Linux).

## Coding Standards

- Never allow silent failures. Prefer proper error handling or `throw`.
- In paired header/source files, the order of function and method definitions in the source must match the
  order of their declarations in the header. The header is the reference; flatten its declarations to a single
  top-to-bottom sequence, ignoring access-specifiers and nested-class boundaries. Blank lines between consecutive
  definitions in the source must mirror the header: where the header has no blank line between two declarations
  (comment lines are transparent and do not count as spacing), the source must have no blank line between their
  definitions; where the header has one or more blank lines, the source must have exactly one.
- The Rational type can easily grow out of bounds. Thus, any calculation utilizing Rational should be eagerly done while
  booting the game/engine.
