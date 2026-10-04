# Security Decisions

## R0.9 baseline

1. Security impact must be assessed for public API changes.
2. Capability identity is not a security credential.
3. Logical Component identity is not authenticated identity.
4. Core does not infer authorization from lifecycle, health, or capability claims.
5. New security mechanisms require explicit architecture justification.

## R0.9 capability, requirement and readiness decisions

Recorded at R09-006 (see `planning/milestones/KF-CORE-R09/R09_SECURITY_REVIEW.md`; classification **SECURITY IMPACT: DOCUMENTATION ONLY**).

- **SD-R09-01** A `Capability` and its `CapabilityId` are descriptive contract metadata, never a credential, token, proof of trust or evidence (`docs/api/capability/CAPABILITY.md`).
- **SD-R09-02** Capability matching is identity-only; a satisfied requirement shows only that a provider *declared* an id and is never an authorization decision (`docs/api/capability/CAPABILITY_REQUIREMENTS.md`, `docs/api/platform/PLATFORM_REQUIREMENTS.md`).
- **SD-R09-03** Capability names, versions and provider information are untrusted descriptive metadata; Core never uses them for a decision.
- **SD-R09-04** An entry with an invalid `CapabilityId` is storable data, not an authoritative declaration, and can never satisfy a requirement; providers must not publish one.
- **SD-R09-05** Core calculates no readiness and infers nothing from lifecycle, health or capability claims about authorization or trust (`docs/api/lifecycle/LIFECYCLE.md`).
- **SD-R09-06** No discovery, change notification, background execution, retry or recovery exists around capabilities; a Component decides for itself whether a prerequisite is sufficient for one of its own lifecycle operations.
- **SD-R09-07** No security mechanism (authentication, authorization, capability tokens, cryptographic identity, key management, secure transport) is introduced; each requires a separate architecture decision.

## R1.0 compatibility, package and release decisions

Recorded at R10-008 (see `planning/milestones/KF-CORE-R10/R10_SECURITY_REVIEW.md`; classification **SECURITY IMPACT: DOCUMENTATION ONLY**; requirement `CORE-SEC-001`).

- **SD-R10-01** Compatibility is not authenticity or provenance. A compatible version, a stable classification or a satisfied package request says nothing about who produced an artifact or whether it is trustworthy (`docs/compatibility/COMPATIBILITY_POLICY.md`, `docs/compatibility/VERSIONING_POLICY.md`).
- **SD-R10-02** Package version selection is not a trust decision. `find_package(kritva_core ...)` accepts whichever installed package satisfies the version rule; which package is found, and that it is the intended one, is controlled by the integrator through the package search path and artifact provenance, not by Core (`docs/compatibility/VERSIONING_POLICY.md`).
- **SD-R10-03** A `stable` or `Compatible` classification is not a safety or security guarantee. Semantic compatibility preserves documented behavior, including documented limitations; a security or safety defect in a stable item is corrected through the correction rule or the architecture-reviewed emergency exception of `docs/compatibility/VERSIONING_POLICY.md` and is recorded with its justification.
- **SD-R10-04** Deprecation and migration never relax validation, ownership or authority boundaries. A deprecated item keeps its security properties until removal, and a replacement must not weaken them (`docs/compatibility/DEPRECATION_POLICY.md`).
- **SD-R10-05** No signing, provenance, secure-transport, key-management or credential mechanism is introduced in Core 1.0; each requires a separate architecture decision.
- **SD-R10-06** The absence of an ABI promise (`docs/compatibility/ABI_POLICY.md`) makes mixing binaries unsupported and unverified; it is a compatibility statement, not a security boundary.
- **SD-R10-07** A stable C++ API is not a wire, IPC or serialization contract. Nothing in Core 1.x implies a stable or authenticated representation across a process, network or persistence boundary.
