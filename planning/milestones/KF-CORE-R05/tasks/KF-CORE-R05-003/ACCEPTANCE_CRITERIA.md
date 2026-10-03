# KF-CORE-R05-003 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-003`
    - **Title:** Explicit Platform Service Consumption
    - **Requirement:** `CORE-PLAT-014`
    - **Status:** PLANNED
    - **Primary commit message:** `feat(core): define explicit platform service consumption`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Service access is explicit and non-owning.
- [ ] Unavailable services are handled according to documented error semantics.
- [ ] Platform service errors are preserved or explicitly translated.
- [ ] No service is started/stopped implicitly.
- [ ] Callback/lifetime/re-entry rules remain compatible with R0.4 contracts.

    ## New Unit / Contract Tests Required

    - [ ] scheduler access
- [ ] clock access
- [ ] timer access
- [ ] watchdog access
- [ ] service failure propagation
- [ ] unavailable service

    ## Integration Testing Rule

    Integration tests must prove explicit service consumption and error propagation through a reference platform.

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
    feat(core): define explicit platform service consumption
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

Primary commit: `f8cd523` `feat(core): define explicit platform service consumption` (R05-002 accepted at `120d7c8`). **Purely additive:** four new const members on `PlatformContext` (`platform/context.hpp`, added in R05-001 and not yet frozen) and one private helper; no R0.4 signature changed, no `IPlatformAdapter`, service or `RuntimeManager` change; `PlatformContext` is still one pointer, trivially destructible, with no added state (both asserted by tests).

- Changed files: `include/kritva/core/platform/context.hpp` (members, contract text, tag `CORE-PLAT-014`), `tests/unit/platform_service_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-014` and traceability row), `API.md` (section 33), `ARCHITECTURE.md`.
- API (approved Q4): `Result<IScheduler*> require_scheduler() const`, `Result<time::IClock*> require_clock() const`, `Result<time::ITimer*> require_timer() const`, `Result<IWatchdog*> require_watchdog() const`. Success: the adapter-owned, non-owning, non-null pointer (the same one the accessor returns). Failure: `ErrorCode::UNSUPPORTED`, `ErrorSeverity::ERROR`, no component source, when unattached or unsupported; the message names the service and whether "no platform is attached" or it is "not provided by the platform" (the code is the contract).
- Contract: requiring is a query (exactly one accessor call; never starts, stops, configures, creates or owns a service; nothing implicit when a context is copied or destroyed). **Service errors are never translated**: a service's own `Error` keeps code, severity and message; Core's only Error here is the `UNSUPPORTED` above; a Component that propagates a platform Error sets `source = info().id()` and leaves code and message unchanged and the Runtime propagates it under the R0.3 semantics; no wrapping helper added. R0.4 callback, lifetime and re-entry rules unchanged.
- Unit/contract tests (`kritva_core_platform_service`, 13 functions): signatures and "still one pointer"; scheduler, clock, timer and watchdog access (the pointer is the adapter's own, usable through its own contract, and not started by obtaining it: a task runs only when the test starts the scheduler; a one-shot fires only when the test advances the timer; the watchdog is started, kicked and stopped by the caller); unavailable services for an unattached context and for an attached context that lacks the service, each independent, deterministic, UNSUPPORTED/ERROR/no source with a message naming only that service and the right situation; all 16 service combinations agree with the adapter and `supports()`, and a success is never `nullptr`; requiring is a query (a spy adapter: exactly one accessor call per require, no service activation, nothing running); nothing started or stopped implicitly and a service the integrator started stays running after every context is gone; **service errors are never translated** (the timer's `RESOURCE_UNAVAILABLE` "no timer resource", the watchdog's `UNSUPPORTED` range error and `INVALID_ARGUMENT` pass straight through and differ from Core's own unavailable-service message); **integration through public APIs**: an integrator Component using a context fails in `start()` with the platform's own code and message and `source` = its id, the Runtime enters FAULT and propagates it unchanged, and `reset()` recovers explicitly (also for the unavailable-service case); callback/re-entry: a timer callback that carries a context re-enters its own timer through a pointer obtained from it and gets `INVALID_STATE` for both `stop()` and `start()` exactly as in R0.4; `check_required` and `require_*` used together.
- Mutation evidence (20 mutants, each reverted): 19 detected — success with `nullptr` when unavailable, three wrong codes/severities/sources, message omitting or fixing the situation, naming the wrong service, `require_scheduler`/`require_timer` gated on another service or returning null, requiring that starts the scheduler or the watchdog or stops the timer, a second accessor call, a context destructor that stops the watchdog (caught at compile time by the trivially-destructible assertion), cached state in the context (caught by the `sizeof` assertion). The 20th (a pointer "copy") is an equivalent mutant: the replacement expression evaluates to the same pointer.
- Regression: `ctest` 35/35 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); whole existing suite unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (562/568: `context.hpp` fully covered, plus the one exception-unwind brace in `requirements.hpp` and the five baseline lines); `make check` passes with traceability 69 requirements, 68 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete service implementation, no implicit service orchestration, no automatic retry, no watchdog-driven Runtime recovery, no error-wrapping helper.

## Reviewer Sign-Off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `f8cd523` (evidence `cd3010f`) |
| Evidence reference | evidence section above |
| Date | 05-10-2026 |

Reviewer notes: four additive explicit `require_*()` queries that return the adapter-owned pointer or `UNSUPPORTED`; query-only semantics with no activation; platform service errors remain unchanged and are attributed to a component only by that component; the one-pointer invariant of `PlatformContext` is preserved (no cache or mini-registry); 19/20 mutants detected and the 20th is an equivalent mutant. `CORE-PLAT-014` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R05-003 is ACCEPTED.**

