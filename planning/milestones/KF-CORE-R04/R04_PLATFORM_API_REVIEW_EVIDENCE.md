# KF-CORE-R04 — Platform API Review Evidence

Status: SUBMITTED

Baseline: `627ee5f` (R04-004 accepted). Production sources last changed in `4af4756`. Release tag baseline for the API diff: `kritva-core-r0.3`.

## Entry criteria

R04-001 (`d1c5f13`), R04-002 (`eb06fa0`), R04-003 (`67114bb`), R04-004 (`4af4756`) are ACCEPTED by the independent reviewer.

## Public API diff against `kritva-core-r0.3` (`include/` and `src/`)

| File | Change |
|---|---|
| `types/callback.hpp` | **new**: `Callback` (function pointer + opaque non-owning context, `valid()`) |
| `platform/boundary.hpp` | **new**: boundary contract text and `platform::make_error()` |
| `platform/scheduler.hpp` | contract text only; signatures unchanged |
| `platform/watchdog.hpp` | contract text only; signatures unchanged |
| `time/clock.hpp` | comment only |
| `time/timer.hpp` | **breaking, approved (Q2)**: `ITimer::start(Duration)` -> `start(Duration period, TimerMode mode, Callback callback)`; new `TimerMode`; contract text |
| `core.hpp` | includes the two new headers |

`git diff kritva-core-r0.3 HEAD -- src include/kritva/core/runtime` is empty: the R0.3 Runtime (Component, ComponentInfo, ComponentId, ComponentRegistry, DependencyGraph, `runtime::Runtime`, RuntimeManager) is byte-identical. 7 files changed in `include/`, 363 insertions, 13 deletions; no change in `src/`.

## Contract summary (frozen if accepted)

- **Boundary:** implementations live outside kritva-core; integrator owns adapters and services; Core holds non-owning references, creates and destroys no platform object; no singleton, global or service locator; a platform object outlives the Core objects that reference it. Failure through `Result`/`Error` with `INVALID_ARGUMENT`, `INVALID_STATE`, `UNSUPPORTED`, `RESOURCE_UNAVAILABLE`, `TIMEOUT`, atomically. Thread safety adapter-defined; no real-time guarantee through any platform contract.
- **Callback:** function (null invalid where required) + opaque non-owning caller-owned context (null valid); never dereferenced by Core; callbacks never throw; each service states its execution context, blocking, re-entrancy and context lifetime; no Core-owned thread.
- **Scheduler (`IScheduler`):** TaskId 0 invalid, unique ids, atomic create, relative priority with adapter mapping, affinity 0 unconstrained / UNSUPPORTED / INVALID_ARGUMENT, period 0 aperiodic, negative invalid, dynamic creation as adapter policy, idempotent all-or-nothing start, idempotent synchronous stop, no entry after stop, no self-overlap, `INVALID_STATE` for stop from an entry, 32-bit affinity mask kept as a documented limitation.
- **Clock (`time::IClock`):** canonical; `platform::IClock` remains an identical alias; fixed domain per instance; timestamps of different domains not comparable (unchanged).
- **Timer (`ITimer`):** elapsed monotonic time, no `ClockDomain`; STOPPED/RUNNING; atomic `start(period, mode, callback)`; ONE_SHOT exactly once then restartable; PERIODIC without overlap; idempotent synchronous `stop`; `INVALID_STATE` for start/stop from its own callback; callback and context rules; no Runtime recovery.
- **Watchdog (`IWatchdog`):** STOPPED/RUNNING; `start` only while stopped (`INVALID_STATE` otherwise); `kick` only while running and never starts it; `stop` idempotent, may be `UNSUPPORTED` for adapters that cannot disable; fresh activation on restart; expiry action adapter-defined and never triggers `RuntimeManager::reset()`.

## Mapping to the mandatory architectural decisions

1 Core platform independent: forbidden-header audit (CORE-PLAT-004) clean. 2 Implementations outside Core: only contracts and test doubles in this repository. 3 No Linux/POSIX/RTOS/vendor header: audit and manual scan clean (the only textual match is `from_microseconds`). 4 `time::IClock` canonical: unchanged. 5 Priority/affinity adapter-defined: documented and tested with two conforming adapter policies. 6 Timer callback context implies no Core thread: documented and tested. 7 Watchdog expiry does not imply Runtime recovery: tested against a real `RuntimeManager`. 8 No singleton: boundary test and header review. 9 R0.3 Runtime authoritative: source diff empty.

## Validation (code state `4af4756`)

`ctest` 29/29 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean over `src/*.cpp` and the new tests; public-header self-containment check passes (`kritva_core_header_checks`, one translation unit per public header); `make traceability-check`: 62 requirements, 61 traced (CORE-ERR-003 reserved), 0 errors, 0 warnings; dependency scan: no threading, OS, vendor, ROS 2/DDS/EtherCAT include, no `find_package` of third-party packages, no `FetchContent`; coverage 98% (438/443), same five justified uncovered lines as the R0.3 baseline; `git diff --check` clean. Mutation testing of every reference implementation and test double was performed per task (see each task's evidence).

## Open issues

None blocking. Carried forward: 32-bit scheduler affinity mask (documented limitation; widening would be a separate reviewed change); `make lint` and `make format-check` remain deferred stubs (as in R0.3); the platform-capability adapter contract (R04-005) and conformance suites (R04-006) are the next tasks and are not part of this freeze.

Reviewer decision: **PASS / FROZEN** (04-10-2026; see `R04_PLATFORM_API_REVIEW.md`)
