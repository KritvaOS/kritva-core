# KF-CORE-R05 — Platform Runtime Integration Foundation

## Status

PLANNED — Architecture Proposal Approved

## Target

- Version: `0.5.0`
- Release tag: `kritva-core-r0.5`

## Objective

Establish a controlled, platform-independent mechanism by which Kritva Core functionality can explicitly consume externally owned platform services while preserving Runtime determinism, platform ownership, lifetime rules, and the R0.4 platform contracts.

R0.5 is an integration-foundation milestone. It does not implement Linux, PREEMPT_RT, RTOS, MCU, vendor, Nexus, Edge, EtherCAT, ROS2/DDS, or hardware services.

## Architectural Principle

> Kritva Core defines what platform services mean and how Core may consume them; the platform adapter defines how those services are implemented and executed. Core never owns the platform implementation.

## Approved Architectural Decisions

1. `IPlatformAdapter` remains the authoritative platform boundary.
2. R0.5 introduces a small non-owning `PlatformContext` view; it is not a service registry or service locator.
3. Platform service ownership and lifecycle remain external to Core.
4. Platform service use is explicit; attaching an adapter does not start, stop, create, or configure services.
5. Capability requirements are declarative and capability-driven; Core does not infer support from platform names or versions.
6. Components do not receive a raw `IPlatformAdapter*` as their general-purpose platform interface.
7. R0.5 does not change the R0.3 Runtime lifecycle state machine.
8. No Core-owned background thread, worker pool, scheduler, timer loop, watchdog recovery, or automatic retry is introduced.
9. No generic `ServiceRegistry` is introduced.
10. Concrete platform adapters remain outside `kritva-core`.

## Scope

- PlatformContext definition and ownership/lifetime semantics.
- Platform service access semantics.
- Platform service requirement/capability model.
- Explicit scheduler/clock/timer/watchdog consumption rules.
- Runtime/platform lifecycle separation.
- Reference platform integration harness.
- Platform-aware Runtime integration tests.
- Full regression and release validation.

## Out of Scope

- Concrete OS/RTOS/vendor adapters.
- Hardware drivers.
- ROS2/DDS/EtherCAT.
- PTP implementation.
- Automatic Runtime recovery from watchdog expiry.
- Automatic platform service startup/shutdown.
- Core-owned threads or background execution.
- Generic service locator/registry.

## Task Sequence

```text
R05-001 Platform Context & Service Access Model
        ↓
R05-002 Platform Service Requirement Model
        ↓
R05-003 Explicit Platform Service Consumption
        ↓
R05-004 Runtime–Platform Lifecycle Boundary
        ↓
R05 Platform API Review
        ↓
R05-005 Reference Platform Integration
        ↓
R05-006 Platform Integration & Runtime Tests
        ↓
R05 Platform Integration Freeze
        ↓
R05-007 Full R0.5 Validation
        ↓
R05 Release Gate
```

## Gates

| Gate | Entry | Purpose |
|---|---|---|
| R05 Platform API Review | R05-004 accepted | Freeze PlatformContext, service access, requirements and lifecycle boundary |
| R05 Platform Integration Freeze | R05-006 accepted | Freeze production platform-integration API and semantics |
| R05 Release Gate | R05-007 accepted | Final validation and release authorization |

## Exit Criteria

- All seven tasks accepted.
- Platform API Review PASS / FROZEN.
- Platform Integration Freeze PASS / HONORED.
- All R0.4 public contracts preserved unless explicitly approved.
- Unit tests for every new production contract pass.
- Integration tests exercise Runtime with and without a reference platform.
- Full regression suite passes.
- Debug and Release builds pass.
- ASan/UBSan pass.
- TSan passes where configured.
- `-Werror` passes.
- GCC `-fanalyzer` passes where configured.
- Coverage reviewed.
- Traceability reports zero errors.
- Install-consumer remains green.
- Prohibited dependency scan remains green.
- No concrete platform implementation exists in Core.
- No Core-owned background execution exists.
- Documentation and changelog are reconciled.
- Working tree is clean before release tag.
- Annotated `kritva-core-r0.5` tag is created only after independent reviewer PASS.
