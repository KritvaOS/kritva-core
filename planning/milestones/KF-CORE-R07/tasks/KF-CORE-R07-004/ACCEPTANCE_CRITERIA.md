# KF-CORE-R07-004 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-004 |
| Status | PLANNED |
| Primary commit | `feat(core): define component statistics observation contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Define optional Component-owned operational Statistics semantics using the existing Statistics/Counter/Gauge types.

## Scope

Define optionality, ownership, snapshot/value semantics, non-atomic cross-field semantics and separation from Runtime statistics. Do not force meaningless statistics onto every Component.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Component statistics are optional.
- Existing `Statistics`, `Counter` and `Gauge` semantics are reused.
- Component remains authoritative for its statistics.
- Observation does not expose mutable internal ownership.
- No atomic cross-field snapshot claim is made.
- Runtime statistics remain separate and unchanged.
- No telemetry, export, persistence, rate/histogram framework is introduced.

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
feat(core): define component statistics observation contract
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
