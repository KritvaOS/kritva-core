# KF-CORE-R05-004 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-004`
    - **Title:** Runtime–Platform Lifecycle Boundary
    - **Requirement:** `CORE-PLAT-015`
    - **Status:** PLANNED
    - **Primary commit message:** `feat(core): preserve runtime platform lifecycle boundary`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Runtime lifecycle behavior remains equivalent with no adapter and an attached reference adapter.
- [ ] attach_platform remains setup-only unless explicitly approved by the frozen model.
- [ ] Runtime does not start/stop scheduler/timer/watchdog implicitly.
- [ ] R0.3 dependency ordering, FAULT/reset and statistics behavior remain unchanged.
- [ ] No background execution is introduced.

    ## New Unit / Contract Tests Required

    - [ ] Runtime differential with/without adapter
- [ ] attach-state rules
- [ ] no implicit service lifecycle
- [ ] fault/reset preservation
- [ ] statistics preservation

    ## Integration Testing Rule

    A mandatory differential integration test compares complete Runtime behavior with no platform and with an attached reference platform.

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
    feat(core): preserve runtime platform lifecycle boundary
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

Primary commit: `f23777b` `feat(core): preserve runtime platform lifecycle boundary` (R05-003 accepted at `d2cd961`). **Contract text plus tests; no Runtime API, state or behavior change** (Q5): `git diff d2cd961 HEAD -- include src` contains only comment lines in `runtime_manager.hpp` (the new RUNTIME AND PLATFORM LIFECYCLE ARE SEPARATE section and the added `CORE-PLAT-015` tag) and **no change to `src/`**; the Runtime lifecycle state machine, `attach_platform()`/`platform()` and every R0.3/R0.4 signature are untouched.

- Changed files: `include/kritva/core/runtime/runtime_manager.hpp` (comment only), `tests/unit/runtime_platform_lifecycle_test.cpp` (new), `tests/integration/runtime_scenarios.hpp` (new: seeded Runtime scenarios replayable with and without a platform, reusable by R05-006), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-015` and traceability row), `API.md` (section 34), `ARCHITECTURE.md`.
- Contract made explicit: two independent lifecycles; the integrator or adapter creates, configures, starts, stops and destroys platform services and Core never does; attachment and every Runtime operation in every state (FAULT and `reset()` included) never start, stop, create, configure, tick, poll or recover a service, never read a clock, never call the adapter, never create a thread or loop; nothing flows back (attachment, provided services, capabilities and service success, failure or expiry change no state, order, fail-fast, FAULT, reset or statistic); a platform failure becomes a Runtime failure only through an integrator Component returning it as its own failed `Result` and is then an ordinary component failure; a watchdog expiry never causes FAULT/reset/recovery and a Runtime FAULT never touches a watchdog; `attach_platform()` stays setup-only (R0.5 changes nothing); the supported consumption path is a `PlatformContext` built from the adapter or `platform()`, components do not receive a raw `IPlatformAdapter*`, the Component lifecycle signature is unchanged.
- Unit/contract tests (`kritva_core_runtime_platform_lifecycle`, 7 functions): **attach-state rules** in every Runtime state (allowed before the first successful `initialize()` and after `configure()`; `INVALID_STATE` with no effect in READY, RUNNING, STOPPED, after `shutdown()`, in a second live period, in FAULT and after `reset()`, with and without an adapter; an invalid topology that fails `initialize()` leaves attachment open; a second attach never replaces); **no implicit service lifecycle**: 60 random scenarios with a scheduler task, a periodic timer and a watchdog started by the integrator: after every Runtime operation the complete service state (running flags, every call counter, timer firings, watchdog expiries, task runs) is identical, the attachment survives (`reset()` included) and the Runtime made zero calls to the adapter; services stay running through FAULT and `shutdown()`; when the integrator starts nothing the Runtime starts nothing; **no background execution**: task and timer counters stay 0 across the Runtime lifecycle and advance only when the test ticks the scheduler or advances the timer; **differential**: 30 seeded scripts x 40 operations are replayed with no adapter and with a reference platform for **each of the 16 service combinations**, with the integrator using a `PlatformContext`, `require_*`, capability queries and `check_required` between every step (the Runtime made no call to the platform during any operation): the complete transcript (results with code, severity, source and message, state, topology flag, fault, statistics, retries, and the component invocation trace) is identical to the no-adapter baseline, over scenarios containing more than 100 failure or invalid-state steps; **a platform failure is an ordinary component failure**: a three-component chain whose middle component fails `start()` because the platform has no timer yields the same states, fault, statistics, source and reference-component trace as the same graph with a plain injected `UNSUPPORTED` failure; **watchdog expiry**: 30 scenarios with a watchdog that expires and stays expired in every state produce transcripts identical to the no-platform baseline, and the Runtime never kicked, stopped or restarted it.
- Mutation evidence on the production Runtime (20 mutants, each reverted, all detected): attach allowed after the topology is fixed, in STOPPED or in FAULT; a second attach replacing; `reset()` or `shutdown()` detaching the platform (the `reset()` mutant survived the first run and was closed by asserting the attachment after every step); `initialize()` starting the timer; `start()` creating a task; `stop()` stopping the scheduler; `shutdown()` stopping the timer; FAULT entry stopping the watchdog; `reset()` kicking the watchdog; `configure()` reading the clock; attach probing the adapter; `start()` failing on a platform without a scheduler; `initialize()` counting an error when an adapter is attached; `stop()` entering FAULT without a watchdog; `reset()` refusing when a platform is attached; the Runtime deleting the adapter; the topology fixed differently with a platform.
- Regression: `ctest` 36/36 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite, including the complete R0.3 runtime group and the R0.4 `runtime_platform` differential, is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (565/571; the same baseline lines plus the one exception-unwind brace in `requirements.hpp`); `make check` passes with traceability 70 requirements, 69 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete adapter, no Runtime asynchronous redesign, no automatic recovery, no new Runtime lifecycle state.

## Reviewer Sign-Off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `f23777b` (evidence `5ab932f`) |
| Evidence reference | evidence section above |
| Date | 05-10-2026 |

Reviewer notes: contract text and tests only, with no `src/` behavior change, no Runtime API change and no lifecycle semantic change; `attach_platform()`/`platform()` are preserved; platform services remain integrator-owned and the Runtime never starts, stops, configures, ticks, polls, recovers or reads them; 20/20 mutants detected including the previously surviving `reset()` detachment mutant. `CORE-PLAT-015` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R05-004 is ACCEPTED.**

