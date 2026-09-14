# Kritva Core Engineering Agent Instructions

## 1. Purpose

These instructions apply to the `kritva-core` repository.

Kritva Core is the platform-independent foundation of the Kritva Open Robotic Computing Platform.

> Kritva connects intelligence to physical action.

## 2. Authority

Consult, in order:

1. `ARCHITECTURE.md`
2. `REQUIREMENTS.md`
3. `API.md`
4. `TESTING.md`
5. parent KritvaOS architecture for cross-repository boundaries
6. relevant ADRs and repository instructions

Do not silently contradict the parent Kritva architecture.

## 3. Core Boundary

Core Foundation owns Identity, Lifecycle, Status, Health, Statistics, Error/Fault, Result, Events, Capability, Configuration, Version, Timestamp, Duration, and Metadata.

Do NOT introduce ROS2, DDS, EtherCAT, hardware drivers, vendor SDKs, motor control, sensor implementations, AI inference, motion planning, cloud services, databases, or robot-specific application logic into Core Foundation.

If functionality appears to require one of these, propose the architectural boundary first.

## 4. Public API Boundary

Public headers belong under `include/kritva/core/`. Implementation belongs under `src/`. Tests belong under `tests/`.

## 5. API-First Development

Before coding:

1. identify requirement
2. identify owning Core domain
3. define API
4. define behavior
5. define error behavior
6. define tests
7. implement
8. verify

## 6. Intern Development Model

Core development may be performed by interns in small bounded work packages. Do not assign the entire Core architecture as one implementation task.

Preferred sequence:

```text
Identity + Version
    ↓
Lifecycle
    ↓
Status + Health
    ↓
Error + Result
    ↓
Statistics
    ↓
Events
    ↓
Capability
    ↓
Configuration
```

Each work package requires requirement IDs, API definition, implementation scope, tests, review criteria, and completion criteria.

## 7. AI Development Model

AI agents are engineering assistants, not autonomous architects.

Agents must inspect before editing, follow requirements, preserve boundaries, make small changes, add tests, report assumptions/failures, avoid speculative APIs, and review the final diff.

Do not silently redesign Core, add unrelated dependencies, remove tests to pass CI, weaken validation, claim unexecuted tests passed, or invent hardware behavior.

## 8. Real-Time Discipline

Any API intended for real-time use must document allocation, blocking, synchronization, complexity, thread-safety, and failure behavior.

## 9. Dependency Discipline

Evaluate necessity, licensing, portability, security, maintenance, build complexity, runtime overhead, and real-time impact before adding dependencies.

## 10. Testing

Code is not complete because it compiles. Run relevant unit tests, contract tests, build validation, and format checks.

## 11. Git Discipline

Prefer small logical commits. Do not perform destructive Git operations without explicit authorization.

Examples:

```text
feat(core): add identity foundation
test(core): add identity contract tests
feat(core): add lifecycle foundation
docs(core): define lifecycle API
```

## 12. Human Review

Human review is required for public API compatibility changes, architectural boundary changes, safety/security behavior, real-time guarantees, and major dependency changes.

## 13. Parent Repository Integration

`kritva-core` may be integrated into KritvaOS as a Git submodule. The parent repository should reference a deliberate Core commit/tag. Do not modify the parent's submodule pointer from this repository.

## 14. Final Rule

> Understand the architecture before changing the code.
