# KF-CORE-R03-004 — Acceptance Criteria

## 1. Task Information

- **Task:** KF-CORE-R03-004
- **Title:** Runtime Manager
- **Milestone:** KF-CORE-R03
- **Requirement:** CORE-RT-006
- **Status:** READY
- **Dependency:** R03 Foundation API Review PASS
- **Implementation commit:** `feat(core): add runtime manager`

## 2. Objective

Implement the concrete synchronous, platform-independent Runtime Manager using the frozen Component, ComponentRegistry and DependencyGraph contracts.

The Runtime Manager **must implement the existing authoritative `runtime::Runtime` interface covered by `CORE-RT-002`**. It must not create a competing public Runtime abstraction.

## 3. Scope

### In scope
- Runtime Manager implementation.
- Existing Runtime interface conformance.
- Registry/DependencyGraph composition.
- Fixed component topology.
- Topology validation.
- Deterministic topology ordering.
- Runtime state representation required by the existing interface.
- Synchronous control-plane orchestration.

### Out of scope
- Foundation API changes.
- New competing Runtime interface.
- Threads/executors/schedulers.
- Automatic recovery/retry.
- Dynamic unregister/topology mutation.
- Platform/hardware dependencies.
- ROS2/DDS/EtherCAT/vendor APIs.
- Logging backend.
- Detailed lifecycle orchestration owned by R03-005.

## 4. Frozen Foundation Contracts

The implementation must not alter:
- ComponentId identity;
- ComponentInfo;
- Component lifecycle contract;
- Registry non-ownership;
- ascending ComponentId registry enumeration;
- dependency edge direction;
- dependency-first ordering;
- lowest ComponentId tie-break;
- cycle rejection;
- atomic failed dependency insertion;
- missing-component `CONFIGURATION_ERROR`;
- `order()` returning ComponentId values;
- append-only registry;
- authoritative `CORE-RT-002`.

Any required change returns to architecture review.

## 5. Acceptance Criteria

### AC-01 — Runtime interface
- Runtime Manager implements the existing `runtime::Runtime`.
- No competing public Runtime abstraction.
- `CORE-RT-002` is not silently modified.

### AC-02 — Composition and ownership
- Runtime Manager composes Registry and DependencyGraph.
- It does not own, copy, move, or delete Components.
- Destroying Runtime Manager does not destroy registered Components.

### AC-03 — Fixed topology
- Component set is fixed for the Runtime instance after setup/initialization.
- No unregister operation.
- No topology mutation during execution.

### AC-04 — Topology validation
- Every dependency endpoint is validated against registered components before lifecycle execution.
- Invalid topology cannot reach lifecycle execution.
- Failed validation leaves Runtime in a documented non-running state.

### AC-05 — Deterministic order
- Runtime obtains ordering from DependencyGraph.
- Frozen ComponentId tie-break is preserved.
- Order is independent of registration and edge insertion order.

### AC-06 — Lifecycle boundary
- R03-004 does not duplicate R03-005 lifecycle sequencing.
- Setup/configuration does not implicitly start execution.
- Existing Runtime state semantics are preserved.

### AC-07 — Allocation boundary
- Snapshot/order allocation is limited to setup/control-plane use.
- No real-time or hard-real-time claim is introduced.

### AC-08 — Error contract
- Existing Result/Status/Error contracts are reused.
- No unnecessary ErrorCode is introduced.
- Component-originated errors preserve source/code semantics.

### AC-09 — Threading
- No background thread or executor is created.
- No new thread-safety guarantee is claimed.

### AC-10 — Documentation
- `CORE-RT-006` is added to authoritative `REQUIREMENTS.md` only after implementation is reviewed.
- Architecture/API documentation explicitly explains Runtime Manager vs `CORE-RT-002`.

## 6. Required Tests

### New unit tests
- Runtime interface conformance.
- Construction/initial state.
- Valid topology.
- Missing dependency.
- Fixed topology.
- Non-owning lifetime behavior.
- Deterministic ordering.
- Registry/dependency error propagation.
- Invalid Runtime operations required by `CORE-RT-002`.

### Regression/integration
- Component + Registry + DependencyGraph + Runtime Manager.
- Registration/edge insertion permutations.
- Existing R03-001..003 suite.

### Mutation
At minimum detect:
- registration-order substitution;
- ignored missing dependency;
- altered Runtime state;
- component ownership/deletion;
- accidental lifecycle invocation;
- swallowed error.

## 7. Build/Test/Quality Gates

- Debug build and tests pass.
- Release build and tests pass.
- ASan + UBSan pass.
- TSan passes where configured.
- `-Werror` passes.
- GCC `-fanalyzer` passes.
- `make check` passes.
- Coverage is at least the established R03 baseline unless an unreachable path is documented.
- Install-consumer passes.
- `git diff --check` passes.
- Working tree is clean.

## 8. Evidence Required

- Primary commit hash.
- Changed-file summary.
- Runtime interface conformance.
- Topology validation and ordering evidence.
- Ownership/lifetime evidence.
- Full test/build/quality results.
- Coverage and traceability.
- Install-consumer result.
- Mutation results.
- Confirmation that frozen R03-001..003 APIs were unchanged.

## 8a. Implementation Evidence (Claude)

- Primary commit: `e4d3a9b` `feat(core): add runtime manager` (planning at `03bf026`; Foundation API Review PASS at `880a8d3`).
- Changed files: `include/kritva/core/runtime/runtime_manager.hpp`, `src/runtime_manager.cpp`, `tests/unit/runtime_manager_test.cpp` (new); `include/kritva/core/core.hpp` (umbrella include), `CMakeLists.txt` (library source, test), `tests/install/consumer/main.cpp` (consumer also uses the installed `RuntimeManager`), `REQUIREMENTS.md` (traceability row for `CORE-RT-002` extended with the new header/source/test), `ARCHITECTURE.md` ("Runtime Manager"), `API.md` (section 23).
- **Frozen APIs unchanged (confirmed):** `git diff --stat` against `component.hpp`, `component_id.hpp`, `component_info.hpp`, `component_registry.hpp`, `dependency_graph.hpp`, `runtime.hpp`, `src/component_registry.cpp`, `src/dependency_graph.cpp` is empty. `runtime::Runtime` / `CORE-RT-002` is not modified.
- **`CORE-RT-006` not defined:** per this task and AC-10 it is not added to `REQUIREMENTS.md` until reviewed; `grep RT-006` over `include/ src/ tests/ *.md` outside `planning/` finds nothing. The new header, source and test are tagged `CORE-RT-002` (the contract they implement), so the traceability audit passes. After review, a follow-up commit should define `CORE-RT-006`, move the tags and add its row.
- **Runtime interface conformance (AC-01):** `RuntimeManager final : public Runtime`; no second abstraction. Used through `Runtime&`, and deleted through `std::unique_ptr<Runtime>`, in the test.
- Public API: `register_component(Component&)`, `add_dependency(dependent, dependency)`, `topology_fixed()`, `component_order()`, `registry()`, `dependencies()` (read-only views), plus the `Runtime` overrides. Not copyable or movable.
- **Decisions (the interface `Runtime` documents none; these define the semantics, for reviewer confirmation):**
  1. Runtime state uses the Core lifecycle states and the same operation table as `Component`, but only the runtime's own state changes: `initialize` from UNKNOWN/STOPPED to READY; `start` READY to RUNNING; `stop` READY/RUNNING to STOPPED; `shutdown` idempotent no-op in UNKNOWN/STOPPED; everything else `INVALID_STATE` with no effect and no component source. Transitions go through the Core `Lifecycle` class, so the Core transition table is enforced.
  2. **Fixed when:** the first successful `initialize()` validates the topology and fixes it permanently (including across `stop()` and re-`initialize()`). Setup afterwards fails with `INVALID_STATE` and changes nothing. There is no separate seal step.
  3. **Failed validation (AC-04):** `initialize()` returns the graph's `CONFIGURATION_ERROR` unchanged (same code, source, message) and leaves the runtime UNKNOWN, not fixed, with setup still open, so the caller can fix the topology and retry. I did not move the runtime to FAULT, because nothing started and failure handling is R03-006.
  4. `FAULT` is not produced by anything here, so `shutdown()` from FAULT is not yet valid (documented); R03-006 defines it.
  5. Setup forwards the registry/graph `Result` unchanged, so duplicate, self, duplicate-edge, cycle and invalid-id errors keep their code, source and message (RM-06).
  6. `component_order()` is `DependencyGraph::order(registry)` and nothing else (RM-05). It recomputes each call; after fixing, the result cannot change.
  7. Runtime errors carry no `ComponentId` source because no component is involved.
  8. `registry()`/`dependencies()` accessors expose read-only views; a const registry still returns mutable `Component*` (shallow-const, as accepted).
- Boundary evidence (AC-06): the runtime calls no component method. Verified by test (component call counters stay 0 through `initialize/start/stop/shutdown` cycles and invalid calls) and by mutation (below). `start()`/`stop()` do not allocate; setup, `initialize()` and `component_order()` allocate (control plane).
- Threading (AC-09): no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>` or `<semaphore>` anywhere in `include/` or `src/` (grep). The only `<chrono>` include is the pre-existing one in `types/duration.hpp`.
- Tests: new CTest `kritva_core_runtime_manager` (10 test functions): interface conformance and the full operation/state matrix over UNKNOWN/READY/RUNNING/STOPPED, composition with registry/graph errors identical to the standalone classes, fixed topology across READY/RUNNING/STOPPED rounds, non-owning behavior (a probe component reports destruction; the runtime never deletes it and the owner still controls it), valid topology and order equal to `DependencyGraph::order()`, missing dependency and unregistered dependent (error identical to the graph's, runtime not running, setup open, recovery after registering the missing component), order independence over registration and all 24 edge-insertion permutations, and "never drives components".
- **Mutation evidence** (each temporary edit reverted; files verified identical; all 10 aborted the test): registration-order substitution; missing dependency ignored; `start()` allowed from UNKNOWN (altered state); destructor deleting components; `initialize()` driving components; error rewrapped as `INTERNAL_ERROR`; setup allowed after fixing; topology never fixed; `add_dependency` error swallowed; failed validation moving the runtime to FAULT.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **22/22** (21 prior + `kritva_core_runtime_manager`). Release 22/22; ASan+UBSan 22/22; TSan (ASLR disabled) 22/22; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 22/22; GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer: passed; the consumer links the installed `RuntimeManager`, fixes its topology and checks that setup is then refused.
- Standalone compile of `runtime_manager.hpp`: OK. Includes only Core headers plus `<string>`, `<vector>`, `<cassert>`; no OS, ROS2/DDS, EtherCAT, vendor or logging headers.
- `make check` passed (traceability 53 requirements, 52 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (354/358, the R03 baseline); `src/runtime_manager.cpp` is 43/43. The four uncovered lines are unchanged pre-existing/exception-unwind lines.
- Final `git status --short`: clean after the commit.
- Known limitations / out of scope: no ordered component invocation (R03-005), no failure/recovery (R03-006), no FAULT handling, no thread-safety, no real-time claim; `initialize()` and `component_order()` recompute the order (control plane).

## 9. Reviewer Sign-off

Only the independent architecture reviewer records:
- PASS
- CHANGES REQUIRED
- BLOCKED

## 10. Git Commit

`feat(core): add runtime manager`
