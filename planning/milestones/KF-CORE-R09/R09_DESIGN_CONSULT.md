# KF-CORE-R09 — Design Consult

## Decision

**APPROVED — Capability Contract & Readiness Boundary direction confirmed.**

## Core Decision

R0.9 shall strengthen and document the existing Capability, CapabilitySet, requirement and lifecycle boundaries without introducing a new dependency-management or readiness-management framework.

The default implementation posture is **API-neutral**. Existing APIs are preferred. A new public production API is not authorized merely to make the architecture appear more explicit; a concrete, reusable contract gap must be demonstrated first.

## Baseline Finding

The current Core already provides:

- `Capability` with `CapabilityId`, name and `Version`.
- `CapabilitySet` with identity-based lookup.
- `Component::capabilities()` as a Component-provided snapshot.
- `IPlatformAdapter::capabilities()` as a platform-provided snapshot.
- `PlatformRequirements` with REQUIRED/OPTIONAL capability declarations.
- Identity-based capability evaluation through `CapabilitySet::contains()`.
- `DependencyGraph` for Component execution ordering.
- Runtime as lifecycle authority.

R0.9 shall not duplicate these mechanisms with a second dependency or readiness abstraction.

## Architecture Questions and Decisions

### Q1 — What is a Capability?

**Decision:** A Capability is an explicitly identifiable contract describing functionality an entity reports as providing. `CapabilityId` is the authoritative identity.

### Q2 — What does Capability Version mean?

**Decision:** `Capability.version` describes the version of the capability contract being provided. It is not runtime state, availability, health, configuration revision, authorization evidence or a dependency ordering value.

### Q3 — Is a Capability a security credential?

**Decision:** No. `CapabilityId` and `Capability` are descriptive contract metadata, not authentication credentials, authorization tokens or proof of trust.

### Q4 — What does CapabilitySet represent?

**Decision:** CapabilitySet is a value/snapshot collection used for capability declaration and lookup. Its semantics must be deterministic and ownership-safe. R0.9 shall define invariants without introducing a registry or global store.

### Q5 — Should Components declare required capabilities through a new Component API?

**Decision:** Not by default. The existing requirement mechanisms shall be reviewed first. A new Component requirement API requires a demonstrated cross-platform contract gap and explicit API Review approval.

### Q6 — Who evaluates a capability requirement?

**Decision:** The consumer/integrator evaluates whether its declared prerequisite is satisfied. Core may provide deterministic generic checking through existing mechanisms, but Runtime does not automatically resolve or bind services.

### Q7 — Does capability matching imply Component dependency ordering?

**Decision:** No. Capability provision/requirements and Runtime Component dependency ordering remain separate concepts. `DependencyGraph` remains ComponentId-based.

### Q8 — Should missing capabilities automatically prevent `initialize()`?

**Decision:** No generic automatic Runtime rule is introduced. A Component may validate prerequisites during its own lifecycle operation and return an appropriate existing error. Core does not create a new readiness state.

### Q9 — Should Core calculate readiness automatically?

**Decision:** No. Readiness is contextual and Component/integrator-defined. Core shall document the boundary rather than infer readiness from capabilities, Health or platform state.

### Q10 — Should Health determine readiness?

**Decision:** No. Health remains Component-reported operational information. Runtime shall not convert Health state into lifecycle or readiness decisions automatically.

### Q11 — Should capability version ranges be introduced?

**Decision:** No generic version-range matching is introduced in R0.9 unless implementation review demonstrates a concrete Core-wide contract gap. Existing identity-based matching remains the default.

### Q12 — Can capability availability change dynamically?

**Decision:** R0.9 makes no generic dynamic discovery or capability-change event commitment. Future dynamic systems require a separate architecture review.

### Q13 — How does security enter R0.9?

**Decision:** R0.9 performs a security architecture assessment of trust and authority boundaries but does not introduce authentication, authorization, cryptography, credentials or a security framework.

### Q14 — How should API documentation evolve?

**Decision:** Markdown under `docs/api/` is the canonical maintained API documentation. Every accepted public API/semantic change updates the appropriate documentation in the same task. HTML may be generated later and is not authoritative.

## Architectural Invariants

1. Core remains platform-independent.
2. Capability, Requirement, Dependency, Lifecycle, Readiness and Health remain distinct concepts.
3. `CapabilityId` is the authoritative capability identity.
4. Capability metadata is descriptive, not security evidence.
5. Runtime remains lifecycle authority.
6. DependencyGraph remains the Runtime execution-order mechanism.
7. Core does not infer readiness automatically.
8. Health does not drive lifecycle automatically.
9. No generic ServiceRegistry, locator, resolver, broker or dependency-injection framework is introduced.
10. No new lifecycle state is introduced by R0.9.
11. Existing mechanisms are preferred over new production APIs.
12. Any new production public API requires explicit Capability API Review approval.
13. API documentation is maintained in Markdown and synchronized with accepted API/semantic changes.
14. Security impact is assessed explicitly without forcing a security subsystem into Core.
15. Nexus/Edge/platform-specific semantics remain outside Core.

## Review Outcome

**APPROVED — suitable for R09 Scope Confirmation.**
