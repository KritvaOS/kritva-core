# KF-CORE-R09-001 — Capability Contract & Provider Semantics

## Status

PLANNED — implementation not started.

## Objective

Define the normative generic meaning of Capability and CapabilityId, including provider semantics, identity ownership, descriptive metadata and explicit non-security semantics, without introducing a new public type.

## Scope

- CapabilityId is the authoritative identity of a capability.
- Capability is descriptive provider metadata.
- Name is human-readable metadata and not authoritative matching identity.
- Capability is not a credential, authorization token, lifecycle state or health signal.
- Capability metadata does not imply platform, vendor, OS or hardware semantics.
- Existing Component::capabilities() and platform capability reporting remain the provider mechanisms.

## Out of Scope
- New capability registry or broker.
- Capability authorization or credential semantics.
- Dynamic discovery/eventing.
- Nexus/Edge-specific capability definitions.

## Dependencies

R09 Scope Confirmation

## Requirement Traceability

CORE-CAP-004

## Implementation Guidance

Use the smallest public surface necessary. Reuse accepted R0.8 capability, platform, Runtime and lifecycle contracts. Any proposed public API addition must be explicitly identified, documented, security-reviewed and routed through the R09 Capability API Review.
