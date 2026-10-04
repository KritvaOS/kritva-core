# KF-CORE-R09-002 — CapabilitySet Invariants & Version Semantics

## Status

PLANNED — implementation not started.

## Objective

Define Capability version meaning and CapabilitySet invariants, ownership/snapshot behavior and deterministic observable semantics while avoiding a registry abstraction.

## Scope

- Capability.version describes the provided capability contract/version.
- Version is not runtime availability, health, configuration revision or authorization evidence.
- CapabilitySet lookup is identity-based.
- Returned capability collections/snapshots follow explicit ownership/lifetime rules.
- Observable ordering and duplicate handling are explicitly defined if the existing implementation exposes them.
- No implicit version-range matching is introduced.

## Out of Scope
- Version compatibility/range solver.
- Capability registry/service locator.
- Dynamic capability-change notifications.

## Dependencies

R09-001

## Requirement Traceability

CORE-CAP-005, CORE-CAP-006

## Implementation Guidance

Use the smallest public surface necessary. Reuse accepted R0.8 capability, platform, Runtime and lifecycle contracts. Any proposed public API addition must be explicitly identified, documented, security-reviewed and routed through the R09 Capability API Review.
