# KF-CORE-R08-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-005 |
| Status | ACCEPTED |
| Primary commit | `test(core): add configuration runtime integration tests` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 3–4 ED |
| Dependency | R08-004 accepted |

## Objective

Prove the Runtime/Component configuration boundary using public APIs and the established deterministic Runtime ordering.

## Proposed Requirements Traceability

CORE-CFG-009, CORE-CFG-010

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Runtime forwards the same logical configuration to Components in dependency order.
- Configuration does not alter Runtime lifecycle state on success or failure.
- First configuration failure stops further configuration calls.
- The failing Component error is propagated unchanged according to the established Runtime error boundary.
- Successful earlier Components are not automatically rolled back.
- Configuration is not retried automatically.
- Configuration failure does not enter Runtime `FAULT`.
- Runtime does not read Component Status or Health to decide configuration behavior.
- ComponentContext is not created, modified or consulted by Runtime as part of configuration.
- Registration/topology semantics remain consistent with the frozen Runtime model.
- Randomized/permuted dependency and registration order confirms deterministic invocation order.
- Mutation testing detects Runtime state transition, retry, rollback and ordering defects.

## New Tests

- End-to-end RuntimeManager configuration scenarios.
- Failure propagation/fail-fast tests.
- No-rollback/no-retry tests.
- Status/Health independence tests.
- Registration/dependency permutation or randomized deterministic-order tests.
- Differential tests against the pre-R08 Runtime lifecycle behavior where applicable.

## Regression Tests

- All R0.3 Runtime lifecycle/failure/reset regression tests.
- All R0.4–R0.7 platform/context/operational integration tests.

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

## Coverage / Quality

- Newly added executable lines must be covered.
- No unexplained coverage exclusion.
- Overall R08 coverage remains at least 98% and has no unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.
- Required sanitizer and strict-analysis evidence is supplied at milestone validation.

## Expected Files Changed

Only files directly required for this task, its tests, and traceability/documentation reconciliation. No unrelated files.

## Git Commit

```text
test(core): add configuration runtime integration tests
```

## Evidence Required from Implementor

- `git status` before and after implementation
- `git diff --check`
- build and test output
- focused test evidence
- mutation evidence for contract-sensitive behavior
- diff/stat summary
- commit SHA
- explicit mapping from every acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `a5dfbc1` `test(core): add configuration runtime integration tests`. **Test-only:** `git diff bdb4b93 HEAD -- include src` is empty (production freeze baseline `bdb4b93`; API Review PASS / FROZEN at `0a73b5a`); no production clarification was needed. Changes: `tests/integration/component_configuration_integration_test.cpp` (new), `tests/runtime/reference_configuration.hpp` (three small test-only additions to the accepted R08-004 harness, below), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CFG-009`, `CORE-CFG-010` + rows), `API.md` section 47, `TESTING.md`.

- Harness additions (test support only, additive): the reference configurable component now records the same `"<id>:configure"` trace entry as the plain reference component, honors the plain component's one-shot `fail_next_configure` with identical code, source and message ("configure failed"), and passes its semantic stage's own message through on rejection, so the two component kinds are comparable in the seeded differential. The R08-004 tests and their per-defect detection are unchanged and green.
- Tests (`kritva_core_component_configuration_integration`, 6 functions), mapped to the acceptance criteria:
  1. **Same logical configuration, dependency order, determinism (seeded differential):** 200 seeds × 40 steps; random topologies, registration orders (permuted), plenty of configure steps with injected failures at random components, explicit resets. Replayed with plain reference components and with configuration-aware components: transcripts **identical** (results, states, topology, faults, statistics, invocation traces). For every configure step the shared call log is checked with the reusable `check_runtime_forwarding` (dependency order from `component_order()`, exactly once, the caller's own object, first failure stops, no retry, no continued sequence), the Runtime state and fault are unchanged by the call, a refusal in READY/RUNNING/FAULT happens before any component is invoked, and **no rollback** is verified from the components' applied state (those before the failing one accepted the new configuration, the failing one and later ones kept what they had). A vacuity guard asserts the seeds covered >1000 configure steps, >100 refusals, >500 forwarded sequences, >150 failures including >30 at a middle position, and >150 successes.
  2. The Runtime **never read Status or Health** in any seeded scenario (spy counters 0) and never counted a retry (`retry_count` 0 at every step).
  3. **First failure stops the sequence; no rollback; no retry:** component 2 of 4 fails: exactly 2 calls (1 accepted, 2 rejected), 3 and 4 never called, component 1 keeps the new configuration, the failing Error is returned unchanged (code, source, message), the Runtime stays UNKNOWN with no fault and counts error 1 / sample 1 (its own calls only); the next explicit configure reaches all four.
  4. **After the topology is fixed:** configure in RUNNING is refused by the Runtime with INVALID_STATE and no component call; after stop, configure from STOPPED reaches 1,2,3,4 with the caller's object.
  5. **Independence from FAULT, Status and Health:** a start failure makes the Runtime FAULT; configure in FAULT is refused and does not leave it; only `reset()` does; a configuration failure after reset leaves the Runtime STOPPED with no fault; no Status/Health read at any point.
  6. **ComponentContext and platform untouched:** an integrator-built `ComponentContext` is unchanged (immutable, never held by the Runtime) and, with a reference platform attached, configuration makes no adapter query and no service call (call log empty).  7. The Runtime forwards the caller's own object and keeps nothing: the caller destroys it, the Runtime continues (initialize/start/stop/shutdown) and every component kept its own copy.
- Mutation evidence (14 production Runtime mutants, each reverted): a retry of a failed configure, a copy of the Configuration, a status read and a health read during configure, a retry counter increment, configure failure entering FAULT, failure swallowed into success, reversed order (topology not fixed and fixed), configure also allowed in FAULT or READY, success advancing the Runtime to INITIALIZING. All consequential mutants detected; the retry-counter mutant survived the first run and was closed by the absolute `retry_count == 0` assertion. Two mutants (marking a component's stage after configure success or failure) are equivalent: the stage map is reset by `initialize()` before it is ever consulted, and `shutdown()` returns early while no component is live.
- Regression: ctest 56/56 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean (including the new integration test); coverage 618/625 unchanged; `make check`, traceability 99 requirements, 98 traced, 0 errors (`CORE-CFG-009`, `CORE-CFG-010` defined with rows); `git diff --check` clean.
- Out of scope confirmed: no production change, no platform lifecycle, no dynamic reconfiguration, no rollback or retry.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `a5dfbc1` (evidence `159b1bc`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: test-only (no production diff since the freeze baseline `bdb4b93`); the seeded differential (200 seeds × 40 steps, vacuity-guarded) gives transcripts identical to the plain baseline; forwarding order, exactly-once delivery of the caller's own object, stop at the first failure with the component's Error unchanged, no retry and no rollback, isolation from Runtime state, FAULT, Status, Health, ComponentContext and the platform are accepted; the three additive harness changes are test-only; the equivalent mutants are accepted as classified. `CORE-CFG-009` and `CORE-CFG-010` are authoritative.

**Reviewer Decision: PASS — KF-CORE-R08-005 is ACCEPTED.**
