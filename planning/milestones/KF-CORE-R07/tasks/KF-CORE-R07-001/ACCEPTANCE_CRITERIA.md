# KF-CORE-R07-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define component operational observation contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Define a deterministic, read-only Component operational observation boundary using existing Core concepts without introducing a new operational state machine.

## Scope

Clarify Component authority, snapshot/value semantics, side-effect rules, coherency limits, thread-safety expectations, allocation/real-time expectations, and Runtime independence. Do not add `statistics()` to the mandatory base Component as part of this task.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Observation queries do not change Component lifecycle, Runtime state or operational data.
- Existing `status()` and `health()` value/snapshot semantics remain intact.
- No new OperationalState API is introduced.
- Cross-property atomic snapshot is not claimed.
- No Runtime polling or background monitoring is introduced.
- Any public API addition is minimal, self-contained and explicitly documented.
- Existing R0.2–R0.6 behavior remains regression-compatible.

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
feat(core): define component operational observation contract
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
