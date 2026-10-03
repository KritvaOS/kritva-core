# KF-CORE-R07-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-005 |
| Status | ACCEPTED |
| Primary commit | `test(core): add component operational reference harness` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Provide a test-only reference Component, observer/sink and reusable contract tests proving the R0.7 operational contracts.

## Scope

Test-only harness, focused contract tests, negative cases, mutation tests and production-isolation checks. No production dependency on the harness.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Reference Component exercises status, health, optional statistics and event reporting.
- Observer captures event contents/source and direct-call behavior.
- Negative tests prove observation does not alter lifecycle or Runtime state.
- Mutation tests cover key operational boundary violations.
- Production isolation audit proves no test-only operational framework enters production.

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
test(core): add component operational reference harness
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

Primary commit: `0b1bd1d` `test(core): add component operational reference harness`. **Test-only:** `git diff 16654e9 HEAD -- include src` is empty (16654e9 is the last production change; the API is frozen at PASS/FROZEN `6b1296f`). Changes: `tests/runtime/reference_operational.hpp`, `tests/runtime/operational_conformance.hpp` (new; the status/health and statistics checks were added in R07-002/004), `tests/unit/reference_operational_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-OPS-009` + row), `TESTING.md`. (Requirement numbering note: `CORE-OPS-007` stays reserved for the Runtime boundary in R07-006, so the harness is `CORE-OPS-009` and the release-validation requirement will be `CORE-OPS-010`; the earlier consult message said 009 for validation.)

- Harness: `RecordingEventSink` (integrator-owned sink recording accepted events, every direct call with count/thread/position in a shared ordered log; fail on chosen calls, throw, re-enter), `ReferenceStatisticsProvider` (read counter), `ReferenceOperationalComponent` (public contract only, on top of the reference component: scripted Status/Health, optional provider that is nullptr unless opted in, a `ComponentEventReporter` that is unbound without a sink, and a scripted action — set Status/Health, update statistics, report events — run at the exact point after a chosen lifecycle operation, returning that operation's own result unchanged), `RecordingObserver` (only calls observe()), `RuntimeProbe` (state, fault, error and sample counts). Reusable checks: `check_status_health_independence`, `check_statistics_provider`, new `check_event_reporter` (templated on a reporter-like type so broken variants can be modelled).
- Tests (`kritva_core_reference_operational`, 14 functions): the real reporter conforms (repeatable); the event check **detects 8 deliberately broken reporters** (overwrites a mismatching source, forwards a mismatch, no stamp, alters a field, swallows a failure, retries, double send, re-attributes a failure) and passes the defect-free model; rejects an unbound or wrong-component reporter; the reference component passes all three checks (a fixture that cannot force lifecycle is reported as failing); scripted actions run at their exact point and order (shared log shows the events inside start(), after it ran); unscripted defaults follow the plain reference component and the owner's override is legal and independent; optional parts are really optional (no sink/no statistics/each alone); an operation's result is returned unchanged even when the sink fails and a failing sink never fails a successful operation; the sink models failure on chosen calls, re-entrancy and exceptions; the observer records detached ordered snapshots; **observation, reports and statistics updates between and during Runtime operations never alter lifecycle or Runtime state**: a two-component dependency run with and without observation (and with an injected start failure plus explicit reset) gives identical RuntimeProbe sequences, invocation traces and outcomes, and with nobody observing nothing read Status/Health/statistics; the Runtime never reads Status, Health, a provider and the only sink call is the component's own scripted report; the probe is sensitive to every field.
- Mutation evidence (production and harness, 21 mutants, each reverted): production observe() (health drives lifecycle, extra status read), reporter (mismatch check removed, severity overwritten, retry), Runtime (health read in call(), status read per success, health read in enter_fault, health masking a failure); harness (result swallowed, plan skipped on failure, events doubled, status action dropped, report codes dropped, provider always non-null, throw hook removed, sink double-record, probe fields zeroed). 19 detected at once; 3 survivors closed or classified: an extra `status()` read in observe() is caught by the R07-001 spy test (not by this file); the `enter_fault` health read and the dropped status action survived and were closed by an operational-reads counter in the control run and by scripting a distinct status code (both now detected).
- Production isolation: `kritva_core_test_isolation` and `make check` pass; no production include of the harness.
- Regression: ctest 50/50 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625; traceability 88 requirements, 87 traced, 0 errors; `git diff --check` clean.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `0b1bd1d` (evidence `2d60432`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: test-only (no production diff since the freeze point `16654e9`); reusable harness on public contracts only; conformance checks that detect broken variants; observation, reporting and statistics updates interleaved with lifecycle, failure and reset leave the Runtime identical to a run with no observation; the Runtime never consumes Status, Health, statistics or sinks; the observe() survivor is covered by the R07-001 suite. Numbering approved: `CORE-OPS-007` Runtime boundary (R07-006), `CORE-OPS-008` integrator policy (R07-003), `CORE-OPS-009` harness (R07-005), `CORE-OPS-010` release validation (R07-007). `CORE-OPS-009` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R07-005 is ACCEPTED.**
