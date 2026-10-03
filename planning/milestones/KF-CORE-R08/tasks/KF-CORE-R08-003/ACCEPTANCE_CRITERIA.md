# KF-CORE-R08-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-003 |
| Status | PLANNED |
| Primary commit | `feat(core): define configuration version and validation contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-002 |

## Objective

Freeze `ConfigurationVersion` semantics and the Core-vs-Component validation boundary.

## Proposed Requirements Traceability

CORE-CFG-007, CORE-CFG-008, CORE-CFG-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- `ConfigurationVersion` is documented and tested as a schema/contract compatibility version.
- It is explicitly not a runtime revision counter, transaction id, update count or history mechanism.
- R0.8 does not create a generic configuration revision API.
- Existing `Configuration::validate()` remains the Core structural validation entry point.
- Component-specific semantic validation remains Component-owned.
- Invalid schema/contract compatibility is reportable using existing configuration/error semantics without adding a per-parameter error-code taxonomy.
- The API documentation does not imply that Core knows domain-specific parameter ranges/enumerations/defaults.
- No unnecessary new version field or API is added merely to expose the alias.
- Mutation testing covers schema/validation boundary conditions.

## New Tests

- ConfigurationVersion semantic tests.
- Structural-valid/semantic-invalid examples.
- Version compatibility rejection tests in the reference Component.
- Mutation tests for boundary confusion between Core structural and Component semantic validation.

## Regression Tests

- Existing configuration unit/contract tests remain green.
- Existing Version tests remain green.

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
feat(core): define configuration version and validation contract
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
