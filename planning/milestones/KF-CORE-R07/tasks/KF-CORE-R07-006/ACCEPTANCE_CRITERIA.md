# KF-CORE-R07-006 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-006 |
| Status | ACCEPTED |
| Primary commit | `test(core): add component operational integration tests` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Prove through public APIs that Component operational information remains orthogonal to Runtime lifecycle orchestration.

## Scope

Integration tests covering status/health/statistics/event behavior together with initialize/start/stop/shutdown/reset, failure propagation, statistics separation and event sink ownership.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Runtime lifecycle behavior is identical with and without operational reporting.
- Health/status/statistics changes do not automatically alter Runtime state.
- Events do not invoke lifecycle/recovery automatically.
- Runtime failure/fault/reset semantics remain R0.3-compatible.
- Runtime-owned statistics remain distinct from Component statistics.
- No adapter/service ownership or platform lifecycle is introduced.

## New Tests

- Add focused unit/contract tests for each accepted semantic rule introduced by this task.
- Add negative/failure tests where applicable.
- Add mutation testing for contract-sensitive behavior.

## Regression Tests

- Complete existing CTest suite remains green.
- Existing R0.2–R0.6 contracts remain unchanged unless explicitly approved.

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Where applicable:

```bash
make check
make traceability-check
```

## Quality

- No unexplained regression from the accepted baseline.
- Public headers self-contained.
- Strict warnings/static analysis clean for affected code.
- Required sanitizer/coverage evidence supplied at milestone validation.

## Expected Files Changed

Implementation and tests only for this task, plus directly required traceability/documentation updates. No unrelated files.

## Git Commit

```text
test(core): add component operational integration tests
```

## Evidence Required from Implementor

- `git status`
- `git diff --check`
- build output
- test output
- focused test evidence
- mutation evidence where required
- diff/stat summary
- commit SHA
- explicit mapping from each acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `3f524cd` `test(core): add component operational integration tests`. **Test-only:** `git diff 16654e9 HEAD -- include src` is empty (API frozen at `6b1296f`, harness accepted at `0b1bd1d`). Changes: `tests/integration/component_operational_integration_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-OPS-007` + row), `API.md` section 43, `TESTING.md`.

- Public APIs only (RuntimeManager, Component, observe(), IComponentStatistics, ComponentEventReporter, IEventSink, the test-only harness and the test-only reference platform). No private access, no hardware.
- Tests (`kritva_core_component_operational_integration`, 9 functions), mapped to the acceptance criteria:
  1. **Lifecycle identical with and without operational reporting (seeded differential):** 150 seeds × 40 steps, random topologies, registration orders, injected failures and explicit resets; baseline = plain reference components; six operational variants (observation only; reporting and worst Status/Health/statistics only; both between steps; components acting inside their own operations; everything; everything with a sink that rejects every call). Every variant gives a transcript **identical** to the baseline (results, states, topology, faults, Runtime statistics, component invocation traces), and a RuntimeProbe taken around each piece of operational activity is unchanged (asserted in the hooks).
  2. **The Runtime consumes nothing operational:** over 100 seeds, through every operation, failure, fault and reset, the sink got 0 calls and no component's `status()`, `health()` or provider was read; no component statistic was adjusted or reset.
  3. **Status/Health do not alter Runtime state:** 25 rounds of worst reports in RUNNING change nothing; normal stop and shutdown follow; Runtime error_count stays 0. **A FAULT ends only by the explicit reset():** HEALTHY everywhere and a flood of events of every type and severity neither clear nor deepen it, nothing is retried (only sink calls appear after the fault in the shared ordered log), reset() then works and a new explicit initialize() succeeds (R0.3-compatible fault/reset semantics).
  4. **Events do not command and do not replace failures:** an ERROR/CRITICAL event reported from a failing start() leaves the Runtime's fault as the operation's Error (TIMEOUT, source = the component) and the event adds no Runtime error.
  5. **Order:** events reported inside operations appear in dependency order 1,2,3 on start and reverse 3,2,1 on stop, independent of registration order.
  6. **Statistics stay distinct:** component counters of 5,000,000/77 do not influence the Runtime's own count (error_count exactly 1, a failed call) and are never reset or adjusted; the provider was read once, by the observer.
  7. **No adapter/service ownership or platform lifecycle:** with a reference platform attached, observation, reporting and worst-case driving make no service call and no adapter query (call log empty, adapter_queries 0), and attachment stays closed afterwards.
  8. The reusable conformance checks hold for a component that lives through a Runtime and change nothing in it.
- Mutation evidence (Runtime production mutants, 10 tried, each reverted): a component's health or status altering a start, an error count, a sample/error count, or shutdown; automatic reset after a fault when a component reports HEALTHY; a stop or start run in the wrong order — **all consequential mutants detected**; two mutants were no-ops by construction (equivalent) and a reversed initialize order is not an operational concern and is killed by the existing R0.3 suites (4 of 51 tests fail), not by this file.
- Regression: ctest 51/51 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean (including the new integration test); coverage 618/625; `make check`, traceability 89 requirements, 88 traced, 0 errors (`CORE-OPS-007` defined with a row); `git diff --check` clean.
- Out of scope confirmed: no production change, no platform lifecycle, no concrete platform.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `3f524cd` (evidence `ceffe92`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: test-only (no production diff since `16654e9`); the seeded differential (150 seeds × 40 steps, six operational variants) gives transcripts identical to the plain baseline; the Runtime never consumes Status, Health, providers or sinks; Health never clears, deepens or triggers FAULT and explicit `reset()` remains the only recovery; Events never replace failures; Runtime and component statistics stay distinct; no adapter or service lifecycle is introduced; the reversed-initialize mutant is rightly owned by the R0.3 suites. `CORE-OPS-007` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R07-006 is ACCEPTED.**
