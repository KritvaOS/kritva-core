# KF-CORE-R03 — Runtime Foundation

## Status

IN PROGRESS

Current progress: 3 / 8 tasks accepted

## Objective

Implement the first concrete, platform-independent Kritva Core runtime foundation on top of the R0.2 contracts.

R03 establishes:

1. Component contract and identity
2. Component registry
3. Dependency management
4. Runtime manager
5. Deterministic lifecycle orchestration
6. Runtime failure/recovery semantics
7. Runtime integration tests
8. Final validation

## R03 Foundation Scope

The first foundation gate consists of:

- KF-CORE-R03-001 — Component Contract & Identity
- KF-CORE-R03-002 — Component Registry
- KF-CORE-R03-003 — Dependency Management

After R03-003, the R03 Foundation API Review must PASS before R03-004 begins.

## R03 Gates

| Gate | After | Purpose |
|---|---|---|
| R03 Foundation API Review | 003 | Freeze Component/Registry/Dependency contracts |
| R03 Runtime Contract Review | 006 | Freeze Runtime/Lifecycle/Error semantics |
| R03 Integration Freeze | 007 | No production API changes during final validation |
| R03 Release Gate | 008 | Final acceptance and release tag |

## Out of Scope

R03 does not implement:

- OS-specific runtime execution
- scheduler/executor
- thread pools/background workers
- ROS2/DDS
- EtherCAT implementation
- vendor BSP/HAL
- sensor drivers
- motor-control algorithms
- AI/CV/SLAM
- motion planning
- robot skills
- automatic background recovery

## Exit Criteria

R03 is complete only when:

- all eight tasks are accepted;
- foundation and runtime API gates are PASS;
- integration tests pass;
- Debug and Release builds pass;
- ASan and UBSan pass;
- `-Werror` passes;
- coverage requirement is satisfied;
- traceability and dependency checks pass;
- install-consumer regression remains green;
- no prohibited platform dependencies are introduced;
- final reviewer sign-off is recorded;
- release tag is created only after acceptance.

## Runtime Phase Decisions

The Foundation API Review passed and froze the Component, Registry, and DependencyGraph contracts.

### R03-004 — Runtime Manager
- Implements the existing authoritative `CORE-RT-002` / `runtime::Runtime` interface.
- Does not introduce a competing public Runtime abstraction.
- Treats the component set as fixed after runtime initialization.
- Composes the non-owning registry and dependency graph.
- Performs topology validation before lifecycle execution.
- Remains synchronous and platform independent.

### R03-005 — Runtime Lifecycle
- Owns deterministic runtime lifecycle orchestration.
- Forward lifecycle operations use dependency order.
- Stop/shutdown use reverse dependency order.
- Does not redefine the accepted Component lifecycle contract.
- Does not introduce automatic retry or background recovery.

### R03-006 — Runtime Failure & Recovery
- Preserves originating Component error code/source.
- Defines deterministic fault and partial-progress behavior.
- Recovery is explicit caller-driven behavior only.
- No automatic retry, watchdog, background worker, or timer-driven recovery.
- Health is distinct from error/warning semantics.

The Runtime Contract Review after R03-006 freezes these runtime semantics before R03-007.
