# KF-CORE-R04-005 — Platform Capability & Adapter Contract

## Task Information

- **Task ID:** `KF-CORE-R04-005`
- **Title:** Platform Capability & Adapter Contract
- **Primary Area:** `platform/capability`
- **Requirement:** `CORE-PLAT-008`
- **Status:** PLANNED
- **Dependency:** R04 Platform API Review PASS/FROZEN
- **Exact primary commit:** `feat(core): define platform capability contract`

## Objective

Define minimal platform identity/capability reporting so higher layers can discover adapter capabilities without embedding platform implementation into Core.

## Scope

['platform identity', 'version', 'capability identities', 'supported/unsupported reporting', 'adapter metadata', 'non-owning reporting']

## Out of Scope

['large hardware inventory API', 'driver enumeration', 'automatic feature activation', 'platform singleton', 'vendor-specific capability types']

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
