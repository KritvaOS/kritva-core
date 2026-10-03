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

## Reviewer Decision

`PASS / CHANGES REQUIRED / BLOCKED` — to be completed by ChatGPT only.
