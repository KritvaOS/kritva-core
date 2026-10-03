# Kritva Core — Git Commit and Release Procedure

## Per Task

```bash
git status
git diff --check
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
git diff --stat
git diff
git status
git add <new-or-changed-files>
git commit -m "<exact task message>"
git rev-parse HEAD
```

Use `git add` explicitly when new files are created.

## R03 Task Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R03-001 | `feat(core): define component runtime contract` |
| KF-CORE-R03-002 | `feat(core): add component registry` |
| KF-CORE-R03-003 | `feat(core): add runtime dependency management` |
| KF-CORE-R03-004 | `feat(core): add runtime manager` |
| KF-CORE-R03-005 | `feat(core): implement runtime lifecycle orchestration` |
| KF-CORE-R03-006 | `feat(core): define runtime failure handling` |
| KF-CORE-R03-007 | `test(core): add runtime integration contracts` |
| KF-CORE-R03-008 | `test(core): complete R03 runtime validation` |

## Commit Rules

- One logical task has one primary implementation commit.
- The task acceptance document defines the exact commit message.
- Do not mix unrelated changes.
- Do not rewrite accepted history unless explicitly instructed.
- Fix follow-up defects with a new focused commit if the task is already reviewed.
- A later task must not silently alter an earlier accepted public API.
- A breaking change requires architecture review before implementation continues.

## R03 API Freeze Gates

### After R03-003

The R03 Foundation API Review must PASS before R03-004 begins.

### After R03-006

The R03 Runtime Contract Review must PASS before R03-007 is finalized.

### After R03-007

The R03 Integration Freeze applies. Production API/semantic changes require explicit architecture review.

### After R03-008

The R03 Release Gate reviews the final candidate before tag creation.

## Milestone Release

After all milestone implementation tasks are accepted:

1. Update `MILESTONE_STATUS.md`.
2. Update `CHANGELOG.md`.
3. Run a clean build.
4. Run full regression.
5. Run final quality checks.
6. Perform independent milestone review.
7. Commit documentation changes.
8. Create the release tag.

Example:

```bash
git tag -a kritva-core-r0.3 -m "Kritva Core R0.3"
git show kritva-core-r0.3
```

## R04 Task Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R04-001 | `feat(core): define platform adapter boundary` |
| KF-CORE-R04-002 | `feat(core): harden scheduler platform contract` |
| KF-CORE-R04-003 | `feat(core): define clock and timer platform contracts` |
| KF-CORE-R04-004 | `feat(core): define watchdog platform contract` |
| KF-CORE-R04-005 | `feat(core): define platform capability contract` |
| KF-CORE-R04-006 | `test(core): add platform conformance suite` |
| KF-CORE-R04-007 | `feat(core): define runtime platform integration boundary` |
| KF-CORE-R04-008 | `test(core): complete R04 platform validation` |

## Commit Rules

- One logical task has one primary implementation commit.
- The task acceptance document defines the exact commit message.
- Do not mix unrelated changes.
- Do not rewrite accepted history unless explicitly instructed.
- Fix follow-up defects with a new focused commit if the task is already reviewed.
- A later task must not silently alter an earlier accepted public API.
- A breaking change requires architecture review before implementation continues.

## R04 API Freeze Gates

### After R04-004

The R04 Platform API Review must PASS before R04-005 begins.

### After R04-006

The R04 Platform Integration Freeze applies. Platform production API/semantic changes require explicit architecture review.

### After R04-008

The R04 Release Gate reviews the final candidate before tag creation.

## Milestone Release

After all eight tasks are accepted:

1. Update `MILESTONE_STATUS.md`.
2. Update `CHANGELOG.md`.
3. Verify `VERSION` and CMake version.
4. Run a clean build.
5. Run full regression and final quality checks.
6. Perform independent milestone review.
7. Commit documentation/release-record changes.
8. Create the annotated release tag.

Example:

```bash
git tag -a kritva-core-r0.4 -m "Kritva Core R0.4"
git show kritva-core-r0.4
```


## R05 and R06 Commit Messages

R05 task commits:

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R05-001 | `feat(core): add platform context` |
| KF-CORE-R05-002 | `feat(core): define platform service requirements` |
| KF-CORE-R05-003 | `feat(core): define explicit platform service consumption` |
| KF-CORE-R05-004 | `feat(core): preserve runtime platform lifecycle boundary` |
| KF-CORE-R05-005 | `test(core): add platform integration reference harness` |
| KF-CORE-R05-006 | `test(core): add platform runtime integration tests` |
| KF-CORE-R05-007 | `test(core): complete R0.5 validation` |

R06 task commits:

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R06-001 | `feat(core): define component execution context` |
| KF-CORE-R06-002 | `feat(core): define operational context access policy` |
| KF-CORE-R06-003 | `feat(core): define component context injection boundary` |
| KF-CORE-R06-004 | `feat(core): bind context requirements and capabilities` |
| KF-CORE-R06-005 | `test(core): add component context reference harness` |
| KF-CORE-R06-006 | `test(core): add component context integration tests` |
| KF-CORE-R06-007 | `test(core): complete R0.6 validation` |

R06 implementation is RELEASED/CLOSED. The listed R06 commit messages are retained as historical task conventions.

## R07 Architecture-Confirmed Commit Messages

R07 implementation authorization begins only after each task's `ACCEPTANCE_CRITERIA.md` is issued and the task is not blocked by a prior gate.

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R07-001 | `feat(core): define component operational observation contract` |
| KF-CORE-R07-002 | `feat(core): define component status and health reporting contract` |
| KF-CORE-R07-003 | `feat(core): define component operational event contract` |
| KF-CORE-R07-004 | `feat(core): define component statistics observation contract` |
| KF-CORE-R07-005 | `test(core): add component operational reference harness` |
| KF-CORE-R07-006 | `test(core): add component operational integration tests` |
| KF-CORE-R07-007 | `test(core): complete R0.7 operational validation` |

### R07 Architecture Gates

- R07 Design Consult — APPROVED.
- R07 Scope Confirmation — APPROVED.
- R07 Component Operational API Review must PASS/FROZEN before R07-005.
- R07 Integration Freeze applies after R07-006 and before R07-007.
- R07 Release Gate reviews R07-007 before version/tag creation.

### R07 Commit Rules

- One logical task has one primary implementation commit.
- Exact task commit message is authoritative only for that task's implementation commit.
- Do not amend accepted task commits.
- Corrective changes after review use a focused follow-up commit.
- No production API or semantic change after R07 API Freeze without returning to architecture review.

### R07 Release Procedure

After all R07 implementation tasks are accepted:

1. Update `MILESTONE_STATUS.md`.
2. Update `CHANGELOG.md`.
3. Verify `VERSION` and CMake version.
4. Run clean Debug/Release and complete validation matrix.
5. Perform independent R07 Release Gate review.
6. Create a documentation-only release-record commit.
7. Create the annotated `kritva-core-r0.7` tag only after PASS.
8. Independently verify remote `main`, tag object and peeled tag.
