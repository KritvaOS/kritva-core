# KF-CORE-R07-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-007 |
| Status | PLANNED |
| Primary commit | `test(core): complete R0.7 operational validation` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Perform the complete final validation of R0.7 after Integration Freeze and establish release-candidate evidence.

## Scope

Fresh-clone validation, Debug/Release, full CTest, sanitizers, strict warnings, analyzer, coverage, traceability, self-containment, install consumer and isolation/dependency audits.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Integration Freeze is PASS/HONORED.
- Fresh-clone Debug and Release builds pass.
- Full regression and required R0.7 tests pass.
- ASan/UBSan/TSan pass where configured.
- Strict warnings and GCC analyzer pass.
- Coverage and traceability pass.
- Install consumer and production-isolation audits pass.
- No production semantic change remains after freeze.
- Release candidate evidence is complete and reproducible.

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
test(core): complete R0.7 operational validation
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
