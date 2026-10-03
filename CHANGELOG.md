# Changelog

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
