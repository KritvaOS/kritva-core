# KF-CORE-R05-005 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-005`
    - **Title:** Reference Platform Integration
    - **Requirement:** `CORE-PLAT-016`
    - **Status:** PLANNED
    - **Primary commit message:** `test(core): add platform integration reference harness`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Reference adapter implements only test-side contracts.
- [ ] Fake scheduler/clock/timer/watchdog support deterministic success/failure/availability cases.
- [ ] Tests can vary supported services and capabilities.
- [ ] Lifetime and callback behavior can be observed without hardware.
- [ ] No reference platform code enters production targets.

    ## New Unit / Contract Tests Required

    - [ ] reference adapter availability
- [ ] fake scheduler behavior
- [ ] fake clock behavior
- [ ] fake timer behavior
- [ ] fake watchdog behavior
- [ ] test-target isolation

    ## Integration Testing Rule

    The reference harness itself is test infrastructure; it must be exercised by at least one end-to-end public-API integration test.

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
    test(core): add platform integration reference harness
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

Primary commit: `7f30626` `test(core): add platform integration reference harness` (Platform API Review PASS / FROZEN recorded at `ea6ae16`). **No production code changed** (`git diff ea6ae16 HEAD -- include src` is empty); no production API added; the frozen R05 API is consumed, not modified.

- Changed/new files: `tests/platform/reference_platform.hpp` (new, test-only), `tests/unit/reference_platform_test.cpp` (new, 16 functions), `tests/audit/production_isolation.cmake` (new CTest script), `scripts/audit/check_traceability.py` (isolation check), `CMakeLists.txt` (two test registrations), `REQUIREMENTS.md` (`CORE-PLAT-016` and traceability row), `TESTING.md`.
- Reference platform: `ReferencePlatform` implements only the public `IPlatformAdapter`, `IScheduler`, `time::IClock`, `time::ITimer` and `IWatchdog` contracts on top of the existing `tests/contract/` doubles (`FakeScheduler`, `FakeClock`, `FakeTimer`, `FakeWatchdog`). Controls: selectable services and capabilities fixed at construction (the contract requires stable objects, so availability is never changed later); **deterministic fault injection** per service method (`fail_nth`, `fail_all`, `clear_faults`, `reset_counts`): an injected failure is atomic (no state change, no id or slot consumed), counted as a call and logged; an **ordered call log** of every service call, timer callback and scheduler task entry with outcome and code; a **count of queries made to the adapter itself**, so a caller's silence is observable; a controllable clock (a MONOTONIC clock refuses to go backward, a REALTIME clock may); **explicit time advancement** (`let_time_pass` ticks the scheduler, advances the timer and clock; the watchdog advances only by an explicit call) with nothing advancing by itself (no thread, loop or real time); a `LifetimeProbe` owned by the test; a conformance `Environment` wired to the platform. A bug found while writing the tests (a rejected `start()` of a running timer cleared its active callback) was fixed in the fake and is now covered by a test.
- Unit/contract tests (`kritva_core_reference_platform`): implements only public contracts (static_asserts); availability for all 16 service combinations with stable objects; identity and capabilities selectable and deterministic; adapter queries counted and service calls not counted as queries; fault injection for the scheduler (Nth call, atomic: no id or slot consumed, `fail_all`, `clear_faults`, log entries with codes), the timer and the watchdog (atomic, state unchanged, the watchdog window restarts only on a real kick); `reset_counts` restarts Nth numbering; the clock is controllable and keeps its domain; nothing advances by itself (a started scheduler, timer and watchdog do nothing until the test advances time, and `let_time_pass` never advances the watchdog); callbacks and task entries logged in order and not after `stop()`; re-entrant use from a callback is observable (the R0.4 `INVALID_STATE` appears in the call log); lifetime observation (contexts never destroy the platform; destroying it destroys its four services); the platform **passes the platform conformance suite for every service combination** and **the suite detects injected faults** (a forbidden code is rejected, a permitted capability limit is a skip, an `UNSUPPORTED` that masks argument validation is rejected); deterministic replay; a rejected timer start never disturbs the running activation.
- **Test-target isolation** (approved Q8, two layers plus the build description): (1) the repository audit (`CORE-PLAT-016`) fails when any file under `include/` or `src/` includes anything under `tests/` or a reference, fake or test-double header, when the production library target lists a test source, or when an `install()` rule installs test support; (2) the CTest `kritva_core_test_isolation` runs the same static include scan and also **compiles every production translation unit and the umbrella header with only `include/` on the include path**, so `tests/` is not reachable. I verified the guards by planting each violation (a production header including `tests/platform/reference_platform.hpp`, a test source in the production target, an `install(DIRECTORY tests/platform ...)` rule): both the audit and the CTest script fail for each, and both pass on the clean tree.
- Mutation evidence on the reference platform (40 mutants, each reverted): 39 detected — `fail_nth` off by one, `fail_all` ignored, every call failing, rules matching other methods, injected calls not counted, `clear_faults`/`reset_counts`/`clear_log` doing nothing, non-atomic injection for scheduler create/start, timer start/stop and watchdog calls, failures or successes not logged, failures logged as ok, callbacks or task entries not logged, adapter queries not counted for info/timer/capabilities, services mapped to the wrong flag or always/never provided, capabilities empty, a MONOTONIC clock allowed to go backward or advanced by negative time, clock domain ignored, `let_time_pass` missing a tick, ticking one too many, not advancing the timer or the clock, platform or service destruction not observed, a no-op conformance environment, identity ignored. Three mutants survived the first run (an injected scheduler-start failure not logged, a rejected timer start disturbing the running callback, `let_time_pass` advancing the watchdog) and were closed by added assertions. The one remaining survivor (a rejected scheduler creation keeping its unused callback wrapper in memory) is equivalent: it has no observable effect.
- Regression: `ctest` 38/38 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (565/571, production lines unchanged); `make check` passes with traceability 71 requirements, 70 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no hardware, no production platform adapter, no OS dependency, no production service implementation, no production target links any reference-platform code.

    ## Reviewer Sign-Off

    | Item | Result |
    |---|---|
    | Reviewer | ChatGPT architecture/review gate |
    | Decision | PENDING |
    | Accepted commit | PENDING |
    | Evidence reference | PENDING |
    | Date | PENDING |

    **Reviewer Decision:** PENDING
