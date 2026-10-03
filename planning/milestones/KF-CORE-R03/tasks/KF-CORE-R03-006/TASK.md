# KF-CORE-R03-006 — Runtime Failure & Recovery

## Task Information
| Field | Value |
|---|---|
| Task ID | KF-CORE-R03-006 |
| Milestone | KF-CORE-R03 |
| Title | Runtime Failure & Recovery |
| Requirement | CORE-RT-008 |
| Status | BLOCKED BY R03-005 |
| Depends on | R03-005 accepted |
| Primary commit | `feat(core): define runtime failure handling` |

## Objective

Define and implement deterministic Runtime failure propagation and explicit caller-driven recovery/reset behavior.

A failed Component operation must remain observable through the returned Error and Runtime state. Recovery must never become an implicit retry loop or autonomous supervision mechanism.

## Scope

### In scope
- Component operation failure propagation.
- Runtime fault/error state semantics.
- Partial lifecycle progress tracking.
- Deterministic cleanup/rollback where required.
- Explicit caller-requested recovery/reset.
- Recovery preconditions and ordering.
- Failure/recovery documentation and tests.
- Error-source preservation.
- Runtime statistics behavior established by the Foundation gate.

### Out of scope
- Automatic retry.
- Background recovery.
- Watchdogs.
- Timers.
- Health-monitoring daemon.
- New logging backend.
- New generic warning API.
- Distributed fault management.
- Platform-specific fault handling.
- Scheduler/executor.

## Frozen Error Policy

- Existing Result/Error contract remains authoritative.
- Component-originated failures preserve the originating ComponentId as `Error.source`.
- Runtime must not replace a useful Component error with a generic Runtime error.
- Existing ErrorCode values are reused unless a genuine gap is separately approved.
- Core has no logging backend.

## Functional Requirements

### FR-01 Failure propagation
A required Component lifecycle failure is returned by Runtime and retains its originating source/code.

### FR-02 Fault state
A failed required lifecycle operation places Runtime in the documented fault/error state.

### FR-03 Partial progress
Runtime tracks sufficient progress to perform deterministic cleanup/recovery. It must never assume a failed operation completed.

### FR-04 Cleanup
Where cleanup is required by the accepted Runtime contract, it uses reverse dependency order, does not retry the failed operation implicitly, and does not destroy the original failure information.

### FR-05 Explicit recovery
Recovery requires explicit caller action. No automatic retry, watchdog, background worker or timer-driven recovery exists.

### FR-06 Recovery preconditions
Recovery is permitted only from documented Runtime states and follows lifecycle transitions allowed by the frozen Component contract.

### FR-07 Recovery semantics
The exact supported recovery/reset transitions must be documented before implementation is accepted. If `CORE-RT-002` does not define a required transition, the implementation must not silently invent one; this is an architecture-review item.

### FR-08 No duplicate attempts
Each lifecycle operation is attempted once per explicit orchestration request unless an accepted explicit recovery operation requests a new attempt.

### FR-09 Health/error distinction
Runtime error/fault state is distinct from `HealthState`. `DEGRADED` is not a warning API and health reporting never triggers recovery.

### FR-10 Statistics
Runtime-owned Statistics follow frozen R02 semantics. Statistics never become a hidden synchronization or recovery mechanism.

### FR-11 Events/diagnostics
If the accepted API exposes events/diagnostics, they remain values/records and do not introduce a delivery backend.

### FR-12 Threading
No autonomous recovery thread, watchdog, scheduler, executor or background worker is introduced.

## Required Tests

### Unit
- Error source preservation.
- Failure during initialize.
- Failure during start.
- Failure during stop.
- Failure during shutdown.
- Partial-progress behavior.
- Reverse-order cleanup.
- Explicit recovery.
- Recovery rejection from invalid state.
- No automatic retry.
- No duplicate lifecycle invocation.
- Error versus health distinction.
- Statistics update behavior where applicable.

### Integration
- Full Runtime + Registry + DependencyGraph failure path.
- Failure in a dependency.
- Failure in a dependent.
- Multiple dependency branches.
- Supported recovery after each accepted failure point.
- Existing R03-001..005 regression suite.

### Mutation Testing
At minimum:
- alter Error.source;
- swallow original error;
- replace original error with generic error;
- insert retry;
- wrong cleanup order;
- recover from invalid state;
- skip failed-component handling;
- duplicate lifecycle invocation;
- trigger recovery from health state.

Every mutation must be detected.

## Validation

Required:
- Debug
- Release
- ASan + UBSan
- TSan where configured
- `-Werror`
- `-fanalyzer`
- `make check`
- coverage reviewed against R03 baseline
- install-consumer
- `git diff --check`
- clean tree

## Documentation

Update:
- `REQUIREMENTS.md` — add `CORE-RT-008` only after implementation/review
- `ARCHITECTURE.md`
- `API.md`
- runtime failure/recovery state matrix
- task evidence

The documentation must explicitly distinguish:
- error;
- fault state;
- health;
- warning;
- diagnostic;
- event;
- application message.

## Evidence Required

Provide:
1. primary commit;
2. complete failure/recovery state matrix;
3. Error source preservation evidence;
4. failure invocation traces;
5. cleanup ordering evidence;
6. explicit recovery evidence;
7. proof that no automatic/background recovery exists;
8. tests/regression;
9. build/sanitizer/static-analysis;
10. coverage/traceability;
11. mutation testing;
12. install-consumer;
13. confirmation that foundation APIs remain unchanged.

## Acceptance

Reviewer decision only:
- PASS
- CHANGES REQUIRED
- BLOCKED

## Git

Exact implementation commit:

`feat(core): define runtime failure handling`
