# KF-CORE-R03-007 — Runtime Integration Tests

## Task Information
- **Task:** KF-CORE-R03-007
- **Title:** Runtime Integration Tests
- **Milestone:** KF-CORE-R03
- **Requirement:** CORE-RT-009
- **Status:** Planned
- **Dependency:** R03 Runtime Contract Review — PASS
- **Primary commit:** `test(core): add runtime integration contracts`

## Objective
Build the platform-independent end-to-end integration test layer for the accepted R03 runtime foundation. The tests shall exercise Component, ComponentRegistry, DependencyGraph and RuntimeManager together through public APIs and reference components, validating the frozen runtime contracts without adding production functionality.

## Scope
### In scope
- End-to-end runtime topology construction.
- Dependency-aware lifecycle execution.
- Failure and explicit reset/recovery paths.
- Cross-component state/progress interactions.
- Determinism across registration and dependency insertion permutations.
- Regression coverage of accepted R03-004..006 behavior.
- Reference/fake components and test-only observers/fault injection.
- Integration-level API contract checks.

### Out of scope
- Production API changes.
- New runtime behavior not already accepted by R03-004..006.
- OS threads/executors.
- ROS2/DDS/EtherCAT/vendor integrations.
- Hardware/network dependencies.
- New ErrorCode values.
- Performance benchmarking or real-time qualification.

## Frozen Inputs
R03-007 must consume the accepted and frozen contracts for:
- Component / ComponentInfo / ComponentId.
- ComponentRegistry.
- DependencyGraph.
- `CORE-RT-002` Runtime.
- `CORE-RT-006` Runtime Manager.
- `CORE-RT-007` Runtime Lifecycle.
- `CORE-RT-008` Runtime Failure & Recovery.

Any production contract change required by testing stops the task and returns to architecture review.

## Expected Deliverables
- New/expanded integration test suite registered with CTest.
- Reusable test fixtures/reference components where justified.
- No production runtime behavior changes.
- `CORE-RT-009` added to authoritative requirements after implementation review.
- Integration evidence suitable for the R03 Integration Freeze.

## Implementation Rules
1. Tests use public APIs unless a test-only seam is explicitly justified.
2. Tests must not depend on registration/container incidental ordering.
3. Tests must assert observable contract behavior, not private implementation details.
4. Test-only fault injection may be used to deterministically exercise failure paths.
5. Integration tests must remain platform independent.
6. No production API is changed merely to make an integration test easier.

## Required Evidence
- Primary commit hash.
- Test list and CTest result.
- Integration scenario matrix.
- Proof of deterministic behavior across relevant permutations.
- Existing unit/regression test results.
- Confirmation of zero production API changes after Runtime Contract Review, or explicit architecture approval if an exception is unavoidable.

## Reviewer Gate
After acceptance, the **R03 Integration Freeze** becomes active.

## Git Commit
`test(core): add runtime integration contracts`
