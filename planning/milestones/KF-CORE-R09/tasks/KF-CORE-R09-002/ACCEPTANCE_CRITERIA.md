# KF-CORE-R09-002 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-002 |
| Status | PLANNED |
| Primary commit | `feat(core): define capability set and version semantics` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-001 |

## Objective

Define Capability version meaning and CapabilitySet invariants, ownership/snapshot behavior and deterministic observable semantics while avoiding a registry abstraction.

## Proposed Requirements Traceability

CORE-CAP-005, CORE-CAP-006

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Version semantics are explicit and unambiguous.
- CapabilitySet invariants are documented and testable.
- Ownership/lifetime rules are explicit.
- Duplicate identity behavior is either defined and enforced or demonstrably absent by the existing contract.
- No hidden global state or registry is introduced.
- `docs/api/capability/CAPABILITY_SET.md` documents the accepted contract.
- Focused tests and mutation evidence cover contract-sensitive invariants.

## New Tests

- Copy/snapshot lifetime tests.
- Identity lookup tests.
- Duplicate/invalid capability tests as applicable.
- Mutation tests for identity and snapshot semantics.

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

- `CapabilitySet public/source files only if needed.`
- `tests/unit/capability_set_contract_test.cpp or equivalent.`
- `docs/api/capability/CAPABILITY_SET.md.`

## Git Commit

```text
feat(core): define capability set and version semantics
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

**Reviewer Decision: PENDING — KF-CORE-R09-002 is not yet accepted.**
