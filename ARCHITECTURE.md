# Kritva Core Architecture

## 1. Purpose

This document defines the architecture of `kritva-core`, the platform-independent foundation of the Kritva Open Robotic Computing Platform.

## 2. Architectural Role

```text
Application
    ↓
SDK
    ↓
Skill
    ↓
Mind / Motion
    ↓
Sense
    ↓
Kritva Core
    ↓
Hardware Abstraction
    ↓
Nexus / Edge
```

Core defines reusable software contracts. It does not own physical hardware implementation.

## 3. Core Foundation

```text
types
├── ID
├── Version
├── Timestamp
├── Duration
└── Metadata

lifecycle → Lifecycle
status → Status
health → Health
statistics → Counter / Gauge / Statistics
error → ErrorCode / Error / Result
event → EventType / Event
capability → CapabilityID / Capability / CapabilitySet
configuration → Parameter / Configuration / ConfigurationVersion
```

### Lifecycle transitions

`Lifecycle` starts in `UNKNOWN`. Only the transitions below are valid; every other transition (including a transition to the current state) is rejected with `ErrorCode::INVALID_STATE` and leaves the state unchanged.

| From | Allowed targets |
|---|---|
| `UNKNOWN` | `INITIALIZING` |
| `INITIALIZING` | `READY`, `FAULT` |
| `READY` | `RUNNING`, `STOPPED`, `FAULT` |
| `RUNNING` | `STOPPING`, `FAULT` |
| `STOPPING` | `STOPPED`, `FAULT` |
| `STOPPED` | `INITIALIZING` |
| `FAULT` | `RECOVERING`, `STOPPED` |
| `RECOVERING` | `READY`, `FAULT` |

This table (15 valid transitions out of 64 state pairs) is verified exhaustively by `tests/contract/foundation_contract_test.cpp`.

### Runtime component contract

A runtime `Component` (`runtime/component.hpp`) has an immutable identity (`ComponentId`, valid iff non-zero) and metadata (`ComponentInfo`: id, non-empty name, version) fixed at construction. Components are owned by the integrator; Core holds only non-owning references and never copies, moves or deletes a component. It reuses the lifecycle states and transition table above; no state is added.

| Operation | Valid from | On success | Core transitions |
|---|---|---|---|
| `configure()` | `UNKNOWN`, `STOPPED` | state unchanged | none |
| `initialize()` | `UNKNOWN`, `STOPPED` | `READY` | `INITIALIZING`, `READY` |
| `start()` | `READY` | `RUNNING` | `READY` to `RUNNING` |
| `stop()` | `READY`, `RUNNING` | `STOPPED` | (`STOPPING`,) `STOPPED` |
| `shutdown()` | `UNKNOWN`, `STOPPED` | state unchanged (idempotent) | none |
| `shutdown()` | `FAULT` | `STOPPED` | `FAULT` to `STOPPED` |

An operation invalid for the current state fails with `INVALID_STATE` and changes nothing. A valid `initialize()`, `start()` or `stop()` that fails moves the component to `FAULT` and returns the cause; a failed `shutdown()` leaves the state unchanged. `FAULT` is left only by `shutdown()`; recovery semantics are defined by KF-CORE-R03-006. Every `Error` returned by an operation of an instantiated component has `source` equal to the component id (errors from `ComponentInfo::create()` occur before a component exists and are exempt). Transient states are not observable once an operation returns. See `include/kritva/core/runtime/component.hpp`.

### Component registry

`runtime::ComponentRegistry` (`runtime/component_registry.hpp`) records which components exist, keyed by `ComponentId`, and nothing else. It is non-owning: the integrator owns each component and must keep it alive, at the same address, while the registry is used; the registry never owns, copies, moves, deletes or calls a component. Registering an id that is already registered fails with `INVALID_ARGUMENT` (source = the id) and changes nothing. `find()` and `contains()` are side-effect free. `components()` returns a snapshot of non-owning pointers, each component once, in ascending `ComponentId` order regardless of registration order. There is no unregister operation. The registry is not thread-safe and is control-plane only. Dependency ordering and lifecycle orchestration are separate (KF-CORE-R03-003/004/005).

### Dependency graph

`runtime::DependencyGraph` (`runtime/dependency_graph.hpp`) records "dependent depends on dependency" edges between `ComponentId`s and computes a dependency order. It holds only ids: no component, pointer or registry, and it never calls a component. `add_dependency()` rejects, with `INVALID_ARGUMENT` and the dependent as error source, the invalid id, a self dependency, a duplicate edge (never silently merged) and any edge that would create a cycle (the cycle is listed in the message), leaving the graph unchanged; the graph is therefore always acyclic. `order(registry)` returns every registered component exactly once with dependencies before dependents; among components that are simultaneously ready the lowest `ComponentId` goes first, so the result does not depend on registration or insertion order. An edge endpoint that is not registered makes `order()` fail with `CONFIGURATION_ERROR` and no order is returned. Lifecycle orchestration using this order belongs to KF-CORE-R03-004/005.

### Runtime Manager

`runtime::Runtime` (`runtime/runtime.hpp`, `CORE-RT-002`) is the authoritative runtime contract and is unchanged. `runtime::RuntimeManager` (`runtime/runtime_manager.hpp`) is its concrete, synchronous, platform-independent implementation; there is no second Runtime abstraction. It composes a `ComponentRegistry` and a `DependencyGraph`, owns no component, and creates no thread, executor, scheduler or timer.

**Topology.** Setup (`register_component()`, `add_dependency()`) forwards to the registry and graph and returns their results unchanged. The first successful `initialize()` validates the topology with `DependencyGraph::order()` (the only ordering algorithm: dependencies first, lowest-`ComponentId` tie-break) and then fixes it for the life of the manager: later setup fails with `INVALID_STATE`. A failed validation returns the graph's `CONFIGURATION_ERROR` unchanged, leaves the runtime `UNKNOWN` with setup still open, and invokes no component.

**Lifecycle orchestration.** Each operation first checks the runtime's own state (the Component operation table); an invalid call fails with `INVALID_STATE`, changes nothing and invokes no component. Otherwise it invokes the corresponding Component operation once per component:

| Operation | Valid from | Component call | Order | On success |
|---|---|---|---|---|
| `configure(cfg)` | `UNKNOWN`, `STOPPED` | `configure` | forward | state unchanged |
| `initialize` | `UNKNOWN`, `STOPPED` | `initialize` | forward | `READY` |
| `start` | `READY` | `start` | forward | `RUNNING` |
| `stop` | `READY`, `RUNNING` | `stop` | reverse | `STOPPED` |
| `shutdown` | `UNKNOWN`, `STOPPED` | `shutdown` | reverse | state unchanged |

Forward is dependency order; reverse is exactly the reverse sequence, so dependents are torn down before dependencies. The runtime state changes only after the whole sequence succeeded: it is never `READY` or `RUNNING` before every component has succeeded. The runtime records each component's lifecycle progress itself (never initialized, live, shut down) rather than inferring it from `Component::lifecycle_state()`: `shutdown()` invokes only live components that have not yet been shut down in the current live period, so repeated calls never invoke a component twice, and a failed `shutdown()` keeps the progress already made (a retry resumes with the failing component, then the rest, in reverse order). `initialize()` starts a new live period. `configure()` does not initialize, start, change the runtime state or fix the topology.

**Failure boundary.** The first failing component ends the sequence: later components are not invoked, nothing is retried, and nothing is rolled back. That component's own `Error` is returned unchanged (code, source, message). A failed `configure()` or `shutdown()` leaves the runtime state unchanged; a failed `initialize()`, `start()` or `stop()` moves the runtime to `FAULT`, in which every operation fails with `INVALID_STATE` until recovery and reset are defined (KF-CORE-R03-006). No automatic retry, watchdog, timer or background work exists.

### Runtime failure and recovery

A failed component operation is returned by the runtime exactly as the component returned it (code, severity, source = that component's id, message); the runtime never replaces it with a generic error. The runtime records, per component and per live period, what it has seen complete (none, initialized, started, stopped, faulted, shut down) instead of reading component state; a failed component is recorded as faulted and is never assumed to have completed.

| Event | Runtime state | Components | Observable |
|---|---|---|---|
| `initialize`, `start` or `stop` fails at component C | `FAULT` | earlier ones keep their state, C is in `FAULT`, later ones are not invoked | the component's `Error`; `fault_error()` |
| `configure` or `shutdown` fails | unchanged | sequence ends at the failure; `shutdown` progress is kept | the component's `Error` |
| any operation except `reset()` while in `FAULT` | `FAULT` | none invoked | `INVALID_STATE` |
| `reset()` outside `FAULT` | unchanged | none invoked | `INVALID_STATE` |
| `reset()` succeeds | `STOPPED` | cleaned up (below) | `fault_error()` becomes null (the pointer is valid only while the runtime is in `FAULT`) |
| `reset()` fails | `FAULT` | progress kept; no successful step repeated | the cleanup `Error`; `fault_error()` still the original |

`reset()` is explicit and caller-driven, valid only in `FAULT`, always in reverse dependency order: pass 1 stops every component recorded initialized or started; pass 2 shuts down every component recorded stopped or faulted (a faulted component leaves `FAULT` only through `shutdown()`). Components never invoked are untouched. The failed operation is not retried by `reset()`; the caller may then call `initialize()` again, which is a new explicit attempt. `FAULT` to `RECOVERING` to `READY` exists in the lifecycle table but is not used because no Component operation produces it. Nothing retries, supervises or recovers automatically, and component health is never consulted. The runtime owns a `Statistics` (`statistics()`): `sample_count` counts successful component invocations, `error_count` failed ones (including cleanup), `retry_count` stays zero, the other fields are unused.

Terminology: an *error* is a failed `Result`; the *fault state* is the runtime lifecycle state `FAULT`, entered only through an error and left only through `reset()`; *health* is what a component reports about itself, independent of the fault state and never a trigger (`DEGRADED` is not a warning); there is no *warning* API; a *diagnostic* is a structured value (the error, `fault_error()`, `statistics()`, `state()`), not a log; an *event* describes a Core-internal state change and an *application message* carries application data, and the runtime emits neither.

### Platform adapter boundary

Core defines contracts for platform services (scheduler, clock, timer, watchdog, platform identity); platform-specific repositories implement them. R0.4 establishes platform contracts and integration boundaries; it does not implement a concrete Linux, RTOS, MCU, vendor, Nexus, or Edge platform adapter. Core production sources include no operating-system, RTOS, vendor, hardware-driver, ROS 2/DDS or EtherCAT interface; the repository audit enforces this. The integrator owns every adapter and service; Core holds only non-owning references, creates and destroys no platform object, and has no singleton, global platform or service locator; a platform object must outlive every Core object that references it. `kritva::core::Callback` (`types/callback.hpp`) is the shared function plus opaque, non-owning, caller-owned context pair (null function invalid where required, null context valid); each callback-bearing service states its execution context, blocking and re-entrancy rules, context lifetime and stop/destruction behavior, callback functions never throw, and no callback runs on a Core-owned thread. Adapters report failure through `Result`/`Error` (`INVALID_ARGUMENT`, `INVALID_STATE`, `UNSUPPORTED`, `RESOURCE_UNAVAILABLE`, `TIMEOUT`), atomically. Thread safety is adapter-defined and no real-time guarantee is made. See `include/kritva/core/platform/boundary.hpp`.

The timer contract (`time::ITimer`, `start(period, mode, callback)`) measures elapsed monotonic time independently of `IClock`; Core contains no timer implementation. See API.md section 27 and CORE-PLAT-006.

The watchdog contract (`platform::IWatchdog`) defines STOPPED/RUNNING behavior for start, kick and stop; the expiry action is adapter-defined and never triggers Runtime recovery. See API.md section 28 and CORE-PLAT-007.

The platform adapter contract (`platform::IPlatformAdapter`) reports a platform's identity (`PlatformInfo`: name and version), which of the four services it provides (non-owning pointers, `nullptr` meaning unsupported) and its capabilities (an owned `CapabilitySet` snapshot). Core ships no adapter, registry or singleton; the integrator owns the adapter. See API.md section 29 and CORE-PLAT-008.

The Runtime-platform boundary (CORE-PLAT-010) is one optional, additive pair on `RuntimeManager`: `attach_platform()` (setup only, never replaces) and `platform()`. The Runtime stores a non-owning reference and never calls the adapter or any platform service, so its behavior is identical with and without one; the integrator owns the adapter, and platform failures and watchdog expiry reach the Runtime only through integrator-written components. See API.md section 30.

R0.5 adds `platform::PlatformContext` (CORE-PLAT-012): a copyable, non-owning view over one `IPlatformAdapter` for integrator-written code. It owns nothing, is not a registry or service locator, and forwards service and capability queries to the adapter on demand; the integrator keeps ownership and the adapter must outlive every context. See API.md section 31.

Platform requirements (CORE-PLAT-013) are declarative: `PlatformRequirements` lists required and optional services and capabilities by identity, and `evaluate()`/`check_required()` compare them with a `PlatformContext` without side effects. Capability identity alone decides; no platform name or version is ever consulted. See API.md section 32.

Explicit consumption (CORE-PLAT-014): `PlatformContext::require_scheduler()/require_clock()/require_timer()/require_watchdog()` return the adapter-owned service or `UNSUPPORTED`; they only query, start and stop nothing, and platform service errors are never translated by Core (a Component that propagates one sets its own source). See API.md section 33.

## 4. Platform Independence

Core must be usable across Linux, PREEMPT_RT, RTOS, MCU, ARM, RISC-V, x86, simulation, FPGA, and future Kritva silicon.

Core must not require a specific OS, processor, bus, middleware, or vendor SDK.

## 5. Dependency Boundary

Higher-level Kritva components depend on Core. Core must not depend on Sense, Mind, Motion, Skill, SDK, ROS2, EtherCAT, or hardware drivers.

## 6. Public API

Public headers live under `include/kritva/core/`; implementation lives under `src/`.

## 7. Real-Time Considerations

APIs that may be used in real-time paths must document allocation, blocking, synchronization, complexity, thread-safety, and failure behavior.

## 8. Error Model

Operational failures should use explicit error handling such as `Result<T>`. Errors should provide diagnostic context without forcing exceptions into real-time paths.

## 9. Time Model

Core provides timestamp and duration abstractions. Core does not implement PTP or a specific clock synchronization protocol.

## 10. Configuration

Configuration APIs define typed parameters, constraints, schema/version information, and validation behavior. Core does not mandate YAML, JSON, databases, or persistence technology.

## 11. Events

Events provide a common envelope capable of identifying event ID, source ID, event type, timestamp, severity where applicable, and correlation ID where applicable.

## 12. Capability

Capabilities provide discoverable representations of component functionality with stable identity and versioning.

## 13. Compatibility

Public API changes require explicit review, documented behavior, tests, and compatibility assessment.

## 14. Design Principle

Core should remain small, deterministic where required, portable, testable, dependency-light, understandable, and reusable.

> Define the contract once; allow many implementations.
