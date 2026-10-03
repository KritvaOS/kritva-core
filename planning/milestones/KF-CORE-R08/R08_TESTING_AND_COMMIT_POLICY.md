# KF-CORE-R08 — Testing and Commit Policy

## Testing Baseline

The R0.7 release candidate established a 51/51 CTest baseline, 98.9% line coverage (618/625), clean Debug/Release builds, sanitizer validation, strict warning validation, analyzer validation, traceability checks, public-header self-containment and install-consumer validation. R08 must preserve the established quality model and explain any meaningful regression.

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

## R08 Contract Testing

Every configuration rule must have:

- positive conformance coverage;
- invalid-state coverage;
- failure/non-partial-application coverage where applicable;
- ownership/lifetime coverage where applicable;
- Runtime boundary coverage where applicable;
- mutation testing for contract-sensitive branches.

The test harness must detect deliberately broken conforming-component variants rather than merely exercise the reference implementation.

## Regression Policy

- Existing R0.2-R0.7 tests remain green.
- New tests are separate from regression tests in acceptance evidence.
- A changed production contract must have focused tests before it is accepted.
- No acceptance is granted from compilation alone.

## Coverage Policy

- All newly added executable production lines must be exercised.
- No new coverage exclusion may be added without reviewer approval.
- Overall coverage target: remain at or above 98% and avoid an unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.

## Commit Policy

One logical task = one primary implementation commit. The exact commit message in the task acceptance criteria is authoritative for that task. Do not amend an accepted task commit. If review identifies a defect after a task commit exists, use a focused follow-up commit.

## Freeze Gates

- R08 Configuration API Review after R08-003.
- R08 Integration Freeze after R08-005 and before R08-006/R08-007.
- R08 Release Gate after R08-007.

## Reviewer Rule

Claude/Codex supplies objective evidence. ChatGPT performs the independent acceptance decision: PASS, CHANGES REQUIRED, or BLOCKED.
