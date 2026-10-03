# KF-CORE-R04-006 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-006`
- **Title:** Platform Conformance Tests
- **Requirement:** `CORE-PLAT-009`
- **Status:** PLANNED
- **Primary commit message:** `test(core): add platform conformance suite`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Provide reusable contract tests for future platform adapters using public APIs and no physical hardware.

## Scope

['scheduler conformance', 'clock conformance', 'timer conformance', 'watchdog conformance', 'adapter-defined behavior separation', 'test-only fakes', 'mutation testing']

## Out of Scope

['real Linux adapter', 'real RTOS adapter', 'hardware CI requirement', 'platform implementation']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-009` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Conformance tests use only public Core APIs.', 'Tests run without hardware, network or vendor SDK.', 'Mandatory Core semantics are tested independently from adapter policy.', 'Adapter-defined behavior is not incorrectly asserted as universal.', 'Negative and failure paths are covered.', 'Tests are reusable by future external adapters.', 'Mutation testing demonstrates that important contract violations are detected.', 'No production API is added solely to make tests easier.', 'Existing R0.3 regression remains green.']

## New Tests Required

['all platform contract suites', 'negative cases', 'permutation/ordering where applicable', 'mutation testing', 'full regression']

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
test(core): add platform conformance suite
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `460de87` `test(core): add platform conformance suite` (R04-005 accepted at `71ea9d3`). **No production code changed** (`git diff 71ea9d3 460de87 -- include src` is empty); the only edits to existing test files remove `final` from the reference test doubles so faulty variants can derive from them.

- Changed/new files: `tests/platform/` (new, reusable, header-only): `conformance.hpp` (`Report`, `Environment`, `KRITVA_CONFORMANCE_CHECK`), `scheduler_conformance.hpp`, `clock_conformance.hpp`, `timer_conformance.hpp`, `watchdog_conformance.hpp`, `adapter_conformance.hpp`; `tests/unit/platform_conformance_test.cpp` (new, runs and validates the suite); `tests/contract/reference_*.hpp` (`final` removed); `CMakeLists.txt`; `REQUIREMENTS.md` (`CORE-PLAT-009`, traceability row); `TESTING.md` (conformance suite section).
- Design: an external adapter builds an `Environment` (`let_time_pass` sleeps or advances a fake clock; timer period, task period and watchdog timeout), constructs a FRESH service or adapter and calls `check_scheduler`, `check_clock`, `check_timer`, `check_watchdog`, `check_adapter_description` or `check_platform_adapter`; failures are collected in a `Report` (with explicit skips) instead of aborting, so adapters can print them. Only public Core headers and the standard library are used; no hardware, network or vendor SDK; no production API was added.
- Mandatory vs adapter-defined: the suite asserts only what CORE-PLAT-001..008 mandate (argument validation and codes, state transitions, idempotency, atomic failure, no entry/callback before start or after stop returned, one-shot exactly once and restartable, re-entrancy gives `INVALID_STATE`, identity/discovery/capability determinism). Everything adapter-defined is accepted in all permitted forms and never asserted universally: priority range, affinity support, capacity, dynamic task creation, periodic timer support, timer resolution/period range, watchdog timeout range, whether a watchdog can be stopped. A feature an adapter legitimately cannot provide (`UNSUPPORTED`/`RESOURCE_UNAVAILABLE`) is a recorded skip. Any other code is a failure. Documented limits: it cannot observe a hardware watchdog's expiry action or timing accuracy/jitter.
- Suite validated against conforming adapters under different adapter policies: scheduler dynamic creation on/off x capacity 2/4/64 x affinity supported/unsupported; timer with and without periodic support, coarse resolution, no resource, configured period below resolution; watchdog stoppable and unstoppable, out-of-range timeout, no resource; REALTIME clock that steps backward (allowed); the adapter check over all 16 service combinations. The suite never lets a watchdog expire.
- Mutation testing, level 1 (faulty test doubles the suite must reject, each asserted): 69 faulty doubles in total: timer 13 (zero and negative period accepted, null callback accepted, start while running succeeds, stop fails when stopped, one-shot fires again, stop does not stop, stop from callback allowed, wrong code for a bad period, internal-error codes for one-shot and periodic start, a rejected start that leaves a timer running, a one-shot that never becomes restartable), watchdog 13 (zero and negative timeout accepted, kick on stopped succeeds or starts it, start while running succeeds, stop fails when stopped, stop does not stop, wrong stop code, internal-error start, rejected start leaves it running, kick always fails, no restart after stop, unstoppable watchdog reporting success), scheduler 29 (id zero, duplicate ids, null entry/name and negative period accepted, start or stop not idempotent, entries after stop, entries before start, restart refused, wrong exhaustion code, and ids/resource/internal-error faults injected at each of the first six registrations), clock 2 (domain flips, monotonic decreases), adapter 12 (empty name, unstable info values or object, unstable scheduler/clock/timer/watchdog objects, nondeterministic or reordered capabilities, changed name/id/version, a faulty service behind a healthy description). All are rejected.
- Mutation testing, level 2 (the suite itself): each of the 92 `KRITVA_CONFORMANCE_CHECK` sites was neutralized in turn (assertion removed, side effects kept) and the conformance test re-run. Result: 35 killed, 57 survived. Honest analysis of the survivors: (a) redundant repeats of one property, for example `stop()` asserted twice for idempotency, `kick()` on a stopped watchdog asserted at several points, domain checked on each sample; (b) checks whose fault is also detected by a later check through state corruption (for instance a timer that wrongly accepts a zero period is then also seen as running when the one-shot starts); (c) four structural checks on `supports()` versus the accessors, which cannot be violated because `supports()` is non-virtual in `IPlatformAdapter` and are kept as defensive checks; (d) a smaller group of checks for which no dedicated isolating double exists yet. I did not hide this: the level-1 detection is the contract evidence, level 2 shows the checks overlap, and I report the 57 survivors as a known strictness gap rather than claiming each check is independently proven.
- Build 0 warnings; `ctest` 31/31 in Debug, Release, ASan+UBSan, strict `-Werror`, TSan (ASLR off); `-fanalyzer` clean; coverage 98% (446/451, same five uncovered lines); `make check` passes, traceability 64 requirements, 63 traced, 0 errors; `git diff --check` clean.
- Out of scope confirmed: no real Linux or RTOS adapter, no hardware CI, no platform implementation.

## Reviewer Sign-off

- [ ] Scope satisfied
- [ ] Requirement traceability satisfied
- [ ] Tests satisfied
- [ ] Quality checks satisfied
- [ ] Evidence reproducible
- [ ] Architecture boundary preserved
- [ ] No unresolved blocker

Final reviewer decision is made independently after evidence review.
