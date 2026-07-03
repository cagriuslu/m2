# Engine Classes Worth Unit-Testing

An inventory of **engine** classes (everything outside the top-level `game/` directory) that carry
non-trivial behavior worth unit-testing, classified by how much effort testing them takes. Produced
by a fan-out exploration of the whole engine, independent of the existing `test/` suite.

## How to read this

Each class/function is classified into exactly one tier:

- **EASY** — Constructible and exercisable with plain C++. No running `Game`/`Level`, no global
  singletons (`M2_GAME` / `M2_LEVEL` / `M2G_PROXY` / `M2_PLAYER`), no SDL/OpenGL, no live Box2D
  world, no sockets, no threads whose timing matters, no real filesystem, no GPU. Deterministic and
  self-contained. (A compile-time dependency on a generated `*.pb.h` header does **not** disqualify a
  type — that is a build-tree coupling, not a runtime one.)
- **REFACTOR** — The useful logic is entangled with a singleton / side-effecting / heavy dependency,
  but a **small, runtime-perf-neutral** change unlocks it (extract a pure free function, pass a value
  instead of reading a global, split a compute step from I/O, lazy-init a texture, promote a private
  helper). Every REFACTOR entry names the exact change. None add virtual dispatch on hot paths, heap
  allocations, or copies.

---

## EASY — full list by subsystem

### UI (`include/m2/ui`, `src/ui`) — remaining, all [BLOCKED]

These are pure functions whose logic sits in a `.cc` that references `M2_GAME`, so the translation
unit cannot link into the `test` target. Each needs its pure surface extracted (or its body moved
inline into the header) before it is unit-testable here.

- **CalculateWidgetRect** (`include/m2/ui/UiPanel.h:128`, impl `src/ui/UiPanel.cc:437`) — maps a child's unitless `x,y,w,h` into a pixel `RectF` within a parent rect. **[BLOCKED]** the impl is in `src/ui/UiPanel.cc`, which references `M2_GAME`. Extract the pure function to unblock.
- **UiWidget::calculate_wrapped_text_rect** (`include/m2/ui/UiWidget.h:69`) — pure text-rect alignment math + throw-on-too-wide. `protected static`. **[BLOCKED]** the body is in `src/ui/UiWidget.cc` (references `M2_GAME`). Move the body inline into the header (it's already `Deps: none`) to unblock.
- **IntegerSelection** (`include/m2/ui/widget/IntegerSelection.h`) — `SetValue` range-clamp (note asymmetric `<` on max vs. the "inclusive" comment — a testable quirk) + `onAction` + `value()`. **[BLOCKED]** ctor and `SetValue` are in `src/ui/widget/IntegerSelection.cc` (references `M2_GAME`, builds GPU text textures).
- **TextSelection** (`include/m2/ui/widget/TextSelection.h`) — selection-state logic: `SetOptions`, `SetUniqueSelectionIndex`, `GetSelectedOptions`/`Indexes`/`IndexOfFirstSelection`. **[BLOCKED]** logic is in `src/ui/widget/TextSelection.cc` (references `M2_GAME`).
- **Text** (`include/m2/ui/widget/Text.h`) — `set_text`/`set_color` with cache-invalidation semantics + `trigger_action`. **[BLOCKED]** logic is in `src/ui/widget/Text.cc` (references `M2_GAME`).

---

## REFACTOR — testable after a small, perf-neutral change

Each names the exact change. All are code motion / value-injection / lazy-init with no hot-path cost.

### UI
- **ImageSelection** (`include/m2/ui/widget/ImageSelection.h`) — `select()`/scroll-clamp logic is pure, but the **constructor eagerly builds "+"/"-" GPU textures** via `M2_GAME.GetRenderer()`. **Refactor:** lazily create `_plusTexture`/`_minusTexture` in `OnDraw` (as `IntegerSelection`/`TextSelection` already do). Textures still built once, on first draw.
- **CheckboxWithText** (`include/m2/ui/widget/CheckboxWithText.h`; toggle in `AbstractButton::trigger_action`) — the toggle (`_state = !_state` + `onAction`) is pure, but the **ctor eagerly creates its label texture**. **Refactor:** make `_textTexture` a lazily-created `std::optional` built in `OnDraw`.
- **UiWidget::calculate_filled_text_rect** (`include/m2/ui/UiWidget.h:71`) — aspect-fit + alignment math, pure except it measures the string via `M2_GAME.font` / `M2G_PROXY.default_font_size`. **Refactor:** split font measurement (stays at call sites) from a pure helper taking the already-measured glyph `VecF` — mirrors `calculate_wrapped_text_rect`.

### Multiplayer (lockstep — several collapse to EASY after one shared change)
The common blocker: `SmallMessagePasser`/`MessagePasser` peer-parameter logic can't be constructed without a live `UdpSocket`. Making the ACK/reorder/sequence state socket-free unlocks all of the below.
- **SmallMessagePasser::PeerConnectionParameters** (`include/m2/multiplayer/lockstep/SmallMessagePasser.h`) — sliding-window ACK bookkeeping (`GetMostRecentAck`/`GetAckHistoryBits`/`GetOldestNack`), NACK pruning (`ProcessPeerAcks`), gap-based in-order reassembly (`ProcessReceivedMessages`). Holds **no socket** itself. **Refactor:** promote the private nested class to a test-accessible type and pass `gameHash` as a param to the three `CreateOutgoingPacket*` methods instead of calling `M2_GAME.Hash()`. The ACK/reorder methods then need no change.
- **turnbased::MessagePasser** (`include/m2/multiplayer/turnbased/MessagePasser.h`) — NUL-delimited TCP framing: scan buffer for whole messages, JSON-parse, vacuum buffer, serialize outgoing with overflow detection. **Refactor:** extract `parseFramedMessages(buffer, &len, gameHash, outQueue) -> ReadResult` and `frameMessageInto(buffer, msg) -> SendResult` operating on the buffers the class already holds; leave `recv`/`send`/hash as thin callers.
- **lockstep::MessagePasser** (`include/m2/multiplayer/lockstep/MessagePasser.h`) — per-peer sequence-number ordering (`QueueMessage` assigns seq + rejects oversize; `ReadMessages` enforces `seq == last+1`). **Refactor:** unlocked by the `SmallMessagePasser` change above (or split the seq-tracking `PeerConnectionParameters` + find/create into a socket-free helper).
- **ConnectionToPeer** (`include/m2/multiplayer/lockstep/ConnectionToPeer.h`) — per-timecode input buffering with a chained hash (`StorePlayerInputsReceivedFrom`, capacity eviction). Ctor needs a `MessagePasser&`. **Refactor:** extract the `map<Timecode, PlayerInputsAndHash>` + `StorePlayerInputsReceivedFrom` + getters into a standalone `InputBuffer` struct with no `MessagePasser` reference; the extracted struct is then EASY.
- **ConnectionToClient** (`include/m2/multiplayer/lockstep/ConnectionToClient.h`) — running input-hash chain (`StoreRunningInputHash`, capacity eviction) + `GetInputHash(timecode)`. Ctor needs a `MessagePasser&`. **Refactor:** extract the `list<Hash>` running-hash buffer + those two methods into a standalone struct free of `MessagePasser`.

### Core engine
- **GameDimensions** (`include/m2/GameDimensions.h`, impl `src/GameDimensions.cc`) — coordinate-space/letterbox/scale geometry; all instance math reads the window size only through `WindowDimensions()` → non-virtual, unmockable `Window::GetSize()`. **Refactor:** replace the `const Window&` member with an injected `VecI` window-dimensions value (ctor + `OnWindowResize(VecI)` — the caller already fetches the size once per resize). Then all rect/scale math is directly drivable. (Its `static` helpers `EstimateMinimumWindowDimensions`/`NextPixelatedScale`/`PrevPixelatedScale` are already EASY.)
