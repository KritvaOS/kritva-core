# KF-CORE-R07-007 — Full R0.7 Validation

## Status

PLANNED — implementation not started.

## Objective

Perform the complete final validation of R0.7 after Integration Freeze and establish release-candidate evidence.

## Scope

Fresh-clone validation, Debug/Release, full CTest, sanitizers, strict warnings, analyzer, coverage, traceability, self-containment, install consumer and isolation/dependency audits.

## Out of Scope

- Runtime lifecycle redesign.
- Core-owned background execution.
- Concrete platform implementation.
- Unrelated API changes.

## Dependencies

See the R0.7 milestone dependency graph and `ACCEPTANCE_CRITERIA.md`.

## Implementation Guidance

Use existing Core contracts where possible. Do not introduce parallel abstractions without explicit architecture review.
