# KF-CORE-R09-005 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-005 |
| Status | PLANNED |
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

To be completed by implementor after implementation.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT |
| Decision | **PENDING** |
| Accepted commit | Pending |
| Evidence reference | Pending |
| Date | Pending |

**Reviewer Decision: PENDING — KF-CORE-R09-005 is not yet accepted.**
