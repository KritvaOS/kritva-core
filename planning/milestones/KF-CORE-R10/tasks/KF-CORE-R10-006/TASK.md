# KF-CORE-R10-006 — Compatibility & Boundary Validation Harness

## Objective

Implement automated checks for the accepted R1.0 public API/compatibility contract without making test-only support production API.

## Requirement

CORE-COMPAT-009, defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (validates the frozen policy; adds no production code)

- `tests/compat/api_surface.snapshot` and `scripts/audit/check_api_surface.py` (`--update`, `--self-test`): ordered public and protected declaration surface of every installed header; differences reported as Incompatible / Review-required / Compatible candidates; wired into `make check` (`api-surface-check`, `api-surface-update`) and two CTests.
- `tests/unit/api_compat_boundary_test.cpp` and CTest `kritva_core_api_compat_boundary`: underlying type and value of every public enumerator with exhaustive switches; type properties of stable value types; the pure-virtual set and signatures of every client-implemented interface through conforming implementers.
- Mutation evidence on a copy of the headers (never the real tree).
- Requirement CORE-COMPAT-009 with a process row; `docs/compatibility/README.md` (index and workflow); `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No header, `src/` or behavior change; no production compatibility framework; no new public API; no policy change (the policy is frozen at `55e57df`); no package or install-consumer behavior (R10-007); no version change.

## Dependencies

R10 API / Compatibility Review (PASS / FROZEN).
