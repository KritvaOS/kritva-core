# Kritva Core — Consolidated Milestone Status

## Current Snapshot

| Milestone | Status | Progress | Review | Release |
|---|---|---:|---|---|
| KF-CORE-R01 | COMPLETE | 100% | PASS | `kritva-core-r0.1` (referenced; tag not present in Git, see R0.2 Milestone Gate note) |
| KF-CORE-R02 | RELEASED | 8 / 8 tasks accepted | PASS | `kritva-core-r0.2` |
| KF-CORE-R03 | RELEASED | 8 / 8 tasks accepted | Foundation API Review, Runtime Contract Review and Release Gate PASS; Integration Freeze honored | `kritva-core-r0.3` (annotated tag on release-record commit `cc16ec9`; tag object `0dfccab`; pushed to origin) |
| KF-CORE-R04 | RELEASED | 8 / 8 tasks accepted | Platform API Review PASS / FROZEN, Platform Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.4` (annotated tag on release-record commit `b31108d`; tag object `9e1bc7b`; pushed to origin) |
| KF-CORE-R05 | RELEASED | 7 / 7 tasks accepted | Platform API Review PASS / FROZEN, Platform Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.5` (annotated tag on release-record commit `adf8ac2`; tag object `aecb045`; pushed to origin) |
| KF-CORE-R06 | RELEASED | 7 / 7 tasks accepted | Component API Review PASS / FROZEN, Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.6` (annotated tag on release-record commit `a4c41aa`; tag object `fb9d631`; pushed to origin) |
| KF-CORE-R07 | RELEASED | 7 / 7 tasks accepted | Component Operational API Review PASS / FROZEN, Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.7` (annotated tag on release-record commit `424984f`; tag object `4aa3fab`; pushed to origin) | Target `kritva-core-r0.7` / version 0.7.0 |
| KF-CORE-R08 | RELEASED | 7 / 7 tasks accepted | Configuration API Review PASS / FROZEN, Integration Freeze PASS / HONORED, Release Gate PASS | `kritva-core-r0.8` (annotated tag on release-record commit `cbbec81`; tag object `2d576d7`; pushed to origin) | Target `kritva-core-r0.8` / version 0.8.0 |
| KF-CORE-R09 | RELEASED | 7 / 7 tasks accepted | Capability API Review PASS / FROZEN, Integration Freeze PASS / HONORED, Security Architecture Review PASS, Release Gate PASS | `kritva-core-r0.9` / version 0.9.0 (annotated tag on release-record commit `d72343a`; tag object `f77fecb`; pushed to origin) |

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
| KF-CORE-R05-004 | Runtime–Platform Lifecycle Boundary | ACCEPTED (f23777b) | R0.4 released |
| R05 Platform API Review | PASS / FROZEN (`05d981e`) | R05-004 |
| KF-CORE-R05-005 | Reference Platform Integration | ACCEPTED (7f30626) | R0.4 released |
| KF-CORE-R05-006 | Platform Integration & Runtime Tests | ACCEPTED (fe04d35) | R0.4 released |
| R05 Platform Integration Freeze | PASS / HONORED (`cf6e617`) | R05-006 |
| KF-CORE-R05-007 | Full R0.5 Validation | ACCEPTED (`8384f5d`; candidate `5fb5e69`) | R0.4 released |

### R0.5 Release Gate
Status: PASS (05-10-2026)
Release: Kritva Core R0.5, version 0.5.0; release candidate `5fb5e69`; release-record commit `adf8ac2`; tag `kritva-core-r0.5` (annotated, published). Record: `planning/milestones/KF-CORE-R05/R05_RELEASE_GATE.md`.


## R0.6 Task Status

| ID | Status | Dependency | Est. Effort |
|---|---|---|---:|
| KF-CORE-R06-001 | ACCEPTED (8031c47) | R0.5 released | 3–4 ED |
| KF-CORE-R06-002 | ACCEPTED (072b713) | R06-001 | 3–4 ED |
| KF-CORE-R06-003 | ACCEPTED (adb0e08) | R06-002 | 3–4 ED |
| KF-CORE-R06-004 | ACCEPTED (5b755af) | R06-002, R06-003 | 2–3 ED |
| R06 Component API Review | PASS / FROZEN (`06207c7`) | R06-001..004 | 1 ED |
| KF-CORE-R06-005 | ACCEPTED (c7f7b46) | Component API Review PASS/FROZEN | 3–4 ED |
| KF-CORE-R06-006 | ACCEPTED (2fd5424) | R06-005 | 3–4 ED |
| R06 Integration Freeze | PASS / HONORED | R06-006 | 0.5 ED |
| KF-CORE-R06-007 | ACCEPTED `7351db6` (candidate b473e5d) | Integration Freeze PASS/HONORED | 2–3 ED |
| R06 Release Gate | PASS (05-10-2026) | R06-007 | 1 ED |

### R0.6 Status

Status: PASS (05-10-2026)

Release: Kritva Core R0.6, version 0.6.0; release candidate `b473e5d`; tag `kritva-core-r0.6` (annotated, on release-record commit `a4c41aa`; pushed to origin). Record: `planning/milestones/KF-CORE-R06/R06_RELEASE_GATE.md`.


## R0.7 Task Status

| ID | Status | Dependency | Est. Effort |
|---|---|---|---:|
| R07 Design Consult | APPROVED | R0.6 released | 2–3 ED |
| R07 Scope Confirmation | APPROVED | Design Consult | 1 ED |
| KF-CORE-R07-001 | ACCEPTED (6849a73) | Scope Confirmation | 3–4 ED |
| KF-CORE-R07-002 | ACCEPTED (61e0067) | R07-001 | 2–3 ED |
| KF-CORE-R07-003 | ACCEPTED (56ff226) | R07-002 | 3–4 ED |
| KF-CORE-R07-004 | ACCEPTED (16654e9) | R07-003 | 2–3 ED |
| R07 Component Operational API Review | PASS / FROZEN (`6b1296f`) | R07-001..004 | 1 ED |
| KF-CORE-R07-005 | ACCEPTED (0b1bd1d) | API Review PASS/FROZEN | 3–4 ED |
| KF-CORE-R07-006 | ACCEPTED (3f524cd) | R07-005 | 3–4 ED |
| R07 Integration Freeze | PASS / HONORED (`142a32e`) | R07-006 | 0.5 ED |
| KF-CORE-R07-007 | ACCEPTED `86dfcb9` (candidate d83e1ba) | Integration Freeze PASS/HONORED | 2–3 ED |
| R07 Release Gate | PASS (05-10-2026) | R07-007 | 1 ED |

### R0.7 Status

Status: PASS (05-10-2026)

Release: Kritva Core R0.7, version 0.7.0; release candidate `d83e1ba`; tag `kritva-core-r0.7` (annotated, on release-record commit `424984f`; pushed to origin). Record: `planning/milestones/KF-CORE-R07/R07_RELEASE_GATE.md`.


## KF-CORE-R08 Task Status

| Task | Status | Dependency | Est. Effort |
|---|---|---|---:|
| R08 Design Consult | APPROVED | R0.7 released | 2–3 ED |
| R08 Scope Confirmation | APPROVED | Design Consult | 1 ED |
| KF-CORE-R08-001 | ACCEPTED (605516b) | Scope Confirmation | 2–3 ED |
| KF-CORE-R08-002 | ACCEPTED (6efeaac) | R08-001 | 2–3 ED |
| KF-CORE-R08-003 | ACCEPTED (bdb4b93) | R08-002 | 2–3 ED |
| R08 Configuration API Review | PASS / FROZEN (`0a73b5a`) | R08-001..003 | 1 ED |
| KF-CORE-R08-004 | ACCEPTED (f4b6de4) | API Review PASS/FROZEN | 3–4 ED |
| KF-CORE-R08-005 | ACCEPTED (a5dfbc1) | R08-004 | 3–4 ED |
| R08 Integration Freeze | PASS / HONORED (`e1051a0`) | R08-005 | 0.5 ED |
| KF-CORE-R08-006 | ACCEPTED (94fad8e) | Integration Freeze PASS/HONORED | 2–3 ED |
| KF-CORE-R08-007 | ACCEPTED `1aa3611` (candidate 1e7ba2b) | R08-006 | 2–3 ED |
| R08 Release Gate | PASS (05-10-2026) | R08-007 | 1 ED |

### R0.8 Scope Summary

R0.8 establishes a precise, platform-independent Component Configuration Contract around the existing Configuration and `configure()` path. It explicitly excludes dynamic reconfiguration, parameter services, persistence, remote configuration, configuration transactions/rollback, configuration event infrastructure, ComponentContext expansion, Runtime lifecycle changes, automatic recovery and robotics-specific configuration semantics.

### R0.8 Quality Baseline

The R0.7 release candidate baseline is 51/51 CTest, 98.9% line coverage, Debug/Release clean, sanitizers/strict/analyzer clean, traceability clean, public-header self-containment and install-consumer validation. R0.8 preserves this quality model; the exact final test count will increase as R08 tests are added.

### R0.8 Status

Status: PASS (05-10-2026)

Release: Kritva Core R0.8, version 0.8.0; release candidate `1e7ba2b`; tag `kritva-core-r0.8` (annotated, on release-record commit `cbbec81`; pushed to origin). Record: `planning/milestones/KF-CORE-R08/R08_RELEASE_GATE.md`.


## R09 Capability Contract & Readiness Boundary

- [ ] R09-001 through R09-003 accepted.
- [ ] R09 Capability API Review PASS/FROZEN.
- [ ] R09-004 and R09-005 accepted.
- [ ] R09 Integration Freeze PASS/HONORED.
- [ ] R09-006 and R09-007 accepted.
- [ ] API documentation synchronized in Markdown under `docs/api/`.
- [ ] Security architecture/impact review completed.
- [ ] Fresh-clone validation PASS.
- [ ] Requirements/API/planning documentation reconciled.
- [ ] Version and CMake metadata agree at 0.9.0.
- [ ] Release record is documentation-only.
- [ ] Annotated `kritva-core-r0.9` tag authorized only after PASS.
- [ ] Remote `main`, tag object and peeled tag independently verified.
- [ ] RELEASED / SYNCHRONIZED / CLOSED recorded.


## R0.9 Task Status

| ID | Status | Dependency | Est. Effort |
|---|---|---|---:|
| R09 Design Consult | APPROVED | R0.8 released | 2–3 ED |
| R09 Scope Confirmation | APPROVED | Design Consult | 1 ED |
| KF-CORE-R09-001 | ACCEPTED (4081dc0) | Scope Confirmation | 2–3 ED |
| KF-CORE-R09-002 | ACCEPTED (4bd241e) | R09-001 | 2–3 ED |
| KF-CORE-R09-003 | ACCEPTED (e91a51f) | R09-002 | 2–3 ED |
| R09 Capability API Review | PASS / FROZEN (`c84bb9c`) | R09-001..003 | 1 ED |
| KF-CORE-R09-004 | ACCEPTED (2608795) | API Review PASS/FROZEN | 3–4 ED |
| KF-CORE-R09-005 | ACCEPTED (0f6b6c3) | R09-004 | 3–4 ED |
| R09 Integration Freeze | PASS / HONORED (`553258b`) | R09-005 | 0.5 ED |
| KF-CORE-R09-006 | ACCEPTED (04f0859) | Integration Freeze PASS/HONORED | 2–3 ED |
| KF-CORE-R09-007 | ACCEPTED `683a3ce` (candidate ef14e99) | R09-006 | 2–3 ED |
| R09 Release Gate | RELEASED (`d72343a`) | R09-007 | 1 ED |
