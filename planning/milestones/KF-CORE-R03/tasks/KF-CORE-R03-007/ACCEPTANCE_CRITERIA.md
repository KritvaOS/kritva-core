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

## 6a. Implementation Evidence (Claude)

- Primary commit: `9d1c7d1` `test(core): add runtime integration contracts` (Runtime Contract Review PASS at `8ec7861`).
- Changed files: `tests/integration/runtime_integration_test.cpp` (new) and `CMakeLists.txt` (CTest registration `kritva_core_runtime_integration`). **No production file changed:** `git diff --stat 8ec7861 -- include src` is empty; the frozen R03-001..006 public APIs are unchanged. `VERSION` and all release metadata are untouched (version-neutral, per the Runtime Contract Review).
- **`CORE-RT-009` not defined** (per the task: only after review); `grep RT-009` outside `planning/` finds nothing. The new test is tagged `CORE-RT-006, CORE-RT-007, CORE-RT-008`; after review it should be retagged `CORE-RT-009` with a traceability row.
- Public-API discipline: the test uses `Component`, `ComponentInfo`, `RuntimeManager` (its `registry()`/`dependencies()` views), `Result`, `Error`, `Statistics`, plus the existing test-only `ReferenceComponent` (fault injection and shared invocation trace, introduced in R03-001/R03-005). No production API was added for the tests and no private detail is read.
- **Integration scenario matrix** (12 test functions, all through the public API):

| Required matrix item | Test |
|---|---|
| Successful multi-component lifecycle; configure -> initialize -> start -> stop -> shutdown -> new initialize/start | `test_full_lifecycle_across_two_live_periods` (8 components, exact traces per operation, exactly-once invocation counts per live period, statistics 8 x 9 samples) |
| Multiple dependency branches | `test_branch_shapes_order_forward_and_reverse` (fan-in, fan-out, deep chain with ids against the order, diamond, independent branches; forward/reverse traces and dependency-before-dependent checks) |
| Registration-order and edge-insertion permutations | `test_registration_and_edge_order_permutations_are_unobservable` (all 24 edge insertion orders x 5 random registration orders = 120 runs, plus 200 random shuffles on the 8-component topology; identical full-cycle traces) |
| Invalid lifecycle operations | `test_invalid_operations_do_not_disturb_a_lifecycle` (invalid calls in UNKNOWN/READY/RUNNING/STOPPED interleaved with a lifecycle: zero invocations, unchanged state, final trace identical to a clean run) |
| Initialize / start / stop failure; reset after each; new explicit attempt | `test_failure_then_reset_then_new_attempt_at_representative_positions` (3 operations x first/middle/last positions: original error fields, exact fail-fast prefix, FAULT, `fault_error()`, only reset accepted, two-pass reverse cleanup trace from an independent stage model, failed op not retried, STOPPED, then a full new initialize/start) |
| Failed reset and resumed cleanup | `test_failed_reset_preserves_fault_and_resumes_without_repeating_cleanup` (original fault kept, exact resume trace, no repeated stop or shutdown) |
| Repeated successful cleanup not invoked twice | `test_repeated_cleanup_is_never_invoked_twice` |
| Topology freeze interaction | `test_topology_freeze_interacts_correctly_with_failure_and_recovery` (configure and failed validation do not freeze; freeze persists through FAULT, reset and re-initialize) |
| Statistics interaction | `test_statistics_account_for_every_component_call` and the model test (samples + errors equals the sum of component call counters after every operation) |
| Non-owning lifetime/destruction | `test_runtime_never_owns_or_destroys_components` (runtime destroyed while RUNNING and while FAULT; a probe component is never deleted by the runtime) |
| No health/background side effects | `test_health_does_not_drive_the_runtime` (1000 observations, no calls, `retry_count` 0, no threads/timers/logging used) |
| Cross-contract random check | `test_model_based_random_scenarios`: seeded (20261004), 400 random topologies (1-7 components, random DAGs, shuffled registration and edge order) x 40 random operations (configure/initialize/start/stop/shutdown/reset) with random one-shot failure injection; after every operation the result code and source, the exact invocation trace, runtime state, `topology_fixed()`, `fault_error()`, statistics and every component state are compared with an independent model of the frozen contract (16,000 operations, over 2,000 failures and over 1,000 resets in the run; the test asserts these minimums so the generator cannot go vacuous). The existing complete R03 unit/regression suite also runs and passes. |

- **Determinism evidence:** the permutation and shuffle tests above, plus fixed seeds throughout; the run is repeatable.
- **Failure/reset traces** (example, 8-component topology `1->4,1->5,2->5,3->6,4->7,5->7,6->8,7->8`; forward `8,6,3,7,4,5,1,2`): a failed `start` at 1 followed by a failed cleanup `stop` at 4 gives `reset` = `2:stop 5:stop 4:stop` (FAULT kept, original fault kept); the next `reset` = `7:stop 3:stop 6:stop 8:stop` then `2,1,5,4,7,3,6,8` `:shutdown`; no component is stopped or shut down twice and no failed operation is retried.
- **Mutation evidence** for the integration invariants (each temporary production edit reverted; file verified identical; all aborted the integration test except one equivalent mutant): start in reverse order; registration-order order; stage not cleared on initialize; shutdown ignoring progress; fault not cleared by reset; reset stopping faulted components; samples counted on failure; failure not fail-fast; reset pass 2 in forward order; failed cleanup stop not recorded faulted; invalid-state error code changed; configure fixing the topology. **Equivalent mutant (survived, not a test gap):** removing `components_live_ = false` from `reset()`: after a reset every invoked component is recorded `SHUT_DOWN` and the rest `NONE`, so a later `shutdown()` skips all components either way and the flag is not observable there.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **25/25** (24 prior + `kritva_core_runtime_integration`). Release 25/25; ASan+UBSan 25/25; TSan (ASLR disabled) 25/25; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 25/25; GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer: passed (unchanged).
- `make check` passed (traceability 56 requirements, 55 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (434/439), unchanged: no production code changed and `runtime_manager.cpp` stays 120/121.
- Final `git status --short`: clean after the commit.
- Known limitations: the model mirrors the documented contract and the reference component, so it checks implementation consistency with the frozen contract, not that the contract is optimal; no threads, platform or hardware are exercised by design.

## 7. Reviewer Sign-off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 03-10-2026 |
| Decision | **PASS** |

Accepted commit: `9d1c7d1` `test(core): add runtime integration contracts`; evidence `1da11e2`, `2a858da`. No production API or implementation change; no corrective commit required.

Reviewer notes: the 400-topology x 40-operation model-based test is particularly strong evidence (result code and source, invocation trace, runtime state, topology freeze state, `fault_error()`, statistics and every component state compared after every operation). The single surviving mutation (removing `components_live_ = false` in `reset()`) is correctly identified as equivalent. `CORE-RT-009` is deliberately defined after acceptance, not during implementation: the test temporarily references `CORE-RT-006`, `CORE-RT-007` and `CORE-RT-008`.

**Reviewer Decision: PASS — KF-CORE-R03-007 is ACCEPTED. The R03 Integration Freeze is now ACTIVE.**
