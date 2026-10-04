# Changelog

## Unreleased — Core 1.0 API Maturity & Compatibility Foundation (KF-CORE-R10, in progress)

- **R10-001 — Public API inventory (CORE-COMPAT-001).** `docs/compatibility/API_INVENTORY.md` lists and classifies all 49
  installed public headers (every one `stable`; sensitivity flags; owning documentation and status), with the
  documentation decisions recorded. `scripts/audit/check_api_inventory.py` audits it against `include/kritva/core/`
  (`make check`, two CTests). No header, source or behavior change; no ABI or compatibility rule is defined by the
  inventory itself.
- **R10-002 — Source and semantic compatibility policy (CORE-COMPAT-002, CORE-COMPAT-003).**
  `docs/compatibility/COMPATIBILITY_POLICY.md` fixes the contract sources and precedence and classifies source-level and
  semantic changes to stable items as Compatible, Review-required or Incompatible (review-required is treated as
  incompatible until the evolution process of R10-004 exists). `scripts/audit/check_compat_policy.py` audits its
  structure and references (`make check`, two CTests). No header, source or behavior change; no ABI, version-number,
  deprecation or package rule is defined.
- **R10-003 — ABI / binary compatibility policy (CORE-COMPAT-004).** `docs/compatibility/ABI_POLICY.md` decides that
  Core 1.x promises no ABI or binary compatibility (with the rationale, what is not promised, what clients may rely on
  and the conditions for any future promise). The policy audit now also fails if ABI machinery (SOVERSION, visibility,
  export header, shared library) appears in `CMakeLists.txt`. No ABI mechanism, header, source or behavior change.
- **R10-004 — Versioning and API evolution policy (CORE-COMPAT-005..007).** `docs/compatibility/VERSIONING_POLICY.md`
  maps the compatibility classes to MAJOR/MINOR/PATCH release impact, fixes the evolution rules for enumerations,
  `ErrorCode`, virtual interfaces and constants, defines the API evolution review and states the installed-package
  version-selection rule for 1.x (same MAJOR, installed >= requested). The audit checks the release-impact table for
  consistency with the compatibility classes and the package examples against the rule. No header, source, CMake build
  or version change (R10-007 implements the package rule; R10-009 sets 1.0.0).
- **R10-005 — Deprecation and migration policy (CORE-COMPAT-008).** `docs/compatibility/DEPRECATION_POLICY.md` defines
  the lifecycle (stable, deprecated, removable, removed in a MAJOR release), the requirements for deprecating an item,
  the compatibility window and migration guidance, and `docs/compatibility/DEPRECATIONS.md` is the register (empty:
  no stable item is deprecated at 1.0.0). The audit checks the register against the public headers. No header, source
  or behavior change.
- **R10 API / Compatibility Review corrections.** Four cross-document inconsistencies found by the review are closed:
  the most severe class applies where a change matches several rows (removing a documented `noexcept` is
  incompatible); a stable item is removed only after it has been published as deprecated; the inventory class
  `deprecated` applies to a whole header only, item-level deprecation living in the register; patch releases are tagged
  `kritva-core-rMAJOR.MINOR.PATCH` (`MAJOR.MINOR.0` releases keep `kritva-core-rMAJOR.MINOR`). The compatibility audit
  checks each rule mechanically. No header, source or behavior change.

## 0.9.0 — Capability Contract & Readiness Boundary (KF-CORE-R09)

A precise, generic contract around the existing capability mechanisms (`Capability`, `CapabilityId`, `CapabilitySet`,
provider snapshots, `PlatformRequirements` and the context binding) and their boundary with Component dependency
ordering, lifecycle, readiness and health. R0.9 adds **no production type, signature or behavior**: the production change
is the normative contract text in three capability headers. It introduces no service registry, locator, resolver,
dependency injection, discovery, readiness state, capability credential or security mechanism.

### Contract
- **Capability** is descriptive metadata about a contract (identity, name, provided-contract version), not a credential,
  token, proof of trust, state or health signal. `CapabilityId` is the only identity; the name is metadata. A
  declaration is the provider's claim; providers publish by-value snapshots.
- **CapabilitySet** is an ownership-safe value and snapshot, not a registry: at most one entry per identity, replacement
  in place (latest wins, no history), first-insertion order, identity-only lookup, only grows. An entry with an invalid
  identity is storable data, not an authoritative capability, and can never satisfy a requirement.
- **Capability version** is the version of the provided capability contract; Core never compares, orders or ranges it.
- **Provision is not requirement; matching is identity-only.** Names, versions, provider information and lookalikes are
  never consulted; evaluation is explicit, side-effect free and takes at most one snapshot; the Runtime never evaluates,
  resolves, binds, retries or recovers because of a capability.
- **Capability requirements are not Component dependencies**; capability presence never changes the dependency order.
- **Readiness** is calculated by no one in Core: a Component decides whether a prerequisite is sufficient for one of its
  own lifecycle operations and fails it with an existing error; the lifecycle keeps its eight states.

### Documentation and security
- Markdown API documentation under `docs/api/` (maintained pages for capability, platform requirements, lifecycle and the
  dependency graph; labelled stubs for unchanged domains) with a mechanical audit (`scripts/audit/check_api_docs.py`).
- Security assessment recorded: **SECURITY IMPACT: DOCUMENTATION ONLY** (decisions SD-R09-01..07 under `docs/security/`).

### Build and validation
- New requirements `CORE-CAP-004` to `CORE-CAP-011`, traced.
- A test-only capability harness (set, provider and requirement fixtures with 45 deliberately defective variants and
  reusable conformance checks), a seeded Runtime/Component readiness model, and a compile-time snapshot of the frozen
  capability/requirement/readiness boundary.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).
- Nine API documentation pages for unchanged domains are labelled stubs and are replaced when their contract next changes.
- References from `CapabilitySet`, a context, reporter or provider must not outlive what they refer to (documented
  undefined behavior; non-owning by design).

## 0.8.0 — Component Configuration Foundation (KF-CORE-R08)

A precise, platform-independent contract around the existing configuration path: `Configuration`,
`ConfigurationVersion`, `Component::configure(const Configuration&)` and `RuntimeManager::configure()`. R0.8 adds no
dynamic reconfiguration, no parameter server, persistence, event or background activity, no new lifecycle state, no
new `ErrorCode` and no change to `Component`, `RuntimeManager` or `ComponentContext`. R0.8 adds **no production type,
signature or behavior**: the production change is the normative contract text in two configuration headers.

### Contract
- **Lifecycle eligibility:** `configure()` is valid only from `UNKNOWN` and `STOPPED`; from `READY`, `RUNNING` and `FAULT`
  it fails with `INVALID_STATE` and has no other effect. A successful or rejected `configure()` never changes the
  lifecycle state; `RuntimeManager::configure()` follows the same rule and never enters `FAULT`. There is no
  `CONFIGURED`/`RECONFIGURING` state and no `reconfigure()`, `set_parameter()`, `get_parameter()` or `configuration()`.
- **Ownership:** the caller owns the `Configuration` (const reference, valid only for the call); a conforming component
  copies what it keeps and retains no address, reference or pointer into it; Core stores and caches nothing and the
  Runtime forwards the caller's own object.
- **Atomic application:** all-or-nothing; on any failure the previously accepted configuration (or the initial state) is
  unchanged and the lifecycle state is unchanged. There is no cross-component transaction or Runtime rollback.
- **Validation boundary:** Core owns structural validation (`Configuration::validate()`, one invariant enforced at
  `set()`); the component owns semantic validation. Errors use the existing codes: `INVALID_STATE`, `INVALID_ARGUMENT`,
  `CONFIGURATION_ERROR`, returned unchanged by the Runtime.
- **`ConfigurationVersion`** is the schema/contract compatibility version (an alias of `Version`), not a revision,
  counter, transaction id or history; compatibility is the component's policy.
- **Runtime forwarding:** the same caller object reaches every component exactly once in dependency order, stopping at
  the first failure with that component's `Error` unchanged, no retry and no rollback, whether or not the topology is
  fixed; configuration is independent of Status, Health, `FAULT`, `ComponentContext` and the platform.

### Build and validation
- New requirements `CORE-CFG-004` to `CORE-CFG-013`, traced.
- A test-only configuration harness (a contract-following component with 22 deliberately broken variants, reusable
  contract and Runtime-forwarding checks), a seeded Runtime differential (200 seeds) and a compile-time snapshot of the
  frozen configuration boundary that detects any signature, enumerator or reconfiguration/revision drift.
- Validated from a fresh clone with ASan/UBSan, TSan, strict warnings, GCC `-fanalyzer` and coverage.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and `make format-check` are placeholders).
- The scheduler CPU affinity mask is 32 bits; widening it would be a separately reviewed change.
- The conformance suite's single-check mutation strictness is documented as a known gap (see R04-006).
- A context, reporter, statistics provider or retained configuration pointer must not outlive what it refers to
  (documented undefined behavior; they are non-owning by design).

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
