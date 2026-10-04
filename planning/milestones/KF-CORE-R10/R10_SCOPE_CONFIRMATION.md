# KF-CORE-R10 — Scope Confirmation

## Decision

**APPROVED — implementation scope confirmed for R1.0 planning.**

## Objective

Convert the R0.9 functional foundation into an explicitly governed Core 1.x compatibility and evolution contract, with automated validation of the public API and installed package.

## In Scope

- Complete public API inventory from the R0.9 baseline.
- Public API stability classification.
- Source compatibility rules.
- Semantic compatibility rules.
- ABI/binary compatibility policy and support matrix decision.
- SemVer release-impact classification.
- Enum/ErrorCode/virtual-interface/ownership compatibility rules.
- Deprecation and removal lifecycle.
- API change proposal/review process.
- Package/version-selection semantics.
- Installed downstream consumer compatibility checks.
- Compatibility/boundary/reference tests.
- Migration guidance.
- Documentation governance and API-documentation updates.
- Security-impact assessment.
- Requirements traceability.
- Full validation and release-candidate preparation.

## Out of Scope

- New runtime architecture.
- New dependency/readiness/discovery framework.
- Concrete platform implementation.
- Robotics middleware integration.
- Security enforcement subsystem.
- Universal ABI promise across arbitrary C++ toolchains.
- Retroactive edits to R0.9 release evidence.

## Public API Policy

R1.0 is authorized to classify and document the existing public API and to add narrowly scoped enforcement/testing/package mechanisms required to make the approved compatibility contract verifiable.

No new functional public Core API is authorized merely for policy convenience. Any new public production symbol requires explicit R10 API/Compatibility Review approval.

## Requirement Domain

Reserve a new `CORE-COMPAT-*` / `CORE-REL-*` requirement family for R1.0. Exact IDs and wording become authoritative only after task acceptance and root `REQUIREMENTS.md` reconciliation.

## Documentation Boundary

The R0.9 stub-page policy remains active. A page changes from `stub` to `maintained` only through an explicit R1.0 documentation/compatibility decision or because its public contract changes during an accepted task.

## Implementation Authorization

This scope confirmation authorizes preparation of task packages. It does not authorize R10-001 implementation until its task-specific `TASK.md` and `ACCEPTANCE_CRITERIA.md` are reviewed.
