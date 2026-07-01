---
name: add-audio
description: Add a sound effect or music track to a game in this engine. Use when the user wants to add, create, play, or trigger a sound, sound effect, SFX, jingle, music, song, or background theme in any game (CuZn, RPG, PINBALL, QNT, WAR, ...).
---

# Add a sound effect or music to a game

## The one thing to understand first

This engine has **no file-based audio**. There is no WAV/MP3/OGG loading anywhere. Every
sound — a UI blip, an impact, or a looping background theme — is the **same** unit: a
`Song`, **procedurally synthesized** at game-load time from a `SynthSong` protobuf
definition (tracks of notes, each with a waveform shape and an ADSR amplitude envelope).

There is therefore **no structural difference between a "sound effect" and "music"** in the
engine. Both are a `SongType` enum value plus a synth definition in `Songs.json`. The
difference is entirely in *how you play it*:

| | Sound effect (SFX) | Music |
|---|---|---|
| Length | Short (still ≥ ~12.5 ms, see limits) | Long, designed to loop seamlessly |
| `PlayPolicy` | `ONCE` (auto-frees when finished) | `LOOP` |
| Trigger | On a game event (collision, hit, click) | Once on entering a level/menu |
| Keep the `PlaybackId`? | Usually no | Yes, if you ever need to `Stop()` it |
| Positional? | Optionally, via a `SoundEmitter` | No |

Read the cited files rather than trusting this document for exact syntax — they are always
current.

## Canonical reference files

- **Synth data schema:** `pb/Synth.proto` (`SynthSong`/`SynthTrack`/`SynthNote`/`Envelope`/
  `SoundWaveShape`) and `pb/Song.proto` (`Song`/`Songs`).
- **Per-game enum:** `game/RPG/pb/m2g_SongType.proto` (note its SFX-vs-music numbering
  convention), `game/PINBALL/pb/m2g_SongType.proto`. Fallback when a game has none:
  `pb-default/m2g_SongType.proto`.
- **Per-game data:** `resource/game/PINBALL/Songs.json` (rich SFX, incl. `NOISE` impacts and
  envelopes), `resource/game/RPG/Songs.json` (one SFX **and** a looping `SONG_MAIN_THEME`
  music track), `resource/game/CuZn/Songs.json` (minimal one-SFX example).
- **Playback API:** `include/m2/audio/AudioManager.h`.
- **Trigger examples:**
  - Global one-shot SFX: `game/CuZn/src/ui/ActionNotification.cc:58`,
    `game/PINBALL/src/objects/CircularBumper.cc:35`.
  - Looping music: `game/RPG/src/Proxy.cc:155`.
  - Positional SFX (SoundEmitter): `game/RPG/src/object/Enemy.cc:39-52`.
- **Synthesis internals (read only if a sound comes out wrong):** `src/audio/Song.cc`,
  `include/m2/audio/synthesizer/MixNote.h` (waveform + ADSR), `src/audio/synthesizer/Detail.cc`
  (length math).

## Step 0 — Auto-detect before asking

Do this yourself; do not ask the user.

1. **Target game.** If the user named one, use it. Otherwise check the configured build under
   `build/` (e.g. `build/<GAME>-<HOST>-DEBUG`) and/or ask in Step 1.
2. **Does the game already have its own audio files?**
   - Proto: `game/<GAME>/pb/m2g_SongType.proto` — if missing, the game currently uses the
     default (only `NO_SONG`) and you must **create** this file.
   - Data: `resource/game/<GAME>/Songs.json` — if missing, the game currently uses
     `resource/default/Songs.json` and you must **create** this file.
3. **Read the game's existing `Songs.json`** (or PINBALL's if the game has none) and mirror its
   structure and idioms.

## Step 1 — Gather the decisions (AskUserQuestion)

Use `AskUserQuestion`; skip anything already answered by Step 0 or the request, and recommend
an answer for each.

1. **Target game.**
2. **SFX or music?** (decides `PlayPolicy`, length, and trigger site).
3. **Enum value name** — e.g. `SONG_DOOR_OPEN` (SFX) or `SONG_BATTLE_THEME` (music).
4. **Trigger** — *what event* plays it (which `.cc` and what condition), or for music *which
   level/menu* starts it.
5. **Positional?** (SFX only) — should volume depend on where the sound is in the world
   relative to the camera? Most SFX are **not** positional (simpler — just call `Play`). Use a
   `SoundEmitter` only when the user wants distance/direction falloff.
6. **The sound itself** — does the user have a synth definition in mind (BPM, notes,
   waveform), or should you design one? If designing, propose something simple and iterate.

## Step 2 — Implement

### 2a. Declare the `SongType` enum value

File: `game/<GAME>/pb/m2g_SongType.proto`, `package m2g.pb;`, `enum SongType`.

- **If the file does not exist, create it.** It then *replaces* the default entirely, so it
  **must** start with `NO_SONG = 0;`. Model it on `game/RPG/pb/m2g_SongType.proto`.
- Add your value: `SONG_<NAME> = <number>;`.
- **Numbering convention (from RPG):** sound effects take low numbers (`1, 2, ...`); music
  takes high numbers (`1001, 1002, ...`). Sparse/non-contiguous numbers are fine — the loader
  indexes songs by their *declaration order* in the `.proto` (`enum_index` →
  protobuf `value->index()`), not by the literal number. Keep entries declared in increasing
  order to match the examples, and never reuse a number.

### 2b. Define the synth in `Songs.json`

File: `resource/game/<GAME>/Songs.json` — the JSON form of `pb::Songs { repeated Song songs }`.

- **If the file does not exist, create it** (model on `resource/game/CuZn/Songs.json`).
- **CRITICAL — every enum value must have an entry.** `MessageLUT::load`
  (`include/m2/protobuf/MessageLUT.h`) throws *"Item is not defined"* at startup if any
  `SongType` lacks an entry, and *"duplicate definition"* if one appears twice. So:
  - Always include an entry for `NO_SONG` (`{ "type": "NO_SONG" }`, no `synthSong`).
  - When you add a new enum value in 2a, you **must** add its entry here too.
- A `Song` entry is `{ "type": "<SONG_ENUM>", "synthSong": { ... } }`. An entry with no
  `synthSong` synthesizes to a zero-length, unplayable placeholder (fine for `NO_SONG` or a
  not-yet-authored slot, e.g. PINBALL's `SONG_FLIPPER_FLIP_DOWN_SOUND`).

**`synthSong` shape** (see `pb/Synth.proto`):

```jsonc
{
  "bpm": 480,                 // beats per minute; higher bpm => shorter sound
  "tracks": [                 // tracks are mixed (summed) together
    {
      "comment": "Bell",      // optional, for humans
      "shape": "SINE",        // SINE | SQUARE | TRIANGLE | SAWTOOTH | NOISE
      "volume": 0.8,          // [0,1] track gain
      "balance": 0,           // [-1,1] stereo pan: -1 left, +1 right
      "amplitudeEnvelope": {  // optional ADSR, applied to EVERY note in the track
        "attackDuration":  {"n": 1, "d": 128},  // beats, as a Rational {n,d}
        "decayDuration":   {"n": 1, "d": 128},
        "sustainVolume":   0.7,                  // [0,1] multiplies note volume
        "releaseDuration": {"n": 5, "d": 16}     // extends the note past its duration
      },
      "notes": [
        {
          "startBeat": {"n": 0, "d": 1},   // Rational, in beats
          "duration":  {"n": 1, "d": 8},   // Rational, in beats
          "frequency": 415,                // Hz, [0,24000]; ignored for NOISE
          "volume":    1.0                 // [0,1]
        }
      ]
    }
  ]
}
```

Design guidance:
- **SFX:** a few short notes. `NOISE` makes percussive impacts/hiss; `SAWTOOTH`/`SQUARE` are
  buzzy/harsh; `SINE`/`TRIANGLE` are pure/soft. Layer a `NOISE` "impact" track over a tonal
  track for punchy hits (see PINBALL `SONG_CIRCULAR_BUMPER_SOUND`). Use the envelope's
  `releaseDuration` to give bells/tones a natural tail.
- **Music:** one or more tracks of many notes forming a loop. RPG's `SONG_MAIN_THEME` uses a
  `TRIANGLE` bass-drum track plus a `NOISE` hi-hat/snare track. Make the last beat line up with
  the first so `LOOP` is seamless. The song's length is the **longest** track, so pad shorter
  tracks if needed.

### 2c. Trigger playback

Access the loaded song by enum value: `M2_GAME.songs[m2g::pb::SONG_<NAME>]`. The audio manager
lives at `M2_GAME.audio_manager`. Signature:
`PlaybackId Play(const Song* song, PlayPolicy policy, float volume = 1.0f)`.

**Pattern A — global one-shot SFX** (most common; not positional):
```cpp
M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_<NAME>], m2::AudioManager::ONCE, 0.25f);
```
`ONCE` auto-frees the playback when it finishes; you can ignore the returned id. Put this at the
event site (a `Physique` contact callback, a widget `onAction`, a character `OnMessage`, etc.).

**Pattern B — looping music** (start once on entering a level/menu, keep the id if you want to
stop it):
```cpp
auto musicId = M2_GAME.audio_manager->Play(
    &M2_GAME.songs[m2g::pb::SONG_<NAME>], m2::AudioManager::LOOP, 0.5f);
// later, e.g. on leaving the level:
M2_GAME.audio_manager->Stop(musicId);
```
RPG starts the main theme in a menu button's `onAction` right after `LoadSinglePlayer`
(`game/RPG/src/Proxy.cc:155`). Nothing auto-stops music on level change — stop it yourself if
you don't want it to persist.

**Pattern C — positional SFX via `SoundEmitter`** (only if Step 1 chose positional). The
emitter carries a world position and an `update` callback the engine ticks every frame;
`Game::RecalculateDirectionalAudio()` adjusts each live playback's left/right volume from the
`SoundListener`s the `Camera` sets up. Mirror `game/RPG/src/object/Enemy.cc:39-52`:
```cpp
if (GetOwner().GetSoundId() == 0) {                 // not already emitting
    auto& soundEmitter = GetOwner().AddSoundEmitter();
    soundEmitter.update = [&](m2::SoundEmitter& se, const m2::Stopwatch::Duration&) {
        if (se.playbacks.empty()) {
            auto id = M2_GAME.audio_manager->Play(
                &M2_GAME.songs[m2g::pb::SONG_<NAME>], m2::AudioManager::ONCE, 0.10f);
            se.playbacks.emplace_back(id);          // tracked for directional volume
        } else {
            // the ONCE playback has finished; clean up the emitter
            M2_LEVEL.deferredActions.push(m2::CreateSoundEmitterDeleter(GetOwner().GetId()));
        }
    };
}
```
The emitter's `position` defaults from the object; the engine reads it for falloff
(`max_hearing_distance_m = 20`). Remove the emitter when done (as above) so it doesn't leak.

## Step 3 — Build, then verify

Pick the right rebuild depth — this is the most common source of "my sound didn't change":

- **Edited `m2g_SongType.proto`, or created any new file** (a first-time `m2g_SongType.proto`
  *or* a first-time `Songs.json`): **reconfigure**, because protos and bundle resources are
  globbed at configure time:
  ```sh
  cmake --preset <GAME>-POSIX-DEBUG
  cd build/<GAME>-$(uname -s)-DEBUG && ninja m2
  ```
- **Only edited an existing `Songs.json` and/or `.cc` trigger code:** a plain rebuild is enough
  (on macOS the resource is a bundle source and is re-copied on build):
  ```sh
  cd build/<GAME>-$(uname -s)-DEBUG && ninja m2
  ```

If the build fails or the game throws at startup, check:
- *"Item is not defined: SONG_..."* — you added an enum value without a `Songs.json` entry (2b).
- *"Playing short audio is not supported"* (thrown from `AudioManager::Play`) — the song is
  shorter than one audio buffer. Minimum playable length is
  `DEFAULT_AUDIO_SAMPLE_RATE / AUDIO_CALLBACK_FREQUENCY = 48000 / 80 = 600 samples ≈ 12.5 ms`.
  The song's length is its longest track:
  `samples = sampleRate * (maxOver notes of startBeat + duration + envelope.releaseDuration) * 60 / bpm`.
  Lower the `bpm`, lengthen a note, or add `releaseDuration` until it clears 600 samples.
- *"Frequency out-of-bounds"* — a note's `frequency` is outside `[0, 24000]` (NOISE ignores it).

**Always ask the user for a verification method if they didn't give one.** Audio is hard to
assert programmatically here, so the realistic check is: run the configured game
(`build/<GAME>-<HOST>-DEBUG`), trigger the event, and listen. Offer to do that, or to add a temporary
trigger (e.g. a key binding) so the sound can be auditioned in isolation.

## Quick reference

- **Waveforms:** `SINE` (pure), `TRIANGLE` (soft), `SQUARE` (hollow/buzzy), `SAWTOOTH`
  (bright/harsh), `NOISE` (percussion/impact; `frequency` ignored).
- **ADSR envelope** is per-track and applied to every note: Attack ramps 0→peak over
  `attackDuration`, Decay falls peak→`sustainVolume` over `decayDuration`, Sustain holds until
  the note's `duration` ends, Release fades to 0 over `releaseDuration` (which *extends* the
  note past its `duration`).
- **Mixing:** all tracks of a song, and all simultaneously-playing songs (up to 32), are summed.
  Keep `volume`s modest to avoid clipping.
- **Audio format:** stereo 32-bit float, 48000 Hz (`common/include/m2/common/Constants.h`).
- **API surface** (`include/m2/audio/AudioManager.h`): `Play(song, policy, volume) -> id`,
  `Stop(id)`, `HasPlayback(id)`, `SetPlaybackVolume(id, v)`, `SetPlaybackLeftVolume`,
  `SetPlaybackRightVolume`. `PlayPolicy` is `ONCE` or `LOOP`.
