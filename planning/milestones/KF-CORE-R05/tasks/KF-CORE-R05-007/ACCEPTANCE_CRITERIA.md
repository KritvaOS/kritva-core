# KF-CORE-R05-007 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-007`
    - **Title:** Full R0.5 Validation
    - **Requirement:** `CORE-PLAT-018`
    - **Status:** PLANNED
    - **Primary commit message:** `test(core): complete R0.5 validation`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Fresh clean Debug and Release builds pass.
- [ ] Full CTest regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict warnings and GCC analyzer pass where configured.
- [ ] Coverage is reviewed.
- [ ] Traceability has zero errors.
- [ ] Install-consumer passes.
- [ ] Prohibited dependency audit passes.
- [ ] Production diff since Integration Freeze contains only authorized release-candidate changes.

    ## New Unit / Contract Tests Required

    - [ ] release validation is primarily a validation task; no new feature tests unless a validation gap is found

    ## Integration Testing Rule

    No new integration architecture is introduced; rerun the complete existing integration suite and release checks.

    - [ ] Integration tests use only public APIs.
    - [ ] No physical hardware is required.
    - [ ] Test-only reference/fake services are used where needed.
    - [ ] No production test hook or private API is introduced solely to make integration testing possible.

    ## Regression Testing Rule

    Every task must run the complete existing regression suite, not only newly added tests.

    ```bash
    cmake -S . -B build
    cmake --build build -j$(nproc)
    ctest --test-dir build --output-on-failure
    ```

    Required where supported by the repository:

    ```bash
    make check
    make traceability-check
    ```

    - [ ] New/focused tests pass.
    - [ ] Full CTest regression passes.
    - [ ] No previously passing test regresses.
    - [ ] Regression output is supplied as evidence.

    ## Quality Requirements

    - [ ] No compiler warnings in the supported clean build.
    - [ ] `-Werror` passes where configured.
    - [ ] ASan/UBSan passes where configured.
    - [ ] TSan passes where configured for concurrency-sensitive changes.
    - [ ] GCC `-fanalyzer` passes where configured.
    - [ ] Coverage is reviewed.
    - [ ] Traceability audit reports zero errors.
    - [ ] Prohibited dependency audit reports zero production violations.
    - [ ] Public API changes are documented.
    - [ ] No unrelated generated files or changes are present.

    ## Expected Files Changed

    Actual changed files must be listed by the implementation agent. Do not pre-authorize unrelated files.

    Expected categories may include:

    - public headers;
    - production source where required;
    - focused unit/contract tests;
    - integration tests;
    - CMake/test registration;
    - `REQUIREMENTS.md` when the proposed requirement becomes authoritative;
    - directly relevant architecture/API documentation.

    ## Commit

    Use exactly:

    ```text
    test(core): complete R0.5 validation
    ```

    One logical task = one primary implementation commit.

    ## Evidence Required From Implementation Agent

    1. [ ] Primary commit SHA.
    2. [ ] Exact changed files.
    3. [ ] `git diff --check` result.
    4. [ ] Build command/result.
    5. [ ] Focused unit-test command/result.
    6. [ ] Full regression command/result.
    7. [ ] Integration-test command/result, where applicable.
    8. [ ] Sanitizer/static-analysis result, where applicable.
    9. [ ] Coverage result, where applicable.
    10. [ ] Traceability result.
    11. [ ] Prohibited-dependency scan result.
    12. [ ] Public API diff/summary.
    13. [ ] Explicit out-of-scope confirmation.
    14. [ ] Final working-tree status.

## Validation Evidence (Claude)

Validated release candidate: HEAD `5fb5e69` (`build(core): prepare 0.5.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). The validation record itself is committed afterwards as `test(core): complete R0.5 validation` (documentation only). No new production functionality: the task validates the frozen release candidate.

### Candidate history since the Platform Integration Freeze (production freeze point `f23777b`)
| Commit | Purpose | Production code? |
|---|---|---|
| `7f30626` `test(core): add platform integration reference harness` | R05-005 (accepted) | no (tests and audit tooling) |
| `fe04d35` `test(core): add platform runtime integration tests` | R05-006 (accepted) | no (tests only) |
| `946dd5d` `docs(core): define R05 release validation requirement` | defines `CORE-PLAT-018` and its traceability rows | no |
| `5fb5e69` `build(core): prepare 0.5.0 release metadata` | `VERSION` and CMake project version 0.5.0; README `find_package(kritva_core 0.5)`; REQUIREMENTS/API titles R0.5; TESTING; root CHANGELOG 0.5.0 section; install consumer default request 0.5 and a `PlatformContext`, requirement check and explicit service consumption built against the installed package | no |

**Production-change audit:** `git diff f23777b HEAD -- include src` is **empty**: production code is byte-identical to the Platform Integration Freeze point. Against the released `kritva-core-r0.4` tag, `src/` is unchanged and the only production headers that differ are the reviewed R05 set: `platform/context.hpp` (new), `platform/requirements.hpp` (new), `runtime/runtime_manager.hpp` (comment only) and `core.hpp` (two includes). Every R0.4 and R0.3 frozen header is byte-identical to the tag.

### AC-01 Clean build
Fresh clone: Debug exit 0, 0 warnings; Release (`-DCMAKE_BUILD_TYPE=Release`) 0 warnings. GCC 11.4.0, CMake 3.30.2, Linux 6.8. The public-header self-containment check (`kritva_core_header_checks`, one translation unit per public header) is part of the build and passed. No build artifacts are committed.

### AC-02 Complete regression
`ctest`: **39/39 passed** in Debug and in Release (unit, contract, conformance, platform, integration, isolation and install tests). 10 randomized-order repetitions (`--repeat until-fail:10 --schedule-random`): all passed. The complete R0.3 runtime group, the R0.4 platform, conformance and runtime-platform tests, and every R0.5 test pass unmodified. No test disabled, skipped or weakened.

### AC-03 Sanitizers and strict quality
ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`): 0 warnings, 39/39. TSan (`-fsanitize=thread`, ASLR disabled with `setarch -R`, the documented environment workaround for this kernel; the suite is single-threaded): 39/39. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: builds, 39/39. GCC `-fanalyzer` over every `src/*.cpp` and the platform, contract and integration tests: no diagnostics.

### AC-04 Coverage
`make coverage` from the fresh clone: **98% (565/571 lines)**, equal to the R0.4 percentage. `context.hpp` 19/19; `requirements.hpp` 50/51 (the one uncovered line is the exception-unwind closing brace of `evaluate()`, of the same kind as `component_registry.cpp:55`); `runtime_manager.cpp` 125/126, `dependency_graph.cpp` 84/84. The five baseline uncovered lines are unchanged and justified: `runtime_manager.cpp:98` and `lifecycle.cpp:33` (unreachable returns after exhaustive `switch`es), `component_registry.cpp:55` (exception-unwind brace), `configuration.cpp:40,46` (defensive branches unreachable because `set()` rejects empty names). No coverage exclusion was introduced.

### AC-05 Traceability
`make check`: header-check passed; `make traceability-check`: **73 requirements, 72 traced (CORE-ERR-003 is the reserved/exempt one), 0 errors, 0 warnings**. `CORE-PLAT-012` to `CORE-PLAT-018` are defined exactly once in `REQUIREMENTS.md` and traced; `CORE-PLAT-001` to `011` were not renumbered.

### AC-06 Freeze verification
See the production-change audit above: no production change after the freeze.

### AC-07 Install consumer and version validation
`kritva_core_install_consumer` passed in the fresh clone: it installs to a scratch prefix, checks the layout, then configures, builds and runs an external consumer against the installed package only. The consumer exercises the R0.3 runtime, the R0.4 platform boundary (an `IPlatformAdapter` attached to the installed `RuntimeManager`, second attach rejected, attachment closed after the topology is fixed) and, new in this release, the R0.5 integration foundation through the installed headers and library: a `PlatformContext` built from `runtime.platform()` (attached, identity, every service unsupported, no capability), explicit consumption (`require_scheduler()` and `require_watchdog()` fail with `UNSUPPORTED`), an unattached context, and `PlatformRequirements` (a duplicate by identity rejected, optional-only requirements satisfied but incomplete, a missing required service failing `check_required` with `UNSUPPORTED`). Version validation: the installed `kritva_coreConfigVersion.cmake` reports `PACKAGE_VERSION "0.5.0"`; the consumer checks `kritva_core_VERSION` against the project version (derived, not hard-coded); `find_package(kritva_core 0.5)` accepts `0.5` and `0.5.0` and refuses `0.6`, `0.4` and `9.0`. A wrong expectation (`0.5.1`, `0.4.0`) makes the install test fail (checked manually, both exit code 1).

### AC-08 Dependency / prohibited-header checks
`grep` over `include/` and `src/`: no `<thread> <mutex> <shared_mutex> <condition_variable> <future> <atomic> <semaphore> <iostream> <fstream> <cstdio>`, POSIX, Linux or Windows include; no ROS 2, DDS, EtherCAT, SOEM, FreeRTOS, Zephyr or VxWorks identifier except the sentence in a comment of `platform/boundary.hpp` naming them as forbidden. The traceability audit enforces the threading/logging scan (CORE-RT-010), CORE-GEN-003, the platform scan (CORE-PLAT-004) and the test-only isolation checks (CORE-PLAT-016); the CTest `kritva_core_test_isolation` compiles every production translation unit and the umbrella header with only `include/` on the include path. `CMakeLists.txt` has no `find_package` of third-party packages, `FetchContent` or `ExternalProject`; no `install()` rule installs test support. `<atomic>` and `<functional>` appear only in `tests/`. `git diff --check` clean.

### AC-09 Documentation consistency
`REQUIREMENTS.md` (title R0.5, `CORE-PLAT-004..018`), `API.md` (title R0.5; sections 25-34), `ARCHITECTURE.md`, `README.md` (`find_package(kritva_core 0.5)`), `TESTING.md` (conformance suite, reference platform and isolation, integration tests), root `CHANGELOG.md` (0.5.0 section with the known follow-ups), `VERSION` and the CMake project version (both 0.5.0, enforced by the audit) and the R05 planning documents agree with the frozen implementation. The remaining "R0.4" strings are historical.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `5fb5e69`; this evidence is committed as `test(core): complete R0.5 validation`. Candidate suitable for the R05 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3 and R0.4.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006); a `PlatformContext` outliving its adapter is documented undefined behavior (non-owning by design).
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- The release commit and tag `kritva-core-r0.5` are decided at the R05 Release Gate; nothing is tagged or pushed.

    ## Reviewer Sign-Off

    | Item | Result |
    |---|---|
    | Reviewer | ChatGPT architecture/review gate |
    | Decision | PENDING |
    | Accepted commit | PENDING |
    | Evidence reference | PENDING |
    | Date | PENDING |

    **Reviewer Decision:** PENDING
