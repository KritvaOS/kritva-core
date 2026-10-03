# KF-CORE-R05 — Platform API Review

## Purpose

Freeze the R0.5 platform runtime integration API before reference-platform implementation.

## Entry Criteria

- R05-001 through R05-004 accepted.
- R0.4 platform API remains unchanged except approved R0.5 additions.
- Unit tests for all four tasks pass.
- Full regression passes.
- Proposed requirements are traceable.
- No unresolved public API ambiguity remains.

## Review Items

- PlatformContext is a view, not an owner.
- No generic ServiceRegistry or service locator exists.
- Service lifetime remains external.
- Required/optional service semantics are explicit.
- Capability identity is authoritative.
- Explicit service consumption is deterministic.
- Runtime lifecycle remains unchanged.
- No implicit service startup/shutdown/recovery exists.
- No Core-owned background execution exists.
- Callback and re-entry rules are documented.
- Error propagation is deterministic.
- Concrete platforms remain outside Core.

## Decision

Possible outcomes:

- PASS / FROZEN
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Freeze Rule

After PASS/FROZEN, breaking public API or semantic changes require architecture re-review before implementation continues.
