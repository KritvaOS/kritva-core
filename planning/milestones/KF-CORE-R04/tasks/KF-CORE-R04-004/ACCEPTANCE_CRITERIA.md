# KF-CORE-R04-004 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-004`
- **Title:** Watchdog Contract
- **Requirement:** `CORE-PLAT-007`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define watchdog platform contract`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define a complete watchdog adapter contract without coupling watchdog expiration to Runtime recovery.

## Scope

['watchdog state', 'start/kick/stop', 'timeout validation', 'idempotency', 'failure propagation', 'hardware/software boundary', 'expiry semantics']

## Out of Scope

['hardware watchdog driver', 'automatic Runtime reset/recovery', 'background watchdog service']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-007` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Watchdog initial and running/stopped states are explicit.', 'start, kick and stop semantics are deterministic.', 'Zero and negative timeout behavior is explicitly defined.', 'Repeated start/stop behavior is defined.', 'Kick behavior outside the running state is defined.', 'Failures preserve existing Error/Result semantics.', 'Watchdog expiry behavior is explicitly bounded to the adapter contract.', 'Watchdog expiry does not implicitly trigger RuntimeManager::reset().', 'No Core background worker is required by the contract.']

## New Tests Required

['state tests', 'timeout validation', 'idempotency', 'kick semantics', 'failure propagation', 'no implicit Runtime recovery']

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
feat(core): define watchdog platform contract
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `4af4756` `feat(core): define watchdog platform contract` (R04-003 accepted at `8bc8a48`). Production change: documentation/contract text in `platform/watchdog.hpp` only; **no signature change** (`start(Duration)`, `kick()`, `stop()` kept); the frozen R0.3 runtime sources are unchanged.

- Changed files: `include/kritva/core/platform/watchdog.hpp` (contract text, tags `CORE-PLAT-003, CORE-PLAT-007`), `tests/contract/reference_watchdog.hpp` (new reference), `tests/unit/watchdog_contract_test.cpp` (new, 10 functions), `tests/unit/platform_test.cpp` (the "zero timeout transport" test removed: it asserted that zero is accepted, which the contract now rejects; the permissive FakeWatchdog tests remain), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-007`, traceability row), `API.md` (section 28), `ARCHITECTURE.md`.
- Contract: STOPPED (initial) / RUNNING. `start(timeout)` only while stopped, begins the first window; atomic failures `INVALID_ARGUMENT` (zero/negative), `INVALID_STATE` (already running; never silently reconfigured), `UNSUPPORTED` (range/resolution), `RESOURCE_UNAVAILABLE`. `kick()` restarts the window, valid only while running (`INVALID_STATE` and no effect otherwise; it never starts the watchdog). `stop()` idempotent; an adapter that cannot disable its watchdog returns `UNSUPPORTED` and it stays RUNNING. A restart is a fresh activation. Expiry action is adapter-defined and outside Core; expiry never triggers `RuntimeManager::reset()` or any Runtime recovery; Core never kicks, starts or stops on the caller's behalf; no Core worker. Failures via `Result`/`Error`, atomic, no throw. Integrator owns the watchdog; thread safety and permitted kick contexts adapter-defined; no real-time guarantee.
- New in the contract (needs reviewer confirmation): (1) `start()` while running is `INVALID_STATE` rather than an idempotent no-op or a reconfiguration; (2) `kick()` while stopped is `INVALID_STATE` and does not start the watchdog; (3) `stop()` may be `UNSUPPORTED` for adapters (typically hardware) that cannot disable the watchdog, leaving it running.
- Tests (`kritva_core_watchdog_contract`): shape; initial state; timeout validation (zero, negative) leaving it stopped; adapter limits with defined codes atomically; start while running rejected without reconfiguring; kick restarts the window; kick outside running fails and never starts; stop idempotent and restart fresh (including after an expiry); stop `UNSUPPORTED` keeps running; **no implicit Runtime recovery**: a real `RuntimeManager` in RUNNING stays RUNNING, with no fault and unchanged error statistics, while a never-kicked watchdog expires, and `reset()` is still rejected outside FAULT.
- Mutation evidence on the reference (15 mutants, each reverted; all detected): zero timeout accepted, start while running accepted, range checks removed (min, max), resource check removed, kick on stopped accepted or silently starting, kick not restarting the window, stop ignoring `can_stop`, stop not stopping, elapsed time not reset on start, off-by-one at expiry, stop on a stopped watchdog failing, expiry flag not cleared on restart (found by a first run and closed with an added restart-after-expiry assertion).
- Build 0 warnings; `ctest` 29/29 in Release, ASan+UBSan, strict `-Werror`, TSan (ASLR off), and the default Debug build; `-fanalyzer` clean; coverage 98% (438/443, unchanged uncovered lines); `make check` passes; `git diff --check` clean.
- Out of scope confirmed: no hardware watchdog driver, no Runtime reset/recovery, no background service.

## Reviewer Sign-off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 04-10-2026 |
| Decision | **PASS** |

Accepted commits: `4af4756` (evidence `9216847`)

Reviewer notes: the minimal three-method `IWatchdog` API is retained (design decision Q3) and no `state()` accessor is added; the three proposed contract statements are APPROVED (`start()` while RUNNING is `INVALID_STATE`; `kick()` while STOPPED is `INVALID_STATE` and never starts the watchdog; `stop()` may return `UNSUPPORTED` and leave the watchdog RUNNING). Expiry is bounded to the adapter contract and never triggers Runtime recovery. Reviewer evaluated the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R04-004 is ACCEPTED.**
