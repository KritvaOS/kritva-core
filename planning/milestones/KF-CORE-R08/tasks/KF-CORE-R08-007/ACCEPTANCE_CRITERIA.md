# KF-CORE-R08-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-007 |
| Status | ACCEPTED |
| Primary commit | `test(core): complete R0.8 configuration validation` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-006 accepted |

## Objective

Perform the complete R0.8 final validation and establish a reproducible release-candidate evidence set.

## Proposed Requirements Traceability

CORE-CFG-013

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- R08 Integration Freeze is PASS/HONORED.
- All R08 implementation tasks are independently accepted.
- Fresh-clone Debug and Release builds pass with zero warnings.
- Complete CTest suite passes.
- ASan + UBSan pass.
- TSan passes where configured using the documented environment.
- Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` passes.
- GCC `-fanalyzer` passes.
- Coverage satisfies the R08 policy with no new unexplained exclusions.
- Traceability audit passes.
- Public-header self-containment passes.
- Install consumer passes against the candidate version.
- Isolation/dependency audit passes.
- Version/CMake metadata are consistent at 0.8.0 for the release candidate.
- Release candidate evidence is complete and the tree is clean.

## New Tests

- Final validation is primarily a validation matrix; add only narrowly missing final checks discovered by the acceptance criteria.

## Regression Tests

- Entire R0.2–R0.7 regression suite remains green.
- All accepted R08 unit, contract, integration and harness tests remain green.

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

## Coverage / Quality

- Newly added executable lines must be covered.
- No unexplained coverage exclusion.
- Overall R08 coverage remains at least 98% and has no unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.
- Required sanitizer and strict-analysis evidence is supplied at milestone validation.

## Expected Files Changed

Only files directly required for this task, its tests, and traceability/documentation reconciliation. No unrelated files.

## Git Commit

```text
test(core): complete R0.8 configuration validation
```

## Evidence Required from Implementor

- `git status` before and after implementation
- `git diff --check`
- build and test output
- focused test evidence
- mutation evidence for contract-sensitive behavior
- diff/stat summary
- commit SHA
- explicit mapping from every acceptance criterion to objective evidence

## Implementor Evidence

Validated release candidate: HEAD `1e7ba2b` (`build(core): prepare 0.8.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). R08 Integration Freeze PASS / HONORED (`e1051a0`, recorded at `c358e76`).

### Candidate history since the Integration Freeze (production freeze point `bdb4b93`)
| Commit | Purpose | Production code? |
|---|---|---|
| `f4b6de4` `test(core): add configuration reference harness` | R08-004 (accepted) | no (tests only) |
| `a5dfbc1` `test(core): add configuration runtime integration tests` | R08-005 (accepted) | no (tests only) |
| `94fad8e` `test(core): validate configuration boundary and regression` | R08-006 (accepted): boundary snapshot, install-consumer configuration exercise, `CORE-CFG-013` | no (tests only) |
| `1e7ba2b` `build(core): prepare 0.8.0 release metadata` | `VERSION` and CMake project version 0.8.0; README `find_package(kritva_core 0.8)`; REQUIREMENTS/API titles R0.8; root CHANGELOG 0.8.0 section with known follow-ups; install consumer default request 0.8 and `Version{0,8,0}` | no |

**Production-change audit:** `git diff bdb4b93 HEAD -- include src` is **empty**; the two configuration headers are byte-identical to the Configuration API Review baseline `bdb4b93` (checked per file in the fresh clone); against `kritva-core-r0.7` the only production differences are the reviewed R08 contract text (`configuration.hpp` +127, `configuration_version.hpp` +34 lines); `src/` and `runtime/component.hpp`, `runtime_manager.hpp`, `component_context.hpp` are byte-identical to the released tag.

### AC-01 Clean build
Fresh clone: Debug and Release, 0 warnings each (GCC 11.4.0, Linux 6.8). Public-header self-containment (`kritva_core_header_checks`, one TU per public header) builds in every configuration.

### AC-02 Complete regression
`ctest`: **57/57 passed** in Debug and Release (unit, contract, conformance, platform, context, operational, configuration, integration, isolation and install tests); 10 randomized-order repetitions all pass.

### AC-03 Sanitizers and strict quality
ASan + UBSan: 0 warnings, 57/57. TSan (ASLR off via `setarch -R`): 57/57. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: 0 warnings, 57/57. GCC `-fanalyzer` is clean over every `src/` file and over the contract, platform, context, operational, configuration and integration tests.

### AC-04 Coverage
`make coverage` from the fresh clone: **618/625 lines (98.9%)**, identical to the R0.7 result; R0.8 adds no production code, so no new lines and no new exclusions (the 7 uncovered lines are the established baseline).

### AC-05 Traceability
`make check`: header-check passed; **100 requirements, 99 traced (CORE-ERR-003 reserved), 0 errors, 0 warnings**; `CORE-CFG-004` to `CORE-CFG-013` are defined once (`CORE-CFG-011` at R08-001, shared by R08-001/003; `CORE-CFG-013` at R08-006 covering R08-006 and R08-007); earlier IDs are not renumbered.

### AC-06 Freeze verification
See the production-change audit: no production change after the freeze.

### AC-07 Install consumer and package version
The install test installs to a scratch prefix and builds and runs an external consumer against the installed package only, exercising R0.3 to R0.7 and, new, the R0.8 configuration contract (a component validating structurally, rejecting semantically with `CONFIGURATION_ERROR` and applying all-or-nothing; atomic structural rejection at `set()`; Runtime forwarding in dependency order with the state unchanged; `INVALID_STATE` when READY; stop at the first failure from STOPPED with no fault or rollback; `ConfigurationVersion` is `Version`; return codes 73–83). The installed config reports `PACKAGE_VERSION 0.8.0` and `find_package` accepts 0.8 and 0.8.0; wrong expectations (0.8.1 and 0.7.0) make the install test fail (checked manually, both exit 1; the expected 0.8.0 passes).

### AC-08 Isolation and dependency audits
`kritva_core_test_isolation` and the audit (`CORE-PLAT-016`) pass: no production file includes test-only support, every production TU and the umbrella header compile with only `include/` on the path, no `install()` rule ships test support. Dependency scan: no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<queue>`, `<deque>`, `<iostream>`, `<fstream>` or `<cstdio>` in `include/` or `src/`, no POSIX/Linux/Windows include, no ROS2/DDS/EtherCAT/RTOS identifier (the only hit is a comment in `boundary.hpp` naming them as forbidden); no `find_package`/`FetchContent`/`ExternalProject`; `git diff --check` clean. No configuration store, registry, server, persistence, event or worker exists.

### AC-09 Documentation
REQUIREMENTS (R0.8, `CORE-CFG-001..013`), API sections 44–47, ARCHITECTURE, README, TESTING, root CHANGELOG, `VERSION` and CMake (both 0.8.0, enforced by the audit) agree with the frozen contract.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `1e7ba2b`; this evidence is committed as `test(core): complete R0.8 configuration validation`. Candidate suitable for the R08 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3 to R0.7.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006); a context, reporter, statistics provider or retained configuration pointer used after what it refers to is destroyed is documented undefined behavior (non-owning by design).
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- Process note: during R08-005 a stale incremental build (left by my mutation experiments; `src/` itself was restored and verified) briefly showed failures; every fresh-clone result above rebuilds from scratch.
- The release commit and tag `kritva-core-r0.8` are decided at the R08 Release Gate; nothing is tagged or pushed.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `1aa3611` (release candidate `1e7ba2b`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: fresh-clone validation of the clean candidate, the full quality matrix (Debug/Release, ASan+UBSan, TSan, strict, `-fanalyzer`, randomized order), traceability 100/99/0, coverage 618/625 unchanged, the installed-package consumer including the R0.8 configuration contract and version enforcement, isolation and dependency scans, and an empty production diff since the freeze point `bdb4b93` are accepted. The reviewer noted its connector could not resolve the abbreviated local SHAs, so the verdict relies on the supplied evidence (the commits were local-only). `CORE-CFG-013` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R08-007 is ACCEPTED.**
