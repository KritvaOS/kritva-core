# KF-CORE-R03-004 — Runtime Manager

## Task Information
| Field | Value |
|---|---|
| Task ID | KF-CORE-R03-004 |
| Milestone | KF-CORE-R03 |
| Title | Runtime Manager |
| Requirement | CORE-RT-006 |
| Status | READY |
| Depends on | R03-001, R03-002, R03-003, Foundation API Review PASS |
| Primary commit | `feat(core): add runtime manager` |

## Objective

Implement the concrete, synchronous, platform-independent Runtime Manager on top of the frozen R03 Foundation APIs.

The Runtime Manager **must implement the existing authoritative `runtime::Runtime` interface covered by `CORE-RT-002`**. It must not create a competing public Runtime abstraction.

R03-004 establishes runtime composition and topology management. Detailed lifecycle sequencing is implemented in R03-005. Failure and recovery semantics are implemented in R03-006.

## Scope

### In scope
- Concrete Runtime Manager implementing `runtime::Runtime`.
- Composition of ComponentRegistry and DependencyGraph.
- Fixed component topology for a Runtime instance.
- Registration/configuration of the component set during setup.
- Dependency topology validation before lifecycle execution.
- Deterministic component ordering through DependencyGraph.
- Runtime state representation required by the existing Runtime interface.
- Synchronous control-plane orchestration.

### Out of scope
- Changes to Component, ComponentInfo, ComponentRegistry, or DependencyGraph contracts.
- New Runtime abstraction competing with `CORE-RT-002`.
- Threads, executors, schedulers, background workers or timers.
- Automatic retry/recovery.
- Dynamic unregister/topology mutation.
- OS/platform/hardware dependencies.
- ROS2/DDS/EtherCAT/vendor HAL/BSP.
- Logging backend.
- New ErrorCode unless separately approved by architecture review.
- Detailed lifecycle sequencing beyond what is required by the existing Runtime interface; R03-005 owns lifecycle orchestration.

## Frozen Dependencies

R03-004 shall consume these accepted contracts without redefining them:

1. `ComponentId` is stable component identity.
2. ComponentRegistry is strictly non-owning.
3. Registry enumeration is ascending `ComponentId`.
4. Dependency edge is `dependent -> dependency`.
5. Dependencies precede dependents.
6. Lowest `ComponentId` is the deterministic tie-break.
7. Cycles are rejected when added.
8. Failed dependency mutations are atomic.
9. Missing registered components produce `CONFIGURATION_ERROR`.
10. `DependencyGraph::order()` returns ComponentId values.
11. Registry is append-only for R03.
12. `CORE-RT-002` remains authoritative for Runtime.

Any change to a frozen contract requires architecture review before implementation.

## Functional Requirements

### RM-01 Runtime interface
Runtime Manager implements the existing `runtime::Runtime` interface. Existing `CORE-RT-002` is not silently modified.

### RM-02 Component composition
Runtime Manager uses the accepted ComponentRegistry and DependencyGraph APIs and does not own registered Components.

### RM-03 Fixed topology
Once runtime initialization accepts the component set, the topology is fixed for that Runtime instance. There is no unregister operation.

### RM-04 Topology validation
Before lifecycle execution, Runtime Manager validates that every dependency endpoint is present in the registered component set.

### RM-05 Deterministic topology
Runtime Manager obtains its deterministic order from DependencyGraph. It must not introduce a second ordering algorithm.

### RM-06 Error propagation
Existing Result/Status/Error contracts remain authoritative. Component-originated errors preserve their original source and error semantics.

### RM-07 Allocation boundary
Calls that create snapshots/order vectors are setup/control-plane operations. R03 makes no hard-real-time claim.

### RM-08 Threading boundary
Runtime Manager is synchronous and does not create autonomous threads or execution infrastructure.

## Required Tests

### Unit
- Runtime interface conformance.
- Construction and initial state.
- Valid topology acceptance.
- Missing dependency rejection.
- Fixed topology/no unregister.
- Non-owning component lifetime/destruction behavior.
- Deterministic ordering.
- Error propagation from registry/dependency validation.
- Invalid Runtime operations according to `CORE-RT-002`.

### Integration / Regression
- Component + Registry + DependencyGraph + Runtime Manager.
- Different registration and dependency insertion orders produce identical topology.
- Existing R03-001..003 tests remain green.
- Installed consumer can instantiate/use the Runtime Manager.

### Mutation Testing
At minimum:
- replace dependency order with registration order;
- ignore missing dependency;
- mutate Runtime state transition;
- accidentally own/delete a component;
- introduce lifecycle invocation into topology setup;
- swallow an underlying error.

## Validation

Required before review:
- Debug build
- Release build
- ASan + UBSan
- TSan where configured
- strict `-Werror`
- GCC `-fanalyzer`
- `make check`
- coverage reviewed against R03 baseline
- install-consumer test
- `git diff --check`
- clean working tree

## Documentation

Update, as applicable:
- `REQUIREMENTS.md` — add `CORE-RT-006` only when implementation is complete and reviewed
- `ARCHITECTURE.md`
- `API.md`
- relevant runtime documentation
- task evidence

Do not define `CORE-RT-006` merely as planning text before implementation.

## Evidence Required

Claude must provide:
1. primary commit hash;
2. changed-file list;
3. clean-tree evidence;
4. Runtime interface conformance evidence;
5. topology validation/order evidence;
6. ownership/lifetime evidence;
7. unit/integration/regression results;
8. Debug/Release/sanitizer/static-analysis results;
9. coverage;
10. traceability result;
11. install-consumer result;
12. mutation-testing results;
13. confirmation that frozen R03-001..003 public APIs were not changed.

## Acceptance

Acceptance is independent architecture review. Claude must not mark the reviewer decision.

Reviewer decisions:
- PASS
- CHANGES REQUIRED
- BLOCKED

## Git

Exact implementation commit:

`feat(core): add runtime manager`
