# Kritva Core — Consolidated Milestone Status

## Current Snapshot

| Milestone | Status | Progress | Review | Release |
|---|---|---:|---|---|
| KF-CORE-R01 | COMPLETE | 100% | PASS | `kritva-core-r0.1` (referenced; tag not present in Git, see R0.2 Milestone Gate note) |
| KF-CORE-R02 | RELEASED | 8 / 8 tasks accepted | PASS | `kritva-core-r0.2` |
| KF-CORE-R03 | RELEASED | 8 / 8 tasks accepted | Foundation API Review, Runtime Contract Review and Release Gate PASS; Integration Freeze honored | `kritva-core-r0.3` (annotated tag on release-record commit `cc16ec9`; tag object `0dfccab`; pushed to origin) |
| KF-CORE-R04 | RELEASED | 8 / 8 tasks accepted | Platform API Review PASS / FROZEN, Platform Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.4` (annotated tag on release-record commit `b31108d`; tag object `9e1bc7b`; pushed to origin) |

## R02 Task Status

| Task | Status | Dependency |
|---|---|---|
| 001 Result<T> | ACCEPTED (`df44d38`) | R01 |
| 002 Status | ACCEPTED (`d6d939d`) | 001 |
| 003 Statistics | ACCEPTED (`d7d29cb`) | 002 |
| 004 Scheduler | ACCEPTED (`0773857`, `0cdffdb`) | 003 |
| 005 Clock | ACCEPTED (`6ed8762`, `e1e6cfb`) | 004 |
| 006 Traceability | ACCEPTED (`00899f9`) | 005 |
| 007 Contract Tests | ACCEPTED (`bf2144a`) | 006 |
| 008 Full Validation | ACCEPTED (`1472b79`) | 007 |

## R0.2 Milestone Gate

Status: ACCEPTED
Tasks: 8 / 8 accepted
Release version: 0.2.0
Release commit: `46af52c`
Release tag: `kritva-core-r0.2` (annotated, on `46af52c`; pushed to origin, tag object `71431e4`)

Validation:
- 17/17 tests passed
- 0 build warnings
- ASan + UBSan passed
- TSan passed with documented environment workaround (ASLR disabled)
- Strict compiler checks (`-Werror`) passed
- GCC `-fanalyzer` passed
- 98% line coverage
- `make check` passed
- 49 requirements, 48 traced (1 reserved/exempt), 0 errors
- Fresh-clone validation passed

Release metadata:
- `VERSION` = 0.2.0 (matches `CMakeLists.txt`)
- `CHANGELOG.md` has an R0.2 section

Known follow-ups:
- clang-tidy / cppcheck / lint / format tooling (`make lint` and `make format-check` are placeholders)
- minor documentation cleanup (unused `<chrono>` include in `types/duration.hpp`, stale root `implementation.md`)

Repository-history note:
- R0.1 release tag discrepancy: `kritva-core-r0.1` is referenced by the R0.1 milestone documentation but does not exist in the local or remote Git repository. The historical R0.1 release point is `245d91e` based on repository history, but no retroactive tag is being created as part of R0.2 closure.

Gate decision: ACCEPTED (reviewer: ChatGPT)

## KF-CORE-R03 Task Status

| Task | Status | Dependency | Primary Commit |
|---|---|---|---|
| KF-CORE-R03-001 Component Contract & Identity | ACCEPTED (`655c1dd`, `35efee1`, `9a98ab3`) | R02 | `feat(core): define component runtime contract` |
| KF-CORE-R03-002 Component Registry | ACCEPTED (`7ae9a32`, `4649910`) | R03-001 accepted | `feat(core): add component registry` |
| KF-CORE-R03-003 Dependency Management | ACCEPTED (`795fb94`, `7e2a53b`) | R03-001 + R03-002 accepted | `feat(core): add runtime dependency management` |
| R03 Foundation API Review | PASS / FROZEN | R03-001..003 accepted | 03-10-2026 |
| KF-CORE-R03-004 Runtime Manager | ACCEPTED (`e4d3a9b`, `40e33e9`, `25eb914`) | Foundation API Review PASS | `feat(core): add runtime manager` |
| KF-CORE-R03-005 Runtime Lifecycle | ACCEPTED (`e2b660d`, `a4f2a65`, `eac011f`) | R03-004 accepted | `feat(core): implement runtime lifecycle orchestration` |
| KF-CORE-R03-006 Runtime Failure & Recovery | ACCEPTED (`ee3d55d`, `d651677`) | R03-005 accepted | `feat(core): define runtime failure handling` |
| R03 Runtime Contract Review | PASS / FROZEN | R03-004..006 accepted | 03-10-2026 |
| KF-CORE-R03-007 Runtime Integration Tests | ACCEPTED (`9d1c7d1`) | Runtime Contract Review PASS | `test(core): add runtime integration contracts` |
| R03 Integration Freeze | PASS / ACTIVE | R03-007 accepted | 03-10-2026 |
| KF-CORE-R03-008 Final Validation | ACCEPTED (`407df6b`; candidate `f598fef`) | Integration Freeze PASS | `test(core): complete R03 runtime validation` |
| R03 Release Gate | PASS | R03-008 accepted | 03-10-2026 |

## R03 Acceptance Gates

### Foundation API Review — after R03-003

Must freeze:

- ComponentId semantics
- component lifecycle contract
- metadata
- ownership/lifetime
- registry registration/lookup/enumeration
- deterministic ordering
- dependency representation
- missing/self/duplicate dependency semantics
- cycle detection
- topological ordering
- deterministic tie-break

Cross-cutting policy is reviewed at this gate for:

- Error
- Warning
- Info/diagnostic
- Event versus message
- Statistics updates
- logging boundary

### Runtime Contract Review — after R03-006

Must freeze runtime manager, lifecycle, failure propagation, recovery/reset, diagnostics and statistics runtime semantics.

### Integration Freeze — after R03-007

No production API changes during final validation unless explicitly returned to architecture review.

### Release Gate — after R03-008

All tasks accepted, full validation green, independent review PASS, documentation updated, and release tag created.

## Status Definitions

- PLANNED: not started.
- IN PROGRESS: implementation underway.
- IMPLEMENTATION COMPLETE: developer believes scope is complete.
- REVIEW: evidence submitted for independent review.
- ACCEPTED: reviewer passed all criteria.
- BLOCKED: a specific blocker prevents progress.
- RELEASED: milestone accepted and tagged.

Do not mark a task ACCEPTED based only on compilation. Acceptance requires evidence and independent review.


## R03 Final Gates

### Runtime Contract Review
Status: PASS / FROZEN (03-10-2026)
Entry: R03-004, R03-005 and R03-006 accepted.
Action: froze Runtime Manager, lifecycle, failure/recovery, diagnostics and statistics semantics before R03-007. Record: `planning/milestones/KF-CORE-R03/R03_RUNTIME_CONTRACT_REVIEW.md`.

### Integration Freeze
Status: PASS / HONORED (03-10-2026)
Entry: R03-007 accepted.
Action: froze the production API and accepted runtime semantics before R03-008; the production diff from the Runtime Contract Review commit `8ec7861` to the release candidate `f598fef` is empty. Record: `planning/milestones/KF-CORE-R03/R03_INTEGRATION_FREEZE.md`.

### Release Gate
Status: PASS (03-10-2026)
Release: Kritva Core R0.3, version 0.3.0; release candidate `f598fef`; release-record commit `cc16ec9`; tag `kritva-core-r0.3` (annotated, published). Record: `planning/milestones/KF-CORE-R03/R03_RELEASE_GATE.md`.

## R0.4 Task Status

| ID | Task | Status | Dependency |
|---|---|---|---|
| KF-CORE-R04-001 | Platform Adapter Boundary & Context | ACCEPTED (`d1c5f13`) | R0.3 released |
| KF-CORE-R04-002 | Scheduler Contract Hardening | ACCEPTED (`eb06fa0`) | R04-001 |
| KF-CORE-R04-003 | Clock & Timer Contract | ACCEPTED (67114bb) | R04-001 |
| KF-CORE-R04-004 | Watchdog Contract | ACCEPTED (4af4756) | R04-001 |
| R04 Platform API Review | PASS / FROZEN (`380ade3`) | R04-004 |
| KF-CORE-R04-005 | Platform Capability & Adapter Contract | ACCEPTED (f7231c1) | Platform API Review |
| KF-CORE-R04-006 | Platform Conformance Tests | ACCEPTED (460de87) | R04-005 |
| R04 Platform Integration Freeze | PASS / HONORED (`84046b0`) | R04-006 |
| KF-CORE-R04-007 | Runtime–Platform Integration Boundary | ACCEPTED (36c5cb8) | Platform Integration Freeze |
| KF-CORE-R04-008 | Full R0.4 Validation | ACCEPTED (`f0669eb`; candidate `e7df87c`) | R04-007 |
| R04 Release Gate | PASS | R04-008 | 04-10-2026 |

All eight R0.4 tasks are ACCEPTED on implementation evidence and independent review.

### R0.4 Release Gate
Status: PASS (04-10-2026)
Release: Kritva Core R0.4, version 0.4.0; release candidate `e7df87c`; release-record commit `b31108d`; tag `kritva-core-r0.4` (annotated, published). Record: `planning/milestones/KF-CORE-R04/R04_RELEASE_GATE.md`.

## R0.5 Task Status

| ID | Task | Status | Dependency |
|---|---|---|---|
| KF-CORE-R05-001 | Platform Context & Service Access Model | ACCEPTED (c5910e7) | R0.4 released |
| KF-CORE-R05-002 | Platform Service Requirement Model | ACCEPTED (f9d6007) | R0.4 released |
| KF-CORE-R05-003 | Explicit Platform Service Consumption | ACCEPTED (f8cd523) | R0.4 released |
| KF-CORE-R05-004 | Runtime–Platform Lifecycle Boundary | PLANNED | R0.4 released |
| KF-CORE-R05-005 | Reference Platform Integration | PLANNED | R0.4 released |
| KF-CORE-R05-006 | Platform Integration & Runtime Tests | PLANNED | R0.4 released |
| KF-CORE-R05-007 | Full R0.5 Validation | PLANNED | R0.4 released |
