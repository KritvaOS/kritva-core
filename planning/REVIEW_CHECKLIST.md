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

## R07 Component Operational Foundation Review

Apply during R07 architecture review and before R07-001 implementation authorization:

- [ ] R0.6 ComponentContext and R0.3 Runtime lifecycle boundaries remain authoritative.
- [ ] No new Component operational state machine is introduced.
- [ ] `Status`, `Health`, `Statistics` and `Event` remain distinct concepts.
- [ ] Component remains authoritative for its operational information.
- [ ] Observation is read-only and side-effect free.
- [ ] Snapshot/value semantics are explicit.
- [ ] No cross-property atomic snapshot claim is introduced.
- [ ] Component statistics remain optional unless a later reviewed contract says otherwise.
- [ ] No Core EventBus, queue, broker or dispatcher is introduced.
- [ ] Event reporting is explicit and integrator-owned at the sink boundary.
- [ ] Events do not implicitly trigger lifecycle, recovery or retry behavior.
- [ ] Runtime does not automatically poll or interpret Component operational data.
- [ ] Runtime-owned statistics remain separate from Component statistics.
- [ ] No generic Diagnostic container is introduced without a separately demonstrated contract gap.
- [ ] No Core-owned worker/thread/background operation is introduced.
- [ ] No telemetry/logging backend enters production Core.
- [ ] No platform/vendor/OS dependency enters production Core.
- [ ] Any production API change is explicitly identified before implementation.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R07 Component Operational API Review Gate

- [ ] R07-001 through R07-004 accepted.
- [ ] Existing `Component` lifecycle signatures remain unchanged unless explicitly approved.
- [ ] Any new operational API is minimal, self-contained and traceable.
- [ ] Status/Health semantics are frozen.
- [ ] Optional Statistics semantics are frozen.
- [ ] Event reporting/sink ownership semantics are frozen.
- [ ] No hidden Runtime observation or control path exists.
- [ ] Contract/unit/mutation evidence is sufficient.
- [ ] Full pre-R07 regression remains green.
- [ ] Public-header self-containment passes.
- [ ] Traceability and dependency audits pass.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.
- [ ] On PASS, production operational API is FROZEN.

## R07 Integration Freeze

- [ ] R07-005 and R07-006 accepted.
- [ ] Integration tests use public APIs only.
- [ ] Runtime lifecycle ordering/failure/reset behavior is unchanged.
- [ ] Operational observation causes no lifecycle side effects.
- [ ] Health/status/statistics changes do not automatically change Runtime state.
- [ ] Events do not trigger recovery or lifecycle operations.
- [ ] No Core-owned event queue, worker or telemetry path exists.
- [ ] No test-only production hook exists.
- [ ] Production diff from the API freeze point contains only approved validation/documentation changes.
- [ ] PASS / HONORED / CHANGES REQUIRED / BLOCKED recorded.

## R07 Final Validation Review

- [ ] R07 Integration Freeze is PASS/HONORED.
- [ ] Fresh-clone Debug/Release builds pass.
- [ ] Complete CTest regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict warning build passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage reviewed against accepted baseline.
- [ ] Traceability audit passes.
- [ ] Install consumer passes.
- [ ] Production isolation/dependency audit passes.
- [ ] Public-header self-containment passes.
- [ ] No prohibited event/telemetry/threading infrastructure is present.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R07 Release Gate

- [ ] R07-001 through R07-007 accepted.
- [ ] R07 Component Operational API Review PASS/FROZEN.
- [ ] R07 Integration Freeze PASS/HONORED.
- [ ] Fresh-clone validation PASS.
- [ ] Requirements/API/planning documentation reconciled.
- [ ] Version and CMake metadata agree.
- [ ] Release record is documentation-only.
- [ ] Annotated `kritva-core-r0.7` tag is authorized only after PASS.
- [ ] Remote main/tag/peeled tag independently verified.
- [ ] RELEASED / SYNCHRONIZED / CLOSED recorded.


## R08 Component Configuration Foundation Review

Apply during R08 architecture review and task acceptance:

- [ ] Existing `Configuration`, `ConfigurationVersion`, `Component::configure()` and `RuntimeManager::configure()` concepts are reused rather than duplicated.
- [ ] `configure()` is valid only from `UNKNOWN` and `STOPPED` for R0.8.
- [ ] No new lifecycle state or dynamic reconfiguration API exists.
- [ ] Successful configuration leaves lifecycle state unchanged.
- [ ] Failed configuration leaves lifecycle state unchanged.
- [ ] Configuration input is caller-owned and detached from the Component after the call.
- [ ] Partial configuration application on failure is prohibited and tested.
- [ ] Component owns accepted/applied semantic configuration; Core holds no mirrored truth.
- [ ] Core structural validation and Component semantic validation remain separate.
- [ ] `ConfigurationVersion` means schema/contract compatibility version only.
- [ ] No generic runtime configuration revision/history API is introduced.
- [ ] Runtime forwards configuration deterministically and does not interpret parameter meaning.
- [ ] Runtime does not retry, rollback or enter FAULT because configuration failed.
- [ ] Status and Health remain independent from configuration failure.
- [ ] ComponentContext remains unchanged.
- [ ] No configuration broker, persistence layer, event bus, background worker or remote-control subsystem is added.
- [ ] No platform/vendor/OS/ROS2/DDS/EtherCAT dependency enters production Core.
- [ ] Mutation testing covers contract-sensitive configuration behavior.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R08 Configuration API Review Gate

- [ ] R08-001 through R08-003 accepted.
- [ ] Configuration lifecycle semantics are frozen.
- [ ] Ownership/detachment semantics are frozen.
- [ ] Atomic failure/non-partial application semantics are frozen.
- [ ] Validation and version semantics are frozen.
- [ ] Runtime configuration boundary is frozen.
- [ ] No dynamic reconfiguration API is present.
- [ ] Public-header self-containment passes.
- [ ] Focused contract/mutation evidence is sufficient.
- [ ] Existing regression suite remains green.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.
- [ ] On PASS, production configuration API/semantics are FROZEN.

## R08 Integration Freeze

- [ ] R08-004 and R08-005 accepted.
- [ ] Runtime configuration forwarding and failure semantics are validated with public APIs.
- [ ] Configuration causes no Runtime lifecycle transition.
- [ ] Configuration failure causes no automatic rollback/retry/recovery.
- [ ] Configuration does not automatically drive Status or Health.
- [ ] ComponentContext remains unchanged.
- [ ] No configuration service/event/persistence infrastructure exists in production Core.
- [ ] Deterministic ordering remains invariant under registration/dependency permutations.
- [ ] Production diff from the API Review freeze point contains no unapproved semantic changes.
- [ ] PASS / HONORED / CHANGES REQUIRED / BLOCKED recorded.

## R08 Final Validation Review

- [ ] R08 Integration Freeze is PASS/HONORED.
- [ ] Fresh-clone Debug/Release builds pass.
- [ ] Complete CTest regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict warning build passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage meets the R08 policy.
- [ ] Traceability audit passes.
- [ ] Install consumer passes.
- [ ] Production isolation/dependency audit passes.
- [ ] Public-header self-containment passes.
- [ ] No dynamic configuration/parameter infrastructure has leaked into Core.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R08 Release Gate

- [ ] R08-001 through R08-007 accepted.
- [ ] R08 Configuration API Review PASS/FROZEN.
- [ ] R08 Integration Freeze PASS/HONORED.
- [ ] Fresh-clone validation PASS.
- [ ] Requirements/API/planning documentation reconciled.
- [ ] Version and CMake metadata agree at 0.8.0.
- [ ] Release record is documentation-only.
- [ ] Annotated `kritva-core-r0.8` tag is authorized only after PASS.
- [ ] Remote `main`, tag object and peeled tag are independently verified.
- [ ] RELEASED / SYNCHRONIZED / CLOSED recorded.


## Permanent Documentation Synchronization Gate

Apply to every future milestone and every task that changes or clarifies a contract:

### Per Task
- [ ] Documentation impact is explicitly assessed.
- [ ] Affected `docs/api/` documents are updated in the same task when the public API or semantics change.
- [ ] Affected architecture documentation is updated when architecture or boundary semantics change.
- [ ] Requirements/traceability documentation is updated where applicable.
- [ ] Security documentation is updated where trust, authority, ownership or security assumptions change.
- [ ] Examples/guides are updated where applicable.
- [ ] If documentation impact is none, the task evidence explicitly records `Documentation impact: none`.
- [ ] No known documentation drift remains at task acceptance.

### Milestone Release Gate
- [ ] Complete documentation reconciliation is performed before Release Gate PASS.
- [ ] Public headers and `docs/api/` are consistent.
- [ ] Requirements and implementation are consistent.
- [ ] Architecture decisions and implementation are consistent.
- [ ] Security assumptions/boundaries and implementation are consistent.
- [ ] No stale normative documentation remains.
- [ ] No orphaned normative documentation remains.
- [ ] Documentation indexes/navigation are complete.
- [ ] Documentation reconciliation evidence is recorded.

## R09 Capability Contract & Readiness Boundary Review

Apply during R09 architecture review and task acceptance:

- [ ] Existing Capability, CapabilitySet, PlatformRequirements, DependencyGraph and lifecycle mechanisms are reused rather than duplicated.
- [ ] `CapabilityId` is the authoritative capability identity.
- [ ] Capability version meaning is descriptive contract version only.
- [ ] Capability metadata is not treated as security evidence.
- [ ] Capability requirements remain distinct from ComponentId dependency ordering.
- [ ] Capability matching does not infer name/platform/vendor semantics.
- [ ] No generic version-range solver is introduced without explicit architecture approval.
- [ ] No automatic readiness calculation or new readiness lifecycle state is introduced.
- [ ] Health remains independent from lifecycle/readiness decisions.
- [ ] No ServiceRegistry/locator/resolver/dependency-injection framework exists in production Core.
- [ ] Core remains platform independent.
- [ ] API documentation is maintained in canonical Markdown under `docs/api/`.
- [ ] Every accepted public API/semantic change updates documentation in the same task.
- [ ] Security trust/authority boundaries are documented.
- [ ] Security impact is assessed for affected changes.
- [ ] Mutation testing covers contract-sensitive behavior.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R09 Capability API Review Gate

- [ ] R09-001 through R09-003 accepted.
- [ ] Capability identity/version semantics are frozen.
- [ ] CapabilitySet invariants are frozen.
- [ ] Requirement/matching boundary is frozen.
- [ ] No unauthorized production API has been added.
- [ ] Public API documentation is synchronized.
- [ ] Security impact assessments are present.
- [ ] Public-header self-containment passes.
- [ ] Focused contract/mutation evidence is sufficient.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.
- [ ] On PASS, production capability API/semantics are FROZEN.

## R09 Integration Freeze

- [ ] R09-004 and R09-005 accepted.
- [ ] No automatic dependency resolution exists.
- [ ] No readiness state machine exists.
- [ ] Capability matching does not alter DependencyGraph ordering.
- [ ] Health does not automatically drive lifecycle.
- [ ] Runtime lifecycle behavior remains compatible with R0.8.
- [ ] PlatformRequirements/PlatformContext boundaries remain compatible.
- [ ] No Core-owned background/retry/recovery infrastructure exists.
- [ ] API documentation remains synchronized.
- [ ] Security boundary remains unchanged or explicitly reviewed.
- [ ] Production diff from the API Review freeze point contains no unapproved semantic changes.
- [ ] PASS / HONORED / CHANGES REQUIRED / BLOCKED recorded.

## R09 Final Validation Review

- [ ] R09 Integration Freeze is PASS/HONORED.
- [ ] Fresh-clone Debug/Release builds pass.
- [ ] Complete CTest regression passes.
- [ ] ASan/UBSan pass.
- [ ] TSan passes where configured.
- [ ] Strict warning build passes.
- [ ] GCC `-fanalyzer` passes.
- [ ] Coverage meets the R09 policy.
- [ ] Traceability audit passes.
- [ ] Install consumer passes.
- [ ] API documentation consistency audit passes.
- [ ] Security architecture/impact review passes.
- [ ] Production isolation/dependency audit passes.
- [ ] Public-header self-containment passes.
- [ ] No dependency/readiness framework leaked into production Core.
- [ ] Working tree is clean.
- [ ] PASS / CHANGES REQUIRED / BLOCKED recorded.

## R09 Release Gate

- [ ] R09-001 through R09-007 accepted.
- [ ] R09 Capability API Review PASS/FROZEN.
- [ ] R09 Integration Freeze PASS/HONORED.
- [ ] Fresh-clone validation PASS.
- [ ] API documentation and security planning reconciled.
- [ ] Requirements/API/planning documentation reconciled.
- [ ] Version and CMake metadata agree at 0.9.0.
- [ ] Release record is documentation-only.
- [ ] Annotated `kritva-core-r0.9` tag authorized only after PASS.
- [ ] Remote `main`, tag object and peeled tag independently verified.
- [ ] RELEASED / SYNCHRONIZED / CLOSED recorded.
