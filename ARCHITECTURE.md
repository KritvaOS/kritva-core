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

An operation invalid for the current state fails with `INVALID_STATE` and changes nothing. A valid `initialize()`, `start()` or `stop()` that fails moves the component to `FAULT` and returns the cause; a failed `shutdown()` leaves the state unchanged. `FAULT` is left only by `shutdown()`; recovery semantics are defined by KF-CORE-R03-006. Every `Error` a component returns has `source` equal to the component id. Transient states are not observable once an operation returns. See `include/kritva/core/runtime/component.hpp`.

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
