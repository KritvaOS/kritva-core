# Kritva Core — Master Milestone Roadmap

## Milestone Lifecycle

PLANNED → IN PROGRESS → IMPLEMENTATION COMPLETE → REVIEW → ACCEPTED → RELEASED

## KF-CORE-R01 — Core Foundation

Status: COMPLETE

Established the initial platform-independent Kritva Core API contracts and foundation implementation.

## KF-CORE-R02 — Core Contract Hardening

Status: RELEASED (`kritva-core-r0.2` → `46af52c`, published to origin)

### Objective

Harden the existing public contracts, remove ambiguity in API behavior, close requirements/API traceability gaps, strengthen foundation contract tests, and complete an objective R0.2 validation gate.

### Exit Conditions

- All R02 tasks accepted.
- All new tests pass.
- Full regression passes.
- Public API reviewed.
- Requirements traceability complete.
- Coverage reviewed.
- Sanitizer/static analysis reviewed where configured.
- Documentation and changelog updated.
- Git history reviewed.
- Release tag created.

### Task Order

001 → 002 → 003 → 004 → 005 → 006 → 007 → 008

Some tasks may be developed in parallel only if their dependency conditions are satisfied. Final validation is always last.

## KF-CORE-R03 — Runtime Foundation

Status: ACCEPTED (release gate PASS; tag `kritva-core-r0.3` created locally on the release-record commit; RELEASED after it is pushed)

### Objective

Implement the first concrete, platform-independent Kritva Core runtime foundation on top of the accepted R0.2 contracts.

### Task Order

```text
R03-001 Component Contract & Identity
        ↓
R03-002 Component Registry
        ↓
R03-003 Dependency Management
        ↓
R03 Foundation API Review
        ↓
R03-004 Runtime Manager
        ↓
R03-005 Runtime Lifecycle
        ↓
R03-006 Runtime Failure & Recovery
        ↓
R03 Runtime Contract Review
        ↓
R03-007 Runtime Integration Tests
        ↓
R03 Integration Freeze
        ↓
R03-008 Final Validation
        ↓
R03 Release Gate
```

### R03 Foundation Gate

After R03-003, the combined Component/Registry/Dependency public API must pass the **R03 Foundation API Review** before R03-004 begins.

The review also examines, but does not prematurely implement, the cross-cutting policy for:

- Error
- Warning
- Info/diagnostic messaging
- Event versus message
- Statistics update behavior
- logging backend boundary

### R03 Scope

R03 establishes:

- component contract and identity;
- component registry;
- dependency management;
- runtime manager;
- deterministic lifecycle orchestration;
- deterministic runtime failure/recovery semantics;
- runtime integration tests;
- final validation.

R03 remains platform independent. It does not introduce ROS2/DDS, EtherCAT implementation, vendor HAL/BSP, hardware drivers, AI/CV/SLAM, motion planning, robot skills, or OS-specific runtime execution.

## KF-CORE-R04 — Platform Abstraction

Status: PLANNED

Define validated platform integration boundaries for Linux, MCU/RTOS, Nexus and Edge implementations.
