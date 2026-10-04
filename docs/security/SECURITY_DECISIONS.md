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
