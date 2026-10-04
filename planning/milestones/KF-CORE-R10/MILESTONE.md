# KF-CORE-R10 — Core 1.0 API Maturity & Compatibility Foundation

## Status

ACCEPTED — R10-001..009, API / Compatibility Review, Integration Freeze, Security Architecture Review and Release Gate PASS; `kritva-core-r1.0` (1.0.0) pending owner push and remote verification.

## Objective

Establish Kritva Core as a stable 1.x platform-independent foundation by defining and validating the long-term API evolution, source/semantic compatibility, ABI policy, versioning, deprecation, package/install compatibility, migration, documentation and release-contract rules required for `1.0.0` and subsequent 1.x releases.

R1.0 is intentionally not a feature-accumulation milestone. The functional Core foundation established by R0.2–R0.9 is the baseline. R1.0 defines how that foundation may evolve without accidental breakage or platform-specific coupling.

## Core Architecture Principle

> R1.0 defines the compatibility and evolution contract of Kritva Core; it does not introduce a second runtime framework or force platform-specific policy into Core.

## Baseline

R0.9 is the normative pre-1.0 API/semantic baseline:

- version `0.9.0`;
- `kritva-core-r0.9` released and independently verified;
- R0.9 capability/readiness contracts frozen at the R09 production freeze;
- canonical API documentation under `docs/api/`;
- requirements traceability and API documentation audit established;
- install-consumer validation established;
- security architecture and trust-boundary records established;
- nine unchanged API domains remain explicitly documented as `stub` pages and are not silently promoted to maintained pages by R1.0 planning.

R1.0 shall preserve the useful R0.9 controls while defining the missing long-term compatibility policy.

## In Scope

- Inventory of every public installed API under `include/kritva/core/`.
- Classification of public API items as stable, experimental, deprecated, internal or test-only where applicable.
- Source compatibility policy.
- Semantic compatibility policy.
- ABI/binary compatibility decision and supported scope, if any.
- Semantic Versioning and release-impact classification.
- Public enum/error-value compatibility rules.
- Virtual-interface and ownership/lifetime compatibility rules.
- Deprecation and removal policy.
- API change review/evolution process.
- CMake package/version-selection compatibility policy.
- Installed-package and downstream consumer compatibility validation.
- Compatibility regression/reference tests and boundary snapshots where useful.
- Migration guidance for intentional incompatibilities and deprecations.
- Documentation governance updates for the 1.x compatibility contract.
- Security-impact assessment of compatibility/package/release boundaries.
- Requirements traceability for R1.0 policy and validation requirements.
- Full 1.0 validation and release-candidate preparation.

## Explicit Exclusions

- ROS2/DDS integration.
- EtherCAT implementation.
- Linux/PREEMPT_RT implementation.
- STM32/TI/NXP/vendor platform implementations.
- Nexus/Edge implementation.
- Generic dependency injection.
- Service registry/locator/broker.
- Dynamic service discovery.
- Generic automatic readiness calculation.
- Telemetry/logging framework.
- New Runtime scheduler/executor framework.
- Security subsystem, authentication, authorization, cryptographic identity, secure transport or key management.
- Universal cross-toolchain ABI guarantee without an explicitly approved support matrix.
- Retroactive rewriting of R0.9 release evidence.

## Proposed Compatibility Posture

The following is planning guidance and becomes normative only after R10-001 through R10-004 are accepted:

1. Source and semantic compatibility are first-class 1.x contracts.
2. ABI compatibility is not assumed universally; any ABI promise must identify its supported toolchain/build matrix.
3. Removal of a stable public API normally requires a major release.
4. Deprecation requires a replacement or documented rationale and migration guidance.
5. Public enum/error numeric values that are externally observable are treated as compatibility-sensitive.
6. Signature-compatible changes that alter documented behavior are semantic API changes and are reviewed accordingly.
7. CMake package version-selection semantics are explicitly documented and tested rather than inherited accidentally from pre-1.0 behavior.

## Documentation Policy

Canonical compatibility rules shall be maintained as Markdown under `docs/` and linked from the API documentation guidelines and index as appropriate.

R1.0 does not automatically convert the nine R0.9 stub pages into maintained API pages. A stub becomes maintained when its public contract changes or when a dedicated R1.0 documentation task demonstrates that a complete page is required for compatibility classification; that decision must be recorded rather than implied.

Every accepted public API/semantic change continues to require:

- API documentation synchronization;
- requirements traceability;
- security-impact assessment;
- relevant compatibility-test updates.

## Security Policy

R1.0 addresses compatibility/package/release trust boundaries only. It does not introduce a Core security subsystem.

The security review shall consider:

- version/package substitution risks;
- installed-artifact provenance assumptions;
- compatibility claims being mistaken for security guarantees;
- deprecation and migration paths crossing trust boundaries;
- future remote/IPC consumers of Core APIs.

No authentication, authorization, signing, key management or secure-transport mechanism is authorized by this milestone unless separately approved by architecture review.

## Quality Gates

- R10 Design Consult — PASS/APPROVED before Scope Confirmation.
- R10 Scope Confirmation — PASS/APPROVED before R10-001 implementation.
- R10 API/Compatibility Review — PASS/FROZEN before compatibility harness and package integration implementation beyond the approved contract boundary.
- R10 Security Architecture Review — required before R10-008 acceptance.
- R10 Integration Freeze — PASS/HONORED before final documentation/validation and release-candidate preparation.
- R10 Release Gate — PASS before creating `kritva-core-r1.0`.

## Planned Effort

**25–35 engineering-days (estimate)** including architecture, compatibility inventory, policy implementation, tests, package validation, documentation, review/fix, full validation and release audit. Actual effort is recorded only when supported by engineering evidence.

## Release Target

Version `1.0.0`, annotated tag `kritva-core-r1.0`, subject to the final Release Gate.
