# KF-CORE-R05-006 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-006`
    - **Title:** Platform Integration & Runtime Tests
    - **Requirement:** `CORE-PLAT-017`
    - **Status:** PLANNED
    - **Primary commit message:** `test(core): add platform runtime integration tests`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Runtime with and without platform is differentially validated against the frozen contract.
- [ ] Service availability and failure cases are covered.
- [ ] No direct access to private implementation details is required.
- [ ] Lifecycle ordering, failure propagation, reset and statistics remain unchanged.
- [ ] All existing tests remain green.

    ## New Unit / Contract Tests Required

    - [ ] cross-service integration
- [ ] Runtime differential model
- [ ] service failure integration
- [ ] lifecycle/fault/reset integration
- [ ] statistics integration

    ## Integration Testing Rule

    Integration testing is the primary acceptance mechanism for this task.

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
    test(core): add platform runtime integration tests
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

## Implementation Evidence (Claude)

Primary commit: `fe04d35` `test(core): add platform runtime integration tests` (R05-005 accepted at `8362103`). **No production code changed** (`git diff ea6ae16 HEAD -- include src` is empty since the Platform API Review freeze); no production API added.

- Changed/new files: `tests/integration/runtime_platform_integration_test.cpp` (new, 10 functions), `CMakeLists.txt` (registration), `REQUIREMENTS.md` (`CORE-PLAT-017` and traceability row), `TESTING.md`. It reuses `tests/integration/runtime_scenarios.hpp` (R05-004) and the test-only reference platform (R05-005). Public APIs only (`RuntimeManager`, `PlatformContext`, `PlatformRequirements`, the service contracts, the reference platform); no private detail; no hardware.
- **Central proof: equivalence.** An integrator-written `AppComponent` uses scheduler, clock, timer and watchdog services through a `PlatformContext` (roles: create a task, start the scheduler, start a timer, start and kick the watchdog, read the clock; stop in reverse). A three-component chain whose middle component is an `AppComponent` is run next to the same graph with a plain `ReferenceComponent` failing in the same place. For **every one of the 8 service methods x 3 attempts x 3 error codes (72 cases)**, over three full lifecycle cycles with `reset()` and with the integrator healing its own services between cycles, the two Runtimes show identical states, fault, error code and source, statistics, topology flag, reset cleanup and invocation of the other components, and every injected platform failure really reached the Runtime as a fault.
- Tests (`kritva_core_runtime_platform_integration`): (1) **cross-service integration in dependency order**: four components with different roles; the platform call log is exactly create-task, scheduler start, timer start, watchdog start, kick in dependency order, nothing runs until the integrator advances time, and `stop()` produces exactly watchdog stop, timer stop, scheduler stop (reverse); a second attach never replaces; attachment is closed once the topology is fixed; a Runtime whose components do not use the platform makes zero adapter queries and zero service calls; (2) the 72-case equivalence above; (3) **service availability**: all 16 combinations give the outcome derived independently from what `AppComponent` needs (no scheduler fails `initialize()`, no timer, watchdog or clock fails `start()`, each as `UNSUPPORTED` with the component as source, FAULT, explicit `reset()`, no retry); (4) **requirement gating**: 16 availability combinations x capability present/absent x 4 requirement sets, with an independent oracle: `check_required` and `evaluate` agree with it, checking touches no service (empty call log), an unsatisfied set means the integrator declines to build the Runtime and nothing is left running, a satisfied set runs a full lifecycle; (5) **seeded differential model**: 40 scripts x 40 operations replayed with no platform and with a reference platform for each of the 16 service combinations, with **every platform method failing from its first call** and the platform used between steps: identical results, states, faults, statistics, retries and invocation traces, with zero Runtime queries to the adapter and zero service calls during any operation, and the attachment surviving every operation; (6) **fault, reset and recovery**: a platform timer failure becomes a component failure with the platform's own code and message and the component as source, FAULT accepts only `reset()`, exactly one error and one sample are counted, the faulted component's partial platform state is still the integrator's to clean up, and after the platform recovers a new explicit attempt works with no Runtime retry; (7) **statistics** count component calls (4 per component) and never platform calls (a component making many platform calls changes nothing); (8) **watchdog expiry, callbacks and task entries never enter the Runtime** (still RUNNING, no fault, `reset()` still FAULT-only); (9) the integrator owns the platform (probe: not destroyed by the Runtime or components, four services destroyed with it); (10) attachment closed in READY, RUNNING, STOPPED, FAULT and after `reset()` when no adapter was attached, with the adapter untouched.
- Mutation evidence: replaying the earlier Runtime-coupling, context, consumption and requirement mutants (70) against this test alone killed 31 and left unit-level contract mutants (declaration semantics, message wording, caching) that their own unit tests already killed; the survivors that concern **Runtime integration** (attach allowed after the topology is fixed or in STOPPED or FAULT, a second attach replacing, `reset()`/`shutdown()` detaching, FAULT stopping the watchdog, `configure()` reading the clock) exposed real gaps and were closed by added assertions (a zero adapter-query and empty-service-log check after every Runtime operation in the differential model, the attachment surviving every operation, a no-adapter attachment matrix, a second-attach check), after which all eight are detected. Eleven further Runtime failure-propagation mutants were run (failure replaced by a generic error, source lost, code rewritten, failed call not counted or also counted as a sample, failed stop not recorded as faulted, sequence not fail-fast, reset also stopping faulted components or skipping its shutdown pass, fault code rewritten, fault not cleared by reset): all detected, one (a failed call also counted as a sample) after an added statistics assertion.
- Regression: `ctest` 39/39 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite, including the R0.3 runtime group and the R0.4/R0.5 platform tests, is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (565/571, unchanged); `make check` passes with traceability 72 requirements, 71 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no private access, no new production API, no hardware, no concrete platform.

## Reviewer Sign-Off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `fe04d35` (evidence `f3085d3`) |
| Evidence reference | evidence section above |
| Date | 05-10-2026 |

Reviewer notes: no production code or API change; public APIs only; the 72-case equivalence of a platform failure with an ordinary component failure, the 16-combination availability and gating matrices, the seeded differential model with every platform method failing, fault/reset/recovery, statistics (a failed call counts as an error and never as a sample), watchdog and callback independence, attachment rules and integrator ownership are accepted; the survivors that concerned Runtime integration were closed by added assertions. `CORE-PLAT-017` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R05-006 is ACCEPTED.**

