# Changelog

## 0.7.0 — Component Operational Foundation (KF-CORE-R07)

A narrow, platform-independent way to observe a Component's operational information and to report its occurrences,
built on the existing `Status`, `Health`, `Statistics` and `Event` types and on the unchanged R0.3 runtime and R0.6
component context. R0.7 adds no operational state machine, no event bus, queue, broker or dispatcher, no telemetry or
logging backend, no Runtime polling and no health-driven recovery, retry or restart. All R0.7 production API is
additive: three new public headers and no change to `src/`.

### Observation
- `runtime::observe()` and `runtime::ComponentObservation` (`runtime/component_observation.hpp`): a read-only, detached
  value holding the component's id, lifecycle state, `Status`, `Health` and an optional `Statistics` snapshot, produced
  from the component's own accessors in a documented order. The Component is authoritative: Core keeps no mirror or
  cache. There is no cross-property atomic snapshot, and purity of the accessors is a contract on conforming
  implementations.
- `Status`, `Health` and lifecycle are independent value snapshots: any combination is legal and passed on unchanged,
  and Health is information only (never a recovery trigger and independent of a Runtime FAULT).
- `runtime::IComponentStatistics` (`runtime/component_statistics.hpp`): an **optional**, Component-owned statistics
  provider that is not a base of `Component` (no `statistics()` is added to it, no RTTI is used). Values pass through
  unchanged; the Runtime's own statistics remain distinct.

### Events
- `runtime::IEventSink` (integrator-owned) and `runtime::ComponentEventReporter` (`runtime/component_events.hpp`): a
  copyable value of two non-owning pointers, immutable after construction. `report()` stamps a zero `source_id` with the
  component's id, forwards a matching one unchanged, rejects a mismatching one with `INVALID_ARGUMENT` without calling
  the sink, calls the sink exactly once synchronously and returns its `Result` unchanged. Nothing is buffered, retried,
  queued or dispatched, and an Event never commands the Runtime. The sink must outlive every reporter.

### Runtime boundary
- The Runtime has no observation API and never reads Status, Health, a statistics provider or a sink; replaying seeded
  Runtime scenarios with and without operational activity gives identical results, states, faults, statistics and
  invocation traces.

### Build and validation
- New requirements `CORE-OPS-001` to `CORE-OPS-010`, traced.
- A test-only operational harness (recording sink, scripted operational component, statistics provider, observer and
  runtime probe) and reusable conformance checks that fail for deliberately broken implementations.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).
- A context, reporter or statistics provider must not outlive what it refers to (documented undefined behavior; they
  are non-owning by design).

## 0.6.0 — Component Execution Context (KF-CORE-R06)

An explicit, deterministic, non-owning execution context for integrator-written Components, on top of the unchanged
R0.3 runtime and the R0.4/R0.5 platform contracts. R0.6 adds no service registry or locator, no automatic injection,
no Component-to-Component access, no concrete platform and no thread, executor or background execution. All R0.6
production API is additive: one new public header.

### Component context
- `runtime::ComponentContext` (`runtime/component_context.hpp`): a copyable value of exactly two non-owning
  pointers, the component's immutable `ComponentInfo` and the R0.5 `PlatformContext`. It is **immutable after
  construction** (no setter, reset or rebinding; not assignable), unbound by default, and a temporary identity or
  component is refused at compile time. It owns nothing; copying neither extends a lifetime nor transfers ownership.
- A closed, typed, side-effect-free access surface: identity, the R0.5 view by const reference, `require_scheduler()`,
  `require_clock()`, `require_timer()`, `require_watchdog()`, `supports()`, `has_capability()`, `attribute()`,
  `evaluate()` and `check_required()`. There is no generic or by-name access and no path to the Runtime, registry,
  another component, configuration, statistics, health or the raw adapter.
- Error attribution: when the context is bound the Core availability error (`UNSUPPORTED`) carries the component as its
  source (code, severity, timestamp and message are R0.5's), so a component can return it directly; `attribute()`
  replaces only the `source` of an `Error`; platform service errors are never touched.
- Construction-time injection only: the Runtime never creates, stores, passes or probes a context, and `Component`
  and `RuntimeManager` are unchanged. Requirement binding applies the R0.5 identity-based requirement model; the
  context stores no requirements.

### Build and validation
- New requirements `CORE-CTX-001` to `CORE-CTX-007`, traced.
- A test-only reference context component (scripted plans through a context), reusable context contract checks, and a
  Runtime/component integration suite proving that a platform failure reaching the Runtime through a context is exactly
  an ordinary component failure (72 service-method, attempt and error-code cases, full invocation traces), and that the
  Runtime is identical whatever components do with their context.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and 99% line coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).
- A context (like `PlatformContext`) must not outlive what it refers to (documented undefined behavior; the context is
  non-owning by design).

## 0.5.0 — Platform Runtime Integration Foundation (KF-CORE-R05)

A controlled, platform-independent way for integrator-written code to consume externally owned platform services,
on top of the unchanged R0.4 contracts and R0.3 runtime. R0.5 does not implement a concrete Linux, PREEMPT_RT, RTOS,
MCU, vendor, Nexus or Edge platform, adds no service registry or locator, and adds no thread, executor or background
execution. All R0.5 production API is additive.

### Platform integration
- `platform::PlatformContext` (`platform/context.hpp`): a small, copyable, non-owning view over one
  `IPlatformAdapter` (nullable, so `RuntimeManager::platform()` can be passed directly). It owns nothing, caches
  nothing and holds one pointer; copying it neither extends the adapter's lifetime nor transfers ownership.
  Service and capability queries forward to the adapter; capability identity alone decides.
- Explicit consumption: `require_scheduler()`, `require_clock()`, `require_timer()` and `require_watchdog()` return
  the adapter-owned service or `UNSUPPORTED`; they only query. Platform service errors are never translated.
- `platform/requirements.hpp`: declarative `PlatformRequirements` (required/optional services and capabilities,
  duplicates rejected by identity, atomically), a structured `PlatformRequirementReport`, `evaluate()` and
  `check_required()`; no platform name or version is ever consulted.
- Runtime/platform lifecycle separation (contract text and tests, no behavior change): the Runtime never starts,
  stops, ticks, reads or recovers platform services and is unaffected by anything the platform does; a platform
  failure becomes a Runtime failure only through an integrator-written component, as an ordinary component failure.

### Build and validation
- New requirements `CORE-PLAT-012` to `CORE-PLAT-018`, traced. The audit also forbids test-only support in
  production sources, the production target and the install rules; a CTest compiles every production translation
  unit with only `include/` on the include path.
- A test-only reference platform (selectable services and capabilities, deterministic atomic fault injection, call
  logs, controllable time, lifetime observation) and a Runtime/platform integration suite that proves a platform
  failure is exactly an ordinary component failure for every service method, attempt and error code, and that the
  Runtime is identical with and without a platform.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and 98% line coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).
- A `PlatformContext` must not outlive its adapter (documented undefined behavior; the context is non-owning by design).

## 0.4.0 — Platform Abstraction (KF-CORE-R04)

Platform contracts and integration boundaries on top of the unchanged R0.3 runtime. R0.4 does not implement a
concrete Linux, RTOS, MCU, vendor, Nexus or Edge platform adapter, and adds no thread, executor or background
execution.

### Platform contracts
- `Callback` (`types/callback.hpp`): function plus opaque, non-owning, caller-owned context. Platform boundary
  rules (`platform/boundary.hpp`): adapters live outside Core, the integrator owns adapters and services, no
  singleton or service locator, failures through `Result`/`Error` with defined meanings, atomic failure,
  adapter-defined thread safety, no real-time guarantee.
- `platform::IScheduler` hardened (task ids, atomic creation, idempotent start and stop, no entry after stop,
  adapter-defined priority, affinity and capacity). Signatures unchanged; the 32-bit affinity mask is a
  documented limitation.
- **Breaking:** `time::ITimer::start(Duration)` is now `start(Duration period, TimerMode mode, Callback
  callback)` (`ONE_SHOT`/`PERIODIC`); synchronous idempotent `stop()`; no clock domain on timers. `time::IClock`
  remains canonical and `platform::IClock` remains an alias.
- `platform::IWatchdog` hardened (STOPPED/RUNNING, `start`, `kick`, `stop` semantics; expiry is
  adapter-defined and never triggers Runtime recovery). Signatures unchanged.
- `platform::IPlatformAdapter`, `PlatformInfo`, `PlatformService`: identity, service discovery (non-owning
  pointers, `nullptr` meaning unsupported, non-virtual `supports()`) and capability snapshot.

### Runtime
- Additive, optional `RuntimeManager::attach_platform()` and `platform()`: setup-only, non-owning, never
  replaces, never probes; the Runtime never calls the adapter, so behavior is identical with and without one
  (proven by a differential test). No R0.3 signature or semantic changed.

### Build and validation
- New requirements `CORE-PLAT-004` to `CORE-PLAT-011`, traced; the audit also forbids operating-system and
  vendor headers in production sources. Public-header self-containment is checked by the build.
- Reusable platform conformance suite in `tests/platform/` for future external adapters, validated against
  conforming adapters under different adapter policies and 69 deliberately faulty doubles.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and 98% line coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are
  placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).

## 0.3.0 — Runtime Foundation (KF-CORE-R03)

Platform-independent, synchronous runtime foundation on top of the R0.2 contracts. No scheduler, executor,
threads, platform implementation or logging backend was added.

### Runtime
- `runtime::Component` now has an immutable identity and metadata (`ComponentId`, `ComponentInfo`), a
  documented lifecycle contract reusing the Core lifecycle table, attributable errors and a non-owning
  ownership model. **Breaking:** a component must be constructed with a `ComponentInfo`.
- `ComponentRegistry`: deterministic, non-owning registry keyed by `ComponentId`; duplicates rejected;
  enumeration in ascending `ComponentId` order.
- `DependencyGraph`: dependencies by `ComponentId`; self, duplicate and cycle-closing edges rejected so the
  graph is always acyclic; deterministic dependency order with a lowest-`ComponentId` tie-break.
- `RuntimeManager`: concrete implementation of the unchanged `runtime::Runtime` interface. Topology is
  validated and fixed by the first successful `initialize()`. `configure`, `initialize` and `start` run in
  dependency order, `stop` and `shutdown` in reverse; the first failing component ends the sequence and its
  own error is returned unchanged; a failed initialize, start or stop moves the runtime to `FAULT`;
  `reset()` is the only, explicit, caller-driven recovery (two reverse-order cleanup passes, ends in
  `STOPPED`, never retries the failed operation); the runtime tracks per-component progress itself and owns a
  `Statistics`. No automatic retry, health-driven recovery or background activity.

### Build and validation
- The library installs as a CMake package (`find_package(kritva_core 0.3)`, `kritva_core::kritva_core`).
- New requirements `CORE-RT-003` to `CORE-RT-010`, traced; the traceability audit now also checks version
  consistency and the absence of threading/logging headers in production sources.
- New unit, contract and integration tests, including a seeded model-based test; validated from a fresh clone
  with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and 98% line coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are
  placeholders).

## 0.2.0 — Core Contract Hardening (KF-CORE-R02)

Contract-hardening release of the Kritva Core foundation. No Runtime Manager,
platform implementation, or new top-level domain was added.

### Contracts
- `Result<T>` / `Result<void>`: explicit success/failure semantics, documented
  copy/move behavior, and a `failure()` precondition (`ErrorCode::NONE` is
  rejected; asserted in debug builds).
- `Status` / `StatusCode`: self-contained headers with documented construction,
  mutability, allocation, and thread-safety.
- Statistics: `Counter` (wraps modulo 2^64), `Gauge` (stored as-is), and the
  `Statistics` aggregate documented, including field meanings and units.
- Scheduler (`platform::IScheduler`): `TaskConfig` field semantics, entry and
  context lifetime, create/start/stop behavior, `TaskId` rules, and error
  mapping. Contract only; adapters own platform policy.
- Clock: `time::IClock` is the canonical clock contract with fixed clock
  domains; `Timestamp` has no ordering or subtraction across domains.
  `platform::IClock` remains a compatibility alias of the same type.

### Quality
- Requirement/API/test traceability closed: all referenced IDs defined, process
  requirements traced, and `make traceability-check` added to `make check`.
- New `kritva_core_foundation_contract` test, including an exhaustive 8x8
  lifecycle transition matrix; reusable `IScheduler` conformance checks.
- Validated from a fresh clone: ASan/UBSan, TSan, strict warnings, GCC
  `-fanalyzer`, 98% line coverage, 17/17 tests.

### Documentation and build
- Lifecycle transition table documented in `ARCHITECTURE.md`.
- `make coverage` now works from a fresh clone.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and
  `make format-check` are placeholders).

## 0.2.0-proposed
- Freeze Core/platform boundary.
- Add runtime, messaging, time and platform contracts.
- Replace header-only CMake target with compiled library.
- Add lifecycle validation and initial contract tests.
- Add machine-readable architecture manifest.
