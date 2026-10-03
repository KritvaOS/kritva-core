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

## 8a. Implementation Evidence (Claude)

- Primary commit: `ee3d55d` `feat(core): define runtime failure handling` (R03-005 accepted at `3623a26`).
- Changed files: `include/kritva/core/runtime/runtime_manager.hpp`, `src/runtime_manager.cpp`, `tests/unit/runtime_failure_test.cpp` (new), `tests/install/consumer/main.cpp`, `CMakeLists.txt`, `REQUIREMENTS.md` (traceability row for `CORE-RT-007` extended; the "until recovery is defined" clause reworded because recovery is now defined), `ARCHITECTURE.md` ("Runtime failure and recovery"), `API.md` (section 24).
- **Frozen APIs unchanged (confirmed):** `git diff` is empty for `component.hpp`, `component_id.hpp`, `component_info.hpp`, `component_registry.hpp`, `dependency_graph.hpp`, `runtime.hpp`, `src/component_registry.cpp`, `src/dependency_graph.cpp`. `CORE-RT-002` is unchanged. No new `ErrorCode`.
- **`CORE-RT-008` not defined** (per the task: only after review); `grep RT-008` outside `planning/` finds nothing; the new test is tagged `CORE-RT-007`.
- Public additions on `RuntimeManager`: `Result<void> reset()`, `const Error* fault_error() const noexcept`, `const Statistics& statistics() const noexcept`.
- **Complete failure/recovery state matrix** (full text in `ARCHITECTURE.md` and `runtime_manager.hpp`):

| Event | Runtime state | Components | Observable |
|---|---|---|---|
| `initialize`/`start`/`stop` fails at component C | FAULT | earlier keep state, C in FAULT, later not invoked | the component's own `Error`; `fault_error()` |
| `configure` or `shutdown` fails | unchanged | sequence ends at the failure; shutdown progress kept | the component's own `Error` |
| any operation except `reset()` in FAULT | FAULT | none invoked | `INVALID_STATE` |
| `reset()` outside FAULT | unchanged | none invoked | `INVALID_STATE` |
| `reset()` succeeds | STOPPED | pass 1: stop initialized/started (reverse); pass 2: shutdown stopped/faulted (reverse) | `fault_error()` null |
| `reset()` fails at a cleanup step | FAULT | progress kept, no successful step repeated; a failed stop is recorded faulted | cleanup `Error`; `fault_error()` still the original |

- **Recovery transitions (AC-06):** only `FAULT -> STOPPED`, an edge of the frozen Core table, via the frozen Component operations `stop()` and `shutdown()` (a faulted component leaves FAULT only through `shutdown()`). `FAULT -> RECOVERING -> READY` exists in the table but is NOT used: no Component operation produces it, and I did not invent one. Re-establishing READY after a reset is the caller's explicit `initialize()`, a new attempt. **Possible architecture-review item:** if a reviewer wants recovery to go directly to READY without releasing components, that needs a new Component operation.
- Decisions for reviewer confirmation: (1) the recovery operation is named `reset()` and ends in STOPPED, not READY; (2) the runtime records per-component stages itself (none, initialized, started, stopped, faulted, shut down) per live period, extending the R03-005 shutdown-progress model, and never reads component state or health (the health probe in the tests counts zero observer calls); (3) cleanup is two passes, each in reverse dependency order, instead of per-component stop-then-shutdown; (4) a cleanup failure keeps the runtime in FAULT with `fault_error()` still the original; a failed stop during cleanup is recorded as faulted so the next `reset()` shuts the component down instead of stopping it again; (5) statistics semantics: `sample_count` = successful component invocations, `error_count` = failed ones including cleanup, `retry_count` never incremented, other fields unused; (6) `fault_error()` returns a pointer to the stored original `Error` (valid until the next state change).
- Failure invocation traces (topology `1->3, 2->3, 3->4, 5->2`; forward `4,3,1,2,5`, reverse `5,2,1,3,4`), asserted exactly: dependency failure `3:initialize` fails -> `4:initialize 3:initialize`, then `reset()` -> `4:stop 3:shutdown 4:shutdown`; dependent failure `5:initialize` fails -> `reset()` -> `2:stop 1:stop 3:stop 4:stop 5:shutdown 2:shutdown 1:shutdown 3:shutdown 4:shutdown`; failed cleanup stop at 2 after a failed start at 1 -> `5:stop 2:stop` (FAULT kept), retry -> `3:stop 4:stop 5:shutdown 2:shutdown 1:shutdown 3:shutdown 4:shutdown` (no repeated stop of 5 or 2).
- Tests: new CTest `kritva_core_runtime_failure` (11 test functions): propagation, error preservation and `fault_error()` over all 15 failure points (3 operations x 5 positions); `reset()` cleanup trace against an independent stage model at all 15 points, resulting component states, failed operation not retried, and a successful re-`initialize()`/`start()` afterwards; `reset()` valid only in FAULT; FAULT blocks everything except `reset()`; failed cleanup stop and failed cleanup shutdown (original fault kept, no duplicate attempts, resume); nothing happens without an explicit request (1000 observations, no calls, `retry_count` 0); health never triggers recovery and the runtime never calls `health()`/`status()`/`lifecycle_state()`/`capabilities()`; statistics updates; dependency-failure and dependent-failure scenarios.
- **Mutation evidence** (each temporary edit reverted; file verified identical; all detected): altered `Error.source`; original error swallowed; generic error replacement; automatic retry of the failed operation; forward (wrong) cleanup order; `reset()` allowed from invalid states; faulted component skipped in cleanup; duplicate invocation in cleanup; recovery triggered by component health; fault not recorded; fault cleared by a failed `reset()`; recovery to READY through RECOVERING; failed stop not recorded as faulted; failure counted as success; failed cleanup stop retried. (An earlier health mutation placed only inside `reset()` was unreachable by the test, so a reachable one, the runtime faulting after `start()` when a component reports UNHEALTHY, was used and detected.)
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **24/24** (23 prior + `kritva_core_runtime_failure`). Release 24/24; ASan+UBSan 24/24; TSan (ASLR disabled) 24/24; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 24/24 (the strict build caught an unused helper in the new test; removed); GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer: passed; it also checks that `reset()` is rejected outside FAULT and that the installed runtime's `statistics()` counts invocations.
- `make check` passed (traceability 55 requirements, 54 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean. No `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<semaphore>`, `<iostream>` or `<cstdio>` in `include/` or `src/`: no automatic or background recovery exists.
- Coverage: `make coverage` — 98% (434/439, the R03 baseline); `src/runtime_manager.cpp` is 120/121; the one uncovered line is the unreachable return after the exhaustive `switch` in `call()`.
- Final `git status --short`: clean after the commit.
- Known limitations / out of scope: no recovery directly to READY (needs a new Component operation, an architecture-review matter); no events or logging; `fault_error()` and `statistics()` expose references without synchronization; no thread safety; no real-time claim.

## 9. Reviewer Sign-off

Only the independent architecture reviewer records PASS / CHANGES REQUIRED / BLOCKED.

## 10. Git Commit

`feat(core): define runtime failure handling`
