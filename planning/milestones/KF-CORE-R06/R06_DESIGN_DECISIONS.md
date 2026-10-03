# KF-CORE-R06 — Design Decisions

## D01 — R0.5 remains authoritative

`IPlatformAdapter` and `PlatformContext` remain the platform boundaries. R06 must not replace them.

## D02 — Context is not a registry

No generic string/name lookup, dynamic service map, singleton, global context or service locator.

## D03 — Non-owning by default

Core does not own platform services or integrator resources through the context.

## D04 — Explicit access only

A context query must not implicitly start, stop, create, destroy, configure or recover any service.

## D05 — Runtime lifecycle remains unchanged

The Component context must not cause RuntimeManager to start using platform services implicitly. R0.3 lifecycle ordering, failure propagation, reset and statistics remain authoritative.

## D06 — Public API only at integration tests

Integration tests must prove the contract without accessing private members or adding test-only production hooks.

## D07 — API freeze before implementation integration

R06-001 through R06-004 must converge before the Component API Review. No implementation integration work proceeds on an unfrozen public contract.

## D08 — Requirement identity remains authoritative

R0.5 `PlatformRequirements`/capability identity semantics are reused; no platform name/version inference is permitted.
