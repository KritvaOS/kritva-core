# KF-CORE-R07 — Component Operational Foundation

## Status

RELEASED (`kritva-core-r0.7` -> `424984f`, published to origin)

## Target

Version: 0.7.0
Tag: `kritva-core-r0.7`

## Objective

Establish a narrow, platform-independent Component operational observation/reporting contract using the existing Core concepts (`Status`, `Health`, `Statistics`, and `Event`) without changing Runtime lifecycle semantics or introducing a Core operational framework.

## Dependency

R0.6 released and closed at version 0.6.0.

## Architectural Position

- Component remains authoritative for its operational information.
- Runtime remains authoritative for lifecycle orchestration and Runtime-owned statistics.
- Existing typed operational concepts are preferred over parallel abstractions.
- Observation is read-only and side-effect free.
- No new Component Operational State machine is introduced.
- Component statistics are optional; they are not mandatory on the base `runtime::Component` interface.
- Events are explicitly reported to integrator-owned sinks; Core does not provide an EventBus, queue, broker or dispatcher.
- Operational information does not automatically drive lifecycle, recovery, retry or restart.
- No Core-owned background execution, telemetry backend, logging backend or platform-specific implementation is introduced.

## Task Order

```text
R07 Design Consult
        ↓
R07 Scope Confirmation
        ↓
R07-001 Component Operational Observation Contract
        ↓
R07-002 Component Status & Health Reporting Contract
        ↓
R07-003 Component Operational Event Contract
        ↓
R07-004 Component Statistics Ownership & Observation Contract
        ↓
R07 Component Operational API Review
        ↓
R07-005 Reference Operational Harness & Contract Tests
        ↓
R07-006 Runtime/Component Operational Integration
        ↓
R07 Integration Freeze
        ↓
R07-007 Full R0.7 Validation
        ↓
R07 Release Gate
```

## Effort Estimate

20–28 engineering-days, including architecture and release gates. Actual effort is not recorded until supported by evidence.

## Requirements Domain

Proposed R0.7 requirement domain: `CORE-OPS-*`.

Requirements enter authoritative `REQUIREMENTS.md` only when their implementing task is accepted.
