# KF-CORE-R04-008 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-008`
- **Title:** Full R0.4 Validation
- **Requirement:** `CORE-PLAT-011`
- **Status:** PLANNED
- **Primary commit message:** `test(core): complete R04 platform validation`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Validate the frozen R0.4 platform abstraction release candidate without introducing new functionality.

## Scope

['clean builds', 'CTest', 'sanitizers', 'strict diagnostics', 'static analysis', 'coverage', 'traceability', 'install consumer', 'dependency scan', 'freeze verification', 'documentation consistency']

## Out of Scope

['new production functionality', 'post-freeze API changes', 'platform implementations']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-011` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Fresh clone builds successfully in Debug and Release.', 'Complete CTest suite passes.', 'ASan and UBSan pass.', 'TSan passes where configured.', '-Werror passes.', 'GCC -fanalyzer passes.', 'Coverage is reviewed against the R0.3 baseline and changed contracts are adequately covered.', 'Requirements traceability reports zero errors.', 'Installed package is consumed by an external project.', 'VERSION and CMake project version agree.', 'Production Core sources contain no prohibited platform implementation/dependencies.', 'Production API/semantics remain unchanged after the Platform Integration Freeze unless explicitly reviewed.', 'Working tree is clean and evidence is reproducible.']

## New Tests Required

['full CTest', 'Debug/Release', 'ASan/UBSan', 'TSan where configured', '-Werror', '-fanalyzer', 'coverage', 'traceability', 'install consumer', 'dependency scan', 'freeze diff']

## Regression Tests

At minimum:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

All existing R0.3 tests must remain green.

## Quality Requirements

- No compiler warnings in the supported clean build.
- `-Werror` must pass where configured.
- ASan/UBSan must pass where configured.
- TSan must pass where configured for concurrency-sensitive changes.
- GCC `-fanalyzer` must pass where configured.
- `make traceability-check` must report zero errors.
- Prohibited platform dependency scan must report zero production violations.
- Public API changes must be documented.
- No unrelated generated files or changes.

## Evidence Required From Implementation Agent

Provide:

1. implementation commit SHA;
2. exact files changed;
3. `git diff --check` result;
4. build commands and results;
5. relevant CTest output;
6. sanitizer/static-analysis results where applicable;
7. coverage result where applicable;
8. traceability result;
9. prohibited-dependency scan result;
10. public API diff/summary;
11. explicit confirmation that out-of-scope platform implementations were not added;
12. working-tree status.

## Expected Files Changed

The implementation agent must list the actual files. Do not pre-authorize unrelated files. Public headers, implementation files, tests, CMake registration, requirements and directly relevant documentation may change.

## Commit

Use exactly:

```text
test(core): complete R04 platform validation
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Validation Evidence (Claude)

Validated release candidate: HEAD `e7df87c` (`build(core): prepare 0.4.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). The validation record itself is committed afterwards as `test(core): complete R04 platform validation` (documentation only). No new production functionality: the task validates the frozen release candidate.

### Candidate history since the Platform Integration Freeze (production freeze point `f7231c1`)
| Commit | Purpose | Production code? |
|---|---|---|
| `460de87` `test(core): add platform conformance suite` | R04-006 (accepted) | no (tests only) |
| `36c5cb8` `feat(core): define runtime platform integration boundary` | R04-007 (accepted): the one reviewed additive exception, `RuntimeManager::attach_platform()`/`platform()` | yes, reviewed |
| `30db43d` `docs(core): define R04 release validation requirement` | defines `CORE-PLAT-011` and its traceability rows | no |
| `e7df87c` `build(core): prepare 0.4.0 release metadata` | `VERSION` and CMake project version 0.4.0; README `find_package(kritva_core 0.4)`; REQUIREMENTS/API titles R0.4; root CHANGELOG 0.4.0 section; install consumer default request 0.4 and a platform adapter attached to the installed `RuntimeManager` | no |

**Production-change audit:** `git diff f7231c1 HEAD -- include src` shows only `include/kritva/core/runtime/runtime_manager.hpp` (+51/-1, the PLATFORM ATTACHMENT contract, forward declaration, two members and a Requirements tag) and `src/runtime_manager.cpp` (+7), exactly the change accepted at R04-007. The scheduler, clock, timer, watchdog, callback, boundary and adapter headers are byte-identical to the freeze. Against the `kritva-core-r0.3` tag the Runtime diff is the same two files only: the frozen Component, ComponentRegistry, DependencyGraph and `runtime.hpp` sources and every R0.3 test are unchanged.

### AC-01 Clean build
`rm -rf` + `cmake -S . -B ...; cmake --build ... -j4` in the fresh clone: Debug exit 0, 0 warnings; Release (`-DCMAKE_BUILD_TYPE=Release`) 0 warnings. GCC 11.4.0, CMake 3.30.2, Linux 6.8. The public-header self-containment check (`kritva_core_header_checks`, one translation unit per public header) is part of the build and passed. No build artifacts are committed.

### AC-02 Complete regression
`ctest`: **32/32 passed** in Debug and in Release. 10 randomized-order repetitions (`--repeat until-fail:10 --schedule-random`): all passed. The complete R0.3 group (`runtime`, `component`, `component_registry`, `dependency_graph`, `runtime_manager`, `runtime_lifecycle`, `runtime_failure`, `runtime_integration`) passes unmodified. No test disabled, skipped or weakened; the only tests removed in R0.4 were two that asserted behavior the new contracts forbid (zero timer period and zero watchdog timeout transported unvalidated), each replaced by contract tests.

### AC-03 Sanitizers and strict quality
ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`): 0 warnings, 32/32. TSan (`-fsanitize=thread`, ASLR disabled with `setarch -R`, the documented environment workaround for this kernel; the suite is single-threaded): 32/32. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: builds, 32/32. GCC `-fanalyzer` over every `src/*.cpp` and the platform and contract tests: no diagnostics.

### AC-04 Coverage
`make coverage` from the fresh clone: **98% (452/457 lines)**, equal to the R0.3 baseline percentage. `runtime_manager.cpp` 125/126, `dependency_graph.cpp` 84/84, `component_registry.cpp` 18/19. The same five uncovered lines as the R0.3 baseline remain, each justified and unchanged: `runtime_manager.cpp:98` (unreachable return after an exhaustive `switch`; it moved from line 91 when 7 lines were added above it), `component_registry.cpp:55` (exception-unwind brace), `lifecycle.cpp:33` (unreachable return after an exhaustive `switch`), `configuration.cpp:40,46` (defensive branches unreachable because `set()` rejects empty names). All new R0.4 production code is header-only contract text or covered (`attach_platform` and `platform()` by the platform integration test, the differential test and the install consumer). No coverage exclusion was introduced.

### AC-05 Traceability
`make check`: header-check passed; `make traceability-check`: **66 requirements, 65 traced (CORE-ERR-003 is the reserved/exempt one), 0 errors, 0 warnings**. `CORE-PLAT-004` to `CORE-PLAT-011` are defined exactly once in `REQUIREMENTS.md` and traced; no duplicate IDs; `CORE-PLAT-001` to `003` were not renumbered.

### AC-06 Freeze verification
See the production-change audit above. No Core API or semantic changed after the Platform Integration Freeze other than the reviewed additive R04-007 attachment.

### AC-07 Install consumer and version validation
`kritva_core_install_consumer` passed in the fresh clone: it installs to a scratch prefix, checks the layout, then configures, builds and runs an external consumer against the installed package only. The consumer exercises the R0.3 runtime (topology freeze, orchestration, failure and `reset()` recovery) and, new in this release, the platform boundary through the installed headers and library: a minimal `IPlatformAdapter` is attached to an installed `RuntimeManager`, `platform()` returns it, a second attach is rejected, every service reports unsupported, and attachment is rejected with `INVALID_STATE` once the topology is fixed. Version validation: the installed `kritva_coreConfigVersion.cmake` reports `PACKAGE_VERSION "0.4.0"`; the consumer checks `kritva_core_VERSION` against the project version (derived, not hard-coded); `find_package(kritva_core 0.4)` accepts `0.4` and `0.4.0` and refuses `0.5`, `0.3` and `9.0`. A wrong expectation (`0.4.1`, `0.3.0`) makes the install test fail (checked manually, both exit code 1).

### AC-08 Dependency / prohibited-header checks
`grep` over `include/` and `src/`: no `<thread> <mutex> <shared_mutex> <condition_variable> <future> <atomic> <semaphore> <iostream> <fstream> <cstdio>`, POSIX, Linux or Windows include; no ROS 2, DDS, EtherCAT, SOEM, FreeRTOS, Zephyr or VxWorks identifier except the sentence in a comment of `platform/boundary.hpp` that names them as forbidden. The traceability audit enforces the threading/logging scan (CORE-RT-010), CORE-GEN-003 and the platform scan (CORE-PLAT-004) automatically. `CMakeLists.txt` has no `find_package` of third-party packages, `FetchContent` or `ExternalProject`. No test-only dependency is linked into the production library. `<atomic>` and `<functional>` appear only in `tests/`. `git diff --check` clean.

### AC-09 Documentation consistency
`REQUIREMENTS.md` (title R0.4, `CORE-PLAT-004..011`), `API.md` (title R0.4; sections 25-30: platform boundary, scheduler, clock and timer, watchdog, adapter, Runtime-platform boundary), `ARCHITECTURE.md` (platform adapter boundary, timer, watchdog, adapter and Runtime-platform paragraphs), `README.md` (`find_package(kritva_core 0.4)`), `TESTING.md` (platform conformance suite, extensible reference doubles, new test levels), root `CHANGELOG.md` (0.4.0 section including the breaking `ITimer` change and known follow-ups), `VERSION` and the CMake project version (both 0.4.0, enforced by the audit) and the R04 planning documents agree with the frozen implementation. The remaining "R0.3" strings are historical.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `e7df87c`; this evidence is committed as `test(core): complete R04 platform validation`. Candidate suitable for the R04 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006).
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged from R0.3).
- The release commit and tag `kritva-core-r0.4` are decided at the R04 Release Gate; nothing is tagged or pushed.

## Reviewer Sign-off

- [ ] Scope satisfied
- [ ] Requirement traceability satisfied
- [ ] Tests satisfied
- [ ] Quality checks satisfied
- [ ] Evidence reproducible
- [ ] Architecture boundary preserved
- [ ] No unresolved blocker

Final reviewer decision is made independently after evidence review.
