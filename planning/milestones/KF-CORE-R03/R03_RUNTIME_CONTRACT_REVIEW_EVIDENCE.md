# KF-CORE-R03 — Runtime Contract Review: Evidence Pack

Prepared by Claude for the architecture reviewer. Objective evidence and the API surface proposed for freeze. It records **no gate decision**; the decision and the checklist boxes in `R03_RUNTIME_CONTRACT_REVIEW.md` belong to the reviewer.

## 1. State under review

HEAD `2c06652` (planning only after `54e14bc`). Production code last changed in `ee3d55d`; later commits are documentation/traceability. Accepted tasks:

| Task | Primary commit | Follow-ups | Acceptance |
|---|---|---|---|
| R03-004 Runtime Manager | `e4d3a9b` | `40e33e9` (freeze-through-views test), `25eb914` (CORE-RT-006) | `d65b7a8` |
| R03-005 Runtime Lifecycle | `e2b660d` | `a4f2a65` (shutdown progress), `eac011f` (CORE-RT-007) | `3623a26` |
| R03-006 Runtime Failure & Recovery | `ee3d55d` | `d651677` (CORE-RT-008) | `54e14bc` |

## 2. Entry-criteria evidence (fresh `git clone` at `2c06652`, clean tree)

| Check | Result |
|---|---|
| `cmake -S . -B build && cmake --build build -j4` | 0 warnings |
| `ctest --test-dir build --output-on-failure` | 24/24 passed |
| Release | 0 warnings, 24/24 |
| ASan + UBSan | 0 warnings, 24/24 |
| TSan (ASLR disabled, as in R0.2) | 24/24 |
| Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` | builds, 24/24 |
| GCC `-fanalyzer` over `src/*.cpp` | no diagnostics |
| `make check` (header-check, traceability-check) | passed; 56 requirements, 55 traced (1 reserved), 0 errors |
| `make coverage` | 98% (434/439); `runtime_manager.cpp` 120/121 (the one line is the unreachable return after an exhaustive `switch`) |
| Prohibited includes in `include/`, `src/` (thread, mutex, condition_variable, future, atomic, semaphore, iostream, cstdio) | none |
| Install-consumer | passed; consumes the installed `RuntimeManager` incl. `start/stop/shutdown/reset/statistics` |
| Frozen files (`component*.hpp`, `component_registry.hpp`, `dependency_graph.hpp`, `runtime.hpp`, their sources) | unchanged since their accepting commits |

`CORE-RT-006`, `CORE-RT-007` and `CORE-RT-008` are defined in `REQUIREMENTS.md` and traced (header, source, tests). The Foundation API Review is PASS / FROZEN (`880a8d3`).

## 3. API surface proposed for freeze (`runtime/runtime_manager.hpp`)

```cpp
class RuntimeManager final : public Runtime {              // Runtime (CORE-RT-002) unchanged
public:
    // setup (until the first successful initialize())
    Result<void> register_component(Component&);           // = ComponentRegistry result, unchanged
    Result<void> add_dependency(ComponentId dependent, ComponentId dependency);  // = DependencyGraph result
    bool topology_fixed() const noexcept;
    Result<std::vector<ComponentId>> component_order() const;   // = DependencyGraph::order(registry)
    const ComponentRegistry& registry() const noexcept;
    const DependencyGraph& dependencies() const noexcept;
    // lifecycle
    Result<void> configure(const Configuration&);
    Result<void> initialize() override;  Result<void> start() override;
    Result<void> stop() override;        Result<void> shutdown() override;
    LifecycleState state() const noexcept override;
    // failure and recovery
    Result<void> reset();
    const Error* fault_error() const noexcept;             // non-null only while FAULT
    const Statistics& statistics() const noexcept;
};
```

### Runtime state table (own state; same table as Component)

| Operation | Valid from | Component calls | Order | On success | On first component failure |
|---|---|---|---|---|---|
| `configure(cfg)` | UNKNOWN, STOPPED | `configure` | forward | unchanged | unchanged |
| `initialize` | UNKNOWN, STOPPED | `initialize` | forward | READY | FAULT |
| `start` | READY | `start` | forward | RUNNING | FAULT |
| `stop` | READY, RUNNING | `stop` | reverse | STOPPED | FAULT |
| `shutdown` | UNKNOWN, STOPPED | `shutdown` (live, not yet shut down) | reverse | unchanged | unchanged (progress kept) |
| `reset` | FAULT only | pass 1 `stop` (initialized/started), pass 2 `shutdown` (stopped/faulted) | reverse, reverse | STOPPED, fault cleared | FAULT, original `fault_error()` kept, progress kept |

Any other (operation, state) pair: `INVALID_STATE`, no effect, no component invoked. In FAULT everything except `reset()` is `INVALID_STATE`. `initialize()` fixes the topology the first time (validated by `DependencyGraph::order()`); later setup is `INVALID_STATE`. A failed validation returns the graph's `CONFIGURATION_ERROR` unchanged, leaves the runtime UNKNOWN and invokes nothing.

## 4. Checklist evidence (reviewer ticks the boxes)

| Item | Evidence |
|---|---|
| `CORE-RT-002` authoritative, no competing abstraction | `RuntimeManager final : public Runtime`; `runtime.hpp` unchanged; `runtime_manager_test::test_implements_the_existing_runtime_interface` |
| Composition, topology validation/freeze, setup rejected after freeze | `runtime_manager_test` (fixed topology over READY/RUNNING/STOPPED; const views cannot call setup, static_assert) |
| Runtime state table, configure | `runtime_manager_test::test_runtime_operation_matrix`, `runtime_lifecycle_test` (invalid/repeated calls in every state; configure semantics) |
| Forward/reverse order, fail-fast/no rollback | `runtime_lifecycle_test` (exact traces; failure at all 25 operation/position pairs; 24 edge permutations x registration permutations) |
| Shutdown progress | `runtime_lifecycle_test` (progress preserved, every failure position, per-live-period reset) |
| Error preservation, FAULT, `fault_error()` | `runtime_failure_test` (15 failure points; pointer contract documented in header, API.md, ARCHITECTURE.md, `CORE-RT-008`) |
| `reset()` explicit/only recovery, ends STOPPED, no RECOVERING, no retry, two-pass reverse cleanup | `runtime_failure_test` (15 reset scenarios vs an independent stage model; failed op never retried; re-initialize works) |
| Failed cleanup and progress | `runtime_failure_test` (cleanup stop failure, cleanup shutdown failure, resume without repeats) |
| Statistics | `runtime_failure_test::test_statistics_follow_the_documented_updates` (`sample_count` = invocations that returned success, `error_count` = failures incl. cleanup, `retry_count` never incremented) |
| Health does not trigger recovery | `runtime_failure_test` (UNHEALTHY/DEGRADED components; zero observer calls); mutation "recovery triggered by health" detected |
| No background recovery/thread/watchdog | include scan above; "nothing happens without an explicit request" test; no clock/timer API used |

Mutation evidence: R03-004 10, R03-005 17 + 5 shutdown-progress, R03-006 15 + 1 health mutants; every one detected (one documented equivalent mutant in R03-005).

## 5. Cross-cutting policy facts

- Error semantics: only R02 `Result`/`Error`; no new `ErrorCode`; component errors returned unchanged.
- Warning: no API. Health: component-reported, never consulted by the runtime. Diagnostics: values only (`Error`, `fault_error()`, `statistics()`, `state()`); no logging backend. Events and application messages: not emitted by the runtime.
- Statistics: runtime-owned `Statistics`, R02 semantics (plain data, not an atomic snapshot).
- Logging boundary: unchanged (none).

## 6. Open items and risks for the reviewer

1. **No recovery directly to READY.** `reset()` ends in STOPPED and the caller re-`initialize()`s; `FAULT -> RECOVERING -> READY` is unused because no Component operation produces it (approved at R03-006).
2. **`fault_error()` is a raw pointer** valid only in FAULT; documented, not enforced.
3. **`configure()` gives every component the same `Configuration`**; per-component configuration would be a new API.
4. **Allocation:** setup, `configure`, `initialize`, `component_order()` allocate (control plane); `start`/`stop`/`shutdown`/`reset` do not allocate in the runtime itself.
5. **No thread safety and no real-time claim.**
6. **Version metadata:** `VERSION` and `CMakeLists.txt` are still `0.2.0`; the release gate proposes `0.3.0`. The install test requests `find_package(kritva_core 0.2)` and checks 0.2 accepted / 0.1, 0.3 refused, so it must be updated together with the version bump at release time (test-only, release metadata).

## 7. Reviewer actions

1. Tick the checklist in `R03_RUNTIME_CONTRACT_REVIEW.md` and record PASS / CHANGES REQUIRED / BLOCKED.
2. Confirm that R03-007 (integration tests) may begin under the freeze rule: no production API or semantic change without architecture review.
