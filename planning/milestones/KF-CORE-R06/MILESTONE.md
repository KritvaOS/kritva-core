# KF-CORE-R06 — Component Execution Context

## Status

RELEASED (`kritva-core-r0.6` -> `a4c41aa`, published to origin)

## Objective

Provide integrator-written Components with one explicit, deterministic, non-owning context for accessing approved operational services while preserving the R0.3 Runtime lifecycle semantics and the R0.5 platform ownership boundary.

## Architectural Principle

> Context carries access; Runtime retains control.

The Context may expose explicitly approved services and identity/diagnostic data. It must not become a registry, locator, owner, lifecycle supervisor or hidden Runtime dependency.

## Scope

R06 evaluates and, if approved, establishes:

- Component-facing context boundary;
- ownership and lifetime semantics;
- controlled access to operational services;
- requirement/capability binding to the context;
- explicit context injection;
- test/reference context;
- Runtime/Component integration without changing lifecycle ordering, failure, reset or statistics semantics.

## Required gates

- R06 Component API Review after R06-004.
- R06 Integration Freeze after R06-006.
- R06 Release Gate after R06-007.

## Acceptance

A task is accepted only after implementation, focused tests, integration tests where applicable, complete regression, quality checks, objective evidence and independent review.
