# Kritva Core — Consolidated Milestone Status

## Current Snapshot

| Milestone | Status | Progress | Review | Release |
|---|---|---:|---|---|
| KF-CORE-R01 | COMPLETE | 100% | PASS | `kritva-core-r0.1` (referenced; tag not present in Git, see R0.2 Milestone Gate note) |
| KF-CORE-R02 | RELEASED | 8 / 8 tasks accepted | PASS | `kritva-core-r0.2` |
| KF-CORE-R03 | IN PROGRESS | 2 / 8 tasks accepted | — | — |
| KF-CORE-R04 | PLANNED | 0% | — | — |

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
| KF-CORE-R03-003 Dependency Management | PLANNED | R03-001 + R03-002 accepted | `feat(core): add runtime dependency management` |
| R03 Foundation API Review | PENDING | R03-001..003 accepted | — |
| KF-CORE-R03-004 Runtime Manager | PLANNED | Foundation API Review PASS | `feat(core): add runtime manager` |
| KF-CORE-R03-005 Runtime Lifecycle | PLANNED | R03-004 accepted | `feat(core): implement runtime lifecycle orchestration` |
| KF-CORE-R03-006 Runtime Failure & Recovery | PLANNED | R03-005 accepted | `feat(core): define runtime failure handling` |
| R03 Runtime Contract Review | PENDING | R03-004..006 accepted | — |
| KF-CORE-R03-007 Runtime Integration Tests | PLANNED | Runtime Contract Review PASS | `test(core): add runtime integration contracts` |
| R03 Integration Freeze | PENDING | R03-007 accepted | — |
| KF-CORE-R03-008 Final Validation | PLANNED | Integration Freeze | `test(core): complete R03 runtime validation` |
| R03 Release Gate | PENDING | R03-008 accepted | — |

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
