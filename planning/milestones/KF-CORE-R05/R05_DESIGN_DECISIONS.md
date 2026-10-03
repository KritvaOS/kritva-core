# KF-CORE-R05 — Design Decisions

## Status

APPROVED FOR TASK PLANNING

## D1 — Platform Boundary

**Decision:** Keep `IPlatformAdapter` as the authoritative platform boundary.

No replacement adapter abstraction is introduced.

## D2 — PlatformContext

**Decision:** Introduce a small non-owning `PlatformContext` view.

It may expose access to the already accepted platform services and capabilities, but:

- owns nothing;
- creates nothing;
- destroys nothing;
- does not register services;
- does not locate services globally;
- does not contain a service registry;
- does not infer capabilities from platform identity.

## D3 — Service Registry

**Decision:** Do not introduce a generic `ServiceRegistry` in R0.5.

Existing abstractions are sufficient:

- `IPlatformAdapter`
- `CapabilitySet`
- `ComponentRegistry`
- `DependencyGraph`
- `RuntimeManager`

## D4 — Service Ownership

**Decision:** Platform service lifecycle remains platform/integrator-owned.

Core does not implicitly start or stop scheduler, timer, clock or watchdog services.

## D5 — Capability Model

**Decision:** Platform requirements are expressed through capability identity rather than platform name/version.

Missing required capability/service is an explicit unsupported/configuration condition according to the operation contract.

## D6 — Component Platform Access

**Decision:** Components do not receive a general raw `IPlatformAdapter*` as their platform interface.

R0.5 first establishes the context and access model. Changes to the Component lifecycle signature are not authorized by this document.

## D7 — Runtime Lifecycle

**Decision:** Preserve the R0.3 Runtime lifecycle exactly.

Platform attachment and service availability do not modify:

- lifecycle states;
- dependency order;
- fail-fast behavior;
- FAULT semantics;
- reset semantics;
- statistics semantics.

## D8 — Background Execution

**Decision:** No Core-owned thread, worker, timer loop, polling loop or asynchronous execution model is introduced.

## D9 — Watchdog

**Decision:** Watchdog expiry never implicitly causes Runtime FAULT, reset or recovery.

## D10 — Concrete Platforms

**Decision:** Concrete Linux, PREEMPT_RT, RTOS, MCU, vendor, Nexus and Edge implementations remain outside `kritva-core`.

## D11 — Testing Architecture

**Decision:** R0.5 integration tests use only test-only reference/fake platform implementations. Physical hardware is not required.

## D12 — API Freeze

**Decision:** After the R05 Platform API Review, public platform-integration API and semantics are frozen. Breaking changes return to architecture review.
