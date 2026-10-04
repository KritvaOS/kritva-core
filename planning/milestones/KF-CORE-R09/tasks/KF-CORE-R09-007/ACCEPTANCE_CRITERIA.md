# KF-CORE-R09-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-007 |
| Status | PLANNED |
| Primary commit | `test(core): complete R0.9 capability validation` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-006 accepted |

## Objective

Run the complete R0.9 quality matrix, verify the release boundary and prepare the 0.9.0 release candidate.

## Proposed Requirements Traceability

CORE-CAP-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Fresh-clone Debug and Release builds pass with zero unexpected warnings.
- Full CTest regression passes.
- ASan/UBSan pass.
- TSan passes using documented environment.
- Strict Werror passes.
- GCC analyzer passes where configured.
- Header self-containment passes.
- Coverage meets milestone policy.
- Traceability is clean.
- Install consumer passes.
- API documentation and security records are consistent.
- Production diff after Integration Freeze contains no unreviewed semantic change.
- Release metadata is internally consistent for 0.9.0.

## New Tests

- Complete validation matrix.
- Release candidate smoke/consumer test.
- Ten randomized order repetitions where ordering/determinism is relevant.

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

- `Validation evidence record.`
- `Release candidate metadata.`
- `No unrelated production files.`

## Git Commit

```text
test(core): complete R0.9 capability validation
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

**Reviewer Decision: PENDING — KF-CORE-R09-007 is not yet accepted.**
