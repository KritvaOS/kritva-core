# Kritva Core — Common Review Checklist

## Architecture
- [ ] Change stays within milestone scope.
- [ ] Core remains platform independent.
- [ ] No ROS2/DDS/EtherCAT/vendor HAL/platform implementation added to Core.
- [ ] No unnecessary redesign of established APIs.
- [ ] Public API semantics are deterministic and documented.
- [ ] Ownership/lifetime semantics are clear.
- [ ] Thread-safety assumptions are explicit.
- [ ] Hard-real-time claims are not introduced without evidence.
- [ ] Existing R0.2 contracts remain authoritative unless a reviewed contract gap is demonstrated.

## Implementation
- [ ] Requirements are traceable.
- [ ] Error paths are handled.
- [ ] Boundary behavior is tested.
- [ ] No unrelated files changed.
- [ ] No hidden API behavior introduced.
- [ ] Later task does not silently change an earlier accepted public API.
- [ ] Any required breaking change is returned to architecture review.

## Testing
- [ ] New unit tests exist for changed behavior.
- [ ] Negative/failure cases are covered.
- [ ] Required integration tests exist.
- [ ] Existing regression suite passes.
- [ ] Debug build passes.
- [ ] Release build passes.
- [ ] `-Werror` passes.
- [ ] ASan/UBSan pass where configured.
- [ ] Coverage is reviewed.
- [ ] Traceability/dependency checks pass.
- [ ] Install-consumer regression remains green.

## R03 Foundation API Review
Apply after R03-003:
- [ ] R03-001 accepted.
- [ ] R03-002 accepted.
- [ ] R03-003 accepted.
- [ ] ComponentId semantics frozen.
- [ ] Component lifecycle contract frozen.
- [ ] Component ownership/lifetime frozen.
- [ ] Registry registration/lookup/enumeration semantics frozen.
- [ ] Registry deterministic ordering frozen.
- [ ] Dependency representation frozen.
- [ ] Missing/self/duplicate dependency behavior frozen.
- [ ] Cycle detection frozen.
- [ ] Topological ordering frozen.
- [ ] Deterministic tie-break frozen.
- [ ] Error/Warning/Info policy reviewed.
- [ ] Event versus message distinction reviewed.
- [ ] Statistics update policy reviewed.
- [ ] Logging backend boundary confirmed.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R03 Runtime Contract Review
Apply after R03-006:
- [ ] Runtime Manager contract frozen.
- [ ] Lifecycle orchestration frozen.
- [ ] Failure propagation frozen.
- [ ] Recovery/reset semantics frozen.
- [ ] Diagnostic semantics frozen.
- [ ] Statistics update semantics frozen.
- [ ] No implicit automatic retry introduced.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R03 Integration Freeze
Apply after R03-007:
- [ ] End-to-end runtime integration tests pass.
- [ ] No unresolved production API changes.
- [ ] Any required API change returned to architecture review.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## Documentation
- [ ] Public contract documentation is updated.
- [ ] REQUIREMENTS.md is consistent.
- [ ] Milestone-specific requirements proposal is reconciled with authoritative requirements.
- [ ] CHANGELOG.md is updated at milestone completion.
- [ ] Evidence is reproducible.

## Git
- [ ] Exact task commit message used.
- [ ] Commit is focused.
- [ ] No accidental generated files.
- [ ] Working tree status recorded.
- [ ] Follow-up fixes use focused commits.
- [ ] Milestone tag created only after final acceptance.
