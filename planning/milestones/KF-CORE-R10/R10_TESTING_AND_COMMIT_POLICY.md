# KF-CORE-R10 — Testing and Commit Policy

## Testing Baseline

Preserve the R0.9 quality model and explain any meaningful regression. R0.9 baseline includes full Debug/Release CTest, ASan/UBSan, TSan where configured, strict `-Werror`, GCC analyzer, header self-containment, install-consumer validation, production isolation, traceability, API documentation audit and approximately 98.9% line coverage (618/625).

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

## R10 Compatibility Testing

The R1.0 validation model shall cover, where applicable:

- public-header/API inventory consistency;
- signature/boundary snapshot validation;
- semantic compatibility fixtures;
- public enum/ErrorCode numeric-value stability;
- ownership/lifetime/thread-safety contract stability;
- deprecated API presence and migration documentation;
- CMake package version-selection behavior;
- external install-consumer builds;
- compatibility behavior across supported 1.x package versions once 1.0 is released.

Compatibility tests must distinguish a deliberate breaking-change candidate from an accidental regression.

## Documentation Testing

- Every accepted public API/semantic change updates the appropriate `docs/api/` material.
- The compatibility policy is referenced consistently from API guidelines and release documentation.
- Stub-to-maintained status changes are explicit.
- Documentation never becomes broader than the accepted contract.

## Security Review

Every R1.0 change that affects public API semantics, package installation, release artifacts or authority/lifetime boundaries receives a security-impact assessment.

No new security mechanism is introduced without explicit architecture approval.

## Regression Policy

The complete historical R0.2–R0.9 regression suite remains green. New compatibility tests are separately reported and then become part of the 1.x regression baseline after R1.0 release.

## Coverage Policy

- All newly added executable production lines must be exercised.
- No new coverage exclusion without reviewer approval.
- Target remains at or above 98% and avoids unexplained degradation greater than 1 percentage point from the R0.9 baseline.

## Commit Policy

One logical task = one primary implementation commit. Exact commit message is defined in each task's `ACCEPTANCE_CRITERIA.md`. Accepted task commits are not amended. Focused corrective follow-up commits are allowed after review.

## Freeze Gates

- R10 API / Compatibility Review after R10-005.
- R10 Integration Freeze after R10-007.
- R10 Release Gate after R10-009.

## Reviewer Rule

Claude/Codex supplies implementation, tests and objective evidence. ChatGPT performs the independent architecture/acceptance decision: PASS, CHANGES REQUIRED, or BLOCKED.
