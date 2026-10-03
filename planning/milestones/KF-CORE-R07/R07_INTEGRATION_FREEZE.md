# R07 Integration Freeze

## Gate

PASS / HONORED is required before R07-007 implementation.

## Entry Criteria

- R07-005 and R07-006 accepted.
- Public operational API has been frozen.

## Freeze Conditions

- Runtime lifecycle behavior is unchanged.
- Health/status/statistics do not automatically change Runtime state.
- Events do not trigger recovery or lifecycle operations.
- No Core EventBus, queue, worker, telemetry path or logging backend exists.
- Production changes are limited to approved validation/documentation work.
- Test-only operational infrastructure remains outside production Core.

## Evidence (submitted 05-10-2026)

Freeze candidate: production sources are final as of `16654e9` (R07-004, the last production change). R07-005 `0b1bd1d` and R07-006 `3f524cd` changed tests and documentation only. Head: `4cfa291`. Component Operational API Review baseline: PASS / FROZEN at `5021172` (evidence `6b1296f`).

### Entry criteria

R07-005 and R07-006 are ACCEPTED (`6ecbf31`, `4cfa291`). The operational integration tests pass and are deterministic (seeded; 5 randomized-order repetitions of the 8 operational, observation, status/health, event, statistics and isolation test groups all pass). All pre-R0.7 regression tests remain green (51/51 in total). The integration tests use public APIs only. The public operational API was frozen at the API Review.

### Production diff

`git diff 16654e9 HEAD -- include src` is **empty** and the three frozen headers (`component_observation.hpp`, `component_statistics.hpp`, `component_events.hpp`) are byte-identical to the API Review baseline `6b1296f`. Against `kritva-core-r0.6` the production change is exactly the reviewed R07 set: three new headers and three includes in `core.hpp`; `src/` is unchanged and every R0.2–R0.6 header is byte-identical to the tag. Since the API freeze the only non-planning changes are test files, `CMakeLists.txt` registrations, `REQUIREMENTS.md`, `API.md` and `TESTING.md`.

### Freeze conditions

| Condition | Evidence |
|---|---|
| Runtime lifecycle behavior is unchanged | `src/` and `runtime_manager.hpp` byte-identical to `kritva-core-r0.6`; a seeded differential (150 seeds × 40 steps, six operational variants including a sink that rejects every call) gives transcripts identical to the plain baseline |
| Health/status/statistics do not automatically change Runtime state | the Runtime never reads them (spies 0 over 100 seeds through every operation, failure, fault and reset); 25 rounds of worst reports in RUNNING change nothing; consequential Runtime mutants that read them are detected |
| Events do not trigger recovery or lifecycle operations | the Runtime never calls or holds a sink; a flood of events of every type after a FAULT triggers no component operation and no retry; a FAULT ends only by the explicit `reset()`; an event from a failing operation never replaces the Runtime's fault |
| No Core EventBus, queue, worker, telemetry path or logging backend | one synchronous call per report, no buffering/retry (mutants detected); no `<thread>`, `<mutex>`, `<atomic>`, `<condition_variable>`, `<future>`, `<queue>`, `<deque>`, `<iostream>`, `<cstdio>` or `<fstream>` in `include/` or `src/` and no OS, RTOS, ROS2, DDS or EtherCAT identifier (word-bounded scan; enforced by the audit) |
| Production changes limited to approved validation/documentation work | empty production diff since `16654e9`; the three headers byte-identical since `6b1296f` |
| Test-only operational infrastructure stays outside production Core | the harness lives only under `tests/runtime/`; the audit (`CORE-PLAT-016`) and `kritva_core_test_isolation` forbid any production include of it |

### Validation (head `4cfa291`; production unchanged since `16654e9`)

`ctest` 51/51 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; public-header self-containment passes; `make traceability-check`: 89 requirements, 88 traced (CORE-ERR-003 reserved), 0 errors, `CORE-OPS-001` to `009` defined once (`CORE-OPS-010`, release validation, comes with R07-007); coverage 618/625; no `find_package` or `FetchContent`; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; a context, reporter or provider used after what it refers to is destroyed is documented undefined behavior (non-owning by design). The only remaining work is R07-007 (validation, release metadata 0.7.0) and the Release Gate; neither may change production API.
