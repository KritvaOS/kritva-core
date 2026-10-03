# KF-CORE-R03-008 — Acceptance Criteria

## 1. Task Information
- **Requirement:** CORE-RT-010
- **Dependency:** R03 Integration Freeze = PASS
- **Primary commit:** `test(core): complete R03 runtime validation`

## 2. Acceptance Criteria

### AC-01 — Clean reproducible build
- Build starts from a clean build directory.
- Debug build succeeds with zero warnings.
- Release build succeeds with zero warnings.
- No generated/build artifacts are committed unexpectedly.

### AC-02 — Complete regression
- All R03 unit and integration tests pass.
- All pre-R03 regression tests pass.
- No test is disabled, skipped or weakened to obtain a pass without documented justification and reviewer approval.

### AC-03 — Sanitizers and strict quality
- ASan + UBSan pass.
- TSan passes where configured, with any environment workaround explicitly documented.
- Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` passes where configured.
- GCC `-fanalyzer` passes.

### AC-04 — Coverage
- Coverage is generated from the clean candidate.
- Coverage is reviewed against the established R03 baseline of 98%.
- Any uncovered production line is either covered or documented as unreachable/otherwise justified.
- No coverage exclusion is introduced merely to improve the percentage.

### AC-05 — Traceability
- `CORE-RT-001` through `CORE-RT-010` are reconciled with authoritative `REQUIREMENTS.md`.
- No duplicate requirement IDs.
- All implemented R03 requirements are traced.
- Traceability audit reports zero errors.

### AC-06 — API and architecture freeze
- Frozen R03-001..007 production APIs remain unchanged.
- No new runtime functionality is introduced by validation.
- No platform-specific runtime dependency is introduced.
- No ROS2/DDS/EtherCAT/vendor HAL/BSP/threading backend is introduced.

### AC-07 — Install-consumer
- Installation succeeds.
- A clean external consumer can compile/link against the installed R03 library.
- The consumer exercises representative RuntimeManager lifecycle and failure/recovery APIs.

### AC-08 — Dependency/prohibited-header checks
- No prohibited runtime/threading/platform headers or libraries appear in Core production code.
- Dependency audit passes.
- `git diff --check` passes.

### AC-09 — Documentation consistency
- API.md, ARCHITECTURE.md, REQUIREMENTS.md and R03 planning documents agree with the frozen implementation.
- Changelog is ready for release update.

### AC-10 — Clean candidate
- Working tree is clean after the validation commit.
- Commit hash and evidence package are recorded.
- Candidate is suitable for the R03 Release Gate.

## 3. Required Evidence
- Clean build logs.
- Complete CTest results.
- Debug/Release results.
- Sanitizer results.
- Strict compiler/static-analysis results.
- Coverage report.
- Traceability audit.
- Install-consumer result.
- Dependency/prohibited-header scan.
- Git status/diff-check evidence.
- Final changed-file and commit summary.

## 3a. Validation Evidence (Claude)

Validated release candidate: HEAD `f598fef` (`test(core): exercise failure recovery in install consumer`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty). The validation record itself is committed afterwards as `test(core): complete R03 runtime validation` (documentation only).

### Candidate history since the Integration Freeze (`4c227b0`)
| Commit | Purpose | Production code? |
|---|---|---|
| `ba30f83` `docs(core): define runtime integration and validation requirements` | defines `CORE-RT-009` and `CORE-RT-010`, retags the integration test, extends the traceability audit (threading/logging header scan, VERSION consistency) | no |
| `c1aaa54` `build(core): prepare 0.3.0 release metadata` | `VERSION` and CMake project version 0.3.0; install-consumer test derives its expectations from the project version; README/API/TESTING/CHANGELOG | no |
| `f598fef` `test(core): exercise failure recovery in install consumer` | the installed-library consumer now drives failure propagation and `reset()` (AC-07) | no |

**Production-change audit:** `git diff --stat 8ec7861 f598fef -- include src` is empty (`8ec7861` = Runtime Contract Review commit; production code last changed in `ee3d55d`). Every file changed after the Integration Freeze is release metadata, requirements, audit tooling, documentation or tests. The frozen Component, Registry, DependencyGraph, `runtime.hpp`, and RuntimeManager sources are byte-identical to their accepted state.

### AC-01 Clean build
`rm -rf build; cmake -S . -B build; cmake --build build -j4` in the fresh clone: exit 0, 0 warnings. Release (`-DCMAKE_BUILD_TYPE=Release`): 0 warnings. GCC 11.4.0, CMake 3.30.2, Linux 6.8. No build artifacts committed (`git status` clean).

### AC-02 Complete regression
`ctest --test-dir build --output-on-failure`: **25/25 passed** (13 R0.1/R0.2 unit and contract tests incl. the foundation contract test, install-consumer, component, registry, dependency graph, runtime manager, lifecycle, failure, integration, and the pre-existing suites). 10 randomized-order repetitions (`--repeat until-fail:10 --schedule-random`): all passed. Release: 25/25. No test disabled, skipped or weakened.

### AC-03 Sanitizers and strict quality
ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`): 0 warnings, 25/25. TSan (`-fsanitize=thread`, ASLR disabled with `setarch -R`, the documented environment workaround for this kernel; the suite is single-threaded): 25/25. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: builds, 25/25. GCC `-fanalyzer` over every `src/*.cpp`: no diagnostics.

### AC-04 Coverage
`make coverage` from the fresh clone: **98% (434/439 lines)**, equal to the R03 baseline. Runtime sources: `runtime_manager.cpp` 120/121, `dependency_graph.cpp` 84/84, `component_registry.cpp` 18/19. The five uncovered lines are justified and unchanged since their accepting commits: `runtime_manager.cpp:91` (unreachable return after an exhaustive `switch`), `component_registry.cpp:55` (exception-unwind brace in `components()`), `lifecycle.cpp:33` (unreachable return after an exhaustive `switch`), `configuration.cpp:40,46` (defensive branch unreachable because `set()` rejects empty names). No coverage exclusion was introduced.

### AC-05 Traceability
`make check`: header-check passed; `make traceability-check`: **58 requirements, 57 traced (CORE-ERR-003 is the reserved/exempt one), 0 errors, 0 warnings**. `CORE-RT-001` to `CORE-RT-010` are defined exactly once in `REQUIREMENTS.md` and traced; the count comes from the authoritative file (56 + `CORE-RT-009` + `CORE-RT-010`). No duplicate IDs.

### AC-06 API and architecture freeze
Production diff empty (above). No new runtime functionality. See AC-08 for the dependency scan.

### AC-07 Install-consumer and version/package validation
`kritva_core_install_consumer` passed in the fresh clone. It installs to a scratch prefix, checks the layout, then configures, builds and runs an external consumer against the installed package only. The consumer exercises `RuntimeManager` setup, topology freeze, `configure`/`initialize`/`start`/`stop`/`shutdown`, rejected `reset()` outside FAULT, statistics, and a **failure/recovery path** (injected `start` failure: original error code/source preserved, FAULT, `fault_error()`, only `reset()` accepted, STOPPED, then a new explicit `initialize()`/`start()`). Version validation: the installed `kritva_coreConfigVersion.cmake` reports `PACKAGE_VERSION "0.3.0"`; the consumer checks `kritva_core_VERSION` against the project version (derived, not hard-coded); `find_package(kritva_core 0.3)` accepts `0.3` and `0.3.0` and refuses `0.4`, `0.2` and `9.0`. A wrong expectation (`0.3.1`, `0.2.0`) makes the test fail (checked manually).

### AC-08 Dependency / prohibited-header checks
`grep` over `include/` and `src/` found no `<thread> <mutex> <shared_mutex> <condition_variable> <future> <atomic> <semaphore> <iostream> <fstream> <cstdio>` include, no ROS2/DDS/EtherCAT/SOEM identifiers, and no POSIX/Linux/Windows platform headers; the traceability audit now enforces the threading/logging scan and CORE-GEN-003 automatically (verified by planting a `<thread>` include, which is reported). `CMakeLists.txt` has no `find_package` of third-party packages and no `FetchContent`. No test-only dependency is linked into the production library target. `git diff --check` clean.

### AC-09 Documentation consistency
`REQUIREMENTS.md` (title now R0.3, `CORE-RT-006..010`), `API.md` (sections 20-24 for Component, Registry, DependencyGraph, RuntimeManager, failure/recovery; title R0.3), `ARCHITECTURE.md` (runtime sections), `README.md` (`find_package(kritva_core 0.3)`), `TESTING.md` (integration and install tests), root `CHANGELOG.md` (0.3.0 section), `VERSION` and the CMake project version (both 0.3.0, enforced by the audit) and the R03 planning documents agree with the frozen implementation. The remaining "R0.2" strings are historical (contract origin, R0.2 task records, a documented R0.2 scheduler limitation) and do not describe the runtime as R0.2-only.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `f598fef`; this evidence is committed as `test(core): complete R03 runtime validation`. Candidate suitable for the R03 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain TODO stubs; static analysis consists of strict warnings and GCC `-fanalyzer` (as in R0.2). Follow-up task.
- Unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- The release commit and tag `kritva-core-r0.3` are decided at the R03 Release Gate; nothing is tagged or pushed.

## 4. Reviewer Sign-off
Only the independent architect/reviewer records PASS / CHANGES REQUIRED / BLOCKED.
