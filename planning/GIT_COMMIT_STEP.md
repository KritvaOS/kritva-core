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


## R08 Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R08-001 | `feat(core): define component configuration contract` |
| KF-CORE-R08-002 | `feat(core): define configuration ownership and atomic application` |
| KF-CORE-R08-003 | `feat(core): define configuration version and validation contract` |
| KF-CORE-R08-004 | `test(core): add configuration reference harness` |
| KF-CORE-R08-005 | `test(core): add configuration runtime integration tests` |
| KF-CORE-R08-006 | `test(core): validate configuration boundary and regression` |
| KF-CORE-R08-007 | `test(core): complete R0.8 configuration validation` |

### R08 Architecture Gates

- R08 Design Consult — APPROVED.
- R08 Scope Confirmation — APPROVED.
- R08 Configuration API Review must PASS/FROZEN before R08-004.
- R08 Integration Freeze applies after R08-005 and before R08-006/R08-007.
- R08 Release Gate reviews R08-007 before version/tag creation.

### R08 Commit Rules

- One logical task has one primary implementation commit.
- The exact task commit message in the acceptance criteria is authoritative.
- Do not amend accepted task commits.
- Corrective changes after review use a focused follow-up commit.
- No unrelated changes.
- A later task must not silently alter an earlier accepted public API or semantic contract.
- Public API/semantic changes after the R08 Configuration API Review require an explicit architecture-review return.

### R08 Release Procedure

After all R08 implementation tasks are accepted:

1. Update `MILESTONE_STATUS.md`, `MASTER_TRACKER.md`, `TASKS.md` and `CHANGELOG.md`.
2. Verify `VERSION` and CMake version.
3. Run clean Debug/Release and complete validation matrix.
4. Perform the independent R08 Release Gate review.
5. Create a documentation-only release-record commit.
6. Create the annotated `kritva-core-r0.8` tag only after PASS.
7. Independently verify remote `main`, tag object and peeled tag.


## R09 Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R09-001 | `feat(core): define capability contract` |
| KF-CORE-R09-002 | `feat(core): define capability set and version semantics` |
| KF-CORE-R09-003 | `feat(core): define capability requirement matching boundary` |
| KF-CORE-R09-004 | `test(core): add capability contract reference harness` |
| KF-CORE-R09-005 | `test(core): add capability readiness lifecycle integration tests` |
| KF-CORE-R09-006 | `test(core): validate R0.9 documentation security and boundaries` |
| KF-CORE-R09-007 | `test(core): complete R0.9 capability validation` |

### R09 Architecture Gates

- R09 Design Consult — APPROVED.
- R09 Scope Confirmation — APPROVED.
- R09 Capability API Review must PASS/FROZEN before R09-004.
- R09 Security Architecture Review is required before R09-006 acceptance; it may conclude `SECURITY IMPACT: NONE`.
- R09 Integration Freeze applies after R09-005 and before R09-006/R09-007.
- R09 Release Gate reviews R09-007 before version/tag creation.

### R09 Commit Rules

- One logical task has one primary implementation commit.
- The exact task commit message in the acceptance criteria is authoritative.
- Do not amend accepted task commits.
- Corrective changes after review use a focused follow-up commit.
- No unrelated changes.
- A later task must not silently alter an earlier accepted API or semantic contract.
- Public API/semantic changes after the R09 Capability API Review require an explicit architecture-review return.
- API documentation changes required by an accepted contract change belong in the same logical task unless explicitly split and justified.

### R09 Release Procedure

After all R09 implementation tasks are accepted:

1. Update `MILESTONE_STATUS.md`, `MASTER_TRACKER.md`, `TASKS.md` and `CHANGELOG.md`.
2. Complete the permanent documentation reconciliation gate across `docs/api/`, architecture, requirements, security and guides.
3. Verify `VERSION` and CMake version metadata.
4. Run clean Debug/Release and complete validation matrix.
5. Perform independent R09 Release Gate review.
6. Create a documentation-only release-record commit.
7. Create annotated `kritva-core-r0.9` tag only after PASS.
8. Independently verify remote `main`, tag object and peeled tag.


## R10 Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R10-001 | `docs(core): inventory and classify R1.0 public API` |
| KF-CORE-R10-002 | `docs(core): define source and semantic compatibility contract` |
| KF-CORE-R10-003 | `docs(core): define R1.0 ABI compatibility policy` |
| KF-CORE-R10-004 | `docs(core): define versioning and API evolution policy` |
| KF-CORE-R10-005 | `docs(core): define API deprecation and migration policy` |
| KF-CORE-R10-006 | `test(core): add R1.0 compatibility boundary harness` |
| KF-CORE-R10-007 | `test(core): validate R1.0 package compatibility` |
| KF-CORE-R10-008 | `test(core): validate R1.0 documentation security and traceability` |
| KF-CORE-R10-009 | `build(core): prepare 1.0.0 release candidate` |

### R10 Architecture Gates

- R10 Design Consult — APPROVED.
- R10 Scope Confirmation — mandatory before R10-001 implementation.
- R10 API / Compatibility Review must PASS/FROZEN before R10-006 and R10-007 implementation proceeds.
- R10 Integration Freeze applies after R10-007 and before R10-008/R10-009.
- R10 Release Gate reviews R10-009 before version/tag creation.

### R10 Release Procedure

After all R10 implementation tasks are accepted:

1. Reconcile `MILESTONE_STATUS.md`, `MASTER_TRACKER.md`, `TASKS.md` and `CHANGELOG.md`.
2. Verify `VERSION` and CMake version are `1.0.0`.
3. Run clean Debug/Release and complete validation matrix.
4. Perform independent R10 Release Gate review.
5. Create a documentation-only release-record commit.
6. Create the annotated `kritva-core-r1.0` tag only after PASS.
7. Independently verify remote `main`, tag object and peeled tag.
