# KF-CORE-R04-006 — Platform Conformance Tests

## Task Information

- **Task ID:** `KF-CORE-R04-006`
- **Title:** Platform Conformance Tests
- **Primary Area:** `tests/platform`
- **Requirement:** `CORE-PLAT-009`
- **Status:** PLANNED
- **Dependency:** R04-005 accepted
- **Exact primary commit:** `test(core): add platform conformance suite`

## Objective

Provide reusable contract tests for future platform adapters using public APIs and no physical hardware.

## Scope

['scheduler conformance', 'clock conformance', 'timer conformance', 'watchdog conformance', 'adapter-defined behavior separation', 'test-only fakes', 'mutation testing']

## Out of Scope

['real Linux adapter', 'real RTOS adapter', 'hardware CI requirement', 'platform implementation']

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
