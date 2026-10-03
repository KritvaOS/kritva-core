# KF-CORE-R07-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-003 |
| Status | PLANNED |
| Primary commit | `feat(core): define component operational event contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Define explicit operational Event reporting to an integrator-owned sink without introducing a Core EventBus or asynchronous infrastructure.

## Scope

Reuse the existing Event envelope. Define source identity, explicit synchronous reporting boundary, sink ownership, delivery/no-buffering/no-retry semantics and event-versus-command separation.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Component explicitly reports an Event; Core does not auto-generate events from observations.
- Event source identity is the reporting Component.
- Sink is integrator-owned and non-owning.
- No Core queue, broker, dispatcher, worker or persistence is introduced.
- No implicit retry, buffering or background delivery is introduced.
- Event reporting never triggers lifecycle/recovery automatically.
- Existing Event envelope remains authoritative or changes are explicitly reviewed.

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
feat(core): define component operational event contract
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
