# KF-CORE-R08-004 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-004 |
| Status | PLANNED |
| Primary commit | `test(core): add configuration reference harness` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 3–4 ED |
| Dependency | R08 Configuration API Review PASS/FROZEN |

## Objective

Provide a test-only conformance harness that can detect violations of the frozen R08 configuration contract.

## Proposed Requirements Traceability

CORE-CFG-012

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Harness uses only public production APIs plus standard test support.
- Reference Component demonstrates all legal configuration states and failure behavior.
- Harness detects invalid-state acceptance.
- Harness detects lifecycle changes caused by configuration.
- Harness detects configuration retained by reference.
- Harness detects partial application after failure.
- Harness detects unexpected Runtime state change, retry or rollback when used in integration tests.
- Harness introduces no production dependency or test hook.
- Mutations intentionally weakening the contract are detected with no unexplained survivors.

## New Tests

- Reference configuration component.
- Reusable conformance functions for lifecycle eligibility, detachment and atomic failure.
- Deliberately broken variants for each contract-sensitive rule.

## Regression Tests

- Full existing suite plus all R08-001..003 focused tests.

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
test(core): add configuration reference harness
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

Primary commit: `f4b6de4` `test(core): add configuration reference harness`. **Test-only:** `git diff bdb4b93 HEAD -- include src` is empty (the production freeze baseline `bdb4b93`, API Review PASS / FROZEN at `0a73b5a`). Changes: `tests/runtime/reference_configuration.hpp` and `tests/runtime/configuration_conformance.hpp` (new), `tests/unit/reference_configuration_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CFG-012` + row), `TESTING.md`.

- Harness (public APIs only): `ReferenceConfigurableComponent` — a conforming Component whose configure() follows the frozen contract exactly (state check first: only UNKNOWN and STOPPED, INVALID_STATE with no other effect; then Core structural validation; then semantic validation against a small schema with CONFIGURATION_ERROR; whole-input staging as a copy; a commit that cannot fail), one-shot failure injection at the structural, semantic and before-commit stages (consumed only after the state check), test-only observation (canonical applied-state string, call and evaluation counters, handed address) and a call log; a menu of **22 deliberately broken Defects** of the same component covering lifecycle eligibility (accepts any state; refuses in UNKNOWN; refuses in STOPPED; wrong code or wrong source on a refusal; a refusal with an effect, an evaluation or a lifecycle change; success changing the lifecycle from UNKNOWN or from STOPPED; a first or a later failure moving the lifecycle), ownership (retains the caller's object; retains get()'s pointer) and atomicity (partial apply; commit-before-validate; half state after a first failure; never replaces; accepts but never applies; poisoned after a rejection; hidden retry; wrong source on a rejection).
- Reusable checks: `check_configuration_contract(factory)` over an abstract `ConfigurationFixture` (lifecycle eligibility in UNKNOWN/STOPPED, INVALID_STATE with no effect, no evaluation and unchanged state in READY, RUNNING and FAULT, state preservation on success and on rejection, atomic application including a first-ever rejection and recovery after a rejection, non-retention by clobbering the caller's object, error attribution) and `check_runtime_forwarding(log, given, expectation, succeeded)` (dependency order, exactly once, the caller's own object, stop at the first failure, no retry/rollback/continued sequence, overall result). A `Keeper` holds every configuration alive during a scenario so a retaining defect is detected by what it reads, never by a use-after-free (ASan+UBSan clean).
- Tests (`kritva_core_reference_configuration`, 7 functions): the reference component conforms; **each of the 22 defects is detected by the clause written for it** (the expected message fragment is asserted per defect); schema and staging (canonical order-independent applied state, five semantic rejections, a configuration replaces the state as a whole); injection at all three stages leaves the applied state untouched, is consumed once, and an invalid state refuses before an injection is consumed; counters, recorded address and call log; **the forwarding check passes for the real RuntimeManager** with shuffled registration and a failure at the first, a middle, the last component or none (and again after the topology is fixed, configuring from STOPPED), **and detects broken drivers**: wrong order, a copy, a retry, a rollback, a continued sequence, a skipped component, an extra trailing call, a swallowed failure and an inverted result.
- Mutation evidence: 27 mutants of the conformance clauses were run (each clause line removed); with the added defects every consequential clause is killed. Three first-run survivors were closed by new defects (`POISONED_AFTER_REJECTION`, `ACCEPTS_BUT_NEVER_APPLIES`) and by splitting defects per clause; the one remaining, the clause that re-reads the applied state after the caller's object is destroyed, is explained: no in-tree defect can express it without undefined behavior (a retaining defect is already caught at the preceding clobber clause, and the clause remains meaningful for out-of-tree fixtures under ASan). 8 production Runtime mutants run against the forwarding check: a retry, a copy, a continue-after-failure, a rollback, reversed order (topology not fixed and fixed), failure swallowed and failure entering FAULT — all detected (the fixed-topology reversal survived the first run and was closed by the configure-from-STOPPED forwarding test); a reversal of the reverse-loop branch is equivalent for configure (that branch is never used by it) and is covered by the stop/shutdown suites.
- Isolation: `kritva_core_test_isolation` and `make check` pass; no production include of the harness; no hook added to production.
- Regression: ctest 55/55 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; traceability 97 requirements, 96 traced, 0 errors (`CORE-CFG-012` defined with a row); `git diff --check` clean.

## Reviewer Decision

`PASS / CHANGES REQUIRED / BLOCKED` — to be completed by ChatGPT only.
