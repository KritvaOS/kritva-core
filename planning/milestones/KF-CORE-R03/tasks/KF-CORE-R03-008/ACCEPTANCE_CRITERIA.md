# KF-CORE-R03-008 — Acceptance Criteria

## 1. Task Information
- **Requirement:** CORE-RT-010
- **Dependency:** R03 Integration Freeze = PASS
- **Primary commit:** `test(core): complete R03 runtime validation`

## 2. Acceptance Criteria

### AC-01 — Clean reproducible build
- Build starts from a clean build directory.
- Debug build succeeds with zero warnings.
- Release build succeeds with zero warnings.
- No generated/build artifacts are committed unexpectedly.

### AC-02 — Complete regression
- All R03 unit and integration tests pass.
- All pre-R03 regression tests pass.
- No test is disabled, skipped or weakened to obtain a pass without documented justification and reviewer approval.

### AC-03 — Sanitizers and strict quality
- ASan + UBSan pass.
- TSan passes where configured, with any environment workaround explicitly documented.
- Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` passes where configured.
- GCC `-fanalyzer` passes.

### AC-04 — Coverage
- Coverage is generated from the clean candidate.
- Coverage is reviewed against the established R03 baseline of 98%.
- Any uncovered production line is either covered or documented as unreachable/otherwise justified.
- No coverage exclusion is introduced merely to improve the percentage.

### AC-05 — Traceability
- `CORE-RT-001` through `CORE-RT-010` are reconciled with authoritative `REQUIREMENTS.md`.
- No duplicate requirement IDs.
- All implemented R03 requirements are traced.
- Traceability audit reports zero errors.

### AC-06 — API and architecture freeze
- Frozen R03-001..007 production APIs remain unchanged.
- No new runtime functionality is introduced by validation.
- No platform-specific runtime dependency is introduced.
- No ROS2/DDS/EtherCAT/vendor HAL/BSP/threading backend is introduced.

### AC-07 — Install-consumer
- Installation succeeds.
- A clean external consumer can compile/link against the installed R03 library.
- The consumer exercises representative RuntimeManager lifecycle and failure/recovery APIs.

### AC-08 — Dependency/prohibited-header checks
- No prohibited runtime/threading/platform headers or libraries appear in Core production code.
- Dependency audit passes.
- `git diff --check` passes.

### AC-09 — Documentation consistency
- API.md, ARCHITECTURE.md, REQUIREMENTS.md and R03 planning documents agree with the frozen implementation.
- Changelog is ready for release update.

### AC-10 — Clean candidate
- Working tree is clean after the validation commit.
- Commit hash and evidence package are recorded.
- Candidate is suitable for the R03 Release Gate.

## 3. Required Evidence
- Clean build logs.
- Complete CTest results.
- Debug/Release results.
- Sanitizer results.
- Strict compiler/static-analysis results.
- Coverage report.
- Traceability audit.
- Install-consumer result.
- Dependency/prohibited-header scan.
- Git status/diff-check evidence.
- Final changed-file and commit summary.

## 4. Reviewer Sign-off
Only the independent architect/reviewer records PASS / CHANGES REQUIRED / BLOCKED.
