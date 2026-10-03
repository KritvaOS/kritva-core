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

## Implementor Evidence

Primary commit: `61e0067` `feat(core): define component status and health reporting contract`. **No new production types:** the production diff is contract text in `runtime/component_observation.hpp` (ownership paragraph and tags CORE-OPS-002/003); `git diff HEAD~1 -- src` is empty; `Status`, `StatusCode`, `Health`, `HealthState`, `Component`, `RuntimeManager` and `ComponentContext` are unchanged.

- Requirements: `CORE-OPS-002` (Status ownership/snapshot) and `CORE-OPS-003` (Health ownership, independence from Runtime FAULT) defined with traceability rows; API.md section 40.
- Test-only conformance helper `tests/runtime/status_health_conformance.hpp`: `check_status_health_independence(Fixture&)` drives the full 8 lifecycle × 10 status codes × 4 health states × {no text, text} space and checks every combination is reported unchanged (nothing derived or normalized), that one change never alters the other two, and that Status/Health snapshots never change after a later report; it returns the first violation.
- Tests (`kritva_core_component_status_health`, 8 functions): a conforming component passes; the check **detects** four non-conforming components (health derived from status, status forced by lifecycle, health forced in FAULT, empty message normalized); the combination space is complete; the surprising legal combinations (UNHEALTHY while RUNNING, HEALTHY while FAULT, non-OK with no text, OK with text); the Runtime **never calls status(), health() or capabilities()** through configure/initialize/start/stop/shutdown even when the component reports the worst values; an UNHEALTHY/INTERNAL_ERROR report observed 20 times causes no stop, reset, restart, fault, lifecycle call or statistics change; Health is independent of Runtime FAULT (component FAULT with HEALTHY, an UNHEALTHY report does not clear or deepen the FAULT, and an explicit reset() never rewrites the component's Health); for every reported Health the same failing start() is handled identically (same Error code/source, FAULT, error_count 1, explicit reset only, Health never read); snapshots by value (300-char strings).
- Mutation evidence (9 mutants, each reverted): health derived from status, lifecycle derived from health, empty message normalized (observe); Runtime reading health() or status() in call(); Runtime masking a failure when the component reports UNHEALTHY. 8 detected at once; the masking mutant survived the first run and was closed by the per-Health failure-handling test (now detected). The 4 non-conforming fixtures prove the conformance check itself is sensitive.
- Regression: ctest 47/47 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan; Debug 47/47; GCC `-fanalyzer` clean; coverage 604/611 (unchanged); `make check`, traceability 84 requirements, 83 traced, 0 errors; `git diff --check` clean.
- Out of scope confirmed: no new status/health enumerator, no Runtime observation API, no health-driven behavior.

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
