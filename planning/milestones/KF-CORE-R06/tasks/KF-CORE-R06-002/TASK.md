# KF-CORE-R06-002 — Operational Context Services & Access Policy

## Task information

- Task ID: `KF-CORE-R06-002`
- Milestone: `KF-CORE-R06`
- Status: PLANNED
- Estimated effort: 3–4 ED
- Dependency: R06-001
- Requirement: `CORE-CTX-002`
- Exact primary commit message: `feat(core): define operational context access policy`

## Objective


Define which operational information/services may be accessed through the context and the side-effect/error rules for each access path.

Preserve R0.5 explicit-consumption semantics.


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
