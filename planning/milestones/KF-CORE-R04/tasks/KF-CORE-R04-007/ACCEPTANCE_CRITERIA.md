# KF-CORE-R04-007 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-007`
- **Title:** Runtime–Platform Integration Boundary
- **Requirement:** `CORE-PLAT-010`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define runtime platform integration boundary`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define how future Runtime integration may consume platform services without changing accepted R0.3 Runtime semantics or adding platform implementation to Core.

## Scope

['Runtime/platform dependency boundary', 'optional scheduler integration', 'clock use', 'watchdog boundary', 'error propagation', 'lifetime/ownership', 'configuration boundary']

## Out of Scope

['thread creation in Runtime', 'Linux/RTOS implementation', 'automatic watchdog recovery', 'global platform services', 'background execution']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-010` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['RuntimeManager remains platform independent.', 'Scheduler integration is explicit and optional.', 'Clock integration preserves accepted R0.3 lifecycle semantics.', 'Watchdog use cannot silently trigger Runtime recovery.', 'Platform errors preserve Result/Error semantics.', 'Platform service ownership/lifetime is explicit.', 'No Core background thread or executor is introduced.', 'No platform-specific header enters Runtime production code.', 'No R0.3 Runtime API or semantic change occurs without architecture review.']

## New Tests Required

['compile/boundary tests', 'Runtime regression', 'platform failure propagation', 'no-thread/prohibited-dependency scan', 'public API compatibility check']

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
feat(core): define runtime platform integration boundary
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `36c5cb8` `feat(core): define runtime platform integration boundary` (Platform Integration Freeze PASS / HONORED, production freeze point `f7231c1`). This is the one explicitly authorized exception to the freeze: an **additive** extension of the frozen `RuntimeManager`; no existing signature or behavior changed; the frozen scheduler, clock, timer, watchdog and adapter contracts are untouched.

- Production change (`git diff 5f755fe HEAD -- include src`): `include/kritva/core/runtime/runtime_manager.hpp` and `src/runtime_manager.cpp` only. Added: a forward declaration of `platform::IPlatformAdapter` (no platform header enters the Runtime), `Result<void> attach_platform(platform::IPlatformAdapter&)`, `platform::IPlatformAdapter* platform() const noexcept`, one private non-owning pointer member, and the PLATFORM ATTACHMENT contract section. No other member or file changed.
- Contract (design decision Q5): attachment is a setup operation valid until the first successful `initialize()` fixes the topology (`INVALID_STATE` afterwards, no effect); the Runtime stores a non-owning reference and **does not probe** the adapter (no `info`, `supports`, service accessor or `capabilities` call) and no other Runtime operation ever calls the adapter or starts, stops or kicks a scheduler, timer or watchdog, reads a clock or creates a thread; the integrator owns the adapter and its services; no singleton, registry or locator; a platform failure reaches the Runtime only through an integrator-written Component and is propagated unchanged; a watchdog expiry never enters the Runtime and a Runtime FAULT never touches a watchdog.
- Design choices to confirm: (1) a second `attach_platform()` while an adapter is attached fails with `INVALID_STATE` and never replaces the adapter, and there is no detach (Q5 did not specify re-attachment; this follows the "no silent reconfiguration" rule used for the timer and watchdog); (2) the same adapter attached twice also fails with `INVALID_STATE`; (3) a failed attach leaves nothing attached; (4) an invalid topology that makes `initialize()` fail (topology not fixed) leaves attachment open, consistent with `register_component()`; (5) the forward declaration keeps platform headers out of `runtime_manager.hpp` at the cost of requiring users of `platform()` to include `platform/adapter.hpp` (`core.hpp` does).
- Files: `runtime_manager.hpp`, `runtime_manager.cpp`, `tests/integration/runtime_platform_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-010`, traceability row), `API.md` (section 30), `ARCHITECTURE.md`. No existing test was modified: the whole R0.3 group (`kritva_core_runtime`, `component`, `component_registry`, `dependency_graph`, `runtime_manager`, `runtime_lifecycle`, `runtime_failure`, `runtime_integration`) passes untouched.
- Tests (`kritva_core_runtime_platform`, 10 functions): public-API compatibility via `static_assert` on the exact signatures of every frozen `RuntimeManager` member plus the two additions, noexcept/final/non-copyable preserved; attach is optional, setup-only, never replaces, fails after the topology is fixed and after stop; a failed attach attaches nothing and an invalid topology keeps attachment open; attach does not probe (a spy adapter records every member call: zero); the Runtime never owns or destroys the adapter (destroyed only by the integrator); **differential test**: 400 seeded scenarios x 40 operations, each run on two RuntimeManagers, one without and one with an attached spy adapter, comparing every result (code, severity, source, message), state, topology-fixed flag, fault, statistics and the full component invocation trace — all identical, while the spy records zero adapter calls and zero service start/stop/kick calls; the fully documented lifecycle trace with an adapter attached; platform failure propagation (a component that starts a timer on a platform without a timer resource returns the platform's code and message unchanged with its own source, the Runtime goes to FAULT, only `reset()` leaves it, and a new explicit attempt works after the platform recovers; the adapter received exactly the component's own three accessor calls and none from the Runtime); watchdog expiry leaves a RUNNING Runtime RUNNING and a Runtime FAULT and `reset()` leave a running watchdog untouched; services obtained through the Runtime are the adapter's own objects and using them changes no Runtime state.
- Mutation evidence on the production code (17 mutants, each reverted, all detected): attach ignoring the topology guard, silently replacing an adapter, storing nothing, probing `info`/`capabilities`/`supports`, `initialize` touching the clock, `start` starting the scheduler, `stop` stopping the watchdog, `reset` kicking the watchdog, FAULT entry stopping the watchdog, `shutdown` consulting `supports`, `start` failing on a platform without a scheduler, the Runtime deleting the adapter, `platform()` returning null, a failed attach leaving an adapter attached, and the wrong error code for a second attach.
- Boundary checks: `runtime_manager.hpp` only forward-declares `IPlatformAdapter`; the audit (CORE-PLAT-004, CORE-RT-010) reports no platform-specific, threading or OS header in production code; no `<thread>`, `<mutex>`, `<atomic>` in `include/` or `src/`; no Core thread or executor introduced.
- Build 0 warnings; `ctest` 32/32 in Debug, Release, ASan+UBSan, strict `-Werror`, TSan (ASLR off); `-fanalyzer` clean; coverage 98% (452/457, same five uncovered lines); `make check` passes, traceability 65 requirements, 64 traced, 0 errors; `git diff --check` clean. A first ASan run reported a leak in my own spy-adapter test helper (a heap flag); I fixed the helper and the second ASan run is clean.
- Out of scope confirmed: no thread creation in the Runtime, no Linux/RTOS implementation, no automatic watchdog recovery, no global platform services, no background execution.

## Reviewer Sign-off

- [ ] Scope satisfied
- [ ] Requirement traceability satisfied
- [ ] Tests satisfied
- [ ] Quality checks satisfied
- [ ] Evidence reproducible
- [ ] Architecture boundary preserved
- [ ] No unresolved blocker

Final reviewer decision is made independently after evidence review.
