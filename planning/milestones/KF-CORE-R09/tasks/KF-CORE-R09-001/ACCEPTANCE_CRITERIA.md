# KF-CORE-R09-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define capability contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09 Scope Confirmation |

## Objective

Define the normative generic meaning of Capability and CapabilityId, including provider semantics, identity ownership, descriptive metadata and explicit non-security semantics, without introducing a new public type.

## Proposed Requirements Traceability

CORE-CAP-004

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The accepted contract states the authoritative identity rule.
- The accepted contract distinguishes provider declaration from consumer requirement.
- Capability metadata is explicitly non-security evidence.
- No new production type/API is introduced unless justified and approved by API Review.
- `docs/api/capability/CAPABILITY.md` is created or updated in the same task.
- Requirements, API documentation and tests are mutually traceable.
- Negative tests cover zero/invalid capability identity where applicable.

## New Tests

- Capability identity equality/mismatch tests.
- Provider metadata independence tests.
- Negative invalid-id tests where the existing API supports them.

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

- `Capability-related public headers only if semantics require clarification.`
- `tests/unit/ capability contract tests.`
- `docs/api/capability/CAPABILITY.md.`
- `Traceability/requirements updates as acceptance reconciliation.`

## Git Commit

```text
feat(core): define capability contract
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

**Reviewer Decision: PENDING — KF-CORE-R09-001 is not yet accepted.**
