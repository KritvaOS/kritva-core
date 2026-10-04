# KF-CORE-R09-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-005 |
| Status | ACCEPTED |
| Primary commit | `test(core): add capability readiness lifecycle integration tests` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 3–4 ED |
| Dependency | R09-004 |

## Objective

Prove that prerequisite availability, readiness, Component lifecycle and Runtime dependency ordering remain distinct and that Core does not introduce automatic readiness inference.

## Proposed Requirements Traceability

CORE-CAP-009

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- A missing prerequisite can be surfaced by a Component without a new Core lifecycle state.
- Runtime propagates the Component failure according to existing semantics.
- A successful prerequisite check does not implicitly change Runtime/Component state outside the documented lifecycle operation.
- Health changes do not automatically alter lifecycle.
- Capability presence does not alter dependency order.
- Repeated runs are deterministic.
- ComponentContext/PlatformContext boundaries remain unchanged unless explicitly approved.

## New Tests

- Missing prerequisite initialize failure.
- Successful prerequisite initialize path.
- Health independence.
- DependencyGraph independence.
- No-retry/no-recovery assertion.

## Regression Tests

- Full pre-R09 CTest suite remains green.
- Existing R0.8 Runtime, Component, Configuration, Platform and Context behavior remains green.

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Where applicable:

```bash
make check
make traceability-check
```

## Coverage / Quality

- Newly added executable production lines must be covered.
- No unexplained coverage exclusion.
- Overall R0.9 coverage remains at or above 98% and has no unexplained regression greater than 1 percentage point from the R0.8 98.9% baseline.
- Required sanitizer, strict-analysis and install-consumer evidence is supplied at milestone validation.

## API Documentation

- Any accepted public API or semantic contract change is documented in the corresponding `docs/api/` Markdown in the same task.
- Documentation must match accepted headers, semantics, ownership/lifetime, lifecycle interaction, errors, threading/real-time expectations and exclusions.
- API/documentation mismatch is an acceptance blocker.

## Security Impact

- The implementor records `SECURITY IMPACT: NONE`, `SECURITY IMPACT: DOCUMENTATION ONLY`, or `SECURITY IMPACT: ARCHITECTURE REVIEW REQUIRED`.
- No new security mechanism is introduced without explicit architecture approval.

## Expected Files Changed

- `tests/integration capability/lifecycle tests.`
- `Existing runtime test harness extensions as needed.`
- `No production API expected by default.`

## Git Commit

```text
test(core): add capability readiness lifecycle integration tests
```

## Evidence Required from Implementor

- `git status` before and after implementation.
- `git diff --check`.
- Build/test output.
- Focused contract-test evidence.
- Mutation evidence for contract-sensitive behavior where applicable.
- API documentation diff.
- Security-impact assessment.
- Diff/stat summary.
- Commit SHA.
- Explicit mapping from every acceptance criterion to objective evidence.

## Implementor Evidence

Primary commit: `0f6b6c3` `test(core): add capability readiness lifecycle integration tests`. **Test-only:** `git diff 4c86b53 HEAD -- include src` is empty (production freeze commit `4c86b53`); no production clarification was needed. Changes: `tests/integration/capability_readiness_integration_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CAP-009` + row), `API.md` section 51, `TESTING.md`, and the documentation this boundary needs, authored in the same commit (D12): `docs/api/lifecycle/LIFECYCLE.md` (full page with the "Readiness and capability boundary" section) and `docs/api/runtime/DEPENDENCY_GRAPH.md` (full page with the "capabilities are not dependencies" rule), each with the 15 required sections and documenting the unchanged headers only.

- Public APIs only. A `ConsumerComponent` (test-only, on the plain reference component) holds its own `PlatformRequirements` and checks them **inside its own `initialize()`** through its `ComponentContext`; when the prerequisite is missing it fails with the context's existing `UNSUPPORTED` Error (attributed to it) and ends in FAULT exactly as a failed initialize does. A test-only `ProvisionAdapter` (no services, a settable capability declaration, a snapshot counter) stands for the provider.
- Tests (`kritva_core_capability_readiness_integration`, 8 functions), mapped to your acceptance criteria:
  1. **A missing prerequisite is surfaced by the Component without a new Core lifecycle state; the Runtime propagates it by existing semantics:** component 2 of a 1→2→3 chain lacks a capability: the Runtime returns that component's own `UNSUPPORTED` Error (source 2), enters FAULT with the same fault error, the dependent (3) is never invoked (fail-fast), observed states are only UNKNOWN/READY/RUNNING/STOPPED/FAULT.
  2. **No capability-driven retry or recovery:** after the failure the missing capability is added to the provider and 50 observations of the Runtime follow; the state stays FAULT, no component is called and **no capability snapshot is taken**; only an explicit `reset()` then an explicit `initialize()` (a second attempt by the caller) succeeds, then `start()` works.
  3. **A successful check changes nothing outside the lifecycle operation:** 20 explicit `has_capability()`/`evaluate()` calls between operations leave the component state, the Runtime state and the Runtime statistics (sample/error 0) unchanged; the Runtime counts only its own component call.
  4. **Health does not change the lifecycle:** all four health values with the provision removed and restored leave both the Runtime and the component RUNNING, with no error and no fault, and the capability set never touched Health.
  5. **The Runtime never takes a capability snapshot or evaluates anything:** with an adapter attached, configure/initialize/start/stop/shutdown take zero snapshots.
  6. **Capability presence never changes the dependency order or the initialization trace:** 60 seeds × 3 random provisions over a random acyclic topology give one identical order.
  7. **A seeded model predicts every outcome** from the dependency order and the provision alone (200 seeds): the failing component, its error code and source, the exact invocation trace (fail-fast: only the components up to the failing one), FAULT versus READY, recovery only by explicit `reset()`, `retry_count` 0; every run is replayed twice and the transcripts are identical (determinism). A vacuity guard asserts >40 failures, >40 successes and >10 failures in the middle of the order.
  8. **ComponentContext/PlatformContext boundaries unchanged:** the integrator-built context is immutable and never created, passed or probed by the Runtime.
- Mutation evidence (8 production Runtime mutants, each reverted; run-time kills re-verified because one mutant first failed to compile — the Runtime source only forward-declares the adapter, so it **cannot** call it without adding an include, which I then added to make the mutant real): a capability snapshot taken in `initialize()` (killed at run time), an initialize retried on failure, an automatic `reset()` after a failure, an automatic second pass that turns a failure into READY, an initialize failure ignored, a retry counter incremented, the initialization order reversed — all killed; one mutant (an early `return result` after marking a stage in `call()`) is **equivalent** (the statement that follows is the same `return result`).
- Regression: ctest 62/62 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean (including the new test); coverage 618/625 unchanged; `make check`, traceability 107 requirements, 106 traced, 0 errors (`CORE-CAP-009` defined with a row); `git diff --check` clean.
- Security-impact: no production change; the pages document lifecycle state as operational information, not authorization or trust; **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `0f6b6c3` (evidence `9a086ae`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: test-only (no production diff since the freeze commit `4c86b53`); consumer components decide their own prerequisite sufficiency inside a lifecycle operation and the Runtime only propagates the failure; no capability-driven retry, recovery, polling, evaluation or snapshot by the Runtime; unchanged order and trace under random provisions; Health independence; the seeded predictive model with a vacuity guard; the synchronized lifecycle and dependency-graph pages and the 8 mutants (one equivalent) are accepted. `CORE-CAP-009` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-005 is ACCEPTED.**
