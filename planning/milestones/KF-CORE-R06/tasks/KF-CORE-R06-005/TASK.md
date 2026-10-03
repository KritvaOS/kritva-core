# KF-CORE-R06-005 — Reference Context Harness & Contract Tests

## Task information

- Task ID: `KF-CORE-R06-005`
- Milestone: `KF-CORE-R06`
- Status: PLANNED
- Estimated effort: 3–4 ED
- Dependency: R06 Component API Review PASS/FROZEN
- Requirement: `CORE-CTX-005`
- Exact primary commit message: `test(core): add component context reference harness`

## Objective


Create test-only reference context/services and contract tests covering ownership, lifetime, side effects, access policy, requirements and deterministic behavior.


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
