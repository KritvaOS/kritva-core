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

## 9. Reviewer Sign-off

Only the independent architecture reviewer records:
- PASS
- CHANGES REQUIRED
- BLOCKED

## 10. Git Commit

`feat(core): add runtime manager`
