# KF-CORE-R07-002 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-002 |
| Status | PLANNED |
| Primary commit | `feat(core): define component status and health reporting contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Formalize ownership and reporting semantics for the existing Status and Health contracts.

## Scope

Define Component authority, snapshot returns, independence of Status/Health, detail semantics and complete independence from Runtime FAULT/recovery. Do not redesign the existing enums.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- `Status` and `Health` remain separate concepts.
- Component is the authoritative source.
- Queries return values/snapshots, not mutable internal references.
- Health changes never trigger Runtime lifecycle changes.
- Status/Health queries have no implicit Event emission.
- Existing enum meanings are preserved.
- Thread-safety/allocation claims remain explicit.

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
feat(core): define component status and health reporting contract
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

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
