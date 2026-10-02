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

## Implementation
- [ ] Requirements are traceable.
- [ ] Error paths are handled.
- [ ] Boundary behavior is tested.
- [ ] No unrelated files changed.
- [ ] No hidden API behavior introduced.

## Testing
- [ ] New tests exist for changed behavior.
- [ ] Negative/failure cases are covered.
- [ ] Existing regression suite passes.
- [ ] Coverage is reviewed.
- [ ] Sanitizer/static analysis is clean where applicable.

## Documentation
- [ ] Public contract documentation is updated.
- [ ] REQUIREMENTS.md is consistent.
- [ ] CHANGELOG.md is updated at milestone completion.
- [ ] Evidence is reproducible.

## Git
- [ ] Exact task commit message used.
- [ ] Commit is focused.
- [ ] No accidental generated files.
- [ ] Working tree status recorded.
- [ ] Milestone tag created only after final acceptance.
