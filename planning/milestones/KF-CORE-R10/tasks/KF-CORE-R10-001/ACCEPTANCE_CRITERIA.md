# KF-CORE-R10-001 — Acceptance Criteria

## Primary Commit

`docs(core): inventory and classify R1.0 public API`

## Acceptance Criteria

1. Every installed/public header under `include/kritva/core/` is inventoried.
2. Every externally visible API element relevant to compatibility is classified.
3. Stable/experimental/deprecated/internal/test-only distinctions are explicit.
4. Ownership/lifetime, thread-safety and semantic compatibility sensitivity are recorded where applicable.
5. API documentation/stub status conflicts are identified and reconciled through explicit decisions.
6. No production behavior or API signature is changed by this task unless separately approved.
7. Traceability requirements are defined/reconciled for the inventory artifact.
8. Documentation reflects the accepted inventory.
9. Security impact is classified.
10. Full task validation passes and no unexplained regression is introduced.

## Security Impact

Record exactly one approved classification.

## Documentation Acceptance

Inventory/classification documentation is canonical and contradicts no public header or accepted contract.

## Git Acceptance

Exact primary commit message above; accepted task commit is not amended.
