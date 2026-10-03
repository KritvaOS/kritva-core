# KF-CORE-R08-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-005 |
| Status | PLANNED |
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

## Reviewer Decision

`PASS / CHANGES REQUIRED / BLOCKED` — to be completed by ChatGPT only.
