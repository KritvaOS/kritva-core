# KF-CORE-R09-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-003 |
| Status | PLANNED |
| Primary commit | `feat(core): define capability requirement matching boundary` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-002 |

## Objective

Clarify and test the relationship between capability provision and capability requirements using existing mechanisms, without adding a generic dependency resolver or Component requirement API by default.

## Proposed Requirements Traceability

CORE-CAP-007, CORE-CAP-008

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The distinction between provided Capability and required Capability is normative.
- Capability matching rules are explicit.
- No implicit name/platform/vendor matching exists.
- DependencyGraph remains independent and ComponentId-based.
- Existing PlatformRequirements behavior remains compatible unless an explicitly reviewed correction is necessary.
- `docs/api/capability/CAPABILITY_REQUIREMENTS.md` and/or `docs/api/platform/PLATFORM_REQUIREMENTS.md` are synchronized.
- Mutation tests detect weakened matching/side-effect boundaries.

## New Tests

- Required/optional capability tests.
- Identity mismatch tests.
- Side-effect-free evaluation tests.
- DependencyGraph independence test.

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

- `Platform requirements/capability files only if needed.`
- `tests/unit capability requirement tests.`
- `docs/api/capability/CAPABILITY_REQUIREMENTS.md.`

## Git Commit

```text
feat(core): define capability requirement matching boundary
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

**Reviewer Decision: PENDING — KF-CORE-R09-003 is not yet accepted.**
