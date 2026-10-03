# KF-CORE-R04 — Implementation Sequence

## Principle

R0.4 proceeds from platform-boundary contracts to platform-service contracts, then conformance, then Runtime integration. API freeze points prevent platform work from changing accepted R0.3 runtime semantics.

```text
R04-001 Platform Adapter Boundary & Context
        ↓
R04-002 Scheduler Contract Hardening
R04-003 Clock & Timer Contract
R04-004 Watchdog Contract
        ↓
R04 Platform API Review
        ↓
R04-005 Platform Capability & Adapter Contract
        ↓
R04-006 Platform Conformance Tests
        ↓
R04 Platform Integration Freeze
        ↓
R04-007 Runtime–Platform Integration Boundary
        ↓
R04-008 Full R0.4 Validation
        ↓
R04 Release Gate
```

## Phase A — Platform Service Contracts

### R04-001
Define the adapter boundary, ownership/lifetime rules, context semantics and platform-independence constraints.

### R04-002
Harden the existing scheduler contract without implementing a scheduler.

### R04-003
Finalize clock and timer platform contracts. Preserve `time::IClock` as the canonical clock abstraction.

### R04-004
Define precise watchdog semantics without coupling watchdog expiry to Runtime recovery.

## Platform API Review

Freeze the combined platform boundary, scheduler, clock, timer and watchdog contracts before capability discovery work.

## Phase B — Capability and Conformance

### R04-005
Define minimal platform identity/capability reporting and adapter contract. Avoid a large hardware inventory API.

### R04-006
Build reusable public-API conformance tests using fake adapters and no hardware.

## Platform Integration Freeze

After R04-006, production platform API and semantics are frozen. Required breaking changes return to architecture review.

## Phase C — Runtime Integration Boundary

### R04-007
Define how future Runtime/platform integration may consume platform services while preserving R0.3 Runtime semantics. Do not add threads or platform implementations.

### R04-008
Validation-only release-candidate verification.

## Implementation Rules

1. One logical task = one primary implementation commit.
2. Follow-up fixes use focused commits.
3. Do not amend accepted implementation commits unless explicitly required.
4. Existing R0.3 contracts remain authoritative.
5. Core remains platform independent.
6. Platform-specific implementations are external to `kritva-core`.
7. A required breaking change stops implementation and returns to architecture review.
