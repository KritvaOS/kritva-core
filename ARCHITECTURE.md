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

Setup (`register_component()`, `add_dependency()`) forwards to the registry and graph and returns their results unchanged. The first successful `initialize()` validates the topology with `DependencyGraph::order()` (the only ordering algorithm, so lowest-`ComponentId` tie-break and dependencies first) and then fixes the topology for the life of the manager: later setup fails with `INVALID_STATE`. A failed validation returns the graph's `CONFIGURATION_ERROR` unchanged and leaves the runtime `UNKNOWN` with setup still open.

The runtime follows the Component operation table for its own state only: `initialize` from `UNKNOWN`/`STOPPED` to `READY`; `start` from `READY` to `RUNNING`; `stop` from `READY`/`RUNNING` to `STOPPED`; `shutdown` a no-op in `UNKNOWN`/`STOPPED`; anything else fails with `INVALID_STATE` and changes nothing. In this task the runtime calls no component; ordered component invocation is KF-CORE-R03-005 and failure/recovery is KF-CORE-R03-006.

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
