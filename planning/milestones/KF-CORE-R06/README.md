# KF-CORE-R06 — Component Execution Context

## Status

PLANNED — Architecture Proposal

## Target

Version: 0.6.0  
Tag: `kritva-core-r0.6`

## Objective

Establish a controlled, explicit execution context for integrator-written Components without changing the frozen R0.3 Runtime lifecycle contract or the frozen R0.5 platform integration boundary.

## Non-goals

- No concrete Linux/RTOS/MCU/vendor/Nexus/Edge implementation in `kritva-core`.
- No Runtime-owned worker thread, event loop, scheduler loop or automatic recovery.
- No replacement of `PlatformContext` or `IPlatformAdapter`.
- No generic ServiceRegistry or service locator.
- No raw `IPlatformAdapter*` exposure as a general Component interface.
- No breaking change to the existing Component lifecycle signatures unless explicitly approved by API review.
- No new ownership of platform services by Core.
