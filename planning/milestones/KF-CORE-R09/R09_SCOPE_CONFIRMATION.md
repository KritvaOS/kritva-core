# KF-CORE-R09 — Scope Confirmation

## Decision

**APPROVED — IMPLEMENTATION SCOPE CONFIRMED.**

## Objective

Establish a precise, generic Capability Contract and Readiness Boundary using existing Core mechanisms while avoiding duplicate dependency-management abstractions and platform-specific coupling.

## In Scope

- Capability identity semantics.
- Capability provider semantics.
- Capability version semantics.
- CapabilitySet invariants, snapshot/ownership semantics and deterministic behavior where applicable.
- Existing capability requirement and matching semantics.
- Clear separation between capability requirements and Runtime Component dependency ordering.
- Clear separation between capability availability and lifecycle/readiness.
- Explicit Component/integrator responsibility for prerequisite validation.
- API documentation foundation under `docs/api/` using Markdown.
- Security architecture assessment of trust and authority boundaries.
- Reference capability/requirement harness and negative/mutation tests.
- Runtime/Component integration tests proving no automatic readiness or dependency inference.
- Boundary/regression validation and release-candidate preparation.

## Out of Scope

- Generic dependency injection.
- Service registry/locator/broker.
- Dynamic discovery.
- Capability eventing.
- Capability credentials/tokens.
- Authentication/authorization implementation.
- Cryptography/key management.
- New lifecycle states.
- Automatic readiness calculation.
- Health-to-lifecycle coupling.
- Automatic retry/recovery.
- Runtime lifecycle redesign.
- Concrete platform implementations.
- Nexus/Edge-specific contracts.
- Linux/RTOS/MCU/EtherCAT/ROS2/DDS/vendor semantics.

## Public API Policy

R09 shall prefer clarification and hardening of existing public APIs. No new production type is authorized merely for convenience. If implementation identifies a genuine API gap, the proposed surface must be recorded, justified against the generic Core architecture, documented, security-reviewed, tested and approved by the R09 Capability API Review before implementation continues beyond the pre-review task boundary.

## Requirement Domain

Reserve `CORE-CAP-004` through `CORE-CAP-011` for R0.9. The authoritative wording becomes effective only when the associated evidence is accepted and the root `REQUIREMENTS.md` traceability table is reconciled.

## Architectural Boundary

```text
Capability provided by entity
          |
          v
      CapabilitySet
          |
          +---- explicit requirement/matching ----+
          |                                       |
          v                                       v
  Component readiness                         Platform use
          |
          v
 Component lifecycle operation
          |
          v
 Runtime lifecycle authority

DependencyGraph remains a separate ComponentId-based execution-order mechanism.
Health remains separate operational information.
```

## Implementation Authorization

R09 implementation may begin with `KF-CORE-R09-001` after this Scope Confirmation is recorded. No later task may silently expand the scope above.
