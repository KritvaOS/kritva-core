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
