# KF-CORE-R10-008 — Documentation / Security / Traceability Validation

## Objective

Reconcile canonical docs, requirements, security assessment and compatibility evidence for release-candidate preparation.

## Requirements

CORE-SEC-001 and CORE-REL-001, defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (approved at scope alignment; documentation, security records and audit only)

- Security: SD-R10-01..07 in `docs/security/SECURITY_DECISIONS.md`; updates to `TRUST_BOUNDARIES.md`, `THREAT_MODEL.md`, `SECURITY_ARCHITECTURE.md`; `planning/milestones/KF-CORE-R10/R10_SECURITY_REVIEW.md` (`SECURITY IMPACT: DOCUMENTATION ONLY`).
- `scripts/audit/check_security_docs.py` with `--self-test` (`make check`, two CTests): required documents, SD-R10 identifiers unique, contiguous and referenced, exactly one valid classification per accepted R1.0 task record; consistency only.
- Four stub API pages promoted to maintained 15-section pages, restating existing header contracts without new semantics: `ERROR_CODES`, `COMPONENT`, `PLATFORM_ADAPTER`, `RUNTIME`. Decision D-INV-6 recorded in the inventory (the five other stubs and the 29 headers without a page stay header-contract-authoritative; post-1.0 backlog).
- Reconciliation: `API.md` sections 52-55, `docs/README.md`, `docs/api/API_INDEX.md` (15 documents, 10 maintained, 5 stubs), requirements `CORE-SEC-001` and `CORE-REL-001`, `TESTING.md`, `CHANGELOG.md`.

## Exclusions

- No compatibility-semantic change; no header, `src/`, API or CMake package change; no `VERSION` change (R10-009); no ABI mechanism; no authentication, authorization, signing, provenance or security runtime mechanism; no promotion of the other five stubs; no creation of pages for the 29 headers without one.

## Dependencies

R10 Integration Freeze (PASS / HONORED).
