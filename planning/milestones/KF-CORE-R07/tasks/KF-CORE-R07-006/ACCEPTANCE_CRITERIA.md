# KF-CORE-R07-006 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-006 |
| Status | PLANNED |
| Primary commit | `test(core): add component operational integration tests` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Prove through public APIs that Component operational information remains orthogonal to Runtime lifecycle orchestration.

## Scope

Integration tests covering status/health/statistics/event behavior together with initialize/start/stop/shutdown/reset, failure propagation, statistics separation and event sink ownership.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Runtime lifecycle behavior is identical with and without operational reporting.
- Health/status/statistics changes do not automatically alter Runtime state.
- Events do not invoke lifecycle/recovery automatically.
- Runtime failure/fault/reset semantics remain R0.3-compatible.
- Runtime-owned statistics remain distinct from Component statistics.
- No adapter/service ownership or platform lifecycle is introduced.

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
test(core): add component operational integration tests
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
