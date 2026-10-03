# KF-CORE-R03 — Release Gate

## Gate
**Timing:** After KF-CORE-R03-008 and before creation of the R03 release tag.

## Proposed Release
- **Milestone:** KF-CORE-R03 — Runtime Foundation
- **Version:** `0.3.0`
- **Release tag:** `kritva-core-r0.3`

The version/tag are release candidates until this gate is PASS. The authoritative `VERSION`/build metadata must be checked before tagging.

## Purpose
Provide the final independent acceptance decision for R03 and authorize creation of the release tag.

## Entry Criteria
- [x] R03-001 through R03-008 accepted.
- [x] Foundation API Review = PASS/FROZEN.
- [x] Runtime Contract Review = PASS/FROZEN.
- [x] Integration Freeze = PASS.
- [x] R03-008 final validation = PASS.
- [x] No unresolved CHANGES REQUIRED/BLOCKED review item.

## Release Acceptance Criteria

### Architecture and API
- [x] Component, Registry and DependencyGraph foundation contracts remain frozen.
- [x] CORE-RT-002 remains authoritative.
- [x] CORE-RT-006 through CORE-RT-010 are authoritative and traceable.
- [x] No unapproved public API changes exist.
- [x] No prohibited platform/runtime dependencies were introduced.

### Functional behavior
- [x] Component registration and dependency validation are deterministic.
- [x] Runtime topology validation/freeze is deterministic.
- [x] Lifecycle ordering and reverse teardown are deterministic.
- [x] Failure propagation preserves originating errors.
- [x] Explicit reset/recovery is deterministic.
- [x] No automatic/background recovery exists.

### Validation
- [x] Complete CTest suite passes.
- [x] Debug and Release pass.
- [x] ASan/UBSan pass.
- [x] TSan passes where configured.
- [x] Strict `-Werror` passes.
- [x] GCC `-fanalyzer` passes.
- [x] Coverage requirement is satisfied/reviewed.
- [x] Traceability audit has zero errors.
- [x] Install-consumer passes.
- [x] Working tree is clean.

### Documentation
- [x] REQUIREMENTS.md reconciled.
- [x] API.md reconciled.
- [x] ARCHITECTURE.md reconciled.
- [x] MILESTONE_STATUS.md updated.
- [x] CHANGELOG.md contains the R0.3 release entry.
- [x] Release evidence is archived/reproducible.

### Release metadata
- [x] `VERSION` and build metadata agree with the intended R0.3 version.
- [x] Release commit is identified.
- [x] Release tag name is verified before creation.
- [x] Tag points to the accepted release commit.
- [x] Annotated tag message identifies Kritva Core R0.3.

## Release Procedure
After PASS:
1. Commit final documentation/status changes.
2. Record the final release commit hash.
3. Verify clean tree.
4. Create annotated tag `kritva-core-r0.3`.
5. Verify tag target with `git show kritva-core-r0.3`.
6. Push branch/tag only when explicitly authorized.

## Reviewer Decision
Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R03 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.3`) | `kritva-core-r0.3` (annotated, on `cc16ec9`, pushed to origin) | ChatGPT | 03-10-2026 |

## Recorded release

- Release: Kritva Core R0.3 — Runtime Foundation
- Version: `0.3.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `f598fef` (fresh-clone validation; validation record `407df6b`; R03-008 accepted at `ebe79f0`)
- Decision: **PASS**
- Release tag: `kritva-core-r0.3`, annotated, on the documentation-only release-record commit that records this gate (not on `f598fef`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, `MILESTONE.md`, the planning `CHANGELOG.md`); no implementation, API or behavior change.
- Push: performed by the user; `origin/main` and the tag `kritva-core-r0.3` (tag object `0dfccab`, target `cc16ec9`) are published.

Deferred as post-R0.3 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, the historical `kritva-core-r0.1` tag discrepancy, no recovery directly to READY, the raw `fault_error()` pointer, the single shared `Configuration` for all components, and no thread-safety or real-time guarantee.
