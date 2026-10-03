# KF-CORE-R08-002 — Configuration Ownership & Atomic Application

## Status

PLANNED — implementation not started.

## Objective

Define input ownership, lifetime detachment and atomic/non-partial application semantics for conforming Components. Tests must be able to detect reference retention and partial application in deliberately broken implementations. Do not add a generic Component configuration accessor.

## Scope

- Implement only the configuration behavior explicitly assigned to this task.
- Preserve R0.2–R0.7 contracts unless a demonstrated R08 gap is within scope.
- Keep Core platform independent and synchronous.

## Out of Scope

- Dynamic runtime reconfiguration.
- Parameter server or configuration broker.
- Configuration persistence or file-format framework.
- Configuration event bus, callbacks or background worker.
- ComponentContext redesign.
- Runtime lifecycle redesign.
- Platform/vendor/ROS2/DDS/EtherCAT implementation.

## Dependencies

R08-001

## Requirement Traceability

CORE-CFG-005, CORE-CFG-006

## Implementation Guidance

Use the smallest public surface necessary. Reuse existing `Configuration`, `ConfigurationVersion`, `Component` and `RuntimeManager` contracts. Any proposed public API addition must be explicitly called out for the R08 Configuration API Review.
