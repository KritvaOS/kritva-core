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
- [ ] R03-001 through R03-008 accepted.
- [ ] Foundation API Review = PASS/FROZEN.
- [ ] Runtime Contract Review = PASS/FROZEN.
- [ ] Integration Freeze = PASS.
- [ ] R03-008 final validation = PASS.
- [ ] No unresolved CHANGES REQUIRED/BLOCKED review item.

## Release Acceptance Criteria

### Architecture and API
- [ ] Component, Registry and DependencyGraph foundation contracts remain frozen.
- [ ] CORE-RT-002 remains authoritative.
- [ ] CORE-RT-006 through CORE-RT-010 are authoritative and traceable.
- [ ] No unapproved public API changes exist.
- [ ] No prohibited platform/runtime dependencies were introduced.

### Functional behavior
- [ ] Component registration and dependency validation are deterministic.
- [ ] Runtime topology validation/freeze is deterministic.
- [ ] Lifecycle ordering and reverse teardown are deterministic.
- [ ] Failure propagation preserves originating errors.
- [ ] Explicit reset/recovery is deterministic.
- [ ] No automatic/background recovery exists.

### Validation
- [ ] Complete CTest suite passes.
- [ ] Debug and Release pass.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict `-Werror` passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage requirement is satisfied/reviewed.
- [ ] Traceability audit has zero errors.
- [ ] Install-consumer passes.
- [ ] Working tree is clean.

### Documentation
- [ ] REQUIREMENTS.md reconciled.
- [ ] API.md reconciled.
- [ ] ARCHITECTURE.md reconciled.
- [ ] MILESTONE_STATUS.md updated.
- [ ] CHANGELOG.md contains the R0.3 release entry.
- [ ] Release evidence is archived/reproducible.

### Release metadata
- [ ] `VERSION` and build metadata agree with the intended R0.3 version.
- [ ] Release commit is identified.
- [ ] Release tag name is verified before creation.
- [ ] Tag points to the accepted release commit.
- [ ] Annotated tag message identifies Kritva Core R0.3.

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
| R03 Release Gate | PENDING | — | — | — | — |
