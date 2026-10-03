# KF-CORE-R08-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-007 |
| Status | PLANNED |
| Primary commit | `test(core): complete R0.8 configuration validation` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-006 accepted |

## Objective

Perform the complete R0.8 final validation and establish a reproducible release-candidate evidence set.

## Proposed Requirements Traceability

CORE-CFG-013

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- R08 Integration Freeze is PASS/HONORED.
- All R08 implementation tasks are independently accepted.
- Fresh-clone Debug and Release builds pass with zero warnings.
- Complete CTest suite passes.
- ASan + UBSan pass.
- TSan passes where configured using the documented environment.
- Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` passes.
- GCC `-fanalyzer` passes.
- Coverage satisfies the R08 policy with no new unexplained exclusions.
- Traceability audit passes.
- Public-header self-containment passes.
- Install consumer passes against the candidate version.
- Isolation/dependency audit passes.
- Version/CMake metadata are consistent at 0.8.0 for the release candidate.
- Release candidate evidence is complete and the tree is clean.

## New Tests

- Final validation is primarily a validation matrix; add only narrowly missing final checks discovered by the acceptance criteria.

## Regression Tests

- Entire R0.2–R0.7 regression suite remains green.
- All accepted R08 unit, contract, integration and harness tests remain green.

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
test(core): complete R0.8 configuration validation
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
