# KF-CORE-R08-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define component configuration contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08 Scope Confirmation |

## Objective

Define the normative semantics of `Component::configure()` without introducing a new lifecycle state or dynamic configuration path.

## Proposed Requirements Traceability

CORE-CFG-004, CORE-CFG-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- `configure()` is explicitly valid only from `UNKNOWN` and `STOPPED` for R0.8.
- A configuration call from `READY`, `RUNNING`, `STOPPING` or `FAULT` returns `INVALID_STATE` and leaves the Component unchanged.
- Successful `configure()` leaves lifecycle state unchanged.
- Configuration is synchronous and control-plane; no Core real-time guarantee is implied.
- No `CONFIGURED`, `RECONFIGURING` or equivalent lifecycle state is introduced.
- No `reconfigure()`/`set_parameter()` production API is introduced.
- Existing Component lifecycle semantics remain compatible with R0.3/R0.7.
- Contract wording is explicit in the authoritative public/API documentation.
- Mutation testing detects removal or weakening of invalid-state/state-preservation checks.

## New Tests

- Unit/contract coverage for all lifecycle states against `configure()`.
- Negative tests verify no lifecycle transition on success or invalid call.
- Mutation tests for legal-state boundaries and state preservation.

## Regression Tests

- Full pre-R08 CTest suite remains green.
- Existing Runtime lifecycle/failure/reset tests remain green.

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
feat(core): define component configuration contract
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
