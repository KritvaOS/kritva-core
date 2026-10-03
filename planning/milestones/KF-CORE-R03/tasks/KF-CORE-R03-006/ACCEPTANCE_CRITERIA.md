# KF-CORE-R03-006 — Acceptance Criteria

## 1. Task Information

- **Task:** KF-CORE-R03-006
- **Title:** Runtime Failure & Recovery
- **Milestone:** KF-CORE-R03
- **Requirement:** CORE-RT-008
- **Status:** BLOCKED BY R03-005
- **Dependency:** R03-005 accepted
- **Implementation commit:** `feat(core): define runtime failure handling`

## 2. Objective

Implement deterministic Runtime failure propagation and explicit caller-driven recovery/reset semantics.

Failure must preserve the originating Component error. Recovery must never become implicit retry or autonomous supervision.

## 3. Scope

### In scope
- Component failure propagation.
- Runtime FAULT/error semantics.
- Partial lifecycle progress.
- Deterministic cleanup/rollback.
- Explicit recovery/reset.
- Recovery preconditions and ordering.
- Error-source preservation.
- Runtime statistics semantics established at the Foundation gate.

### Out of scope
- Automatic retry.
- Background recovery.
- Watchdogs/timers.
- Health-monitoring daemon.
- Logging backend.
- New generic warning API.
- Distributed fault management.
- Platform-specific fault handling.
- Scheduler/executor.

## 4. Frozen Error Policy

- Existing Result/Error contract remains authoritative.
- Component-originated failures preserve ComponentId as Error.source.
- Runtime does not replace useful Component errors with generic errors.
- Existing ErrorCodes are reused unless a genuine gap is separately approved.
- Core has no logging backend.

## 5. Acceptance Criteria

### AC-01 — Failure propagation
- Required Component failure is returned by Runtime.
- Returned Error retains originating source and code.

### AC-02 — Fault state
- Failed required operation places Runtime in the documented fault/error state.
- Runtime never reports success after an unsuccessful required operation.
- Failed Component follows frozen Component semantics.

### AC-03 — Partial progress
- Runtime tracks enough progress for deterministic cleanup/recovery.
- A failed operation is never assumed to have completed.

### AC-04 — Cleanup
- Required cleanup is deterministic.
- Cleanup uses reverse dependency order where lifecycle teardown is required.
- Cleanup does not retry the failed operation.
- Original failure remains observable.

### AC-05 — Explicit recovery
- Recovery requires explicit caller action.
- No automatic retry.
- No background worker, watchdog, scheduler or timer-driven recovery.
- Recovery has documented preconditions.

### AC-06 — Recovery transitions
- Recovery uses only transitions permitted by the accepted Component lifecycle contract.
- Failure/recovery behavior is explicitly specified for initialize/start/stop/shutdown failures.
- If `CORE-RT-002` lacks a required recovery transition, implementation must return the ambiguity to architecture review rather than invent semantics.

### AC-07 — No duplicate attempts
- One lifecycle operation is attempted once per explicit request.
- A new attempt is distinguishable as explicit recovery.

### AC-08 — Health/error distinction
- Runtime error/fault state is distinct from HealthState.
- `DEGRADED` is not treated as a warning API.
- Health reporting never triggers recovery.

### AC-09 — Statistics
- Runtime-owned Statistics follow frozen R02 semantics.
- Statistics never act as synchronization or recovery control.

### AC-10 — Events/diagnostics
- If exposed, events/diagnostics remain structured values/records.
- No delivery backend is introduced.

### AC-11 — Threading
- No autonomous recovery thread.
- No watchdog.
- No timer-driven retry.
- No scheduler/executor dependency.

### AC-12 — Documentation
- `CORE-RT-008` is added to authoritative requirements only after review.
- Complete failure/recovery state matrix is documented.
- Error, fault, health, warning, diagnostic, event and message distinctions are documented.

## 6. Required Tests

### Unit
- Error source preservation.
- Initialize failure.
- Start failure.
- Stop failure.
- Shutdown failure.
- Partial progress.
- Reverse-order cleanup.
- Explicit recovery.
- Recovery rejected from invalid state.
- No automatic retry.
- No duplicate lifecycle invocation.
- Error/health distinction.
- Statistics behavior where applicable.

### Integration
- Runtime + Registry + DependencyGraph failure path.
- Dependency failure.
- Dependent failure.
- Multiple dependency branches.
- Supported recovery paths.
- Full prior R03 regression suite.

### Mutation
At minimum detect:
- altered Error.source;
- swallowed error;
- generic error replacement;
- inserted retry;
- wrong cleanup order;
- invalid-state recovery;
- skipped failure handling;
- duplicate invocation;
- recovery triggered by health state.

## 7. Build/Test/Quality Gates

Debug, Release, ASan/UBSan, TSan where configured, `-Werror`, `-fanalyzer`, `make check`, coverage baseline, install-consumer, `git diff --check`, clean tree.

## 8. Evidence Required

- Primary commit.
- Complete failure/recovery state matrix.
- Error-source evidence.
- Failure invocation traces.
- Cleanup order evidence.
- Explicit recovery evidence.
- Proof of no automatic/background recovery.
- Test/quality results.
- Coverage/traceability.
- Mutation results.
- Install-consumer.
- Foundation API unchanged.

## 9. Reviewer Sign-off

Only the independent architecture reviewer records PASS / CHANGES REQUIRED / BLOCKED.

## 10. Git Commit

`feat(core): define runtime failure handling`
