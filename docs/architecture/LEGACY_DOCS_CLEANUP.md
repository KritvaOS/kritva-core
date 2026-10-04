# Legacy Documentation Cleanup

The uploaded documentation snapshot contained an R0.2 baseline review, audit CSV, an R0.2-era file manifest, an audit-status record, an architecture-boundaries note and intern work-package guidance.

## Actions

- R0.2 audit artifacts are **relocated** (not deleted) under `architecture/archive/r0.2/` for historical traceability; their previous paths are listed below and their full history remains in Git.
- Current architecture is represented by `PRINCIPLES.md`, `BOUNDARIES.md`, and `DOCUMENTATION_GOVERNANCE.md`.
- API documentation is separated under `api/`, one directory per domain (`api/capability/`, `api/configuration/`, `api/context/`, `api/error/`, `api/lifecycle/`, `api/platform/`, `api/runtime/`).
- Security planning is introduced under `security/`.
- The old R0.2 file manifest is retained as history and is not treated as a current inventory.

## Relocation record (provenance)

| Previous path | Current location | Note |
|---|---|---|
| `docs/architecture/audit-status.md` | `docs/architecture/archive/r0.2/audit-status.md` | traced by `CORE-ARCH-003` at its new path |
| `docs/architecture/baseline-review.md` | `docs/architecture/archive/r0.2/baseline-review.md` | historical, not normative |
| `docs/architecture/baseline-audit.csv` | `docs/architecture/archive/r0.2/baseline-audit.csv` | historical, not normative |
| `docs/architecture/file-manifest.yaml` | `docs/architecture/archive/r0.2/file-manifest.yaml` | R0.2-era inventory, superseded |
| `docs/architecture/boundaries.md` | `docs/architecture/BOUNDARIES.md` | one canonical file; the lower-case name collided with `BOUNDARIES.md` on case-insensitive file systems; `CORE-GEN-001` and `CORE-ARCH-001` retargeted |
| (staged deletion of) `docs/development/intern-work-package.md` | restored in place | still the artifact of `CORE-DEV-001`; whether that requirement belongs in the long-term set is a separate documentation-governance review |

## Follow-up

The current source repository inventory should remain synchronized with the implementation and milestone planning records. A new authoritative file manifest should be generated from the current repository structure rather than edited from the R0.2 snapshot.
