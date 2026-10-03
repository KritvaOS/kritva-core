# KF-CORE-R07-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-005 |
| Status | PLANNED |
| Primary commit | `test(core): add component operational reference harness` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Provide a test-only reference Component, observer/sink and reusable contract tests proving the R0.7 operational contracts.

## Scope

Test-only harness, focused contract tests, negative cases, mutation tests and production-isolation checks. No production dependency on the harness.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Reference Component exercises status, health, optional statistics and event reporting.
- Observer captures event contents/source and direct-call behavior.
- Negative tests prove observation does not alter lifecycle or Runtime state.
- Mutation tests cover key operational boundary violations.
- Production isolation audit proves no test-only operational framework enters production.

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
test(core): add component operational reference harness
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
