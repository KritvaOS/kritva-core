# KF-CORE-R03-005 — Acceptance Criteria

## 1. Task Information

- **Task:** KF-CORE-R03-005
- **Title:** Runtime Lifecycle
- **Milestone:** KF-CORE-R03
- **Requirement:** CORE-RT-007
- **Status:** BLOCKED BY R03-004
- **Dependency:** R03-004 accepted
- **Implementation commit:** `feat(core): implement runtime lifecycle orchestration`

## 2. Objective

Implement deterministic Runtime lifecycle orchestration over the accepted Runtime Manager.

Forward lifecycle operations use dependency order. Stop/shutdown use reverse dependency order. The accepted Component lifecycle contract is not changed.

## 3. Scope

### In scope
- Runtime lifecycle state machine.
- Configure/initialize/start/stop/shutdown.
- Dependency-aware forward ordering.
- Reverse dependency-aware teardown.
- Deterministic invocation.
- Runtime state transitions.
- Invalid lifecycle operation handling.
- Explicit repeated-call behavior where defined by `CORE-RT-002`.

### Out of scope
- Component API changes.
- Registry/DependencyGraph changes.
- Automatic retry/recovery.
- Background monitoring.
- Threads/executors.
- Dynamic topology.
- Platform-specific behavior.
- Full recovery policy owned by R03-006.

## 4. Frozen Component Lifecycle

| Operation | Semantics |
|---|---|
| configure | valid UNKNOWN/STOPPED; no Component state change |
| initialize | UNKNOWN/STOPPED → READY |
| start | READY → RUNNING |
| stop | READY/RUNNING → STOPPED |
| shutdown | UNKNOWN/STOPPED no-op; FAULT → STOPPED |

Invalid Component operations return `INVALID_STATE` without changing Component state.

## 5. Acceptance Criteria

### AC-01 — Forward ordering
- Configure/initialize/start invoke Components in dependency order where applicable.
- Each required Component is invoked exactly once for a successful operation.
- Dependencies precede dependents.

### AC-02 — Reverse ordering
- Stop/shutdown invoke Components in reverse dependency order.
- Dependents are torn down before dependencies.

### AC-03 — Runtime state
- Runtime state transitions are explicit and deterministic.
- Runtime is never RUNNING before all required starts succeed.
- A failed required operation cannot produce a successful Runtime transition.

### AC-04 — Configure
- Configure does not implicitly initialize or start.
- Successful configure reaches the documented Runtime state.

### AC-05 — Initialize
- Valid only from its documented Runtime state.
- Components initialize in dependency order.
- Successful completion reaches the documented ready/initialized state.

### AC-06 — Start
- Requires successful initialization.
- Components start in dependency order.
- Runtime becomes RUNNING only after all starts succeed.

### AC-07 — Stop
- Explicit operation.
- Components stop in reverse dependency order.
- Does not implicitly restart/reinitialize.

### AC-08 — Shutdown
- Explicit operation.
- Components shut down in reverse dependency order.
- Leaves Runtime in its documented stopped/terminal state.

### AC-09 — Repeated calls
- Repeated lifecycle calls follow `CORE-RT-002`.
- No duplicate Component invocation unless the accepted contract explicitly requires it.
- Ambiguous behavior is returned to architecture review rather than invented silently.

### AC-10 — Call discipline
- Runtime invokes only the lifecycle operation being orchestrated.
- Runtime never directly manipulates Component state.

### AC-11 — No automatic recovery
- No automatic retry.
- No background recovery.
- No watchdog/timer/executor.

### AC-12 — Documentation
- `CORE-RT-007` is added to authoritative requirements only after review.
- Lifecycle state/transition and ordering semantics are documented.

## 6. Required Tests

### Unit
- Runtime state transition matrix.
- Configure order.
- Initialize order.
- Start order.
- Stop reverse order.
- Shutdown reverse order.
- Invalid Runtime lifecycle calls.
- Repeated calls.
- Exact Component invocation counts.
- Independent dependency branches.
- Ordering permutation tests.

### Failure boundary
Inject failure at each lifecycle operation and verify:
- returned error;
- source;
- invocation order;
- completed progress;
- Runtime state.

Detailed recovery is R03-006.

### Regression
All previous R03 tests remain green.

### Mutation
Detect at minimum:
- wrong ordering direction;
- registration-order substitution;
- skipped operation;
- duplicate invocation;
- premature RUNNING;
- invalid transition accepted;
- automatic retry.

## 7. Build/Test/Quality Gates

Debug, Release, ASan/UBSan, TSan where configured, `-Werror`, `-fanalyzer`, `make check`, coverage baseline, install-consumer, `git diff --check`, clean tree.

## 8. Evidence Required

- Primary commit.
- Runtime state/transition matrix.
- Component invocation traces.
- Forward/reverse ordering evidence.
- Failure-boundary evidence.
- Tests and all quality checks.
- Coverage and traceability.
- Mutation results.
- Confirmation that foundation APIs are unchanged.

## 9. Reviewer Sign-off

Only the independent architecture reviewer records PASS / CHANGES REQUIRED / BLOCKED.

## 10. Git Commit

`feat(core): implement runtime lifecycle orchestration`
