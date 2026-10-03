# KF-CORE-R06-007 — Full R0.6 Validation

## Task information

- Task ID: `KF-CORE-R06-007`
- Milestone: `KF-CORE-R06`
- Status: PLANNED
- Estimated effort: 2–3 ED
- Dependency: R06 Integration Freeze PASS/HONORED
- Requirement: `CORE-CTX-007`
- Exact primary commit message: `test(core): complete R0.6 validation`

## Objective


Perform fresh-clone final validation and release-candidate checks for version 0.6.0 without changing the frozen production API.


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
