# KF-CORE-R04-002 — Scheduler Contract Hardening

## Task Information

- **Task ID:** `KF-CORE-R04-002`
- **Title:** Scheduler Contract Hardening
- **Primary Area:** `platform/scheduler`
- **Requirement:** `CORE-PLAT-005`
- **Status:** PLANNED
- **Dependency:** R04-001 accepted
- **Exact primary commit:** `feat(core): harden scheduler platform contract`

## Objective

Harden CORE-PLAT-001 into a precise scheduler adapter contract without implementing a scheduler.

## Scope

['TaskId', 'TaskConfig', 'create_task semantics', 'start/stop', 'dynamic creation', 'affinity/priority policy', 'period semantics', 'context/entry lifetime', 'resource failure', 'teardown', 'thread-safety and RT boundary']

## Out of Scope

['Linux scheduler', 'RTOS scheduler', 'thread pool', 'hard RT guarantee', 'scheduler-owned Runtime recovery']

## Architecture Rules

1. `kritva-core` remains platform independent.
2. Existing accepted R0.3 contracts remain authoritative.
3. Platform-specific implementation belongs outside Core.
4. Any breaking public API change requires architecture review.
5. No production thread/background execution is introduced unless explicitly accepted by a later architecture decision.

## Deliverables

- Implementation or contract documentation required by the task.
- Required tests.
- Updated requirement traceability.
- Evidence suitable for independent review.
- Focused Git commit using the exact message above.
