# KF-CORE-R08 — Scope Confirmation

## Decision

**APPROVED — IMPLEMENTATION SCOPE CONFIRMED.**

## Objective

Establish a precise, platform-independent Component Configuration Contract around the existing `Component::configure(const Configuration&)` operation, including lifecycle eligibility, ownership, atomic application, validation/error semantics, schema-version semantics, Runtime forwarding, and failure isolation.

## In Scope

- Component configuration lifecycle semantics.
- Configuration ownership and detached-value behavior.
- Atomic/non-partial application contract for conforming Components.
- Core structural validation versus Component semantic validation boundary.
- `ConfigurationVersion` schema/contract version meaning.
- Runtime forwarding semantics for `RuntimeManager::configure()`.
- Configuration failure propagation and state-preservation rules.
- Reference configuration harness and conformance tests.
- Runtime/Component configuration integration tests.
- R0.8 documentation and requirements traceability.
- Full validation and release-candidate preparation.

## Out of Scope

- `reconfigure()` or dynamic runtime configuration.
- `set_parameter()` / `get_parameter()` management APIs on Component.
- Parameter server or configuration broker.
- Configuration persistence/database/file-format framework.
- YAML/JSON/CLI configuration framework.
- Configuration transactions across multiple Components.
- Cross-component rollback.
- Automatic retry or configuration recovery.
- Configuration events/event bus.
- Health-driven behavior or Runtime recovery.
- ComponentContext changes.
- Runtime lifecycle redesign.
- Platform-specific implementations.
- ROS2/DDS/EtherCAT/vendor dependencies.
- Robotics-specific parameter semantics.

## Public API Policy

R08 shall prefer clarification and hardening of existing public APIs. No new public production type is authorized merely for convenience. A proposed public API addition or semantic incompatibility must be explicitly identified and pass the R08 Configuration API Review before implementation continues beyond the pre-review tasks.

## Required Requirement Domain

Reserve the contiguous R08 requirement domain `CORE-CFG-004` through `CORE-CFG-013`. The exact authoritative wording is established only when the corresponding implementation/validation task is accepted and reconciled into root `REQUIREMENTS.md`.

## Architectural Boundary

```text
Caller-owned Configuration
          |
          v
RuntimeManager::configure()
          |
          | dependency order / same input value
          v
Component::configure()
          |
          v
Component-owned applied configuration semantics

Status / Health / Runtime FAULT remain independent.
```

## Implementation Authorization

R08 implementation may begin from `KF-CORE-R08-001` after this Scope Confirmation is recorded. No later task may silently expand the scope above.
