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

## KF-CORE-R06 — Component Execution Context

Status: PLANNED — Architecture Proposal

### Objective

Provide integrator-written Components with one explicit, deterministic, non-owning context for accessing approved operational services while preserving the R0.3 Runtime lifecycle semantics and the R0.5 platform ownership boundary.

### R0.6 Scope Status

R0.6 is an architecture proposal only. Implementation authorization begins only after the architecture/design review confirms the direction and the R06 task package is accepted for execution.

### Task Order

```text
R06-001 Component Execution Context & Ownership Model
        ↓
R06-002 Operational Context Services & Access Policy
        ↓
R06-003 Context Injection Without Runtime Lifecycle Change
        ↓
R06-004 Context Requirements & Capability Binding
        ↓
R06 Component API Review
        ↓
R06-005 Reference Context Harness & Contract Tests
        ↓
R06-006 Runtime/Component Context Integration Tests
        ↓
R06 Integration Freeze
        ↓
R06-007 Full R0.6 Validation
        ↓
R06 Release Gate
```

### R0.6 Architectural Rules

- R0.5 remains authoritative.
- `IPlatformAdapter` and `PlatformContext` are not replaced by R0.6.
- Context is not a service registry or locator.
- Core does not acquire ownership of platform services or integrator resources.
- Context access is explicit and has no hidden lifecycle side effects.
- Runtime lifecycle semantics remain unchanged unless an explicit architecture review approves a change.
- Integration tests use public APIs only.
- No concrete Linux/RTOS/MCU/vendor/Nexus/Edge platform enters `kritva-core`.
- No Core-owned background execution or automatic recovery is introduced.
- Any breaking or semantic API change returns to architecture review before implementation continues.

See `planning/milestones/KF-CORE-R06/` for the R0.6 architecture proposal, task package and gate definitions.
