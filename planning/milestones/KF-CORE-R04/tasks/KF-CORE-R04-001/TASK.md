# KF-CORE-R04-001 — Platform Adapter Boundary & Context

## Task Information

- **Task ID:** `KF-CORE-R04-001`
- **Title:** Platform Adapter Boundary & Context
- **Primary Area:** `platform/boundary`
- **Requirement:** `CORE-PLAT-004`
- **Status:** PLANNED
- **Dependency:** R0.3 released
- **Exact primary commit:** `feat(core): define platform adapter boundary`

## Objective

Define the stable boundary between platform-independent Core contracts and external platform adapter implementations.

## Scope

['adapter interface/boundary semantics', 'ownership and lifetime rules', 'opaque context semantics', 'error propagation', 'platform-independence constraints', 'public-header contract tests']

## Out of Scope

['Linux/POSIX implementation', 'RTOS implementation', 'vendor BSP/HAL', 'hardware drivers', 'platform singleton', 'background execution']

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
