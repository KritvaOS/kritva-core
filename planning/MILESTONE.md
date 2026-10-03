# Kritva Core — Master Milestone Roadmap

## Milestone Lifecycle

PLANNED → IN PROGRESS → IMPLEMENTATION COMPLETE → REVIEW → ACCEPTED → RELEASED

## KF-CORE-R02 — Core Contract Hardening

Status: RELEASED (`kritva-core-r0.2`)

## KF-CORE-R03 — Runtime Foundation

Status: RELEASED (`kritva-core-r0.3`)

## KF-CORE-R04 — Platform Abstraction

Status: RELEASED (`kritva-core-r0.4`)

R0.4 established stable platform contracts and the Runtime/platform integration boundary without implementing concrete platforms.

## KF-CORE-R05 — Platform Runtime Integration Foundation

Status: RELEASED (`kritva-core-r0.5` -> `adf8ac2`, published to origin)

### Objective

Establish a controlled, platform-independent mechanism by which Kritva Core functionality can explicitly consume externally owned platform services while preserving Runtime determinism, platform ownership and the R0.4 contracts.

### Task Order

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

### Architectural Rules

- `IPlatformAdapter` remains the authoritative platform boundary.
- `PlatformContext` is a non-owning view, not a service registry.
- Platform service ownership/lifecycle remains external.
- Service use is explicit.
- Capability identity is authoritative.
- Runtime lifecycle semantics remain unchanged.
- No Core-owned background execution.
- No concrete platform implementation in `kritva-core`.
- Breaking API/semantic changes require architecture review.

See `planning/milestones/KF-CORE-R05/` for the complete proposal and task acceptance package.
