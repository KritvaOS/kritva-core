# KF-CORE-R06-006 — Runtime/Component Context Integration Tests

## Task information

- Task ID: `KF-CORE-R06-006`
- Milestone: `KF-CORE-R06`
- Status: PLANNED
- Estimated effort: 3–4 ED
- Dependency: R06-005
- Requirement: `CORE-CTX-006`
- Exact primary commit message: `test(core): add component context integration tests`

## Objective


Prove Context + Component + Runtime integration through public APIs while preserving Runtime lifecycle ordering, failure propagation, reset semantics, statistics and R0.5 platform isolation.


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
