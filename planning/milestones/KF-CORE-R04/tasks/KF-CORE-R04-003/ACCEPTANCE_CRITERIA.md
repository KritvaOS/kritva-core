# KF-CORE-R04-003 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-003`
- **Title:** Clock & Timer Contract
- **Requirement:** `CORE-PLAT-006`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define clock and timer platform contracts`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Complete the clock/timer platform boundary while preserving time::IClock as the canonical clock contract.

## Scope

['clock canonicalization', 'clock domains', 'timer lifecycle', 'one-shot/periodic behavior', 'cancellation', 'callback/context lifetime', 'execution context', 'ownership', 'failure semantics']

## Out of Scope

['hardware clock implementation', 'OS timer implementation', 'Core-owned worker thread', 'automatic Runtime recovery']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-006` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['time::IClock remains the canonical clock abstraction.', 'platform::IClock, if retained, remains an alias and not a competing contract.', 'Clock-domain semantics remain explicit and cross-domain timestamps remain incomparable.', 'Timer lifecycle and state transitions are explicitly defined.', 'One-shot and periodic timer behavior is unambiguous.', 'Cancellation semantics and race boundaries are documented.', 'Timer callback and context lifetime are explicit.', 'Timer callback execution context does not imply a Core-owned thread.', 'Invalid duration/error behavior is explicit.', 'Timer ownership and destruction semantics are explicit.']

## New Tests Required

['clock alias/domain tests', 'timer lifecycle tests', 'one-shot/periodic tests', 'cancellation tests', 'callback/context lifetime tests', 'negative duration/error tests']

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
feat(core): define clock and timer platform contracts
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `67114bb` `feat(core): define clock and timer platform contracts` (R04-002 accepted at `537dc82`). **Approved breaking change (design decision Q2):** `time::ITimer::start(Duration)` became `start(Duration period, TimerMode mode, Callback callback)`; `stop()` is unchanged. No other production signature changed; the frozen R0.3 runtime sources are untouched.

- Changed files: `include/kritva/core/time/timer.hpp` (`TimerMode`, new `start`, contract text, tags `CORE-TIME-002, CORE-PLAT-006`), `include/kritva/core/time/clock.hpp` (comment only: timer/clock relation), `tests/contract/reference_timer.hpp` (new reference implementation), `tests/unit/timer_contract_test.cpp` (new), `tests/unit/time_test.cpp` (FakeTimer and callers migrated; the zero-duration pass-through test removed because zero is now `INVALID_ARGUMENT`), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-006`, traceability row), `API.md` (section 27), `ARCHITECTURE.md`.
- Acceptance mapping: `time::IClock` stays canonical and `platform::IClock` stays an alias (unchanged, existing tests); clock domains and non-comparable timestamps unchanged; timers are independent of the clock and carry no domain; lifecycle STOPPED/RUNNING with atomic `start` (`INVALID_ARGUMENT` for period <= 0 or null callback function, null context valid; `INVALID_STATE` while running or from the callback; `UNSUPPORTED`; `RESOURCE_UNAVAILABLE`); ONE_SHOT fires exactly once, no earlier than one period, then is stopped and restartable; PERIODIC fires until `stop`, adapter-defined jitter/overrun, never overlapping; `stop` idempotent and synchronous (race boundary: when it returns no callback is executing or will begin), `INVALID_STATE` instead of self-waiting from the callback; callback invoked by the adapter, never throws, short and non-blocking by default, no Core-owned thread; context opaque, non-owning, valid until `stop` returned (or one-shot callback returned); teardown requires orderly stop; adapter documents resolution and maximum period; no real-time guarantee; expiry never triggers Runtime recovery.
- New in the contract (needs reviewer confirmation): (1) a timer measures elapsed monotonic time and carries no `ClockDomain`; (2) a restarted timer carries nothing over from the previous activation (period, mode, callback, elapsed time); (3) `stop()` from the timer's own callback and `start()` from it both fail with `INVALID_STATE`.
- Tests (`kritva_core_timer_contract`, 9 functions): shape; invalid arguments leave the timer stopped and a null context is valid; one-shot fires once not early and is restartable; periodic fires until stop and nothing after; start while running rejected with no effect; stop idempotent and restart clean; re-entrant start/stop fail without deadlock; adapter limits (resolution, mode, resource) use the defined codes atomically; context never touched. `kritva_core_time` migrated to the new signature.
- Mutation evidence on the reference (13 mutants, each reverted): 11 detected (zero period accepted, null callback accepted, start-while-running accepted, periodic/resolution/resource checks removed, elapsed time not reset on start, stop from callback allowed, one-shot not stopping, off-by-one on the period boundary, callback not stored). 2 survivors are equivalent mutants: dropping `in_callback_` from the start guard (`running_` is already true during any callback) and dropping the `!running_` early return in `advance()` (elapsed time is reset by `start()`, so it is unobservable).
- Build 0 warnings; `ctest` 28/28 in Debug, Release, ASan+UBSan, TSan (ASLR off), strict `-Werror`; `-fanalyzer` clean; coverage 98% (438/443, unchanged uncovered lines); `make check` passes (header-check, traceability, 0 errors); `git diff --check` clean.
- Out of scope confirmed: no OS/hardware timer, no worker thread, no Runtime recovery.

## Reviewer Sign-off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 04-10-2026 |
| Decision | **PASS** |

Accepted commits: `67114bb` (evidence `e40e77c`)

Reviewer notes: the breaking `ITimer::start(period, mode, callback)` change is consistent with design decision Q2 and keeps activation atomic; the three proposed contract statements are APPROVED (no `ClockDomain` in `ITimer`; a restart is a fresh activation; `start()`/`stop()` from the timer's own callback fail with `INVALID_STATE`); the two surviving mutants are equivalent. Short/non-blocking callbacks remain timer-service guidance, not a property of the generic `Callback`. Reviewer could not inspect the local commit remotely and evaluated the supplied evidence.

**Reviewer Decision: PASS — KF-CORE-R04-003 is ACCEPTED.**
