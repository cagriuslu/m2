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

### Math & RNG (`common/math`, `common/rng`, `include/m2/math`)

- **Exact** (`common/include/m2/common/math/Exact.h`) — 32-bit fixed-point deterministic number; arithmetic, `SquareRoot`, `Round`, `MultiplyDivide`, `Compose`, `ClosestExact` decimal parsing, `ToString`/`ToInteger`. Deps: none.
- **Float** (`common/include/m2/common/math/Float.h`) — float wrapper with tolerance-aware `IsEqual`/`IsLess`, abs/inverse/pow/sqrt. Deps: none.
- **Rational** (`common/include/m2/common/math/Rational.h`) — 64-bit rational, overflow-checked ops, `Simplify`, `Mod`, full comparisons, `double`→`Rational`. Deps: generated pb header (compile-time only).
- **VecF** (`common/include/m2/common/math/VecF.h`) — 2D float vector; length/distance/angle/dot, `Normalize`, `Rotate`, `Lerp`, `Clamp`, `RoundToBin`, `GetAabbCorners`. Deps: VecI (EASY).
- **VecE** (`common/include/m2/common/math/VecE.h`) — exact 2D vector over `Exact`; operators, lengths, `Normalize`, `VecEHash`. Deps: none.
- **VecI** (`common/include/m2/common/math/VecI.h`) — integer 2D vector; Manhattan/Euclidean distance, `IsNear`, hash, comparators. Deps: none.
- **RectF** (`common/include/m2/common/math/RectF.h`) — float rectangle; `CreateFromCorners`, contains/intersection, `Trim*`/`Align*`, `TrimToAspectRatio`, `GetIntersectingCells`, `GetRow`. Deps: VecF/RectI.
- **RectI** (`common/include/m2/common/math/RectI.h`) — integer rectangle; `ForEachCell`, `ForDifference`, `TrimToSquare`/`TrimToAspectRatio`, `GetIntersection`, `ApplyRatio`. Deps: VecI/RectF.
- **RectE** (`common/include/m2/common/math/RectE.h`) — exact rectangle; corners/centers/`operator bool`. Deps: none.
- **Line** (`common/include/m2/common/math/Line.h`) — 2D line; slope, intercepts, `GetIntersectionPointWith`, angle helpers, perpendicular. Deps: VecF.
- **XsrRng** (`common/include/m2/common/rng/XsrRng.h`) — deterministic xoshiro256++; fixed seeds → reproducible sequences. Ideal for golden-value tests. Deps: none.
- **CustomOptionDistribution** (`common/include/m2/common/rng/Distribution.h`) — picks from an option vector via `rng % size`. Deps: an `Rng` instance (use `XsrRng`).
- **Bit helpers** (`common/include/m2/common/math/Bit.h`) — `RotateLeft64`/`RotateRight64`/`PickSequentialBits`. Deps: none.
- **HashI overloads** (`common/include/m2/common/math/Hash.h`) — incremental FNV-ish hashing over scalars/strings/bytes/`Exact`. Deps: none.
- **Math.h deterministic helpers** (`common/include/m2/common/Math.h`) — tolerance compares, `ToRadians`/`ToDegrees`, `ClampRadiansTo2Pi`/`...ToPi`, `AngleAbsoluteDifference`/`AngleDifference`, `ToFloat(string)`, `RoundToBin`, `Normalize`, `Lerp`, `AtLeastZero`/`AtMostZero`. Deps: none.

### Containers & general utilities (`common/containers`, `common/*`, engine containers)

- **Pool** (`common/include/m2/common/containers/Pool.h`) — generational slot-map; emplace/free/get/contains, pointer↔id lookup. Deps: none. (`GetShiftedPoolId` uses a process-global counter — not reproducible across runs, but all container behavior is deterministic.)
- **Cache** (`common/include/m2/common/containers/Cache.h`) — memoizing wrapper over `unordered_map` + generator functor. Deps: none.
- **MapF** / **Quadrant** / **MapPoolItem** (`common/include/m2/common/containers/MapF.h`, `detail/MapF.h`) — quad-tree spatial map with overflow list and `ForEach*` traversals. Deps: Pool + RectF.
- **SplitString / Trim\* / GetTrimmedRange / GetTrimmedView** (`common/include/m2/common/String.h`) — pure string ops. Deps: none.
- **CompileTimeString** (`common/include/m2/common/String.h`) — constexpr fixed-size string with `GetSize`/`Find`; `static_assert`-testable. Deps: none.
- **Graph** (`common/include/m2/common/Graph.h`) — weighted digraph; `AddEdge` validation, `FindNodesReachableFrom`, `FindPathTo` (A*), `OrderByBestCost`. Deps: FE/VecFE (pure).
- **FsmBase** (`common/include/m2/common/Fsm.h`) — abstract state machine; Enter/Exit, alarm arm/disarm/time, signal transitions. Drive by a trivial subclass + manual `delta_time_s`. Deps: none.
- **handle_signal_using_handler_map** (`common/include/m2/common/Fsm.h`) — table-driven signal dispatch. Deps: none.
- **SpinLock** (`common/include/m2/common/SpinLock.h`) — atomic TAS spinlock; single-threaded lock/unlock is deterministic. Deps: none.
- **Semaphore** (`common/include/m2/common/Semaphore.h`) — counting semaphore; test the non-blocking counting path deterministically. Deps: none.
- **RGB / RGBA** (`common/include/m2/common/video/Color.h`) — color arithmetic with `RoundU8` rounding (alpha unscaled), conversions, named constants. Deps: Meta.
- **PartialEnumLUT** (`common/include/m2/common/protobuf/EnumLUT.h`) — compile-time enum→value table; one-to-one + uniqueness `static_assert`, `Get`/`TryGet`/`ForEach`. Deps: none.
- **IsValuePackUnique / IsDerivedFromField + concepts** (`common/include/m2/common/reflect/Detail.h`) — constexpr uniqueness check + compile-time predicates; test via `static_assert`. Deps: Exact (pure).
- **Meta utilities** (`common/include/m2/common/Meta.h`) — `AreArrayElementsUnique`, `DoesArrayContainElement`, `ConcatArray`, `TransformIf`, `Flush`, `ForEachAdjacentPair`, `ForEachZip`, `ToValues`/`ToVector`/`ToSet`, `Build`, `TransformTuple`, `IsFirstEquals`/`IsSecondEquals`, `GetIndexInVariant`/`GetIndexInTuple`. Deps: none.

### UI (`include/m2/ui`, `src/ui`)

- **MakeVerticalLayout** (`src/ui/Layout.cc`, decl `include/m2/ui/Layout.h`) — pure vertical layout: distributes space, handles fixed/dynamic spacers + integer-division remainders, `expected` errors on overflow. Deps: none.
- **CalculateWidgetRect** (`include/m2/ui/UiPanel.h:128`, impl `src/ui/UiPanel.cc:437`) — maps a child's unitless `x,y,w,h` into a pixel `RectF` within a parent rect. Deps: none.
- **UiWidget::calculate_wrapped_text_rect** (`include/m2/ui/UiWidget.h:69`) — pure text-rect alignment math + throw-on-too-wide. `protected static` → reach via a one-line test subclass. Deps: none.
- **IntegerSelection** (`include/m2/ui/widget/IntegerSelection.h`) — `SetValue` range-clamp (note asymmetric `<` on max vs. the "inclusive" comment — a testable quirk) + `onAction` + `value()`. Build with `nullptr` parent + stack blueprint. Deps: none.
- **TextSelection** (`include/m2/ui/widget/TextSelection.h`) — selection-state logic: `SetOptions`, `SetUniqueSelectionIndex`, `GetSelectedOptions`/`Indexes`/`IndexOfFirstSelection`. Deps: none for these paths.
- **Text** (`include/m2/ui/widget/Text.h`) — `set_text`/`set_color` with cache-invalidation semantics + `trigger_action`. Deps: none (only `OnDraw` touches GPU).
- **UiAction / Continue / Return\<T\> / AnyReturnContainer** (`include/m2/ui/UiAction.h`) — `IsReturn<T>()`, the `IfContinue/IfReturn<T>/IfVoidReturn` dispatch helpers, `Continue::Focus()`, move semantics. Deps: none. Borderline (small surface).

### Audio / video / mt / reflect / protobuf / compression (`src/audio`, `src/video`, `src/mt`, `src/reflect`, `src/protobuf`, `src/thirdparty`)

**Audio (synthesizer):**
- **NoteSampleCount / TrackBeatCount / TrackSampleCount / SongSampleCount** (`include/m2/audio/synthesizer/Detail.h`) — sample-count math from beats/BPM/rational + proto descriptors. Deps: proto + Rational.
- **AudioSample::operator+ / operator+=** (`include/m2/audio/synthesizer/Detail.h`) — stereo sample mixing. Deps: none.
- **MixNote / MixTrack / MixSong** (`include/m2/audio/synthesizer/MixNote.h`, `MixTrack.h`, `MixSong.h`) — synthesize + ADSR + balance into a caller range; out-of-bounds throws. Deps: proto + Rational. *Test SINE/SQUARE/TRIANGLE/SAWTOOTH — NOISE uses nondeterministic `RandomF()`.*
- **Song** (`include/m2/audio/Song.h`, impl `src/audio/Song.cc`) — builds full sample buffer from `pb::Song`. Deps: only the free-function global logger (harmless).

**Reflect:**
- **reflect::Struct / reflect::Variant** (`include/m2/reflect/Composite.h`) — field-id → tuple/variant storage; `Get/Mutate/HoldsAlternative/TryGet/Emplace/Visit`. Deps: reflect Detail + Meta.
- **reflect::SequenceContainer (Deque/List/Vector)** (`include/m2/reflect/Container.h`) — reflective std-container wrapper. Deps: none.
- **StoreToProto / StoreAccessor** (`include/m2/reflect/utils/Proto.h`, impl `src/reflect/utils/Proto.cc`) — serialize a reflective composite into `pb::Reflective`. Deps: proto.
- **Hash / HashAccessor** (`include/m2/reflect/utils/Hash.h`, impl `src/reflect/utils/Hash.cc`) — deterministic hash walk over a reflective composite. Deps: HashI (pure).

**Protobuf wrappers:**
- **enum_value_count / enum_index / enum_value / enum_name / for_each_enum_value** (`include/m2/protobuf/Detail.h`) — enum ↔ index ↔ name LUTs. Deps: proto enum.
- **json_string_to_message / message_to_json_string** (`include/m2/protobuf/Detail.h`, impl `src/protobuf/Detail.cc`) — JSON ↔ proto string. Deps: proto. (`*_file` variants are HARD.)
- **mutable_get_or_create / mutable_insert** (`include/m2/protobuf/Detail.h`) — grow/insert-with-shift on repeated fields. Deps: none.
- **std::formatter\<ProtoEnum\>** (`include/m2/protobuf/Detail.h`) — proto enum → name. Deps: none.
- **MessageLUT** (`include/m2/protobuf/MessageLUT.h`) — `LoadProtoItems`/`load` completeness + duplicate detection; `operator[]`. Deps: pass a non-existent override path so it falls back to the in-memory envelope.
- **thirdparty::protobuf::ToMessage** (`include/m2/thirdparty/protobuf/Deserialize.h`) — bytes → message (`nullopt` on failure). Deps: none.
- **thirdparty::protobuf::IsEqual** (`include/m2/thirdparty/protobuf/Compare.h`) — `MessageDifferencer::Equals`. Deps: none.
- **thirdparty::protobuf::ToJsonString** (`include/m2/thirdparty/protobuf/Json.h`, impl `src/thirdparty/protobuf/Json.cc`) — message → JSON. Deps: none.

**Compression:**
- **compression::Deflate** (`include/m2/thirdparty/compression/Deflate.h`) — zlib compress of a byte vector. Deps: zlib.
- **compression::Inflate** (`include/m2/thirdparty/compression/Inflate.h`) — zlib decompress to a known size. Best as a Deflate→Inflate round-trip + error path. Deps: zlib.

**MT (threading — deterministic single-threaded surfaces):**
- **ProtectedObject\<T\>** (`include/m2/mt/ProtectedObject.h`) — mutex/condvar-guarded object; test `Read`/`Write`, condition-already-true (no block), short-timeout-returns-false. Deps: none.
- **MessageBox\<T\>** (`include/m2/mt/actor/MessageBox.h`) — synchronized FIFO; `Push/Peek/TryPop/TryHandleMessages(Until)` (bounded count, empty exit, handler-stop). Deps: ProtectedObject.
- **SendQuestionReceiveAnswerSync** (`include/m2/mt/actor/MessageBox.h`) — push-then-wait-then-dispatch; pre-populate `recvBox` so `WaitMessage` returns immediately. Deps: none.
- **CooperativeSleepUntil / CooperativeSleepUntilOrTimeout** (`include/m2/mt/CooperativeSleep.h`) — predicate-poll loops; predicate-true returns immediately, false returns false after ~timeout. Deps: none. Low value.

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
