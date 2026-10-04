# KF-CORE-R09 — Testing and Commit Policy

## Testing Baseline

R0.8 established a 57/57 CTest baseline, 98.9% line coverage (618/625), clean Debug/Release builds, ASan/UBSan, TSan, strict warnings, GCC analyzer, header self-containment, install-consumer validation, traceability and dependency/prohibited-header audits. R0.9 shall preserve the established quality model and explain any meaningful regression.

## Per-Task Minimum

```bash
git status
git diff --check
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
git diff --stat
git diff
git status
```

Where applicable:

```bash
make check
make traceability-check
```

## R09 Contract Testing

Every R0.9 contract rule must have:

- positive conformance coverage;
- negative/error-path coverage;
- ownership/lifetime coverage where applicable;
- deterministic behavior coverage where applicable;
- Runtime boundary coverage where applicable;
- mutation testing for contract-sensitive behavior.

The reference harness must detect deliberately broken behavior and must not merely exercise the conforming reference implementation.

## Documentation Testing

- All accepted public API/semantic changes update `docs/api/` in the same task.
- Documentation links to requirements, headers and tests are verified.
- No documented API may differ from the accepted public headers/semantics.

## Security Testing / Review

- Every production API/semantic change receives a security-impact assessment.
- Trust/authority assumptions are documented.
- No new security mechanism is introduced without explicit architecture approval.

## Regression Policy

- Existing R0.2-R0.8 regression suite remains green.
- New tests are reported separately from historical regression results.
- No acceptance is granted from compilation alone.

## Coverage Policy

- All newly added executable production lines must be exercised.
- No new coverage exclusion without reviewer approval.
- Overall coverage target: remain at or above 98% and avoid an unexplained regression greater than 1 percentage point from the R0.8 98.9% baseline.

## Commit Policy

One logical task = one primary implementation commit. The exact commit message in each task's acceptance criteria is authoritative. Accepted task commits are not amended. Focused follow-up commits are permitted after review.

## Freeze Gates

- R09 Capability API Review after R09-003.
- R09 Integration Freeze after R09-005.
- R09 Release Gate after R09-007.

## Reviewer Rule

Claude/Codex supplies objective implementation/test evidence. ChatGPT performs the independent acceptance decision: PASS, CHANGES REQUIRED, or BLOCKED.
