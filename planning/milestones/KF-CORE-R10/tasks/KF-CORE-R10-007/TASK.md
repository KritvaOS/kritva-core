# KF-CORE-R10-007 — Package / Install Compatibility

## Objective

Validate CMake package version-selection and installed downstream consumer behavior under the accepted 1.0 policy.

## Requirement

CORE-COMPAT-010, defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (the one approved production build change)

- `CMakeLists.txt`: the package version file uses `SameMajorVersion` (was `SameMinorVersion`); this is the only production-side change and is limited to the package build (no header, `src/`, API or version change).
- `tests/install/run_install_test.cmake` and the consumer project: requests derived from the project version (accepted: major, `major.0`, `major.minor`, full version; refused: newer patch, newer minor, next and previous major; `EXACT` accepted only for the full installed version string).
- `tests/install/package_version_matrix.cmake`: `find_package` against a model of the VERSIONING_POLICY section 6 rule for several installed versions, with and without `EXACT`; a second CTest shows the matrix detects the old same-minor mode.
- `scripts/audit/check_compat_policy.py`: fails if the package mode is not `SameMajorVersion`.
- Requirement CORE-COMPAT-010 with a process row; `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No header, `src/` or API change; no policy document change (frozen at `55e57df`); the project version stays `0.9.0` until R10-009; no ABI mechanism; no signing, provenance or authenticity mechanism.

## Dependencies

R10-006 (ACCEPTED).
