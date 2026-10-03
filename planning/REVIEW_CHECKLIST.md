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

## R03 Runtime Implementation Review
Apply to R03-004 through R03-006:
- [ ] R03-004 implements existing `CORE-RT-002` / `runtime::Runtime`.
- [ ] No competing Runtime abstraction introduced.
- [ ] Frozen Foundation APIs unchanged.
- [ ] Runtime topology is fixed after setup.
- [ ] Dependency ordering is deterministic.
- [ ] Forward lifecycle order is dependency-first.
- [ ] Teardown order is reverse dependency order.
- [ ] Runtime state transitions are documented.
- [ ] Component-originated Error source/code preserved.
- [ ] Failure/partial-progress behavior is deterministic.
- [ ] Recovery is explicit caller-driven only.
- [ ] No automatic retry/background recovery/watchdog.
- [ ] Health is distinct from warning/error.
- [ ] Statistics policy matches Foundation/R02 decisions.
- [ ] No logging backend introduced.

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


## R03 Runtime Integration Review
Apply to R03-007:
- [ ] Runtime Contract Review is PASS/FROZEN.
- [ ] End-to-end Component + Registry + DependencyGraph + RuntimeManager scenarios pass.
- [ ] Registration/dependency insertion permutations remain deterministic.
- [ ] Successful lifecycle, failure and reset/recovery paths are integrated.
- [ ] No production API added solely for testing.
- [ ] CORE-RT-009 traceability is complete.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R03 Final Validation Review
Apply to R03-008:
- [ ] Integration Freeze is PASS.
- [ ] Clean Debug/Release builds pass.
- [ ] Complete regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict `-Werror` passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage reviewed against the R03 baseline.
- [ ] CORE-RT-010 traceability is complete.
- [ ] Install-consumer passes.
- [ ] Prohibited dependency/header scan passes.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R03 Release Gate
Apply after R03-008:
- [ ] All eight R03 tasks accepted.
- [ ] Foundation API Review PASS/FROZEN.
- [ ] Runtime Contract Review PASS/FROZEN.
- [ ] Integration Freeze PASS.
- [ ] Final validation PASS.
- [ ] Requirements/API/docs reconciled.
- [ ] Release version/tag target verified.
- [ ] Annotated `kritva-core-r0.3` tag authorized only after PASS.
- [ ] Final release decision recorded.

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

## R04 Platform Boundary Review
- [ ] Adapter boundary is explicit.
- [ ] Core does not own platform implementation objects unless explicitly contracted.
- [ ] Context lifetime is explicit.
- [ ] Scheduler semantics distinguish Core requirements from adapter policy.
- [ ] Clock domain semantics remain authoritative.
- [ ] Timer execution context and ownership are explicit.
- [ ] Watchdog expiry does not implicitly trigger Runtime recovery.
- [ ] Unsupported platform capabilities are represented explicitly.
- [ ] No global platform singleton is introduced.
- [ ] No platform-specific headers enter production Core sources.

## Implementation
- [ ] Requirements are traceable.
- [ ] Error paths are handled.
- [ ] Boundary behavior is tested.
- [ ] No unrelated files changed.
- [ ] No hidden API behavior introduced.
- [ ] Later task does not silently change an earlier accepted public API.
- [ ] Any breaking change is returned to architecture review.

## Testing
- [ ] New unit/contract tests exist for changed behavior.
- [ ] Negative/failure cases are covered.
- [ ] Adapter-defined behavior is not incorrectly asserted as universal Core behavior.
- [ ] Existing R0.3 regression suite passes.
- [ ] Debug build passes.
- [ ] Release build passes.
- [ ] `-Werror` passes.
- [ ] ASan/UBSan pass where configured.
- [ ] TSan passes where configured.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage is reviewed.
- [ ] Traceability/dependency checks pass.
- [ ] Install-consumer regression remains green.

## R04 Platform API Review
Apply after R04-004:
- [ ] R04-001 through R04-004 accepted.
- [ ] Adapter boundary frozen.
- [ ] Scheduler contract frozen.
- [ ] Clock contract frozen.
- [ ] Timer contract frozen.
- [ ] Watchdog contract frozen.
- [ ] Ownership/lifetime semantics frozen.
- [ ] Thread-safety boundary frozen.
- [ ] Real-time guarantee boundary frozen.
- [ ] Platform-specific implementation remains out of Core.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R04 Platform Integration Freeze
Apply after R04-006:
- [ ] R04-005 accepted.
- [ ] Conformance suite passes.
- [ ] Core public platform contracts are frozen.
- [ ] No unresolved production API changes.
- [ ] Any required API change returned to architecture review.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R04 Runtime–Platform Integration Review
Apply to R04-007:
- [ ] Platform Integration Freeze is PASS/FROZEN.
- [ ] RuntimeManager remains platform independent.
- [ ] Scheduler integration is explicit and optional.
- [ ] Clock integration preserves R0.3 lifecycle semantics.
- [ ] Watchdog does not trigger automatic Runtime recovery.
- [ ] Platform errors preserve Result/Error semantics.
- [ ] No thread/background execution introduced into Core.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R04 Final Validation Review
Apply to R04-008:
- [ ] Platform Integration Freeze is PASS.
- [ ] Clean Debug/Release builds pass.
- [ ] Complete regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict `-Werror` passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage reviewed.
- [ ] Requirements traceability reports zero errors.
- [ ] Install-consumer passes.
- [ ] Prohibited platform dependency scan passes.
- [ ] Production code remains platform independent.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R04 Release Gate
- [ ] All eight R04 tasks accepted.
- [ ] Platform API Review PASS/FROZEN.
- [ ] Platform Integration Freeze PASS/HONORED.
- [ ] Final validation PASS.
- [ ] Requirements/API/docs reconciled.
- [ ] VERSION and CMake version agree.
- [ ] Release version/tag target verified.
- [ ] Annotated `kritva-core-r0.4` tag authorized only after PASS.
- [ ] Final release decision recorded.

## Documentation
- [ ] Public contract documentation is updated.
- [ ] REQUIREMENTS.md is consistent.
- [ ] R04 requirements proposal is reconciled with authoritative requirements.
- [ ] CHANGELOG.md is updated.
- [ ] Evidence is reproducible.

## Git
- [ ] Exact task commit message used.
- [ ] Commit is focused.
- [ ] No accidental generated files.
- [ ] Follow-up fixes use focused commits.
- [ ] Accepted history is not rewritten.
- [ ] Milestone tag created only after final acceptance.


## R06 Component Execution Context Review

Apply during R06 architecture/design review and before R06-001 implementation authorization:

- [ ] R0.5 `IPlatformAdapter` and `PlatformContext` remain authoritative.
- [ ] Component context ownership/lifetime is explicit.
- [ ] Context is non-owning unless a reviewed contract says otherwise.
- [ ] No generic service registry, locator, singleton, global or thread-local context.
- [ ] Context access has no hidden lifecycle side effects.
- [ ] Approved operational services are explicitly enumerated and deterministic.
- [ ] Platform capability identity semantics are reused.
- [ ] No platform name/version inference.
- [ ] No raw `IPlatformAdapter*` is exposed as a general Component interface.
- [ ] Runtime lifecycle semantics remain unchanged unless explicitly approved.
- [ ] Component lifecycle signature changes, if any, are explicitly reviewed before implementation.
- [ ] Error propagation preserves the established Result/Error contract.
- [ ] Public API is self-contained and documented.
- [ ] No platform/vendor/OS dependency enters production Core.
- [ ] No Core-owned background execution or automatic recovery.
- [ ] Test plan covers ownership, lifetime, side effects, failure and determinism.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R06 Component API Review Gate

- [ ] R06-001 through R06-004 accepted.
- [ ] All proposed public contracts are internally consistent.
- [ ] `PlatformContext` remains authoritative for platform access.
- [ ] No hidden lifecycle or ownership path exists.
- [ ] Unit/contract tests and mutation evidence are sufficient.
- [ ] Full regression remains green.
- [ ] Traceability and dependency audits pass.
- [ ] Public-header self-containment passes.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.
- [ ] On PASS, API is FROZEN.

## R06 Integration Freeze

- [ ] R06-005 and R06-006 accepted.
- [ ] Integration tests use public APIs only.
- [ ] Runtime lifecycle ordering and failure/reset semantics remain unchanged.
- [ ] No Core-owned platform-service lifecycle exists.
- [ ] No hidden context ownership exists.
- [ ] No test-only production hook exists.
- [ ] No unresolved production API changes.
- [ ] Production diff from the API freeze point is clean unless explicitly approved.
- [ ] PASS / HONORED / CHANGES REQUIRED / BLOCKED recorded.

## R06 Final Validation Review

- [ ] Integration Freeze is PASS/HONORED.
- [ ] Fresh-clone Debug/Release builds pass.
- [ ] Complete CTest regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict warning build passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage reviewed against baseline.
- [ ] Traceability audit passes.
- [ ] Install consumer passes.
- [ ] Prohibited dependency/isolation audit passes.
- [ ] Public-header self-containment passes.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R06 Release Gate

- [ ] R06-001 through R06-007 accepted.
- [ ] Component API Review PASS/FROZEN.
- [ ] Integration Freeze PASS/HONORED.
- [ ] Fresh-clone validation PASS.
- [ ] Requirements/API/planning documentation reconciled.
- [ ] Version and CMake metadata agree.
- [ ] Release record is documentation-only.
- [ ] Annotated `kritva-core-r0.6` tag is authorized only after PASS.
- [ ] Remote main/tag/peeled tag are independently verified.
- [ ] RELEASED / SYNCHRONIZED / CLOSED recorded.
