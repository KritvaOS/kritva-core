# KF-CORE-R06-004 — Context Requirements & Capability Binding

## Task information

- Task ID: `KF-CORE-R06-004`
- Milestone: `KF-CORE-R06`
- Status: PLANNED
- Estimated effort: 2–3 ED
- Dependency: R06-002, R06-003
- Requirement: `CORE-CTX-004`
- Exact primary commit message: `feat(core): bind context requirements and capabilities`

## Objective


Bind Component context requirements to the R0.5 requirement/capability identity model without introducing platform-name/version inference or a generic registry.


## Scope

### In scope

- Public/production contract needed for this task.
- Focused unit/contract tests.
- Documentation and traceability updates required by the task.
- No unrelated API changes.

### Out of scope

- Concrete platform implementations.
- Generic service registry/locator.
- Automatic Runtime recovery/retry.
- Core-owned background execution.
- Silent modification of accepted R0.5 semantics.

## Architecture constraints

- R0.5 remains authoritative.
- `IPlatformAdapter` remains the authoritative platform boundary.
- `PlatformContext` remains a non-owning view.
- Runtime lifecycle semantics remain unchanged unless a separately approved API review explicitly changes them.
- No OS/vendor/ROS2/DDS/EtherCAT dependency in production Core.

## Evidence expected

Provide:
- implementation commit SHA;
- changed-file summary;
- focused unit/contract test results;
- complete CTest regression result;
- quality-check results;
- traceability result;
- coverage result;
- mutation results for contract-sensitive behavior;
- explicit out-of-scope confirmation;
- `git diff --check` result;
- clean-tree result.
