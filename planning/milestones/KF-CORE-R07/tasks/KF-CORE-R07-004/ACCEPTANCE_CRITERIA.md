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

## Implementor Evidence

Primary commit: `16654e9` `feat(core): define component statistics observation contract`. **No new production types:** the production diff is contract text in `runtime/component_statistics.hpp` (tag CORE-OPS-004, pairing and pass-through paragraphs); `git diff HEAD~1 -- src` is empty; `Component`, `RuntimeManager`, `Statistics`, `Counter` and `Gauge` are unchanged and no `statistics()` is added to `Component`.

- Requirements: `CORE-OPS-004` defined with a traceability row; API.md section 42.
- Test-only conformance helper `tests/runtime/statistics_conformance.hpp`: `check_statistics_provider(Fixture&)` requires repeated reads equal and never reset, each update reported exactly, one field never moving another, and earlier snapshots never changing.
- Tests (`kritva_core_component_statistics`, 9 functions): shape and optionality (not a base of Component and no `statistics()` on Component or a plain component, by-value, trivially copyable Statistics); a component without statistics is fully usable and observes as nullopt; a conforming provider passes and every field is observed; the check **detects** three non-conforming providers (reset on read, frozen first snapshot, coupled fields); values pass through unchanged (uint64 max, int64 min, utilization 1000, nothing added); snapshots independent in both directions; **component statistics separate from Runtime statistics** (component 1,000,000/777 versus Runtime 4/0, the Runtime never read the provider, only observe() did, and later updates never reach the Runtime); through FAULT and explicit reset the Runtime never reads, resets or adjusts the provider while its own error_count is 1; pairing is the caller's.
- Mutation evidence (7 mutants, each reverted, **7/7 detected**): utilization clamped, retry_count reset, drop_count incremented, statistics replaced by an empty value, an engaged value for a null provider (observe); the Runtime reading a provider after a successful or failed component call. (An eighth mutant was malformed and discarded without running; the one stray edit it left in `src/runtime_manager.cpp` was reverted with `git checkout` and `git diff HEAD~1 -- src` is empty.)
- Regression: ctest 49/49 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 617/624; `make check`, traceability 87 requirements, 86 traced, 0 errors; `git diff --check` clean.
- Out of scope confirmed: no telemetry transport, export or serialization, no background update, no statistics on the base Component.

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
