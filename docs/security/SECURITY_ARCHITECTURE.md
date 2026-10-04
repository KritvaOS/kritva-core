# Security Architecture

## Purpose

Establish security assumptions and trust boundaries without introducing a security subsystem into Core.

## Current assumptions

- Component and platform capability claims are trusted architectural inputs unless a higher layer establishes authentication or authorization.
- `CapabilityId` is an identifier, not a credential.
- `ComponentId` is a logical identity, not proof of authenticated origin.
- Configuration is functionally validated by Core/Component contracts; authenticity and authorization are outside current R0.8 scope.
- Core does not provide process isolation or a security enforcement boundary.
- Security-impact assessment is required for new public APIs and trust-boundary changes.

## Out of scope

Authentication, authorization frameworks, key management, secure boot, TLS, cryptographic credential handling, and security daemons are not introduced by R0.9 absent explicit requirements.

## R0.9 security-impact classification

R0.9 (Capability Contract & Readiness Boundary) is classified **SECURITY IMPACT: DOCUMENTATION ONLY**: no API, authority boundary, persistence, discovery or execution path was added; the trust assumptions it relies on are recorded in `SECURITY_DECISIONS.md` (SD-R09-01..07) and `TRUST_BOUNDARIES.md`.

## R1.0 security-impact classification

R1.0 (Core 1.0 API Maturity & Compatibility Foundation) is classified **SECURITY IMPACT: DOCUMENTATION ONLY**. It adds no API, authority boundary, persistence, discovery or execution path and introduces no security mechanism. The only production change is the package version-selection mode, which selects a package by version and makes no authenticity statement. The assumptions it relies on are recorded in `SECURITY_DECISIONS.md` (SD-R10-01..07), `TRUST_BOUNDARIES.md` and `THREAT_MODEL.md`; the review is `planning/milestones/KF-CORE-R10/R10_SECURITY_REVIEW.md` (`CORE-SEC-001`). Signing, provenance, secure transport, key management and credential handling remain out of scope and each requires a separate architecture decision.
