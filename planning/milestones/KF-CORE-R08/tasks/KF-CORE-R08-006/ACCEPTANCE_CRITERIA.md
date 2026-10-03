# KF-CORE-R08-006 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-006 |
| Status | PLANNED |
| Primary commit | `test(core): validate configuration boundary and regression` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08 Integration Freeze PASS/HONORED |

## Objective

Validate the frozen R08 configuration boundary and preserve the repository-level quality baseline after Integration Freeze.

## Proposed Requirements Traceability

CORE-CFG-013

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- R08 Integration Freeze is PASS/HONORED before validation begins.
- Fresh-clone Debug and Release builds pass.
- Full regression passes.
- Public-header self-containment passes.
- Traceability reports zero errors.
- Prohibited dependency/isolation audit remains clean.
- Install-consumer validation covers the accepted R08 configuration contract.
- Coverage policy is satisfied.
- Production diff from the Integration Freeze contains no unapproved semantic/API changes.
- `git diff --check` is clean and the working tree is clean.
- Deferred known issues are not silently pulled into R08 scope.

## New Tests

- Add only boundary/regression tests required to prove the freeze and install-consumer contract.
- No new public API may be invented by this validation task.

## Regression Tests

- Complete R0.7 baseline plus all accepted R08 tests; the final count may increase as tests are added.

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
test(core): validate configuration boundary and regression
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
