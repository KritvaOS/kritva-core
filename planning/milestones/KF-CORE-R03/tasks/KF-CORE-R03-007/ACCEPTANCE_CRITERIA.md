# KF-CORE-R03-007 — Acceptance Criteria

## 1. Task Information
- **Requirement:** CORE-RT-009
- **Dependency:** R03 Runtime Contract Review = PASS
- **Primary commit:** `test(core): add runtime integration contracts`

## 2. Acceptance Criteria

### AC-01 — Full component-to-runtime integration
- A runtime can be constructed from the accepted Component/Registry/DependencyGraph contracts.
- Multiple components and dependency branches are exercised through the public RuntimeManager API.
- The test suite verifies the complete topology → lifecycle → failure/recovery path.

### AC-02 — Deterministic topology and lifecycle
- Equivalent component/dependency sets produce identical observable lifecycle order.
- Registration order does not affect results.
- Dependency edge insertion order does not affect results.
- Forward operations use dependency order and teardown/recovery use the accepted reverse order.

### AC-03 — Successful lifecycle integration
At minimum test the complete sequence:
- configure
- initialize
- start
- stop
- shutdown
- new initialize/start after a completed live period where the accepted contract permits it.

Verify component invocation counts, ordering and resulting states.

### AC-04 — Invalid lifecycle integration
Verify invalid Runtime operations are rejected without component invocation and without unintended state/progress changes.

### AC-05 — Failure propagation integration
For representative failures in initialize/start/stop:
- original Error code/severity/source/message are preserved;
- fail-fast behavior is observed;
- later components are not invoked;
- Runtime enters FAULT as specified.

### AC-06 — Reset/recovery integration
- reset() is accepted only in FAULT.
- Cleanup follows the accepted two-pass reverse-order semantics.
- The failed operation is not retried.
- Successful cleanup progress is not repeated.
- Failed cleanup preserves FAULT and the original fault error.
- Successful reset ends in STOPPED and clears the fault.
- A subsequent initialize() constitutes a new explicit attempt.

### AC-07 — Cross-contract behavior
Integration tests must exercise interactions between:
- registry topology freeze;
- dependency validation;
- lifecycle ordering;
- runtime progress tracking;
- fault tracking;
- statistics;
- non-owning component lifetime.

### AC-08 — No health/logging/recovery side effects
- Health state does not trigger recovery.
- No background thread/timer/watchdog is required.
- No logging backend is required.

### AC-09 — Public API discipline
- Tests use accepted public APIs.
- No production API is added solely for test convenience.
- Frozen R03-001..006 public APIs remain unchanged.

### AC-10 — Requirement traceability
- `CORE-RT-009` is added to authoritative `REQUIREMENTS.md`.
- Integration test sources carry the appropriate requirement tags.
- Traceability audit reports zero errors.

## 3. Required Test Matrix
- Successful multi-component lifecycle.
- Multiple dependency branches.
- Registration-order permutations.
- Dependency-edge insertion permutations.
- Invalid lifecycle operations.
- Initialize failure.
- Start failure.
- Stop failure.
- Reset after each representative fault.
- Failed reset and resumed cleanup.
- Repeated successful cleanup not invoked twice.
- Topology freeze interaction.
- Statistics interaction.
- Non-owning lifetime/destruction interaction.
- Existing complete R03 unit/regression suite.

## 4. Validation
- Debug build/test.
- Release build/test.
- ASan + UBSan.
- TSan where configured.
- Strict `-Werror`.
- GCC `-fanalyzer`.
- `make check`.
- Coverage reviewed against the R03 baseline.
- Install-consumer regression.
- `git diff --check`.

## 5. Evidence Required
- CTest output and integration scenario matrix.
- Determinism evidence.
- Failure/reset traces.
- Mutation testing for integration invariants where practical.
- Build/sanitizer/static-analysis results.
- Traceability result.
- Install-consumer result.
- Clean working tree.

## 6. Integration Freeze Trigger
Acceptance of R03-007 activates the **R03 Integration Freeze**. From that point, production API changes are prohibited unless explicitly approved through architecture review.

## 7. Reviewer Sign-off
Only the independent architect/reviewer records PASS / CHANGES REQUIRED / BLOCKED.
