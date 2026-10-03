# KF-CORE-R07-003 — Component Operational Event Contract

## Status

PLANNED — implementation not started.

## Objective

Define explicit operational Event reporting to an integrator-owned sink without introducing a Core EventBus or asynchronous infrastructure.

## Scope

Reuse the existing Event envelope. Define source identity, explicit synchronous reporting boundary, sink ownership, delivery/no-buffering/no-retry semantics and event-versus-command separation.

## Out of Scope

- Runtime lifecycle redesign.
- Core-owned background execution.
- Concrete platform implementation.
- Unrelated API changes.

## Dependencies

See the R0.7 milestone dependency graph and `ACCEPTANCE_CRITERIA.md`.

## Implementation Guidance

Use existing Core contracts where possible. Do not introduce parallel abstractions without explicit architecture review.
