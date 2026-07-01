---
name: add-deterministic-object
description: Add a new object type to a deterministic game in this engine. Use when the user wants to add, create, or define a new object type / game object / entity in one of the deterministic games (those with GAME_IS_DETERMINISTIC ON, e.g. WAR, QNT).
---

# Add a new object type to a deterministic game

This skill defines a **new object type**: its protobuf enum value, its storage/character
model, its object class with a `Create(...)` factory, and its registration. It does **not**
instantiate the object anywhere — that is the game author's job afterwards.

The objects in `game/WAR` and `game/QNT` are the canonical references. Read the cited example
files instead of relying on this document for exact syntax — they are always current.

## Step 0 — Auto-detect before asking

Do this work yourself; do not ask the user about it.

1. **Find the deterministic games.** Grep for `GAME_IS_DETERMINISTIC ON` in
   `game/*/CMakeLists.txt`. Only those games are valid targets (currently WAR, QNT). 
   Non-deterministic games (CuZn, RPG, PINBALL) are out of scope — if the
   user names one, stop and tell them this skill is for deterministic games only.
2. **Read one existing object of the same storage model** in the target game (or, if none,
   in the other deterministic game) and mirror its structure, includes, and idioms.

## Step 1 — Gather the decisions (AskUserQuestion)

Use `AskUserQuestion`. Skip any question you can already answer from Step 0 or from the user's
request. Recommend an answer for each.

1. **Target game** — which deterministic game (from the auto-detected list).
2. **Object name & enum value** — the class name (e.g. `Particle`) and the
   `ObjectType` enum value (e.g. `PARTICLE`).
3. **Storage model:**
   - **ReflectiveCharacter** — deterministic state expressed as reflection fields, auto-hashed
     and serialized. Best for tick-evolving state. Examples: QNT `Particle`, WAR `Transporter`.
   - **Non-character — HeapObjectImpl** — heap-allocated data, no main-loop character. Use for
     non-default-constructible or rarely-accessed data. Example: WAR/QNT `World`.
   - **Non-character — PoolObjectImpl** — data stored in a `m2::Pool` owned by the proxy's
     `LevelState`. Examples: WAR `Highway`, `Street`.
4. **Components** — does it need a `Physique`? Is its position fixed or moving? A `Graphic`?
   A `SoundEmitter` (rare)?
5. **Graphics source** (only if it has a Graphic) — procedural `onDraw` drawing, or a sprite
   (from a `SpriteType`, a card's sprite, or a resource LUT)?
6. **Per-tick logic** — does it need an `OnUpdate(m2::Stopwatch::Duration)` that runs every
   tick? Briefly, what does it do?
7. **Parent/owner object** — is it owned by another object (passed as the `parentId` to
   `CreateObject`, e.g. Factory→Settlement, Settlement→HumanPlayer)?

## Step 2 — Implement (common steps)

All paths are under `game/<GAME>/`. CMake globs sources, so new files are picked up — but a
re-configure is required for the glob to re-run (handled in Step 3).

### 2a. Declare the object type in the proto enum

Add `<NAME> = <next free number>;` to the `ObjectType` enum in `pb/m2g_ObjectType.proto`.
Never reuse or reorder existing numbers.

### 2b. Define the character base header (skip for non-character objects)

Create `include/m2g/characters/<Name>Character.h` defining `<Name>CharacterBase`:

- **ReflectiveCharacter** — model on `QNT/include/m2g/characters/ParticleCharacter.h` (simple)
  or `WAR/include/m2g/characters/TransporterCharacter.h` (Struct/Variant/Deque fields). Use
  `m2::Exact`/`Int64` field types — never `float`/`double`. End with
  `using <Name>CharacterBase = m2::ReflectiveCharacter<<Name>State>;`.

### 2c. Write the object class (`include/m2g/objects/<Name>.h` + `src/objects/<Name>.cc`)

Use `m2g::object::` as the namespace. Branch by storage model:

- **Character object** — `class <Name> final : public <Name>CharacterBase`. Constructor
  `explicit <Name>(m2::Object&, ...)` forwards `object.GetId()` to the base and adds
  components. End the class with `static_assert(m2::CharacterImpl<<Name>>);`. Add
  `void OnUpdate(m2::Stopwatch::Duration);` only if Step 1 said so. Reference: QNT `Particle`,
  WAR `Factory`.
- **HeapObjectImpl** — `class <Name> final : public m2::HeapObjectImpl`, private constructor
  taking `m2::Object&`, deleted copy ctor/assignment. Reference: WAR/QNT `World`. (A heap
  object may *also* add a separate character — see QNT `World` + `WorldSpawner`.)
- **PoolObjectImpl** — needs an owning `m2::Pool<<Name>>` in the proxy `LevelState`; in the
  factory do `objIt->impl.emplace<m2::PoolObjectImpl>(pool, poolIt.GetId());`. Reference:
  WAR `Highway`, `Street`.

**`Create(...)` factory** (the common shape for a character object):

```cpp
<Name>& <Name>::Create(/* params, incl. parent id & position if any */) {
    const auto objIt = m2::CreateObject(m2g::pb::<NAME> /*, parentId */);
    auto& chr = M2_LEVEL.AddCharacterToObject<m2g::ProxyEx::<Name>CharacterStorageIndex>(*objIt, *objIt /*, ctor args */);
    auto& phy = objIt->AddPhysique(position);                  // if it has a Physique
    auto& gfx = objIt->AddGraphic(<layer> /*, sprite */, m2::VecF{position}); // if it has a Graphic
    return chr;
}
```

For HeapObjectImpl the factory instead does
`objIt->impl = std::unique_ptr<<Name>>{new <Name>{*objIt}};` and returns the
`dynamic_cast<<Name>&>(...)` (see `World::Create`).

**Components:**
- **Physique:** `objIt->AddPhysique(position)` (position is a `m2::VecE`). Deterministic games
  allow **only StaticBody** — there is no physics-driven velocity. Move an object by calling
  `phy.SetPosition(...)` from a `postStep`/`preStep` callback using `m2::Exact`/`m2::VecE`
  arithmetic. Reference: QNT `Particle` (`postStep` moves by a per-tick rate).
- **Graphic:** `AddGraphic(layer)` for procedural, `AddGraphic(layer, spriteType, position)`
  for a sprite. Layers are e.g. `m2g::pb::UPRIGHT_GRAPHICS_DEFAULT_LAYER` (sprites/upright)
  or `m2g::pb::FLAT_GRAPHICS_DEFAULT_LAYER` (ground/procedural).
  - *Procedural:* set `gfx.onDraw = [](m2::Graphic& g){ ... };` using `m2::Graphic::ColorRect`,
    `DrawLine`, `DrawCross`, etc. References: QNT `Particle`/`World`, WAR `World`.
  - *Sprite:* pass the `SpriteType` (or a card's `GameSprite()`, or a resource LUT lookup) to
    `AddGraphic`. References: WAR `Factory`, `Settlement`.

### 2d. Register the character in `include/m2g/ProxyEx.h` (skip for pure non-character objects)

Add the registered type to the `CharacterVariants` tuple and add its storage index:

```cpp
static constexpr auto <Name>CharacterStorageIndex = m2::GetIndexInTuple<<RegisteredType>, CharacterVariants>::value;
```

The registered type is the **object class** for Character-derived objects (e.g. `Particle`,
`Factory`), or the **character base alias** for Pool/Heap objects that add a character
separately (e.g. WAR `HighwayCharacterBase`).

## Determinism rules (apply throughout)

- Use `m2::Exact` and `m2::VecE` for **all** state and positions that feed the lockstep
  simulation. Never `float`/`double`/`m2::VecF` for simulation state.
- `m2::VecF`/`float` are allowed **only** for presentation-only values (graphic position,
  camera panning). Existing objects flag these with comments like
  `// ... doesn't need to be deterministic`.
- Randomness uses `m2::XsrRng` seeded with fixed constants (see `World`). Never `rand()`,
  `std::random`, time, or addresses.
- Express rates per-tick (`m2g::Rate` / `perTick`); do not use the `Stopwatch::Duration`
  argument as a wall-clock delta.

## Step 3 — Build & report

```sh
cmake --preset <GAME>-POSIX-DEBUG          # re-runs the source glob to pick up new files
cd build/<GAME>-$(uname -s)-DEBUG && ninja m2
```

Report success, or surface compile errors and fix them. Common mistakes: forgetting the
ProxyEx registration, a missing/duplicate `ObjectType` enum number, or using `float` where
`m2::Exact` is required.
