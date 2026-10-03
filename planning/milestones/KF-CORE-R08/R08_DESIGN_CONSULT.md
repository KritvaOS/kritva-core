# KF-CORE-R08 — Design Consult

## Decision

APPROVED — Component Configuration Foundation direction confirmed.

## Core Decision

R0.8 shall harden the existing `Configuration`, `ConfigurationVersion`, `Component::configure(const Configuration&)`, and `RuntimeManager::configure(const Configuration&)` contracts into a precise, platform-independent control-plane configuration model. It shall not introduce dynamic reconfiguration or a generic configuration-management framework.

## Architecture Questions and Decisions

### Q1 — What does Configuration mean for a Component?

**Decision:** `Configuration` is a caller-owned, detached control-plane value supplied synchronously to `Component::configure()`. The Component owns the semantics of the accepted/applied configuration. Core owns only the generic representation and contract.

### Q2 — Is configuration mutable after initialization?

**Decision:** No. R0.8 configuration is valid only from `UNKNOWN` and `STOPPED`. No `reconfigure()`, `set_parameter()`, or equivalent API is introduced for `READY` or `RUNNING`.

### Q3 — Who owns configuration?

**Decision:** The caller owns the input object; the Component owns any applied semantic state; Core does not maintain a second authoritative configuration store. A conforming Component shall not retain a reference/pointer to the caller's Configuration beyond the synchronous call.

### Q4 — What does ConfigurationVersion mean?

**Decision:** `ConfigurationVersion` is the schema/contract compatibility version represented by `Version`. It is not a configuration instance revision, update counter, transaction id, or runtime history number. R0.8 does not add a generic configuration revision mechanism. It also does not force a new version field into `Configuration` unless implementation review demonstrates a concrete contract gap.

### Q5 — How do configuration errors behave?

**Decision:** Successful configuration establishes the Component's accepted/applied state. A failed configuration is atomic from the Component contract perspective: no partial applied configuration is permitted, and the lifecycle state remains unchanged. The most specific existing `ErrorCode` is used; R0.8 does not create a per-parameter error-code taxonomy.

### Q6 — Can configuration change while RUNNING?

**Decision:** No. Dynamic runtime reconfiguration is explicitly deferred to a future architecture milestone.

### Q7 — Relationship with Status and Health

**Decision:** Configuration is independent from `Status`, `Health`, and Runtime FAULT. A configuration failure does not automatically change Status, Health, or Runtime lifecycle state.

### Q8 — Relationship with ComponentContext

**Decision:** `ComponentContext` remains unchanged. It is not a configuration registry, parameter store, or configuration authority.

### Q9 — Runtime role

**Decision:** `RuntimeManager` forwards the caller-supplied Configuration to Components in the established dependency order. Runtime does not interpret parameter names/values, retain Component configuration truth, retry, rollback, or change lifecycle state because configuration failed.

### Q10 — Dynamic control / robotics-specific configuration

**Decision:** Parameter servers, persistence, remote configuration, YAML/JSON schema frameworks, robotics-specific parameter semantics, live tuning, and configuration event infrastructure remain outside Core.

## Architectural Invariants

1. No new lifecycle state.
2. `configure()` remains control-plane and synchronous.
3. Valid configuration states remain `UNKNOWN` and `STOPPED`.
4. Failed configuration leaves Component lifecycle state unchanged.
5. Failed configuration cannot partially replace the Component's previously accepted configuration.
6. Runtime configuration failure does not enter Runtime `FAULT`.
7. Configuration never automatically changes Health or Status.
8. `ComponentContext` remains configuration-neutral.
9. No Core-owned configuration registry, broker, worker, persistence or telemetry path.
10. No concrete platform or robotics middleware dependency.
11. Breaking or semantic changes after the R08 API Review return to architecture review.

## Review Outcome

**APPROVED** — suitable for R08 Scope Confirmation.
