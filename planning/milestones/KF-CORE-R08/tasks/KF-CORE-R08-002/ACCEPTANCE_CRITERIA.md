# KF-CORE-R08-002 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-002 |
| Status | PLANNED |
| Primary commit | `feat(core): define configuration ownership and atomic application` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-001 |

## Objective

Define detached input ownership and atomic/non-partial configuration application semantics.

## Proposed Requirements Traceability

CORE-CFG-005, CORE-CFG-006

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The caller-owned `Configuration` remains valid independently of the Component after `configure()` returns.
- A conforming Component does not retain the caller object's address/reference for later use.
- A failed configuration does not partially replace the previously accepted configuration state.
- Lifecycle state remains unchanged on configuration failure.
- The contract does not require a generic `Component::configuration()` accessor.
- A reference/broken-component test can detect retained-reference and partial-application mutations.
- No Runtime/global configuration store is added.
- Mutation testing detects partial-commit and reference-retention defects.

## New Tests

- Reference component that records/commits configuration only after successful validation.
- Broken variant retaining a pointer/reference to input must be detected.
- Broken variant partially mutating state before returning failure must be detected.
- Reuse/mutation of caller configuration after `configure()` verifies detachment.

## Regression Tests

- Existing Configuration value/copy behavior tests remain green.
- Existing Component and Runtime regression tests remain green.

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
feat(core): define configuration ownership and atomic application
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
