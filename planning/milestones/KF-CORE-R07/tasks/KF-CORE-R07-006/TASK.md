# KF-CORE-R07-006 — Runtime/Component Operational Integration

## Status

PLANNED — implementation not started.

## Objective

Prove through public APIs that Component operational information remains orthogonal to Runtime lifecycle orchestration.

## Scope

Integration tests covering status/health/statistics/event behavior together with initialize/start/stop/shutdown/reset, failure propagation, statistics separation and event sink ownership.

## Out of Scope

- Runtime lifecycle redesign.
- Core-owned background execution.
- Concrete platform implementation.
- Unrelated API changes.

## Dependencies

See the R0.7 milestone dependency graph and `ACCEPTANCE_CRITERIA.md`.

## Implementation Guidance

Use existing Core contracts where possible. Do not introduce parallel abstractions without explicit architecture review.
