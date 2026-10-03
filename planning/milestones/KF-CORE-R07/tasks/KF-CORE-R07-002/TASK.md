# KF-CORE-R07-002 — Component Status & Health Reporting Contract

## Status

PLANNED — implementation not started.

## Objective

Formalize ownership and reporting semantics for the existing Status and Health contracts.

## Scope

Define Component authority, snapshot returns, independence of Status/Health, detail semantics and complete independence from Runtime FAULT/recovery. Do not redesign the existing enums.

## Out of Scope

- Runtime lifecycle redesign.
- Core-owned background execution.
- Concrete platform implementation.
- Unrelated API changes.

## Dependencies

See the R0.7 milestone dependency graph and `ACCEPTANCE_CRITERIA.md`.

## Implementation Guidance

Use existing Core contracts where possible. Do not introduce parallel abstractions without explicit architecture review.
