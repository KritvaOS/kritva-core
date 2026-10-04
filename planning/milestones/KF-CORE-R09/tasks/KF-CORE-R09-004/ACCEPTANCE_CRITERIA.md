# KF-CORE-R09-004 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-004 |
| Status | PLANNED |
| Primary commit | `test(core): add capability contract reference harness` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 3–4 ED |
| Dependency | R09 Capability API Review PASS/FROZEN |

## Objective

Provide reusable reference conformance checks that detect deliberately broken capability and requirement behavior without expanding production APIs.

## Proposed Requirements Traceability

CORE-CAP-010

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Harness uses public APIs only.
- Each intentional contract defect is detected.
- Reference component/provider behavior is deterministic.
- Harness verifies ownership/lifetime semantics.
- Harness does not require Nexus/Edge/platform-specific code.
- No production source/header changes unless explicitly approved to fix a demonstrated contract gap.

## New Tests

- Reusable conformance suite.
- Mutation matrix with all consequential defects detected.
- Repeated-order deterministic checks.

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

- `tests/runtime/reference_capability.hpp or equivalent.`
- `tests/unit/reference_capability_test.cpp.`
- `tests/unit/capability_conformance.hpp or equivalent.`
- `CMake/test registration.`

## Git Commit

```text
test(core): add capability contract reference harness
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

**Reviewer Decision: PENDING — KF-CORE-R09-004 is not yet accepted.**
