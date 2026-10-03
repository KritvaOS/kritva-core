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

## 8a. Implementation Evidence (Claude)

- Primary commit: `e2b660d` `feat(core): implement runtime lifecycle orchestration` (R03-004 accepted at `d65b7a8`).
- Changed files: `include/kritva/core/runtime/runtime_manager.hpp` and `src/runtime_manager.cpp` (orchestration), `tests/unit/runtime_lifecycle_test.cpp` (new), `tests/unit/runtime_manager_test.cpp` (adapted, see below), `tests/contract/reference_component.hpp` (optional shared invocation `trace`, additive), `tests/install/consumer/main.cpp`, `CMakeLists.txt`, `REQUIREMENTS.md`, `ARCHITECTURE.md`, `API.md` (section 23 rewritten).
- **Frozen APIs unchanged (confirmed):** `git diff` is empty for `component.hpp`, `component_id.hpp`, `component_info.hpp`, `component_registry.hpp`, `dependency_graph.hpp`, `runtime.hpp`, `src/component_registry.cpp`, `src/dependency_graph.cpp`. `CORE-RT-002` / `runtime::Runtime` is unchanged. The only public addition is `RuntimeManager::configure(const Configuration&)`.
- **`CORE-RT-007` not defined** (per the task: only after review); `grep RT-007` outside `planning/` finds nothing; the new test is tagged `CORE-RT-006`. One consequence to review: the accepted `CORE-RT-006` text said the manager "never ... calls a component"; that sentence is now false by design, so I changed it to say it never owns, copies or deletes a component and that invoking lifecycle operations is "specified separately from this requirement". The R03-004 header, `ARCHITECTURE.md` and `API.md` statements that no component is called were rewritten accordingly. The R03-004 tests `test_runtime_never_drives_components` (replaced by `test_setup_failed_validation_and_invalid_calls_invoke_no_component`, which keeps the real boundary: failed validation, rejected setup and invalid calls invoke nothing) and the probe-call assertion in `test_runtime_does_not_own_components` (now expects exactly 4 calls) were adapted; every other R03-004 assertion is unchanged.
- **Runtime state / transition matrix** (runtime state only changes after the whole sequence succeeded):

| Operation | Valid from | Components invoked | Order | On success | On first component failure |
|---|---|---|---|---|---|
| `configure(cfg)` | UNKNOWN, STOPPED | `configure` | forward | state unchanged | state unchanged |
| `initialize` | UNKNOWN, STOPPED | `initialize` | forward | READY | FAULT |
| `start` | READY | `start` | forward | RUNNING | FAULT |
| `stop` | READY, RUNNING | `stop` | reverse | STOPPED | FAULT |
| `shutdown` | UNKNOWN, STOPPED | `shutdown` (live only) | reverse | state unchanged | state unchanged |

  Every other (operation, state) pair: `INVALID_STATE`, no effect, no component invoked. FAULT: every operation `INVALID_STATE` until R03-006.
- **Component invocation traces** (topology `1->3, 2->3, 3->4, 5->2`; forward `4,3,1,2,5`; reverse `5,2,1,3,4`), asserted exactly by the tests: `initialize` -> `4:initialize 3:initialize 1:initialize 2:initialize 5:initialize`; `stop` -> `5:stop 2:stop 1:stop 3:stop 4:stop`; a full cycle is 20 invocations, each component exactly once per operation. Failure example: `configure` failing at component 1 gives `4:configure 3:configure 1:configure` and nothing after.
- Decisions (the contract and `CORE-RT-002` leave these open; for reviewer confirmation):
  1. **`configure`** is a `RuntimeManager` operation (the `Runtime` interface has none). All components get the same `Configuration`, in forward order. It leaves the runtime state unchanged (the documented state), does not initialize/start, and does **not** fix the topology (it uses the current order; components registered later are not configured). With an invalid topology it returns the graph error unchanged and invokes nothing.
  2. **Fail-fast, no rollback, no retry:** the first failing component ends the sequence; later components are not invoked; completed ones keep their state (a failed `initialize` leaves earlier components READY and later ones UNKNOWN). The component's own `Error` (code, severity, source, message) is returned unchanged.
  3. **FAULT:** a failed `initialize`/`start`/`stop` moves the runtime to FAULT (edges exist in the Core table); in FAULT even `shutdown()` is `INVALID_STATE`, because cleaning up a partially progressed runtime is R03-006's recovery/reset. A failed `configure`/`shutdown` leaves the state unchanged.
  4. **Repeated calls (AC-09):** repeated `start`/`initialize`/`stop` are invalid per the existing matrix and invoke nothing. `shutdown` tracks whether components are live (initialized and not shut down since): never-initialized or already-shut-down runtimes are a no-op that invokes nothing, so no duplicate invocation. **Ambiguity for review:** a *failed* `shutdown` may be retried and then invokes every live component again in reverse (relying on `Component::shutdown` idempotence in STOPPED); I did not add per-component progress tracking because partial-progress policy is R03-006.
  5. The order used is the one validated and cached at the first `initialize()`; it is never recomputed.
  6. The runtime state is `INITIALIZING`/`STOPPING` while component calls run (observable only from inside a component call) and never `RUNNING` before all starts succeeded; a test with an observing component asserts exactly this.
- Tests: new CTest `kritva_core_runtime_lifecycle` (18 test functions): forward and reverse orders with exact traces, full-cycle trace and counts, state seen by components during each operation (no premature RUNNING), order independence over 24 edge-insertion orders combined with registration permutations, independent branches/no-edges/empty/chain, invalid and repeated calls in every state invoke nothing, repeated shutdown, stop then re-initialize, configure semantics and its failure modes, **failure injection at every position of every operation** (5 operations x 5 positions: error unchanged, exact invocation prefix, runtime state, per-component call counts), FAULT rejects every operation without invoking anything, partial progress without rollback, failed-shutdown retry, validation failure invokes nothing, no retry/background activity.
- **Mutation evidence** (each temporary edit reverted; file verified identical; every one aborted a test): start in reverse order; stop in forward order; shutdown in forward order; registration-order substitution; last component skipped; duplicate invocation; premature RUNNING; invalid transition accepted (`start` from RUNNING); automatic retry; continue after failure; failure not moving to FAULT; component error rewrapped; shutdown flag never cleared; shutdown invoking never-initialized components; `configure` fixing the topology; `initialize` skipping validation; failed `initialize` recovering to READY.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **23/23** (22 prior + `kritva_core_runtime_lifecycle`). Release 23/23; ASan+UBSan 23/23; TSan (ASLR disabled) 23/23; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 23/23; GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer: passed; the consumer now also runs `start()`, `stop()` and `shutdown()` on the installed `RuntimeManager`, orchestrating a stub component.
- `make check` passed (traceability 54 requirements, 53 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean. No threading headers in `include/` or `src/`.
- Coverage: `make coverage` — 98% (397/402, the R03 baseline). `src/runtime_manager.cpp` is 85/86; the one uncovered line is the unreachable `return Result<void>::success();` after the exhaustive `switch` in the invocation lambda.
- Final `git status --short`: clean after the commit.
- Known limitations / out of scope by design: no recovery or reset (R03-006); FAULT is terminal for a runtime object in this task; no per-component progress tracking; no thread safety; no real-time claim; `configure` gives every component the same `Configuration`.

## 9. Reviewer Sign-off

Only the independent architecture reviewer records PASS / CHANGES REQUIRED / BLOCKED.

## 10. Git Commit

`feat(core): implement runtime lifecycle orchestration`
